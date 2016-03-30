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
#include "AdapterValue.h"
#include "AdapterDevice.h"

#include "ocstack.h"

namespace AdapterLib
{
    //forward declaration
    ref class AdapterAttribute;

    //
    // AdapterProperty.
    // Description:
    // The class that implements BridgeRT::IAdapterProperty.
    //
    ref class AdapterProperty : BridgeRT::IAdapterProperty
    {
    public:
        //
        // Generic for Adapter objects
        //
        virtual ~AdapterProperty();

        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return m_name; }
        }

        virtual property Platform::String^ InterfaceHint
        {
            Platform::String^ get() { return m_interfaceHint; }
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
        AdapterProperty(const std::string& uri, const std::string& resourceType, AdapterDevice^ parent);

        uint32 Add(const OCRepPayloadValue& val, const std::wstring& namePrefix = L"");
        uint32 Update(const OCRepPayloadValue& val, AdapterAttribute^ pAttr);

        AdapterAttribute^ GetAttribute(const std::string& name);

        AdapterAttribute^ GetAttribute(Platform::String^ name);
        
        // Adding Attributes
        AdapterProperty^ operator += (BridgeRT::IAdapterAttribute^ Attribute)
        {
            if (Attribute)
            {
                m_attributes.push_back(Attribute);
            }
            return this;
        }

        property std::string Uri
        {
            std::string get() { return m_uri; }
        }

        property std::string ResourceType
        {
            std::string get() { return m_resourceType; }
        }

        property AdapterDevice^ Parent
        {
            AdapterDevice^ get() { return m_parent; }
        }

        property bool Observable
        {
            bool get() { return m_observable; }
            void set(bool val);
        }

        property OCDoHandle ObserverHandle
        {
            void set(OCDoHandle hdl) { m_observerHandle = hdl; };
        }

    private:
        // Generic
        Platform::String^ m_name;
        Platform::String^ m_interfaceHint;
        AdapterDevice^ m_parent;

        std::vector<BridgeRT::IAdapterAttribute^> m_attributes;

        std::string m_uri;
        std::string m_resourceType;
        bool m_observable;
        OCDoHandle m_observerHandle{ nullptr };
    };

    //
    // AdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    ref class AdapterAttribute : BridgeRT::IAdapterAttribute
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
        AdapterAttribute(const std::wstring& name);

        friend ref class AdapterProperty;

        const size_t* GetDimensions() const
        {
            return m_dimensions;
        }

    private:
        // Generic
        AdapterValue^ m_value;
        size_t m_dimensions[MAX_REP_ARRAY_DEPTH]{}; //if the attribute is an array, holds the dimensions

        std::map<Platform::String^, Platform::String^> m_annotations;
        BridgeRT::E_ACCESS_TYPE m_access = BridgeRT::E_ACCESS_TYPE::ACCESS_READWRITE;    // By default - Read Write access
        BridgeRT::SignalBehavior m_covBehavior = BridgeRT::SignalBehavior::Never;
    };
} //AdapterLib