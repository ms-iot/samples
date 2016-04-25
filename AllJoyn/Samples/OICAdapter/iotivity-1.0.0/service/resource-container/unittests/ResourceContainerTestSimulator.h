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

#include <memory>
#include <mutex>
#include <atomic>

#include "UnitTestHelper.h"

#include "RCSDiscoveryManager.h"
#include "RCSRemoteResourceObject.h"
#include "RCSResourceAttributes.h"
#include "RCSAddress.h"

using namespace testing;
using namespace OIC::Service;

class ResourceContainerTestSimulator
    : public std::enable_shared_from_this<ResourceContainerTestSimulator>
{
    public:
        typedef std::shared_ptr<ResourceContainerTestSimulator> Ptr;

        RCSResourceObject::Ptr server;
        RCSRemoteResourceObject::Ptr remoteResource;

    private:
        std::mutex mutexForDiscovery;

        std::string MULTICASTURI;
        std::string RESOURCEURI;
        std::string RESOURCETYPE;
        std::string RESOURCEINTERFACE;
        std::string ATTR_KEY;
        int ATTR_VALUE;
        std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask;

    public:
        ResourceContainerTestSimulator()
            : server(nullptr), remoteResource(nullptr),
              mutexForDiscovery(),
              MULTICASTURI("/oic/res"),
              RESOURCEURI("/a/TempHumSensor/Container"),
              RESOURCETYPE("resource.container"),
              RESOURCEINTERFACE("oic.if.baseline"),
              ATTR_KEY("Temperature"),
              ATTR_VALUE(0)
        { }

        ~ResourceContainerTestSimulator()
        {
            if (remoteResource != nullptr && remoteResource->isCaching())
            {
                remoteResource->stopCaching();
            }
            if (remoteResource != nullptr && remoteResource->isMonitoring())
            {
                remoteResource->stopMonitoring();
            }
        }

    private:
        void onDiscoveryResource_Impl(RCSRemoteResourceObject::Ptr resourceObject)
        {
            if (remoteResource != nullptr)
            {
                return;
            }

            if (RESOURCEURI.compare(resourceObject->getUri()) != 0)
            {
                return;
            }

            remoteResource = resourceObject;
            mutexForDiscovery.unlock();
        }

        static void onDiscoveryResource(RCSRemoteResourceObject::Ptr resourceObject,
                                        std::weak_ptr<ResourceContainerTestSimulator> rPtr)
        {
            std::shared_ptr<ResourceContainerTestSimulator> ptr = rPtr.lock();
            if (ptr != nullptr)
            {
                ptr->onDiscoveryResource_Impl(resourceObject);
            }
            else
            {
                std::cout << "Aleady delete simulator\n";
            }
        }
        void waitForDiscovery()
        {
            std::chrono::milliseconds interval(100);
            while (true)
            {
                if (mutexForDiscovery.try_lock())
                {
                    mutexForDiscovery.unlock();
                    return;
                }
                std::this_thread::sleep_for(interval);
            }
        }

    public:
        void defaultRunSimulator()
        {
            createResource();
            discoveryResource();
            waitForDiscovery();
        }

        void createResource()
        {
            server = RCSResourceObject::Builder(RESOURCEURI, RESOURCETYPE, RESOURCEINTERFACE)
                     .setDiscoverable(true).setObservable(true).build();
            server->setAttribute(ATTR_KEY, ATTR_VALUE);
        }

        void discoveryResource()
        {
            discoveryResource(RESOURCETYPE);
        }

        void discoveryResource(std::string &resourceType)
        {
            try
            {
                discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
                                    RCSAddress::multicast(), MULTICASTURI, resourceType,
                                    std::bind(onDiscoveryResource, std::placeholders::_1,
                                              std::weak_ptr<ResourceContainerTestSimulator>(shared_from_this())));
                mutexForDiscovery.lock();
            }
            catch (std::exception &e)
            {
                std::cout << "exception : " << e.what() << std::endl;
            }
        }

        std::string getServerUri() const
        {
            return RESOURCEURI;
        }

        RCSResourceObject::Ptr getResourceServer() const
        {
            return server;
        }

        RCSRemoteResourceObject::Ptr getRemoteResource() const
        {
            return remoteResource;
        }

        void ChangeAttributeValue()
        {
            std::chrono::milliseconds interval(100);
            if (server != nullptr)
                server->setAttribute(ATTR_KEY, ATTR_VALUE + 10);
            std::this_thread::sleep_for(interval);
        }

        void ChangeResourceState()
        {
            std::chrono::milliseconds interval(400);
            if (server != nullptr)
                server = nullptr;
            std::this_thread::sleep_for(interval);
        }

};
