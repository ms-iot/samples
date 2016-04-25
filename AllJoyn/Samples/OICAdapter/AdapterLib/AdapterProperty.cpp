// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
#include "pch.h"
#include "AdapterProperty.h"
#include "AdapterDevice.h"
#include "ocstack.h"
#include "ocpayload.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace std;

using namespace BridgeRT;


namespace AdapterLib
{
    AdapterProperty::AdapterProperty(const string& uri, const string& resourceType, AdapterDevice^ parent)
            : m_parent(parent)
            , m_uri(uri)
            , m_resourceType(resourceType)
            , m_observable(false)
        {
            char *ptr = const_cast<char*>(uri.data());
            if (ptr && *ptr == '/') ptr++;
                            
            m_name = ref new String(ConvertTo<wstring, string>(ptr).c_str());
            wstring adapterPrefix(parent->Parent->ExposedAdapterPrefix->Data());
            wstring wszIfName = adapterPrefix + L"." + cAdapterName + L"." + parent->SerialNumber->Data() + L"." + ConvertTo<wstring>(resourceType);
            m_interfaceHint = ref new String(wszIfName.c_str());
        }

    AdapterProperty::~AdapterProperty()
    {
        if (m_observerHandle)
        {
            OCCancel(m_observerHandle, OC_LOW_QOS, NULL, 0);
        }
    }

    uint32 AdapterProperty::Add(const OCRepPayloadValue& val, const wstring& namePrefix)
    {
        uint32 ret = ERROR_SUCCESS;
        AdapterAttribute^ pAttr = nullptr;

        if (!val.name)
        {
            ret = ERROR_INVALID_PARAMETER;
        }
        else
        {
            wstring valName;
            if (namePrefix.length() > 0)
            {
                valName = namePrefix + L".";
            }
            valName += ConvertTo<wstring,string>(val.name);

            //look for the value in AdapterValue
            auto iter = find_if(m_attributes.begin(), m_attributes.end(), [valName](IAdapterAttribute^ adapterAttr)
            {
                return (valName == adapterAttr->Value->Name->Data());
            });

            if (iter == m_attributes.end())
            {
                //create new attribute
                pAttr = ref new AdapterAttribute(valName);

                //set the COV behavior
                pAttr->COVBehavior = m_observable ? (SignalBehavior::Always) : (SignalBehavior::Never);
            }
            else
            {
                pAttr = dynamic_cast<AdapterAttribute^>(*iter);
            }

            //Now update the value
            ret = Update(val, pAttr);

            //Add the attribute to the property if doesn't exist
            if ((iter == m_attributes.end()) && ret == ERROR_SUCCESS)
            {
                m_attributes.push_back(pAttr);
            }
        }
        return ret;
    }

    uint32 AdapterProperty::Update(const OCRepPayloadValue& val, AdapterAttribute^ pAttr)
    {
        uint32 ret = ERROR_SUCCESS;

        if (!val.name || !pAttr)
        {
            ret = ERROR_INVALID_PARAMETER;
        }
        else
        {
            switch (val.type)
            {
            case OCREP_PROP_INT:
            {
                pAttr->Value->Data = PropertyValue::CreateInt64(val.i);
                break;
            }
            case OCREP_PROP_DOUBLE:
            {
                pAttr->Value->Data = PropertyValue::CreateDouble(val.d);
                break;
            }
            case OCREP_PROP_BOOL:
            {
                pAttr->Value->Data = PropertyValue::CreateBoolean(val.b);
                break;
            }
            case OCREP_PROP_STRING:
            {
                if (val.str && *val.str)
                {
                    pAttr->Value->Data = ref new String(ConvertTo<wstring, string>(val.str).c_str());
                }
                else
                {
                    pAttr->Value->Data = ref new String(L" ");
                }
                break;
            }
            case OCREP_PROP_ARRAY:
            {
                auto arr = val.arr;
                size_t dimTotal = calcDimTotal(arr.dimensions);
                if (dimTotal == 0)
                {
                    ret = ERROR_INVALID_PARAMETER;
                }
                else
                {
                    for (int i = 0; i < MAX_REP_ARRAY_DEPTH; ++i)
                    {
                        pAttr->m_dimensions[i] = arr.dimensions[i];
                    }
                    switch (arr.type)
                    {
                    case OCREP_PROP_INT:
                    {
                        pAttr->Value->Data = PropertyValue::CreateInt64Array(ref new Platform::Array<int64_t>(arr.iArray, dimTotal));
                        break;
                    }
                    case OCREP_PROP_DOUBLE:
                    {
                        pAttr->Value->Data = PropertyValue::CreateDoubleArray(ref new Platform::Array<double>(arr.dArray, dimTotal));
                        break;
                    }
                    case OCREP_PROP_BOOL:
                    {
                        pAttr->Value->Data = PropertyValue::CreateBooleanArray(ref new Platform::Array<bool>(arr.bArray, dimTotal));
                        break;
                    }
                    case OCREP_PROP_STRING:
                    {
                        auto stringArray = ref new Platform::Array<String^>(dimTotal);
                        for (size_t i = 0; i < dimTotal; ++i)
                        {
                            stringArray[i] = ref new String(ConvertTo<wstring,string>(arr.strArray[i]).c_str());
                        }
                        pAttr->Value->Data = PropertyValue::CreateStringArray(stringArray);
                        break;
                    }
                    default:
                    {
                        ret = ERROR_INVALID_PARAMETER;
                        break;
                    }
                    }
                }
                break;
            }
            default:
            {
                ret = ERROR_INVALID_PARAMETER;
                break;
            }
            }
        }
        return ret;
    }

    void AdapterProperty::Observable::set(bool val)
    {
        m_observable = val;
        for (auto attribute : m_attributes)
        {
            (dynamic_cast<AdapterAttribute^>(attribute))->COVBehavior = val? (SignalBehavior::Always) : (SignalBehavior::Never);
        }
    }
    
    AdapterAttribute^ AdapterProperty::GetAttribute(const string& name)
    {
        for (auto attr : this->m_attributes)
        {
            if (attr->Value->Name == StringReference(ConvertTo<wstring>(name).c_str()))
            {
                return dynamic_cast<AdapterAttribute^>(attr);
            }
        }

        return nullptr;
    }

    AdapterAttribute^ AdapterProperty::GetAttribute(String^ name)
    {
        return GetAttribute(ConvertTo<string>(name));
    }
    
    //
    // AdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    AdapterAttribute::AdapterAttribute(const wstring& name)
    {
        m_value = ref new AdapterValue(ref new String(name.c_str()));
    }
}   //namespace AdapterLib