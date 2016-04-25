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

#include "ResourceEncapsulationTestSimulator.h"
#include "HostingObject.h"

#include "RCSDiscoveryManager.h"

using namespace testing;
using namespace OIC::Service;

namespace
{
    bool isDeleted = false;
    void onDestroy(std::weak_ptr<HostingObject> rPtr)
    {
        HostingObject::Ptr ptr = rPtr.lock();
        if(ptr) ptr.reset();
        isDeleted = true;
    }
    void onDiscoveryResource(RCSRemoteResourceObject::Ptr){ }

    void onUpdatedCache(const RCSResourceAttributes &) { }
}

class HostingObjectTest : public TestWithMock
{
public:
    ResourceEncapsulationTestSimulator::Ptr testObject;
    RCSRemoteResourceObject::Ptr remoteObject;

    std::mutex mutexForCondition;
    std::condition_variable responseCon;

protected:

    void SetUp()
    {
        TestWithMock::SetUp();

        testObject = std::make_shared<ResourceEncapsulationTestSimulator>();
        testObject->defaultRunSimulator();
        remoteObject = testObject->getRemoteResource();

        isDeleted = false;
    }

    void TearDown()
    {
        TestWithMock::TearDown();

        if(remoteObject.use_count() > 0)
        {
            if(remoteObject->isCaching())
            {
                remoteObject->stopCaching();
            }
            if(remoteObject->isMonitoring())
            {
                remoteObject->stopMonitoring();
            }
        }
        testObject->destroy();
    }

public:
    void waitForCondition(int waitingTime = 1000)
    {
        std::unique_lock< std::mutex > lock{ mutexForCondition };
        responseCon.wait_for(lock, std::chrono::milliseconds{ waitingTime });
    }

    void notifyCondition()
    {
        responseCon.notify_all();
    }

};

TEST_F(HostingObjectTest, startCachingAtInitialize)
{
    HostingObject::Ptr instance = std::make_shared<HostingObject>();
    instance->initializeHostingObject(
            remoteObject, std::bind(onDestroy, std::weak_ptr<HostingObject>(instance)));

    EXPECT_TRUE(remoteObject->isCaching());
}

TEST_F(HostingObjectTest, startMonitoringAtInitialize)
{
    HostingObject::Ptr instance = std::make_shared<HostingObject>();
    instance->initializeHostingObject(
            remoteObject, std::bind(onDestroy, std::weak_ptr<HostingObject>(instance)));

    ASSERT_TRUE(remoteObject->isMonitoring());
}

TEST_F(HostingObjectTest, getRemoteResourceisValid)
{
    HostingObject::Ptr instance = std::make_shared<HostingObject>();
    instance->initializeHostingObject(
            remoteObject, std::bind(onDestroy, std::weak_ptr<HostingObject>(instance)));

    ASSERT_EQ(remoteObject->getUri(), instance->getRemoteResource()->getUri());
}

TEST_F(HostingObjectTest, createMirroredServer)
{
    int waitForResponse = 1000;
    std::string uri = "";

    HostingObject::Ptr instance = std::make_shared<HostingObject>();
    instance->initializeHostingObject(
            remoteObject, std::bind(onDestroy, std::weak_ptr<HostingObject>(instance)));
    std::this_thread::sleep_for(std::chrono::milliseconds {waitForResponse});

    std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask = { };

    mocks.OnCallFunc(onDiscoveryResource).Do(
            [this, &uri, &discoveryTask](RCSRemoteResourceObject::Ptr ptr)
            {
                if(ptr->getUri() == testObject->getHostedServerUri())
                {
                    uri = ptr->getUri();
                    discoveryTask->cancel();
                    notifyCondition();
                }
            });

    discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
            RCSAddress::multicast(), "resource.hosting", onDiscoveryResource);
    waitForCondition(waitForResponse);

    EXPECT_EQ(testObject->getHostedServerUri(), uri);
}

TEST_F(HostingObjectTest, UpdateCachedDataWhenChangedOriginResource)
{
    int waitForResponse = 1000;
    HostingObject::Ptr instance = std::make_shared<HostingObject>();
    instance->initializeHostingObject(
            remoteObject, std::bind(onDestroy, std::weak_ptr<HostingObject>(instance)));
    std::this_thread::sleep_for(std::chrono::milliseconds {waitForResponse});

    std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> discoveryTask = { };
    RCSRemoteResourceObject::Ptr discoveredResource = { };

    mocks.OnCallFunc(onDiscoveryResource).Do(
            [this, &discoveredResource, &discoveryTask](RCSRemoteResourceObject::Ptr ptr)
            {
                if(ptr->getUri() == testObject->getHostedServerUri())
                {
                    discoveredResource = ptr;
                    discoveryTask->cancel();
                    notifyCondition();
                }
            });

    discoveryTask =  RCSDiscoveryManager::getInstance()->discoverResourceByType(
            RCSAddress::multicast(), "resource.hosting", onDiscoveryResource);
    waitForCondition(waitForResponse);

    RCSResourceAttributes::Value result = { };
    mocks.OnCallFunc(onUpdatedCache).Do(
            [this, &result](const RCSResourceAttributes & att)
            {
                result = att.at("Temperature");
                notifyCondition();
            });

    discoveredResource->startCaching(onUpdatedCache);
    std::this_thread::sleep_for(std::chrono::milliseconds {waitForResponse});

    RCSResourceAttributes::Value settingValue = 10;
    testObject->getResourceServer()->setAttribute("Temperature", settingValue);
    waitForCondition(waitForResponse);

    EXPECT_EQ(result.toString(), settingValue.toString());

}
