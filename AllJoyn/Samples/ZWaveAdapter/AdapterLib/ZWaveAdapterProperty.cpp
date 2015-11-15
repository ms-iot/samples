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

#include "ZWaveAdapterProperty.h"
#include "ZWaveAdapterValue.h"
#include "Misc.h"
#include "ZWaveAdapter.h"

#include "Manager.h"
#include "value_classes\ValueID.h"
#include "value_classes\Value.h"
#include "command_classes\CommandClass.h"
#include "command_classes\CommandClasses.h"
#include "command_classes\Basic.h"

#include <sstream>
#include <string>

using namespace std;
using namespace OpenZWave;
using namespace Platform;
using namespace DsbCommon;
using namespace BridgeRT;
using namespace Windows::Foundation;

namespace AdapterLib
{
    ZWaveAdapterProperty::ZWaveAdapterProperty(const OpenZWave::ValueID & value)
        : m_valueId(value)
        , m_InterfaceHint(ref new String(L""))
    {   
    }

    void ZWaveAdapterProperty::Initialize()
    {
        //Get Command Class Info
        string cmdClassName;
        string propertyName;
        string strClass, strLabel;
        if (Manager::Get()->GetNodeClassInformation(m_valueId.GetHomeId(), m_valueId.GetNodeId(), m_valueId.GetCommandClassId(), &cmdClassName))
        {
            //Command class name starts with COMMAND_CLASS_xxx, ignore the COMMAND_CLASS_ part and take the rest
            propertyName = cmdClassName.substr(string("COMMAND_CLASS_").size());
            strClass = EncodePropertyName(propertyName);
        }
        strLabel = EncodePropertyName(Manager::Get()->GetValueLabel(m_valueId));

        propertyName = strClass;

        //if the class name and label are same, just take one of them.
        //This will avoid duplication in interface names like SwitchAll.SwitchAll
        if (strClass != strLabel)
        {
            if (!strClass.empty())
            {
                propertyName += '.';
            }
            propertyName += strLabel;
        }
        
        //name
        m_name = ref new String(ConvertTo<wstring>(propertyName).c_str());

        std::wstring wszIfName = cAdapterPrefix + L"." + cAdapterName + L"." + ConvertTo<wstring>(propertyName);
        m_InterfaceHint = ref new String(wszIfName.c_str());

        GetAttributes();
    }

    void ZWaveAdapterProperty::GetAttributes()
    {
        //Get all the attributes for the Value
        m_attributes.clear();

        //instance
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"instance", (int32)(m_valueId.GetInstance())));

        //index
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"index", (int32)(m_valueId.GetIndex())));

        //genre
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"genre", string(Value::GetGenreNameFromEnum(m_valueId.GetGenre()))));
        
        //label
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"label", Manager::Get()->GetValueLabel(m_valueId)));

        //units
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"units", Manager::Get()->GetValueUnits(m_valueId)));

        //help
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"help", Manager::Get()->GetValueHelp(m_valueId)));

        //min
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"min", Manager::Get()->GetValueMin(m_valueId)));

        //max
        m_attributes.push_back(ref new ZWaveAdapterAttribute(L"max", Manager::Get()->GetValueMax(m_valueId)));

        //values
        ValueID::ValueType type = m_valueId.GetType();
        if (type == ValueID::ValueType::ValueType_List)
        {
            //add list items
            vector<string> items;
            Manager::Get()->GetValueListItems(m_valueId, &items);

            m_attributes.push_back(ref new ZWaveAdapterAttribute(L"valid_values", items));
        }

        UpdateValue();
    }

    void ZWaveAdapterProperty::UpdateValue()
    {
        ValueID::ValueType type = m_valueId.GetType();

        //look for the value in AdapterValue
        auto iter = find_if(m_attributes.begin(), m_attributes.end(), [&](IAdapterAttribute^ adapterAttr)
        {
            return adapterAttr->Value->Name->Data() == ValueName;
        });

        if (iter == m_attributes.end())
        {
            //add the AdapterValue
            m_attributes.push_back(ref new ZWaveAdapterAttribute(ref new String(ValueName.c_str()), nullptr));
            iter = prev(m_attributes.end());

            //add the access
            E_ACCESS_TYPE access = E_ACCESS_TYPE::ACCESS_READWRITE;
            if (Manager::Get()->IsValueReadOnly(m_valueId))
            {
                access = E_ACCESS_TYPE::ACCESS_READ;
            }
            else if(Manager::Get()->IsValueWriteOnly(m_valueId))
            {
                access = E_ACCESS_TYPE::ACCESS_WRITE;
            }

            dynamic_cast<ZWaveAdapterAttribute^>(*iter)->Access = access;

            //add COV signal behavior
            dynamic_cast<ZWaveAdapterAttribute^>(*iter)->COVBehavior = SignalBehavior::Always;
        }

        switch (type)
        {
        case ValueID::ValueType_Bool:
        {
            bool bVal;
            Manager::Get()->GetValueAsBool(m_valueId, &bVal);
            (*iter)->Value->Data = PropertyValue::CreateBoolean(bVal);
        }
        break;
        case ValueID::ValueType_Byte:
        {
            uint8 byteVal;
            Manager::Get()->GetValueAsByte(m_valueId, &byteVal);
            (*iter)->Value->Data = PropertyValue::CreateUInt8(byteVal);
        }
        break;
        case ValueID::ValueType_Decimal:
        {
            stringstream ss;

            float val;
            uint8 precision = 0;
            Manager::Get()->GetValueAsFloat(m_valueId, &val);
            Manager::Get()->GetValueFloatPrecision(m_valueId, &precision);

            (*iter)->Value->Data = PropertyValue::CreateDouble(val);
        }
        break;
        case ValueID::ValueType_Int:
        {
            int32 val;
            Manager::Get()->GetValueAsInt(m_valueId, &val);

            (*iter)->Value->Data = PropertyValue::CreateInt32(val);
        }
        break;
        case ValueID::ValueType_Short:
        {
            int16 val;
            Manager::Get()->GetValueAsShort(m_valueId, &val);

            (*iter)->Value->Data = PropertyValue::CreateInt16(val);
        }
        break;
        case ValueID::ValueType_String:
        {
            string strValue;
            Manager::Get()->GetValueAsString(m_valueId, &strValue);

            (*iter)->Value->Data = PropertyValue::CreateString(ref new String(ConvertTo<wstring>(strValue).c_str()));
        }
        break;
        case ValueID::ValueType_Raw:
        {
            uint8 *pVal = nullptr;
            uint8 len = 0;
            Manager::Get()->GetValueAsRaw(m_valueId, &pVal, &len);

            Platform::Array<uint8_t>^ octetArray = ref new Platform::Array<uint8_t>(pVal, len);
            (*iter)->Value->Data = PropertyValue::CreateUInt8Array(octetArray);

            delete[] pVal;
        }
        break;
        case ValueID::ValueType_List:
        {
            string strValue;
            Manager::Get()->GetValueListSelection(m_valueId, &strValue);

            (*iter)->Value->Data = PropertyValue::CreateString(ref new String(ConvertTo<wstring>(strValue).c_str()));
        }
        break;
        default:
            break;
        }
    }

    uint32 ZWaveAdapterProperty::SetValue(Object^ data)
    {
        uint32 status = ERROR_SUCCESS;

        Value *pVal = nullptr;
        bool bRet = true;
        string strVal = ConvertTo<string>(wstring(data->ToString()->Data()));

        CommandClass *pCmd = CommandClasses::CreateCommandClass(m_valueId.GetCommandClassId(), m_valueId.GetHomeId(), m_valueId.GetNodeId());
        if (pCmd == nullptr)
        {
            status = WIN32_FROM_HRESULT(E_FAIL);
            goto done;
        }
        pVal = pCmd->GetValue(m_valueId.GetInstance(), m_valueId.GetIndex());
        if (pVal == nullptr)
        {
            status = WIN32_FROM_HRESULT(E_FAIL);
            goto done;
        }

        if (pVal->IsReadOnly())
        {
            status = ERROR_ACCESS_DENIED;
            goto done;
        }

        bRet = pVal->SetFromString(strVal);
        if (bRet == false)
        {
            status = WIN32_FROM_HRESULT(E_FAIL);
            goto done;
        }

    done:
        if (pVal)
        {
            pVal->Release();
        }
        return status;
    }

    ZWaveAdapterValue^ ZWaveAdapterProperty::GetAttributeByName(String^ name)
    {
        for (auto attr : this->m_attributes)
        {
            if (attr->Value->Name == name)
            {
                return dynamic_cast<ZWaveAdapterValue^>(attr->Value);
            }
        }

        return nullptr;
    }

    string ZWaveAdapterProperty::EncodePropertyName(const string &name)
    {
        string encodedString;
        
        //apply Pascal casing
        encodedString += char(toupper(name[0]));
        for (size_t i = 1; i < name.size(); ++i)
        {
            if (isalnum(name[i]) || ('.' == name[i]))
            {
                if (!isalpha(name[i - 1]))
                {
                    encodedString += char(toupper(name[i]));
                }
                else
                {
                    encodedString += char(tolower(name[i]));
                }
            }
        }
        return encodedString;
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(String^ name, Object^ data)
        : m_value(ref new ZWaveAdapterValue(name, data))
    {
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(String^ name, const wstring& data)
    {
        m_value = ref new ZWaveAdapterValue(name, PropertyValue::CreateString(ref new String(data.c_str())));
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(String^ name, const string& data)
    {
        m_value = ref new ZWaveAdapterValue(name, PropertyValue::CreateString(ref new String(ConvertTo<wstring>(data).c_str())));
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(String^ name, int32 data)
    {
        m_value = ref new ZWaveAdapterValue(name, PropertyValue::CreateInt32(data));
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(String^ name, bool data)
    {
        m_value = ref new ZWaveAdapterValue(name, PropertyValue::CreateBoolean(data));
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(String^ name, const vector<string>& data)
    {
        Platform::Array<String^>^ stringArray = ref new Platform::Array<String^>(static_cast<unsigned int>(data.size()));
        for (unsigned int i = 0; i < static_cast<unsigned int>(data.size()); ++i)
        {
            stringArray[i] = ref new String(ConvertTo<wstring>(data[i]).c_str());
        }
        m_value = ref new ZWaveAdapterValue(name, PropertyValue::CreateStringArray(stringArray));
    }

    ZWaveAdapterAttribute::ZWaveAdapterAttribute(const ZWaveAdapterAttribute^ other)
        : m_value(other->m_value)
        , m_annotations(other->m_annotations)
        , m_covBehavior(other->m_covBehavior)
        , m_access(other->m_access)
    {

    }
}