//
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

#include "pch.h"
#include "collection.h"
#include "IInterface.h"
#include "AllJoynBusObject.h"

namespace DeviceProviders
{
    ref class AllJoynInterface : public IInterface
    {
        DEBUG_LIFETIME_DECL(AllJoynInterface);

    internal:
        AllJoynInterface(AllJoynBusObject ^ parent, alljoyn_interfacedescription interfaceDescription);
        void Shutdown();

        inline AllJoynBusObject ^ GetBusObject() const { return m_parent; }
        inline alljoyn_proxybusobject GetProxyBusObject() const { return m_parent->GetProxyBusObject(); }
        inline alljoyn_busattachment GetBusAttachment() const { return m_parent->GetBusAttachment(); }
        inline const std::string& GetName() const { return m_name; }

    public:
        virtual ~AllJoynInterface();

        virtual property Windows::Foundation::Collections::IVectorView<IProperty ^>^ Properties
        {
            Windows::Foundation::Collections::IVectorView<IProperty ^>^ get();
        }
        virtual property Windows::Foundation::Collections::IVectorView<IMethod ^>^ Methods
        {
            Windows::Foundation::Collections::IVectorView<IMethod ^>^ get();
        }
        virtual property Windows::Foundation::Collections::IVectorView<ISignal ^>^ Signals
        {
            Windows::Foundation::Collections::IVectorView<ISignal ^>^ get();
        }
        virtual property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ Annotations
        {
            Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ get();
        }
        virtual property Platform::String ^ IntrospectXml
        {
            Platform::String ^ get();
        }
        virtual property Platform::String ^ Name
        {
            Platform::String ^ get();
        }
        virtual property IBusObject^ BusObject
        {
            inline IBusObject^ get() { return m_parent; }
        }

        virtual IProperty^ GetProperty(Platform::String^ propertyName);
        virtual IMethod^ GetMethod(Platform::String^ methodName);
        virtual ISignal^ GetSignal(Platform::String^ signalName);

    private:
        std::atomic<bool> m_active;
        AllJoynBusObject ^ m_parent;
        alljoyn_interfacedescription m_interfaceDescription;
        std::string m_name;
        std::string m_introspectXml;

        void CreateProperties();
        void CreateMethodsAndSignals();
        void CreateAnnotations();

        Platform::Collections::Vector<IProperty ^>^ m_properties;
        Platform::Collections::Vector<IMethod ^>^ m_methods;
        Platform::Collections::Vector<ISignal ^>^ m_signals;
        Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ m_annotations;
    };
}
