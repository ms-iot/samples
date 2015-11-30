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
#include "IService.h"
#include "IBusObject.h"
#include "IAboutData.h"
#include "AllJoynProvider.h"

namespace DeviceProviders
{
    ref class AllJoynAboutData;
    ref class AllJoynBusObject;
    ref class AllJoynSession;
    ref class AllJoynSessionImplementation;

    ref class AllJoynService : public IService
    {
        DEBUG_LIFETIME_DECL(AllJoynService);

    internal:
        AllJoynService(AllJoynProvider ^ provider, std::string serviceName, alljoyn_sessionport port);

        // Note that it is possible to add a BusObject to a service after the initial About broadcast,
        // so this function may be called multiple times with new about info.
        void Initialize(alljoyn_msgarg aboutDataArg, alljoyn_msgarg objectDescriptionArg);
        void Shutdown();

        bool GetHasAnySessions();

        inline AllJoynProvider ^ GetProvider() const { return m_provider; }
        inline alljoyn_busattachment GetBusAttachment() const { return m_provider->GetBusAttachment(); }
        inline const std::string& GetName() const { return m_name; }
        inline alljoyn_aboutobjectdescription GetObjectDescription() const { return m_objectDescription; }
        inline bool GetIsActive()
        {
            AutoLock lock(&m_lock, true);
            return m_active;
        }
        void SessionLost(AllJoynSessionImplementation^ session, alljoyn_sessionlostreason reason);

    private:
        void ClearDeadSessionReferences();
        AllJoynSession^ GetImplicitSession();

    public:
        virtual ~AllJoynService();

        virtual property Windows::Foundation::Collections::IVector<IBusObject ^>^ Objects
        {
            Windows::Foundation::Collections::IVector<IBusObject ^>^ get();
        }
        virtual property Platform::String ^ Name
        {
            Platform::String ^ get();
        }
        virtual property IAboutData^ AboutData
        {
            IAboutData^ get();
        }
        virtual property IProvider^ Provider
        {
            inline IProvider^ get() { return m_provider; }
        }
        virtual property uint16 AnnouncedPort
        {
            inline uint16 get() { return m_announcedPort; }
        }
        virtual property uint16 PreferredPort
        {
            uint16 get()
            {
                AutoLock lock(&m_lock, true);
                return m_preferredPort;
            }
            void set(uint16 value)
            {
                AutoLock lock(&m_lock, true);
                m_preferredPort = value;
            }
        }

        virtual AllJoynStatus^ Ping();
        virtual AllJoynSession^ JoinSession();
        virtual AllJoynSession^ JoinSession(uint16 port);

        virtual bool ImplementsInterface(Platform::String^ interfaceName);
        virtual IBusObject^ GetBusObject(Platform::String^ path);
        virtual Windows::Foundation::Collections::IVector<IBusObject^>^ GetBusObjectsWhichImplementInterface(Platform::String ^ interfaceName);

    private:
        CSLock m_lock;
        bool m_active;
        AllJoynSession^ m_implicitSession;
        std::map<uint16, Platform::WeakReference> m_sessionsMap;
        AllJoynAboutData^ m_aboutData;
        AllJoynProvider ^ m_provider;
        std::string m_name;
        alljoyn_sessionport m_announcedPort;
        alljoyn_sessionport m_preferredPort;
        alljoyn_msgarg m_aboutDataArg;
        alljoyn_msgarg m_objectDescriptionArg;
        alljoyn_aboutobjectdescription m_objectDescription;

        static DWORD s_pingTimeout;
    };
}