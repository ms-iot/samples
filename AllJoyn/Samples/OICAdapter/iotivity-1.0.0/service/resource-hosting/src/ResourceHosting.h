//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef RH_RESOURCEHOSTING_H_
#define RH_RESOURCEHOSTING_H_

#include <cstdbool>
#include <iostream>
#include <list>
#include <memory>
#include <functional>
#include <string>
#include <atomic>

#include "octypes.h"
#include "RCSAddress.h"
#include "PresenceSubscriber.h"
#include "HostingObject.h"
#include "PrimitiveResource.h"
#include "RCSDiscoveryManager.h"

namespace OIC
{
namespace Service
{

class RCSDiscoveryManager;
class ResourceHosting
{
private:
    typedef std::shared_ptr<HostingObject> HostingObjectPtr;
    typedef std::weak_ptr<HostingObject> HostingObjectWeakPtr;

    typedef std::shared_ptr<RCSRemoteResourceObject> RemoteObjectPtr;
    typedef std::shared_ptr<PrimitiveResource> PrimiteveResourcePtr;

    typedef std::function<
            void(std::shared_ptr<RCSRemoteResourceObject>)> DiscoveryCallback;
    typedef std::function<void()> DestroyedCallback;

public:
    void startHosting();
    void stopHosting();

    static ResourceHosting * getInstance();

private:
    ResourceHosting();
    ~ResourceHosting() = default;

    ResourceHosting(const ResourceHosting&) = delete;
    ResourceHosting(ResourceHosting&&) = delete;
    ResourceHosting& operator=(const ResourceHosting&) const = delete;
    ResourceHosting& operator=(ResourceHosting&&) const = delete;

    static ResourceHosting * s_instance;
    static std::mutex s_mutexForCreation;
    std::mutex mutexForList;

    std::list<HostingObjectPtr> hostingObjectList;

    RCSDiscoveryManager * discoveryManager;
    std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask;

    DiscoveryCallback pDiscoveryCB;

    void initializeResourceHosting();

    void requestMulticastDiscovery();

    void discoverHandler(RemoteObjectPtr remoteResource);

    HostingObjectPtr findRemoteResource(RemoteObjectPtr remoteResource);
    bool isSameRemoteResource(RemoteObjectPtr remoteResource_1, RemoteObjectPtr remoteResource_2);

    void destroyedHostingObject(HostingObjectWeakPtr destroyedWeakPtr);

};

} /* namespace Service */
} /* namespace OIC */

#endif /* RH_RESOURCEHOSTING_H_ */
