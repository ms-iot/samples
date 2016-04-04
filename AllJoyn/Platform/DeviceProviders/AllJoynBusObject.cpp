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
#include "AllJoynService.h"
#include "AllJoynInterface.h"
#include "AllJoynHelpers.h"

using namespace std;
using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace Platform::Details;
using namespace Platform;

namespace DeviceProviders
{

    AllJoynBusObject::AllJoynBusObject(AllJoynSessionImplementation^ session, const string& path, const vector<const char*> interfaceNames)
        : m_session(session)
        , m_proxyBusObject(nullptr)
        , m_path(path)
        , m_introspectedSuccessfully(false)
        , m_active(true)
    {
        DEBUG_LIFETIME_IMPL(AllJoynBusObject);

        for (auto interfaceName : interfaceNames)
        {
            m_interfaces[interfaceName] = WeakReference(nullptr);
        }
    }

    AllJoynBusObject::AllJoynBusObject(AllJoynSessionImplementation^ session, const string& path, alljoyn_proxybusobject proxyBusObject)
        : m_session(session)
        , m_proxyBusObject(proxyBusObject)
        , m_path(path)
        , m_introspectedSuccessfully(false)
        , m_active(true)
    {
        DEBUG_LIFETIME_IMPL(AllJoynBusObject);
    }

    AllJoynBusObject::~AllJoynBusObject()
    {
        if (m_active)
        {
            Shutdown();
        }
    }

    void AllJoynBusObject::Shutdown()
    {
        m_active = false;

        {
            AutoLock lock(&m_lock, true);
            for (auto iter : m_interfaces)
            {
                auto iface = iter.second.Resolve<AllJoynInterface>();
                if (iface != nullptr)
                {
                    iface->Shutdown();
                }
            }
            m_interfaces.clear();

            for (auto iter : m_childObjects)
            {
                auto object = iter.second.Resolve<AllJoynBusObject>();
                if (object != nullptr)
                {
                    object->Shutdown();
                }
            }
            m_childObjects.clear();
        }

        if (nullptr != m_proxyBusObject)
        {
            alljoyn_proxybusobject_destroy(m_proxyBusObject);
            m_proxyBusObject = nullptr;
        }
    }

    bool AllJoynBusObject::Introspect()
    {
        if (!m_introspectedSuccessfully)
        {
            if (m_proxyBusObject == nullptr)
            {
                m_proxyBusObject = alljoyn_proxybusobject_create(GetService()->GetBusAttachment(),
                    GetService()->GetName().c_str(),
                    m_path.c_str(),
                    m_session->GetSessionId());
            }

            if (m_proxyBusObject != nullptr)
            {
                if (ER_OK == alljoyn_proxybusobject_introspectremoteobject(m_proxyBusObject))
                {
                    m_introspectedSuccessfully = true;
                }
            }
        }
        return m_introspectedSuccessfully;
    }

    IVector<IInterface ^>^ AllJoynBusObject::Interfaces::get()
    {
        if (!m_active || !this->Introspect())
        {
            return nullptr;
        }

        auto interfaces = ref new Vector<IInterface ^>();
        AutoLock lock(&m_lock, true);

        // First check for interfaces we already know about. Not sure this is necessary. It will only matter if there are interface mention in the
        // About announcement but not returned in alljoyn_proxybusobject_getinterfaces
        for (auto& interfaceNameIterator : m_interfaces)
        {
            auto iface = interfaceNameIterator.second.Resolve<AllJoynInterface>();
            if (iface == nullptr)
            {
                alljoyn_interfacedescription description = alljoyn_proxybusobject_getinterface(m_proxyBusObject, interfaceNameIterator.first.data());
                if (nullptr != description)
                {
                    iface = ref new AllJoynInterface(this, description);
                }
            }
            interfaces->Append(iface);
            m_interfaces[interfaceNameIterator.first] = WeakReference(iface);
        }

        size_t interfaceCount = alljoyn_proxybusobject_getinterfaces(m_proxyBusObject, nullptr, 0);

        if (interfaceCount > 0)
        {
            auto interfaceDescriptions = vector<alljoyn_interfacedescription>(interfaceCount);
            alljoyn_proxybusobject_getinterfaces(m_proxyBusObject, interfaceDescriptions.data(), interfaceCount);

            for (auto& description : interfaceDescriptions)
            {
                // if the interface has already been created (and still exists) don't create it again
                string interfaceName = alljoyn_interfacedescription_getname(description);

                auto iter = m_interfaces.find(interfaceName);
                if (iter == m_interfaces.end() || iter->second.Resolve<AllJoynInterface>() == nullptr)
                {
                    auto iface = ref new AllJoynInterface(this, description);
                    interfaces->Append(iface);
                    m_interfaces[interfaceName] = WeakReference(iface);
                }
            }
        }
        return interfaces;
    }

    IVector<IBusObject ^>^ AllJoynBusObject::ChildObjects::get()
    {
        if (!m_active || !this->Introspect())
        {
            return nullptr;
        }
        auto childObjects = ref new Vector<IBusObject ^>();

        size_t childCount = alljoyn_proxybusobject_getchildren(m_proxyBusObject, nullptr, 0);

        AutoLock lock(&m_lock, true);

        if (childCount > 0)
        {
            auto children = vector<alljoyn_proxybusobject>(childCount);
            alljoyn_proxybusobject_getchildren(m_proxyBusObject, children.data(), childCount);

            for (size_t i = 0; i < childCount; ++i)
            {
                string path = alljoyn_proxybusobject_getpath(children[i]);

                // Check if we have already added this object to our map of children
                auto busObject = this->GetChildIfCreated(path);
                if (busObject == nullptr)
                {
                    // Also check if the service already created this object, but it's not yet in our children map.
                    busObject = m_session->GetBusObjectIfCreated(path);
                    if (busObject == nullptr)
                    {
                        busObject = ref new AllJoynBusObject(m_session, path, children[i]);
                    }
                    m_childObjects[path] = WeakReference(busObject);
                }

                childObjects->Append(busObject);
            }
        }
        return childObjects;
    }

    String ^ AllJoynBusObject::Path::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_path.c_str());
    }

    IInterface^ AllJoynBusObject::GetInterface(String^ interfaceName)
    {
        if (!m_active || !this->Introspect())
        {
            return nullptr;
        }

        string name = AllJoynHelpers::PlatformToMultibyteStandardString(interfaceName);

        AutoLock lock(&m_lock, true);

        auto iter = m_interfaces.find(name);
        if (iter != m_interfaces.end() && iter->second.Resolve<IInterface>() != nullptr)
        {
            return iter->second.Resolve<IInterface>();
        }

        auto interfaceDescription = alljoyn_proxybusobject_getinterface(m_proxyBusObject, name.data());
        if (interfaceDescription != nullptr)
        {
            IInterface^ iface = ref new AllJoynInterface(this, interfaceDescription);
            m_interfaces[name] = WeakReference(iface);
            return iface;
        }

        return nullptr;
    }

    IBusObject^ AllJoynBusObject::GetChild(String^ fullPath)
    {
        if (!m_active || !this->Introspect())
        {
            return nullptr;
        }

        AutoLock lock(&m_lock, true);

        auto path = AllJoynHelpers::PlatformToMultibyteStandardString(fullPath);
        auto busObject = GetChildIfCreated(path);

        if (busObject == nullptr)
        {
            if (path.length() > m_path.length() + 1)
            {
                auto proxyBusObject = alljoyn_proxybusobject_getchild(m_proxyBusObject, path.substr(m_path.length() + 1).data());
                if (proxyBusObject != nullptr)
                {
                    busObject = ref new AllJoynBusObject(m_session, path, proxyBusObject);
                    m_childObjects[path] = WeakReference(busObject);
                }
            }
        }
        return busObject;
    }

    AllJoynBusObject^ AllJoynBusObject::GetChildIfCreated(const std::string& fullPath)
    {
        AllJoynBusObject^ busObject = nullptr;

        if (m_active)
        {
            AutoLock lock(&m_lock, true);

            auto iter = m_childObjects.find(fullPath);
            if (iter != m_childObjects.end())
            {
                busObject = iter->second.Resolve<AllJoynBusObject>();
            }
        }
        return busObject;
    }
}