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
#include "ZWaveAdapterValue.h"

#include "value_classes\ValueID.h"

#include <map>
#include <string>
#include <functional>

namespace AdapterLib
{
    const std::wstring ValueName = L"value";

    ref class ZWaveAdapterProperty : BridgeRT::IAdapterProperty
    {
    public:
        //
        // Generic for DSB objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return m_name; }
        }

        virtual property Platform::String^ InterfaceHint
        {
            Platform::String^ get() { return m_InterfaceHint; }
        }

        // Attributes
        virtual property BridgeRT::IAdapterAttributeVector^ Attributes
        {
            BridgeRT::IAdapterAttributeVector^ get()
            {
                return ref new BridgeRT::AdapterAttributeVector(m_attributes);
            }
        }

    internal:
        friend ref class ZWaveAdapterDevice;
        friend ref class ZWaveAdapter;
        friend ref class UniversalControlPanelHandler;

        ZWaveAdapterProperty(const OpenZWave::ValueID & value);

        void UpdateValue();
        void Initialize(AdapterLib::ZWaveAdapter^ adapter);
        uint32 SetValue(Platform::Object^ data);
        ZWaveAdapterValue^ GetAttributeByName(Platform::String^ name);

    private:
        void GetAttributes();
        std::string EncodePropertyName(const std::string &name);

    private:
        Platform::String^ m_name;
        Platform::String^ m_InterfaceHint;

        std::vector<BridgeRT::IAdapterAttribute^> m_attributes;

        OpenZWave::ValueID m_valueId;
    };

    ref class ZWaveAdapterAttribute : BridgeRT::IAdapterAttribute
    {
    public:
        
        //Value
        virtual property BridgeRT::IAdapterValue^ Value
        {
            BridgeRT::IAdapterValue^ get() { return m_value; }
        }

        // Annotations
        virtual property BridgeRT::IAnnotationMap^ Annotations
        {
            BridgeRT::IAnnotationMap^ get()
            {
                return ref new BridgeRT::AnnotationMap(m_annotations);
            }
        }

        // Access
        virtual property BridgeRT::E_ACCESS_TYPE Access
        {
            BridgeRT::E_ACCESS_TYPE get()
            {
                return m_access;
            }

            void set(BridgeRT::E_ACCESS_TYPE accessType)
            {
                m_access = accessType;
            }
        }

        //Change of Value signal supported
        virtual property BridgeRT::SignalBehavior COVBehavior
        {
            BridgeRT::SignalBehavior get() { return m_covBehavior; }

            void set(BridgeRT::SignalBehavior behavior)
            {
                m_covBehavior = behavior;
            }
        }

    internal:
        friend ref class ZWaveAdapterProperty;

        ZWaveAdapterAttribute(Platform::String^ name, Platform::Object^ data);
        ZWaveAdapterAttribute(Platform::String^ name, const std::wstring& data);
        ZWaveAdapterAttribute(Platform::String^ name, const std::string& data);
        ZWaveAdapterAttribute(Platform::String^ name, int32 data);
        ZWaveAdapterAttribute(Platform::String^ name, bool data);
        ZWaveAdapterAttribute(Platform::String^ name, const std::vector<std::string>& data);

        //copy constructor
        ZWaveAdapterAttribute(const ZWaveAdapterAttribute^ other);

    private:
        // Generic
        ZWaveAdapterValue^ m_value;

        std::map<Platform::String^, Platform::String^> m_annotations;
        BridgeRT::E_ACCESS_TYPE m_access = BridgeRT::E_ACCESS_TYPE::ACCESS_READ;    // By default - Read access only
        BridgeRT::SignalBehavior m_covBehavior = BridgeRT::SignalBehavior::Never;
    };
}