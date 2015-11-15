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
#include "AllJoynService.h"
#include "AllJoynBusObject.h"
#include "AllJoynProvider.h"
#include "AllJoynHelpers.h"
#include "AllJoynAboutData.h"
#include "AllJoynSession.h"
#include "AllJoynSessionImplementation.h"
#include "AllJoynStatus.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace Platform;
using namespace std;

namespace DeviceProviders
{
    DWORD AllJoynService::s_pingTimeout = 1000;

    AllJoynService::AllJoynService(AllJoynProvider ^ provider, std::string serviceName, alljoyn_sessionport port)
        : m_provider(provider)
        , m_name(serviceName)
        , m_announcedPort(port)
        , m_preferredPort(0)
        , m_aboutDataArg(nullptr)
        , m_objectDescriptionArg(nullptr)
        , m_objectDescription(nullptr)
        , m_active(false)
    {
        DEBUG_LIFETIME_IMPL(AllJoynService);
    }

    AllJoynService::~AllJoynService()
    {
        if (GetIsActive())
        {
            Shutdown();
        }
    }

    void AllJoynService::Initialize(alljoyn_msgarg aboutDataArg, alljoyn_msgarg objectDescriptionArg)
    {
        AutoLock lock(&m_lock, true);

        if (m_aboutDataArg != nullptr)
        {
            alljoyn_msgarg_destroy(m_aboutDataArg);
        }
        if (m_objectDescriptionArg != nullptr)
        {
            alljoyn_msgarg_destroy(m_objectDescriptionArg);
        }
        if (m_objectDescription != nullptr)
        {
            alljoyn_aboutobjectdescription_destroy(m_objectDescription);
        }

        m_aboutDataArg = aboutDataArg;
        m_objectDescriptionArg = objectDescriptionArg;
        m_objectDescription = alljoyn_aboutobjectdescription_create_full(m_objectDescriptionArg);

        m_active = true;
        OutputDebugString(L"AllJoynService::Initialize: m_active=true\n");

        ClearDeadSessionReferences();
        for (auto& kvp : m_sessionsMap)
        {
            auto session = kvp.second.Resolve<AllJoynSessionImplementation>();
            if (session)
            {
                session->OnObjectDescriptionChanged();
            }
        }
    }

    void AllJoynService::Shutdown()
    {
        AutoLock lock(&m_lock, true);
        m_active = false;

        m_implicitSession = nullptr;

        for (auto& kvp : m_sessionsMap)
        {
            auto session = kvp.second.Resolve<AllJoynSessionImplementation>();
            if (session)
            {
                session->Shutdown();
            }
        }
        m_sessionsMap.clear();

        if (m_aboutDataArg != nullptr)
        {
            alljoyn_msgarg_destroy(m_aboutDataArg);
            m_aboutDataArg = nullptr;
        }

        if (m_objectDescriptionArg != nullptr)
        {
            alljoyn_msgarg_destroy(m_objectDescriptionArg);
            m_objectDescriptionArg = nullptr;
        }

        if (m_objectDescription != nullptr)
        {
            alljoyn_aboutobjectdescription_destroy(m_objectDescription);
            m_objectDescription = nullptr;
        }
    }

    AllJoynSession^ AllJoynService::JoinSession()
    {
        auto session = this->JoinSession(m_announcedPort);
        return session;
    }

    AllJoynSession^ AllJoynService::JoinSession(uint16 sessionPort)
    {
        AutoLock lock(&m_lock, true);

        QStatus status = ER_OK;
        AllJoynSessionImplementation^ session;

        auto sessionIterator = m_sessionsMap.find(sessionPort);
        if (sessionIterator != m_sessionsMap.end())
        {
            session = sessionIterator->second.Resolve<AllJoynSessionImplementation>();
            if (!session)
            {
                m_sessionsMap.erase(sessionIterator);
            }
        }

        if (!session)
        {
            session = AllJoynSessionImplementation::CreateSession(this, sessionPort);
            if (session)
            {
                session->OnObjectDescriptionChanged();
                m_sessionsMap.insert(make_pair(sessionPort, WeakReference(session)));
            }
        }

        return session ? ref new AllJoynSession(session) : nullptr;
    }

    bool AllJoynService::GetHasAnySessions()
    {
        AutoLock lock(&m_lock, true);
        ClearDeadSessionReferences();
        return !m_sessionsMap.empty();
    }

    void AllJoynService::SessionLost(AllJoynSessionImplementation^ session, alljoyn_sessionlostreason reason)
    {
        GetProvider()->RemoveService(this, reason);
    }

    void AllJoynService::ClearDeadSessionReferences()
    {
        AutoLock lock(&m_lock, true);

        vector<uint16> entriesToErase;
        for (auto kvp : m_sessionsMap)
        {
            if (!kvp.second.Resolve<AllJoynSessionImplementation>())
            {
                entriesToErase.push_back(kvp.first);
            }
        }

        for (auto sessionPort : entriesToErase)
        {
            m_sessionsMap.erase(sessionPort);
        }
    }

    AllJoynSession^ AllJoynService::GetImplicitSession()
    {
        AutoLock lock(&m_lock, true);

        if (!m_implicitSession)
        {
            m_implicitSession = this->JoinSession(m_preferredPort ? m_preferredPort : m_announcedPort);
        }

        return m_implicitSession;
    }

    IVector<IBusObject ^>^ AllJoynService::Objects::get()
    {
        auto session = GetImplicitSession();
        return session ? session->Objects : nullptr;
    }

    String ^ AllJoynService::Name::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_name.c_str());
    }

    IAboutData ^ AllJoynService::AboutData::get()
    {
        AutoLock lock(&m_lock, true);

        if (!m_aboutData && m_aboutDataArg)
        {
            m_aboutData = ref new AllJoynAboutData(this, m_aboutDataArg);
        }

        return m_aboutData;
    }

    AllJoynStatus^ AllJoynService::Ping()
    {
        auto status = alljoyn_busattachment_ping(m_provider->GetBusAttachment(), m_name.c_str(), s_pingTimeout);
        return ref new AllJoynStatus(status);
    }

    bool AllJoynService::ImplementsInterface(String^ interfaceName)
    {
        bool implementsInterface = false;
        if (GetIsActive())
        {
            implementsInterface = alljoyn_aboutobjectdescription_hasinterface(
                m_objectDescription,
                AllJoynHelpers::PlatformToMultibyteStandardString(interfaceName).data());
        }
        return implementsInterface;
    }

    IBusObject^ AllJoynService::GetBusObject(String^ path)
    {
        auto session = GetImplicitSession();
        return session ? session->GetBusObject(path) : nullptr;
    }

    IVector<IBusObject^>^ AllJoynService::GetBusObjectsWhichImplementInterface(String ^ interfaceName)
    {
        auto session = GetImplicitSession();
        return session ? session->GetBusObjectsWhichImplementInterface(interfaceName) : nullptr;
    }
}