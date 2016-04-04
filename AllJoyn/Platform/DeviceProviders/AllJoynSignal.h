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
#include "ISignal.h"

namespace DeviceProviders
{
    interface class IInterface;
    ref class AllJoynInterface;

    ref class AllJoynSignal sealed : public ISignal
    {
        DEBUG_LIFETIME_DECL(AllJoynSignal);

    internal:
        AllJoynSignal(_In_ AllJoynInterface ^ _interface, const alljoyn_interfacedescription_member& signalDescription);
        void Shutdown();

        static void AJ_CALL OnSignal(_In_ const alljoyn_interfacedescription_member *member,
            _In_ const char* srcPath,
            _In_ alljoyn_message message);

    public:
        virtual ~AllJoynSignal();

        virtual event Windows::Foundation::TypedEventHandler<ISignal^, Windows::Foundation::Collections::IVector<Object^>^>^ SignalRaised
        {
            Windows::Foundation::EventRegistrationToken add(Windows::Foundation::TypedEventHandler<ISignal^, Windows::Foundation::Collections::IVector<Object^>^>^ handler);
            void remove(Windows::Foundation::EventRegistrationToken token);
            void raise(ISignal^, Windows::Foundation::Collections::IVector<Object^>^);
        }

        virtual property Windows::Foundation::Collections::IVector<ParameterInfo ^>^ Signature
        {
            Windows::Foundation::Collections::IVector<ParameterInfo ^>^ get();
        }

        virtual property Platform::String ^ Name
        {
            Platform::String ^ get();
        }

        virtual property IInterface ^ Interface
        {
            inline IInterface ^ get() { return m_interface; }
        }

        virtual property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ Annotations
        {
            Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ get() { return m_annotations; }
        }

    private:
        std::atomic<bool> m_active;
        AllJoynInterface ^ m_interface;

        Platform::Collections::Vector<ParameterInfo ^>^ m_signature;
        std::string m_signatureString;
        std::string m_name;
        std::string m_interfaceName;
        std::string m_objectPath;
        IAboutData ^ m_aboutData;

        alljoyn_interfacedescription_member m_member;

        // Internal event so that we can have custom add logic and defer subscribing to the AllJoyn signal
        // until AJX adds event handler to the SignalRaised event
        std::atomic<uint32> m_subscriberCount;
        event Windows::Foundation::TypedEventHandler<ISignal^, Windows::Foundation::Collections::IVector<Object^>^>^ m_signalRaised;

        // This map is required because we need a way to pass the AllJoynSignal to the signal
        // handlers, but the current AllJoyn C API does not allow passing a context to these
        // callbacks. The key uniquely identifies a signal using the following format and is
        // generated using the BuildSignalMapKey function
        static std::map<std::string, Platform::WeakReference> s_signalMap;
        static CSLock s_signalMapLock;
        static std::string BuildSignalMapKey(const char *objectPath, const char *interfaceName, const char *signalName);
        Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ m_annotations;
    };
}