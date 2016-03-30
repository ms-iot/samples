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

#include "UnitTestHelper.h"

#include "RCSRemoteResourceObject.h"
#include "RCSDiscoveryManager.h"
#include "RCSResourceObject.h"
#include "RCSAddress.h"

#include "OCPlatform.h"

#include <condition_variable>
#include <mutex>

using namespace OIC::Service;
using namespace OC::OCPlatform;

constexpr char RESOURCEURI[]{ "/a/TemperatureSensor" };
constexpr char RESOURCETYPE[]{ "resource.type" };
constexpr char RESOURCEINTERFACE[]{ "oic.if.baseline" };
constexpr int DEFAULT_DISCOVERYTASK_DELAYTIME = 3000;

void resourceDiscoveredForCall(RCSRemoteResourceObject::Ptr) {}
void resourceDiscoveredForNeverCall(RCSRemoteResourceObject::Ptr) {}

class DiscoveryManagerTest: public TestWithMock
{
public:

    typedef std::unique_ptr<RCSDiscoveryManager::DiscoveryTask> DiscoveryTaskPtr;
    typedef std::function< void(std::shared_ptr< RCSRemoteResourceObject >) >
                                       ResourceDiscoveredCallback;
public:

    static DiscoveryTaskPtr discoverResource(ResourceDiscoveredCallback cb)
    {
        const std::string uri  = "/oic/res";
        return RCSDiscoveryManager::getInstance()->discoverResourceByType(RCSAddress::multicast(),
                 uri, RESOURCETYPE, cb);
    }

    void startDiscovery()
    {
        discoveryTask = discoverResource(resourceDiscoveredForCall);
    }

    void cancelDiscovery()
    {
        discoveryTask->cancel();
    }

    bool isCanceled()
    {
        return discoveryTask->isCanceled();
    }

    void createResource()
    {
        server = RCSResourceObject::Builder(RESOURCEURI, RESOURCETYPE, RESOURCEINTERFACE).build();
    }

    void proceed()
    {
        cond.notify_all();
    }

    void waitForDiscoveryTask(int waitingTime = DEFAULT_DISCOVERYTASK_DELAYTIME)
    {
        std::unique_lock< std::mutex > lock{ mutex };
        cond.wait_for(lock, std::chrono::milliseconds{ waitingTime });
    }

private:

    std::condition_variable cond;
    std::mutex mutex;
    RCSResourceObject::Ptr server;
    RCSRemoteResourceObject::Ptr object;
    DiscoveryTaskPtr discoveryTask;
};

TEST_F(DiscoveryManagerTest, resourceIsNotSupportedPresenceBeforeDiscovering)
{
    createResource();

    mocks.ExpectCallFunc(resourceDiscoveredForCall).Do(
        [this](RCSRemoteResourceObject::Ptr){ proceed();});

    startDiscovery();
    waitForDiscoveryTask();
}

TEST_F(DiscoveryManagerTest, resourceIsSupportedPresenceBeforeDiscovering)
{
    startPresence(10);
    createResource();

    mocks.ExpectCallFunc(resourceDiscoveredForCall).Do(
        [this](RCSRemoteResourceObject::Ptr){ proceed();});

    startDiscovery();
    waitForDiscoveryTask();
    stopPresence();
}

TEST_F(DiscoveryManagerTest, resourceIsNotSupportedPresenceAfterDiscovering)
{
    mocks.ExpectCallFunc(resourceDiscoveredForCall).Do(
        [this](RCSRemoteResourceObject::Ptr){ proceed();});

    startDiscovery();
    createResource();
    waitForDiscoveryTask();
}

TEST_F(DiscoveryManagerTest, resourceIsSupportedPresenceAndAfterDiscovering)
{
    mocks.ExpectCallFunc(resourceDiscoveredForCall).Do(
        [this](RCSRemoteResourceObject::Ptr){ proceed();});

    startPresence(10);
    startDiscovery();
    createResource();
    waitForDiscoveryTask();
    stopPresence();
}

TEST_F(DiscoveryManagerTest, cancelDiscoveryTaskAfterDiscoveryResource)
{
    startDiscovery();
    cancelDiscovery();

    mocks.NeverCallFunc(resourceDiscoveredForCall);

    waitForDiscoveryTask();
    createResource();
}

TEST_F(DiscoveryManagerTest, cancelDiscoveryTaskNotStartDiscoveryResource)
{
    startDiscovery();
    cancelDiscovery();
    cancelDiscovery();
}

TEST_F(DiscoveryManagerTest, isCanceledAfterCancelDiscoveryTask)
{
    startDiscovery();
    cancelDiscovery();

    ASSERT_TRUE(isCanceled());
}

TEST_F(DiscoveryManagerTest, multipleDiscoveryRequestAndCancelJustOneDiscoveryRequest)
{
    DiscoveryTaskPtr canceledTask = discoverResource(resourceDiscoveredForCall);
    DiscoveryTaskPtr notCanceledTask_1 = discoverResource(resourceDiscoveredForCall);
    DiscoveryTaskPtr notCanceledTask_2 = discoverResource(resourceDiscoveredForCall);

    canceledTask->cancel();

    ASSERT_TRUE(canceledTask->isCanceled());
    ASSERT_FALSE(notCanceledTask_1->isCanceled());
    ASSERT_FALSE(notCanceledTask_2->isCanceled());
}

TEST_F(DiscoveryManagerTest, equalDiscoveryRequestsAndCancelJustOneRequest)
{
    mocks.ExpectCallFunc(resourceDiscoveredForCall).Do(
        [this](RCSRemoteResourceObject::Ptr){ proceed();});

    mocks.NeverCallFunc(resourceDiscoveredForNeverCall);

    DiscoveryTaskPtr notCanceledTask = discoverResource(resourceDiscoveredForCall);
    DiscoveryTaskPtr canceledTask = discoverResource(resourceDiscoveredForNeverCall);
    canceledTask->cancel();

    createResource();
    waitForDiscoveryTask();
}
