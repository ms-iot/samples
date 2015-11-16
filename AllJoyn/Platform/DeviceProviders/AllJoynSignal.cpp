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
#include "AllJoynSignal.h"
#include "AllJoynTypeDefinition.h"
#include "TypeConversionHelpers.h"
#include "AllJoynHelpers.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace Platform::Collections;
using namespace Platform;
using namespace concurrency;
using namespace std;

namespace DeviceProviders
{
    map<string, WeakReference> AllJoynSignal::s_signalMap;
    CSLock AllJoynSignal::s_signalMapLock;

    AllJoynSignal::AllJoynSignal(AllJoynInterface ^ parent, const alljoyn_interfacedescription_member& signalDescription)
        : m_interface(parent)
        , m_name(signalDescription.name)
        , m_interfaceName(parent->GetName())
        , m_objectPath(parent->GetBusObject()->GetPath())
        , m_aboutData(parent->GetBusObject()->GetService()->AboutData)
        , m_signatureString(signalDescription.signature)
        , m_member(signalDescription)
        , m_subscriberCount(0)
        , m_active(true)
    {
        DEBUG_LIFETIME_IMPL(AllJoynSignal);
        m_signature = AllJoynTypeDefinition::CreateParameterInfo(m_signatureString, AllJoynHelpers::TokenizeArgNamesString(signalDescription.argNames));

        {
            AutoLock lockScope(&s_signalMapLock, true);
            string key = BuildSignalMapKey(parent->GetBusObject()->GetPath().c_str(), parent->GetName().c_str(), m_name.c_str());
            s_signalMap[key] = WeakReference(this);
        }

        //annotations
        m_annotations = TypeConversionHelpers::GetAnnotationsView<alljoyn_interfacedescription_member>(
            signalDescription,
            alljoyn_interfacedescription_member_getannotationscount,
            alljoyn_interfacedescription_member_getannotationatindex);
    }

    AllJoynSignal::~AllJoynSignal()
    {
        if (m_active)
        {
            Shutdown();
        }
    }

    void AllJoynSignal::Shutdown()
    {
        m_active = false;

        alljoyn_busattachment_unregistersignalhandler(m_interface->GetBusAttachment(),
            AllJoynSignal::OnSignal,
            m_member,
            m_interface->GetBusObject()->GetPath().c_str());
    }

    string AllJoynSignal::BuildSignalMapKey(const char *objectPath, const char *interfaceName, const char *signalName)
    {
        static const char * formatString = "path=%s,interface=%s,signal=%s";

        int bufferSize = snprintf(nullptr, 0, formatString, objectPath, interfaceName, signalName);
        auto buffer = vector<char>(bufferSize + 1);

        snprintf(buffer.data(), bufferSize + 1, formatString, objectPath, interfaceName, signalName);

        return buffer.data();
    }

    void AllJoynSignal::OnSignal(_In_ const alljoyn_interfacedescription_member *member,
        _In_ const char* srcPath,
        _In_ alljoyn_message message)
    {
        const char *interfaceName = alljoyn_interfacedescription_getname(member->iface);
        string key = BuildSignalMapKey(srcPath, interfaceName, member->name);

        AllJoynSignal ^ thisptr = nullptr;
        {
            AutoLock lockScope(&s_signalMapLock, true);
            auto iter = s_signalMap.find(key);
            if (iter == s_signalMap.end())
            {
                return;
            }

            thisptr = iter->second.Resolve<AllJoynSignal>();
            if (thisptr == nullptr)
            {
                return;
            }
        }

        auto responseSignature = alljoyn_message_getsignature(message);
        if (thisptr->m_signatureString != responseSignature)
        {
            return;
        }

        std::vector<string> completeTypes;
        TypeConversionHelpers::CreateCompleteTypeSignatureArrayFromSignature(responseSignature, completeTypes);

        Platform::Collections::Vector<Object ^> ^ signaledValues = ref new Vector<Object ^>();
        for (size_t i = 0; i < thisptr->m_signature->Size; ++i)
        {
            Object ^ value;
            int32 result = TypeConversionHelpers::GetAllJoynMessageArg(
                alljoyn_message_getarg(message, i),
                completeTypes[i].c_str(),
                &value);

            if (result != ER_OK)
            {
                return;
            }
            signaledValues->Append(value);
        }

        thisptr->SignalRaised(thisptr, signaledValues);
    }

    IVector<ParameterInfo ^>^ AllJoynSignal::Signature::get()
    {
        return m_signature;
    }

    String ^ AllJoynSignal::Name::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_name.c_str());
    }

    Windows::Foundation::EventRegistrationToken AllJoynSignal::SignalRaised::add(Windows::Foundation::TypedEventHandler<ISignal^, Windows::Foundation::Collections::IVector<Object^>^>^ handler)
    {
        if (m_active && m_subscriberCount++ == 0)
        {
            alljoyn_busattachment_registersignalhandler(m_interface->GetBusAttachment(),
                AllJoynSignal::OnSignal,
                m_member,
                m_interface->GetBusObject()->GetPath().c_str());
        }
        return m_signalRaised += handler;
    }

    void AllJoynSignal::SignalRaised::remove(Windows::Foundation::EventRegistrationToken token)
    {
        if (m_active && --m_subscriberCount == 0)
        {
            alljoyn_busattachment_unregistersignalhandler(m_interface->GetBusAttachment(),
                AllJoynSignal::OnSignal,
                m_member,
                m_interface->GetBusObject()->GetPath().c_str());
        }
        m_signalRaised -= token;
    }

    void AllJoynSignal::SignalRaised::raise(ISignal^ sender, Windows::Foundation::Collections::IVector<Object^>^ params)
    {
        m_signalRaised(sender, params);
    }
}