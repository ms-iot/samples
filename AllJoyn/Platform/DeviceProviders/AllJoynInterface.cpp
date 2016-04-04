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

#include "pch.h"
#include "AllJoynInterface.h"
#include "AllJoynProperty.h"
#include "AllJoynMethod.h"
#include "AllJoynHelpers.h"
#include "AllJoynSignal.h"
#include "TypeConversionHelpers.h"

using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace Platform;
using namespace std;

namespace DeviceProviders
{
    AllJoynInterface::AllJoynInterface(AllJoynBusObject ^ parent, alljoyn_interfacedescription interfaceDescription)
        : m_parent(parent)
        , m_interfaceDescription(interfaceDescription)
        , m_active(true)
    {
        DEBUG_LIFETIME_IMPL(AllJoynInterface);

        m_name = alljoyn_interfacedescription_getname(interfaceDescription);

        auto characterCount = alljoyn_interfacedescription_introspect(m_interfaceDescription, nullptr, 0, 2);
        auto textBuffer = vector<char>(characterCount);
        alljoyn_interfacedescription_introspect(m_interfaceDescription, textBuffer.data(), characterCount, 2);

        m_introspectXml = textBuffer.data();
    }

    AllJoynInterface::~AllJoynInterface()
    {
        if (m_active)
        {
            Shutdown();
        }
    }

    void AllJoynInterface::Shutdown()
    {
        m_active = false;

        if (nullptr != m_properties)
        {
            for (auto prop : m_properties)
            {
                dynamic_cast<AllJoynProperty^>(static_cast<Object^>(prop))->Shutdown();
            }
            m_properties->Clear();
            m_properties = nullptr;
        }
        if (nullptr != m_methods)
        {
            for (auto method : m_methods)
            {
                dynamic_cast<AllJoynMethod^>(static_cast<Object^>(method))->Shutdown();
            }
            m_methods->Clear();
            m_methods = nullptr;
        }
        if (nullptr != m_signals)
        {
            for (auto signal : m_signals)
            {
                dynamic_cast<AllJoynSignal^>(static_cast<Object^>(signal))->Shutdown();
            }
            m_signals->Clear();
            m_signals = nullptr;
        }
    }

    void AllJoynInterface::CreateProperties()
    {
        auto propertyCount = static_cast<uint32>(alljoyn_interfacedescription_getproperties(m_interfaceDescription, nullptr, 0));
        m_properties = ref new Vector<IProperty ^>(propertyCount);

        if (propertyCount > 0)
        {
            auto propertyDescriptions = vector<alljoyn_interfacedescription_property>(propertyCount);
            alljoyn_interfacedescription_getproperties(m_interfaceDescription, propertyDescriptions.data(), propertyCount);

            for (uint32 i = 0; i < propertyCount; ++i)
            {
                m_properties->SetAt(i, ref new AllJoynProperty(this, propertyDescriptions[i]));
            }
        }
    }

    void AllJoynInterface::CreateMethodsAndSignals()
    {
        m_methods = ref new Vector<IMethod ^>();
        m_signals = ref new Vector<ISignal ^>();

        auto memberCount = static_cast<uint32>(alljoyn_interfacedescription_getmembers(m_interfaceDescription, nullptr, 0));
        if (memberCount)
        {
            auto memberDescriptions = vector<alljoyn_interfacedescription_member>(memberCount);
            alljoyn_interfacedescription_getmembers(m_interfaceDescription, memberDescriptions.data(), memberCount);

            for (uint32 i = 0; i < memberCount; ++i)
            {
                if (ALLJOYN_MESSAGE_SIGNAL == memberDescriptions[i].memberType)
                {
                    m_signals->Append(ref new AllJoynSignal(this, memberDescriptions[i]));
                }
                else if (ALLJOYN_MESSAGE_METHOD_CALL == memberDescriptions[i].memberType)
                {
                    m_methods->Append(ref new AllJoynMethod(this, memberDescriptions[i]));
                }
            }
        }
    }

    void AllJoynInterface::CreateAnnotations()
    {
        m_annotations = TypeConversionHelpers::GetAnnotationsView<alljoyn_interfacedescription>(
            m_interfaceDescription,
            alljoyn_interfacedescription_getannotationscount,
            alljoyn_interfacedescription_getannotationatindex);
    }

    IVectorView<IProperty ^>^ AllJoynInterface::Properties::get()
    {
        IVectorView<IProperty ^>^ properties = nullptr;
        if (m_active)
        {
            if (m_properties == nullptr)
            {
                this->CreateProperties();
            }
            properties = m_properties->GetView();
        }
        return properties;
    }

    IVectorView<IMethod ^>^ AllJoynInterface::Methods::get()
    {
        IVectorView<IMethod ^>^ methods = nullptr;
        if (m_active)
        {
            if (m_methods == nullptr)
            {
                this->CreateMethodsAndSignals();
            }
            methods = m_methods->GetView();
        }
        return methods;
    }

    IVectorView<ISignal ^>^ AllJoynInterface::Signals::get()
    {
        IVectorView<ISignal ^>^ signals = nullptr;
        if (m_active)
        {
            if (m_signals == nullptr)
            {
                this->CreateMethodsAndSignals();
            }
            signals = m_signals->GetView();
        }
        return signals;
    }

    IMapView<String^, String^>^ AllJoynInterface::Annotations::get()
    {
        IMapView<String^, String^>^ annotations = nullptr;
        if (m_active)
        {
            if (m_annotations == nullptr)
            {
                this->CreateAnnotations();
            }
            annotations = m_annotations;
        }
        return annotations;
    }

    String ^ AllJoynInterface::IntrospectXml::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_introspectXml.c_str());
    }

    String ^ AllJoynInterface::Name::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_name.c_str());
    }

    IProperty^ AllJoynInterface::GetProperty(Platform::String^ propertyName)
    {
        IProperty^ prop = nullptr;
        if (m_active)
        {
            auto properties = this->Properties;
            auto iter = std::find_if(begin(properties), end(properties), [&propertyName](IProperty^ prop) -> bool
            {
                return prop->Name == propertyName;
            });
            prop = iter == end(properties) ? nullptr : *iter;
        }
        return prop;
    }

    IMethod^ AllJoynInterface::GetMethod(Platform::String^ methodName)
    {
        IMethod^ method = nullptr;
        if (m_active)
        {
            auto methods = this->Methods;
            auto iter = std::find_if(begin(methods), end(methods), [&methodName](IMethod^ method) -> bool
            {
                return method->Name == methodName;
            });
            method = iter == end(methods) ? nullptr : *iter;
        }
        return method;
    }

    ISignal^ AllJoynInterface::GetSignal(Platform::String^ signalName)
    {
        ISignal^ signal = nullptr;
        if (m_active)
        {
            auto signals = this->Signals;
            auto iter = std::find_if(begin(signals), end(signals), [&signalName](ISignal^ signal) -> bool
            {
                return signal->Name == signalName;
            });
            signal = iter == end(signals) ? nullptr : *iter;
        }
        return signal;
    }
}