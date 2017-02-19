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
#include "AllJoynBusObject.h"
#include "AllJoynHelpers.h"
#include "AllJoynService.h"
#include "AllJoynSessionImplementation.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation::Collections;

namespace DeviceProviders
{
    AllJoynSessionImplementation::AllJoynSessionImplementation(AllJoynService^ service, uint16 sessionPort, QStatus& status)
        : m_service(service)
        , m_sessionListener(nullptr)
        , m_sessionPort(sessionPort)
        , m_sessionId(0)
        , m_usageCount(1)
    {
        alljoyn_sessionlistener_callbacks sessionListenerCallbacks = { SessionLostThunk, nullptr, nullptr };
        auto pUnknown = reinterpret_cast<IUnknown*>(this);
        pUnknown->AddRef();
        m_sessionListener = alljoyn_sessionlistener_create(&sessionListenerCallbacks, pUnknown);

        alljoyn_sessionopts sessionOpts = alljoyn_sessionopts_create(ALLJOYN_TRAFFIC_TYPE_MESSAGES, QCC_FALSE, ALLJOYN_PROXIMITY_ANY, ALLJOYN_TRANSPORT_ANY);

        status = alljoyn_busattachment_joinsession(m_service->GetBusAttachment(),
            m_service->GetName().c_str(),
            m_sessionPort,
            m_sessionListener,
            &m_sessionId,
            sessionOpts);

        if (status != ER_OK)
        {
            m_service->GetProvider()->RemoveService(m_service, alljoyn_sessionlostreason::ALLJOYN_SESSIONLOST_REMOTE_END_CLOSED_ABRUPTLY);
            m_usageCount = 0;
        }
        else
        {
            m_active = true;
        }
    }

    AllJoynSessionImplementation ^ AllJoynSessionImplementation::CreateSession(AllJoynService ^ service, uint16 sessionPort)
    {
        QStatus status;
        return CreateSession(service, sessionPort, status);
    }

    AllJoynSessionImplementation ^ AllJoynSessionImplementation::CreateSession(AllJoynService ^ service, uint16 sessionPort, QStatus & status)
    {
        status = ER_OK;
        auto session = ref new AllJoynSessionImplementation(service, sessionPort, status);

        if (status != ER_OK)
        {
            session = nullptr;
        }

        return session;
    }

    void AllJoynSessionImplementation::Shutdown()
    {
        AutoLock lock(&m_lock, true);
        if (!m_active)
        {
            return;
        }

        m_active = false;

        // Leave the session if connected
        if (m_sessionId)
        {
            if (m_service)
            {
                auto status = alljoyn_busattachment_leavesession(m_service->GetBusAttachment(), m_sessionId);

                if (ER_OK != status && m_service->GetIsActive())
                {
                    SessionLost(m_sessionId, ALLJOYN_SESSIONLOST_REASON_OTHER);
                }
            }

            m_sessionId = 0;
        }

        // Clear the usage count
        m_usageCount = 0;

        for (auto iter : m_objectsMap)
        {
            auto busObject = iter.second.Resolve<AllJoynBusObject>();
            if (busObject != nullptr)
            {
                busObject->Shutdown();
            }
        }
        m_objectsMap.clear();

        if (m_sessionListener)
        {
            alljoyn_sessionlistener_destroy(m_sessionListener);
            m_sessionListener = nullptr;
        }

        m_service = nullptr;

        auto pUnknown = reinterpret_cast<IUnknown*>(this);
        pUnknown->Release();
    }

    void AllJoynSessionImplementation::IncrementUsage()
    {
        AutoLock lock(&m_lock, true);
        ++m_usageCount;
    }

    int AllJoynSessionImplementation::DecrementUsage()
    {
        AutoLock lock(&m_lock, true);

        // Ensure that usage count doesn't go negative;
        if (m_usageCount == 0)
        {
            return 0;
        }

        // On decrementing last usage, leave the session
        if (--m_usageCount == 0)
        {
            Shutdown();
        }

        return m_usageCount;
    }

    IVector<IBusObject^>^ AllJoynSessionImplementation::GetBusObjects()
    {
        AutoLock lock(&m_lock, true);

        if (m_service && !m_service->GetIsActive())
        {
            return nullptr;
        }

        auto busObjects = ref new Vector<IBusObject^>();

        for (auto& kvp : m_objectsMap)
        {
            auto busObject = GetOrCreateBusObject(kvp.first);
            if (busObject)
            {
                busObjects->Append(busObject);
            }
            else
            {
                break;
            }
        }

        return busObjects;
    }

    IBusObject ^ AllJoynSessionImplementation::GetBusObject(String^ fullpath)
    {
        auto path = AllJoynHelpers::PlatformToMultibyteStandardString(fullpath);
        IBusObject^ busObject = GetOrCreateBusObject(path);

        if (busObject == nullptr)
        {
            // find an ancestor of the provided path if we know of one
            auto iter = std::find_if(m_objectsMap.begin(), m_objectsMap.end(), [&path](pair<string, WeakReference> kvp) -> bool
            {
                return path.substr(0, kvp.first.length()) == kvp.first;
            });

            if (iter != m_objectsMap.end())
            {
                auto ancestor = GetOrCreateBusObject(iter->first);
                busObject = ancestor->GetChild(fullpath);
            }
        }
        return busObject;
    }

    IVector<IBusObject^>^ AllJoynSessionImplementation::GetBusObjectsWhichImplementInterface(String ^ interfaceName)
    {
        if (!m_service->GetIsActive())
        {
            return nullptr;
        }

        auto busObjects = ref new Vector<IBusObject^>();

        string interfaceNameString = AllJoynHelpers::PlatformToMultibyteStandardString(interfaceName);

        AutoLock lock(&m_lock, true);

        // get number of bus object path
        size_t pathCount = alljoyn_aboutobjectdescription_getinterfacepaths(m_service->GetObjectDescription(), interfaceNameString.c_str(), nullptr, 0);
        if (0 != pathCount)
        {
            auto pathArray = vector<const char*>(pathCount);
            alljoyn_aboutobjectdescription_getinterfacepaths(m_service->GetObjectDescription(), interfaceNameString.c_str(), pathArray.data(), pathCount);

            for (auto& path : pathArray)
            {
                busObjects->Append(GetOrCreateBusObject(path));
            }
        }

        return busObjects;
    }

    void AllJoynSessionImplementation::OnObjectDescriptionChanged()
    {
        size_t pathCount = alljoyn_aboutobjectdescription_getpaths(m_service->GetObjectDescription(), nullptr, 0);
        if (0 != pathCount)
        {
            auto pathArray = vector<const char*>(pathCount);
            alljoyn_aboutobjectdescription_getpaths(m_service->GetObjectDescription(), pathArray.data(), pathCount);

            AutoLock lock(&m_lock, true);

            // Add the new paths to the map
            for (auto& path : pathArray)
            {
                m_objectsMap.insert(make_pair(path, WeakReference(nullptr)));
            }

            // Determine which paths have disappeared from this announcement
            auto eraseFromMap = vector<string>();
            auto pathSet = set<string>(pathArray.begin(), pathArray.end());
            for (auto& kvp : m_objectsMap)
            {
                if (pathSet.find(kvp.first) == pathSet.end())
                {
                    eraseFromMap.push_back(kvp.first);
                }
            }

            // Remove the disappeared paths from the map
            for (auto& path : eraseFromMap)
            {
                m_objectsMap.erase(path);
            }
        }
    }

    AllJoynBusObject ^ AllJoynSessionImplementation::GetBusObjectIfCreated(const std::string & path)
    {
        AllJoynBusObject^ busObject = nullptr;

        if (m_service->GetIsActive())
        {
            AutoLock lock(&m_lock, true);
            auto iter = m_objectsMap.find(path);
            if (iter != m_objectsMap.end())
            {
                busObject = iter->second.Resolve<AllJoynBusObject>();
            }
        }

        return busObject;
    }

    AllJoynBusObject ^ AllJoynSessionImplementation::GetOrCreateBusObject(const std::string & path)
    {
        AllJoynBusObject^ busObject = GetBusObjectIfCreated(path);

        if (m_service->GetIsActive())
        {
            AutoLock lock(&m_lock, true);

            if (busObject == nullptr && m_objectsMap.find(path) != m_objectsMap.end())
            {
                size_t interfaceCount = alljoyn_aboutobjectdescription_getinterfaces(m_service->GetObjectDescription(), path.data(), nullptr, 0);
                vector<const char*> interfacesArray(interfaceCount);

                if (interfaceCount)
                {
                    alljoyn_aboutobjectdescription_getinterfaces(m_service->GetObjectDescription(), path.data(), interfacesArray.data(), interfaceCount);
                }

                busObject = ref new AllJoynBusObject(this, path, interfacesArray);
                m_objectsMap[path] = WeakReference(busObject);
            }
        }

        return busObject;
    }

    void AJ_CALL AllJoynSessionImplementation::SessionLostThunk(const void * context, alljoyn_sessionid sessionId, alljoyn_sessionlostreason reason)
    {
        auto session = const_cast<AllJoynSessionImplementation^>(reinterpret_cast<const AllJoynSessionImplementation^>(context));
        if (session)
        {
            session->SessionLost(sessionId, reason);
        }
    }

    void AllJoynSessionImplementation::SessionLost(alljoyn_sessionid sessionId, alljoyn_sessionlostreason reason)
    {
        AutoLock lock(&m_lock, true);

        if (sessionId == GetSessionId())
        {
            m_service->SessionLost(this, reason);
            Shutdown();
        }
    }
}