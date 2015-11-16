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

#include "ZWaveAdapterDevice.h"
#include "ZWaveAdapterProperty.h"
#include "ZWaveAdapterSignal.h"
#include "ZWaveAdapterMethod.h"

#include "Manager.h"
#include "Misc.h"

#include <string>
#include <sstream>

using namespace Platform;
using namespace Windows::Foundation;
using namespace std;
using namespace OpenZWave;
using namespace DsbCommon;
using namespace BridgeRT;

namespace AdapterLib
{
    

    ZWaveAdapterDevice::ZWaveAdapterDevice(uint32 homeId, uint8 nodeId)
        : m_homeId(homeId)
        , m_nodeId(nodeId)
        , m_controlPanel(nullptr)
        , m_lightingServiceHandler(nullptr)
        , m_parent(nullptr)
    {

    }

    void ZWaveAdapterDevice::AddPropertyValue(const ValueID& value)
    {
        //check to see if its a method(button)
        ValueID::ValueType type = value.GetType();
        if (type == ValueID::ValueType::ValueType_Button)
        {
            //add it as a method
            //if the method is already in the list, ignore
            if (find_if(m_methods.begin(), m_methods.end(), [&value](IAdapterMethod^ method)
            {
                return dynamic_cast<ZWaveAdapterMethod^>(method)->m_valueId == value;
            }) == m_methods.end())
            {
                m_methods.push_back(ref new ZWaveAdapterMethod(value));
            }
        }
        else
        {
            //add it as property
            //get the property
            auto adapterProperty = GetProperty(value);
            if (adapterProperty == m_properties.end())
            {
                m_properties.push_back(ref new ZWaveAdapterProperty(value));
            }
        }
    }


    void ZWaveAdapterDevice::UpdatePropertyValue(const ValueID& value)
    {
        //get the property
        auto adapterProperty = GetProperty(value);
        if (adapterProperty != m_properties.end())
        {
            (dynamic_cast<ZWaveAdapterProperty^>(*adapterProperty))->UpdateValue();
        }
    }

    void ZWaveAdapterDevice::RemovePropertyValue(const ValueID & value)
    {
        //get the property
        auto adapterProperty = GetProperty(value);
        if (adapterProperty != m_properties.end())
        {
            m_properties.erase(adapterProperty);
        }
    }

    ZWaveAdapterProperty^ ZWaveAdapterDevice::GetPropertyByName(Platform::String^ name)
    {
        // Try to find the specified adapter property
        auto iter = find_if(begin(Properties), end(Properties), [&name](IAdapterProperty^ adapterProperty)
        {
            ZWaveAdapterProperty^ currProperty = dynamic_cast<ZWaveAdapterProperty^>(adapterProperty);
            return currProperty->Name == name;
        });

        // return null if not found, or the property reference if it was found
        IAdapterProperty^ outProperty = nullptr;
        if (iter != end(Properties))
        {
            outProperty = *(iter);
        }
        return dynamic_cast<ZWaveAdapterProperty^>(outProperty);
    }

    std::vector<BridgeRT::IAdapterProperty^>::iterator ZWaveAdapterDevice::GetProperty(const ValueID & value)
    {
        //find if the property already exist
        auto iter = find_if(m_properties.begin(), m_properties.end(), [&value](IAdapterProperty^ adapterProperty)
        {
            return (dynamic_cast<ZWaveAdapterProperty^>(adapterProperty))->m_valueId == value;
        });

        return iter;
    }

    void ZWaveAdapterDevice::BuildSignals()
    {
        //first clear the list
        m_signals.clear();

        ZWaveAdapterSignal^ signal = ref new ZWaveAdapterSignal(Constants::CHANGE_OF_VALUE_SIGNAL);

        //add params
        ValueID value(uint32(0), uint64(0));    //placeholder value

        ZWaveAdapterProperty^ tmpProperty = ref new ZWaveAdapterProperty(value);
        ZWaveAdapterValue^ tmpValue = ref new ZWaveAdapterValue(L"CovPlaceHolder", nullptr);

        signal->AddParam(ref new ZWaveAdapterValue(Constants::COV__PROPERTY_HANDLE, tmpProperty));
        signal->AddParam(ref new ZWaveAdapterValue(Constants::COV__ATTRIBUTE_HANDLE, tmpValue));

        m_signals.push_back(signal);

    }

    IAdapterSignal^ ZWaveAdapterDevice::GetSignal(String^ name)
    {
        for (auto signal : m_signals)
        {
            if (signal->Name == ref new String(name->Data()))
            {
                return signal;
            }
        }
        return nullptr;
    }

    void ZWaveAdapterDevice::AddLampStateChangedSignal()
    {
        try
        {
            ZWaveAdapterSignal^ lampStateChangedSignal = ref new ZWaveAdapterSignal(Constants::LAMP_STATE_CHANGED_SIGNAL_NAME);

            Platform::Object^ data = PropertyValue::CreateString(m_serialNumber);
            ZWaveAdapterValue^ lampIdSignalParameter = ref new ZWaveAdapterValue(Constants::SIGNAL_PARAMETER__LAMP_ID__NAME, data);
            lampStateChangedSignal->AddParam(lampIdSignalParameter);

            m_signals.push_back(lampStateChangedSignal);
        }
        catch (OutOfMemoryException^ ex)
        {
            return;
        }
    }

    void ZWaveAdapterDevice::Initialize()
    {
        wstringstream ss;

        //prepend the home id and node id to the name
        wstring nodeName = ConvertTo<wstring>(Manager::Get()->GetNodeName(m_homeId, m_nodeId));
        if (nodeName.empty())
        {
            ss << L"HomeID_" << m_homeId << L"_Node_" << (uint32)m_nodeId;
            nodeName = ss.str();
        }

        m_name = ref new String(nodeName.c_str());

        //vendor
        m_vendor = ref new String(ConvertTo<wstring>(Manager::Get()->GetNodeManufacturerName(m_homeId, m_nodeId)).c_str());

        //model
        m_model = ref new String(ConvertTo<wstring>(Manager::Get()->GetNodeProductName(m_homeId, m_nodeId)).c_str());

        //version
        ss.str(L"");
        ss << (uint32)Manager::Get()->GetNodeVersion(m_homeId, m_nodeId);
        m_version = ref new String(ss.str().c_str());

        //description
        m_description = ref new String(ConvertTo<wstring>(Manager::Get()->GetNodeType(m_homeId, m_nodeId)).c_str());

        //FW version
        m_firmwareVersion = ref new String();

        //serial number
        ss.str(L"");
        ss << m_homeId << L"_" << (uint32)m_nodeId;
        m_serialNumber = ref new String(ss.str().c_str());

        BuildSignals();

        //initialize properties
        for (auto prop : m_properties)
        {
            dynamic_cast<ZWaveAdapterProperty^>(prop)->Initialize();
        }

        //initialize methods
        for (auto method : m_methods)
        {
            dynamic_cast<ZWaveAdapterMethod^>(method)->Initialize();
        }
    }
}
