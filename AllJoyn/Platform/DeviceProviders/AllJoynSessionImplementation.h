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

namespace DeviceProviders
{
    ref class AllJoynBusObject;
    ref class AllJoynService;

    ref class AllJoynSessionImplementation
    {
    private:
        AllJoynSessionImplementation(AllJoynService^ service, uint16 sessionPort, QStatus& status);

    internal:
        static AllJoynSessionImplementation^ CreateSession(AllJoynService^ service, uint16 sessionPort);
        static AllJoynSessionImplementation^ CreateSession(AllJoynService^ service, uint16 sessionPort, QStatus& status);
        void Shutdown();

        void IncrementUsage();
        int DecrementUsage();

        Windows::Foundation::Collections::IVector<IBusObject^>^ GetBusObjects();
        IBusObject ^ GetBusObject(Platform::String ^ path);
        Windows::Foundation::Collections::IVector<IBusObject^>^ GetBusObjectsWhichImplementInterface(Platform::String ^ interfaceName);

        void OnObjectDescriptionChanged();

        AllJoynBusObject^ GetBusObjectIfCreated(const std::string& path);
        AllJoynBusObject^ GetOrCreateBusObject(const std::string& path);

        inline AllJoynService^ GetService() { return m_service; }
        inline uint16 GetSessionPort() const { return m_sessionPort; }
        inline alljoyn_sessionid GetSessionId()
        {
            AutoLock lock(&m_lock, true);
            return m_sessionId;
        }
        inline int32 GetUsageCount()
        {
            AutoLock lock(&m_lock, true);
            return m_usageCount;
        };

    private:
        static void AJ_CALL SessionLostThunk(const void * context, alljoyn_sessionid sessionId, alljoyn_sessionlostreason reason);
        void SessionLost(alljoyn_sessionid sessionId, alljoyn_sessionlostreason reason);

    private:
        AllJoynService^ m_service;
        uint16 m_sessionPort;
        alljoyn_sessionlistener m_sessionListener;
        alljoyn_sessionid m_sessionId;


        CSLock m_lock;
        bool m_active;
        int32 m_usageCount;
        std::map<std::string, Platform::WeakReference> m_objectsMap;
    };
}
