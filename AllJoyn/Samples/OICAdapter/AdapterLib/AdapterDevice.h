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
#pragma once

#include "AdapterDefinitions.h"
#include "Adapter.h"
#include "PlatformInfo.h"
#include "AdapterIcon.h"
#include "ocstack.h"

#include <chrono>

namespace AdapterLib
{
    //forward declaration
    ref class AdapterSignal;

    //
    // AdapterDevice.
    // Description:
    // The class that implements BridgeRT::IAdapterDevice.
    //
    ref class AdapterDevice :   BridgeRT::IAdapterDevice
//                               ,BridgeRT::IAdapterDeviceLightingService
//                               ,BridgeRT::IAdapterDeviceControlPanel
    {
    public:
        //virtual ~AdapterDevice();
        //
        // Generic for Adapter objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return m_name; }
        }
        
        //
        // Device information
        //
        virtual property Platform::String^ Vendor
        {
            Platform::String^ get() { return m_vendor; }
        }
        virtual property Platform::String^ Model
        {
            Platform::String^ get() { return m_model; }
        }
        virtual property Platform::String^ Version
        {
            Platform::String^ get() { return m_version; }
        }
        virtual property Platform::String^ FirmwareVersion
        {
            Platform::String^ get() { return m_firmwareVersion; }
        }
        virtual property Platform::String^ SerialNumber
        {
            Platform::String^ get() { return m_serialNumber; }
        }
        virtual property Platform::String^ Description
        {
            Platform::String^ get() { return m_description; }
        }

        // Device properties
        virtual property BridgeRT::IAdapterPropertyVector^ Properties
        {
            BridgeRT::IAdapterPropertyVector^ get()
            {
                return ref new BridgeRT::AdapterPropertyVector(m_properties);
            }
        }

        // Device methods
        virtual property BridgeRT::IAdapterMethodVector^ Methods
        {
            BridgeRT::IAdapterMethodVector^ get()
            {
                return ref new BridgeRT::AdapterMethodVector(m_methods);
            }
        }

        // Device signals
        virtual property BridgeRT::IAdapterSignalVector^ Signals
        {
            BridgeRT::IAdapterSignalVector^ get()
            {
                return ref new BridgeRT::AdapterSignalVector(m_signals);
            }
        }

        virtual property BridgeRT::IAdapterIcon^ Icon
        {
            BridgeRT::IAdapterIcon^ get()
            {
                return m_icon;
            }
        }

    internal:
        property Adapter^ Parent
        {
            Adapter^ get() { return m_parent; }
        }

        property OCDevAddr* DevAddr
        {
            OCDevAddr* get() { return &m_addr; }
        }

        property std::chrono::steady_clock::time_point LastDiscoveredTime
        {
            std::chrono::steady_clock::time_point get() { return m_LastDiscoveredTime; }
            void set(std::chrono::steady_clock::time_point tp) { m_LastDiscoveredTime = tp; }
        }

    internal:
        AdapterDevice(const PlatformInfo& pInfo, const OCDevicePayload& deviceInfo, Adapter^ parent);

        AdapterSignal^ AdapterDevice::GetSignal(Platform::String^ name);

        // Adding Properties
        void AddProperty(BridgeRT::IAdapterProperty^ Property)
        {
            m_properties.push_back(Property);
        }

        //// Adding Methods
        void AddMethod(BridgeRT::IAdapterMethod^ Method)
        {
            m_methods.push_back(Method);
        }

        //// Adding Signals
        void AddSignal(BridgeRT::IAdapterSignal^ Signal)
        {
            m_signals.push_back(Signal);
        }

        // Adding Change_Of_Value Signal
        void AddChangeOfValueSignal(
            BridgeRT::IAdapterProperty^ Property,
            BridgeRT::IAdapterValue^ Attribute);

    private:
        // Generic
        Platform::String^ m_name;
        Adapter^ m_parent;
        
        // Device information
        Platform::String^ m_vendor;
        Platform::String^ m_model;
        Platform::String^ m_version;
        Platform::String^ m_firmwareVersion;
        Platform::String^ m_serialNumber;
        Platform::String^ m_description;
        AdapterIcon^ m_icon;

        // Device properties 
        std::vector<BridgeRT::IAdapterProperty^> m_properties;

        // Device methods
        std::vector<BridgeRT::IAdapterMethod^> m_methods;

        // Device signals 
        std::vector<BridgeRT::IAdapterSignal^> m_signals;

        OCDevAddr m_addr;

        std::chrono::steady_clock::time_point m_LastDiscoveredTime;
    };
} // namespace AdapterLib