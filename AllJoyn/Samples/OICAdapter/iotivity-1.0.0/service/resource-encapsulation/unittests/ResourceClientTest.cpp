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

#include <condition_variable>
#include <mutex>

using namespace OIC::Service;
using namespace OC;

constexpr char RESOURCEURI[]{ "/a/TemperatureSensor" };
constexpr char RESOURCETYPE[]{ "resource.type" };
constexpr char RESOURCEINTERFACE[]{ "oic.if.baseline" };

constexpr char ATTR_KEY[]{ "Temperature" };
constexpr int ATTR_VALUE{ 0 };

constexpr int DEFAULT_WAITING_TIME_IN_MILLIS = 3000;

void getRemoteAttributesCallback(const RCSResourceAttributes&, int) {}
void setRemoteAttributesCallback(const RCSResourceAttributes&, int) {}
void resourceStateChanged(ResourceState) { }
void cacheUpdatedCallback(const RCSResourceAttributes&) {}

class RemoteResourceObjectTest: public TestWithMock
{
public:
    RCSResourceObject::Ptr server;
    RCSRemoteResourceObject::Ptr object;

public:
    void Proceed()
    {
        cond.notify_all();
    }

    void Wait(int waitingTime = DEFAULT_WAITING_TIME_IN_MILLIS)
    {
        std::unique_lock< std::mutex > lock{ mutex };
        cond.wait_for(lock, std::chrono::milliseconds{ waitingTime });
    }

protected:
    void SetUp()
    {
        TestWithMock::SetUp();

        CreateResource();

        WaitUntilDiscovered();

        ASSERT_NE(object, nullptr);
    }

    void TearDown()
    {
        TestWithMock::TearDown();

        // This method is to make sure objects disposed.
        WaitForPtrBeingUnique();
    }

private:
    void CreateResource()
    {
        server = RCSResourceObject::Builder(RESOURCEURI, RESOURCETYPE, RESOURCEINTERFACE).build();
        server->setAttribute(ATTR_KEY, ATTR_VALUE);
    }

    void WaitUntilDiscovered()
    {
        for (int i=0; i<10 && !object; ++i)
        {
            const std::string uri  = "/oic/res";
            auto discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
                    RCSAddress::multicast(), uri, RESOURCETYPE,
                    std::bind(&RemoteResourceObjectTest::resourceDiscovered, this,
                            std::placeholders::_1));
            Wait(1000);
        }
    }

    void WaitForPtrBeingUnique()
    {
        while((object && !object.unique()) || (server && !server.unique()))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
        }
    }

    void resourceDiscovered(RCSRemoteResourceObject::Ptr resourceObject)
    {
        object = resourceObject;

        Proceed();
    }

private:
    std::condition_variable cond;
    std::mutex mutex;
};

TEST_F(RemoteResourceObjectTest, GetRemoteAttributesDoesNotAllowEmptyFunction)
{
    ASSERT_THROW(object->getRemoteAttributes({ }), RCSInvalidParameterException);
}

TEST_F(RemoteResourceObjectTest, GetRemoteAttributesGetsAttributesOfServer)
{
    mocks.ExpectCallFunc(getRemoteAttributesCallback).Match(
            [this](const RCSResourceAttributes& attrs, int)
            {
                RCSResourceObject::LockGuard lock{ server };
                return attrs == server->getAttributes();
            }
    ).Do([this](const RCSResourceAttributes&, int){ Proceed(); });

    object->getRemoteAttributes(getRemoteAttributesCallback);

    Wait();
}

TEST_F(RemoteResourceObjectTest, SetRemoteAttributesDoesNotAllowEmptyFunction)
{
    ASSERT_THROW(object->setRemoteAttributes({ }, { }), RCSInvalidParameterException);
}

TEST_F(RemoteResourceObjectTest, SetRemoteAttributesSetsAttributesOfServer)
{
    constexpr int newValue = ATTR_VALUE + 1;
    RCSResourceAttributes newAttrs;
    newAttrs[ATTR_KEY] = newValue;

    mocks.ExpectCallFunc(setRemoteAttributesCallback).
            Do([this](const RCSResourceAttributes&, int){ Proceed(); });

    object->setRemoteAttributes(newAttrs, setRemoteAttributesCallback);
    Wait();

    ASSERT_EQ(newValue, server->getAttributeValue(ATTR_KEY));
}

TEST_F(RemoteResourceObjectTest, MonitoringIsNotStartedByDefault)
{
    ASSERT_FALSE(object->isMonitoring());
}

TEST_F(RemoteResourceObjectTest, StartMonitoringThrowsIfFunctionIsEmpty)
{
    ASSERT_THROW(object->startMonitoring({ }), RCSInvalidParameterException);
}

TEST_F(RemoteResourceObjectTest, IsMonitoringReturnsTrueAfterStartMonitoring)
{
    object->startMonitoring(resourceStateChanged);

    ASSERT_TRUE(object->isMonitoring());
}

TEST_F(RemoteResourceObjectTest, StartMonitoringThrowsIfTryingToStartAgain)
{
    object->startMonitoring(resourceStateChanged);

    ASSERT_THROW(object->startMonitoring(resourceStateChanged), RCSBadRequestException);
}

TEST_F(RemoteResourceObjectTest, DefaultStateIsNone)
{
    ASSERT_EQ(ResourceState::NONE, object->getState());
}

TEST_F(RemoteResourceObjectTest, CachingIsNotStartedByDefault)
{
    ASSERT_FALSE(object->isCaching());
}

TEST_F(RemoteResourceObjectTest, IsCachingReturnsTrueAfterStartCaching)
{
    object->startCaching(cacheUpdatedCallback);

    ASSERT_TRUE(object->isCaching());
}

TEST_F(RemoteResourceObjectTest, StartCachingThrowsIfTryingToStartAgain)
{
    object->startCaching(cacheUpdatedCallback);

    ASSERT_THROW(object->startCaching(), RCSBadRequestException);
}

TEST_F(RemoteResourceObjectTest, DefaultCacheStateIsNone)
{
    ASSERT_EQ(CacheState::NONE, object->getCacheState());
}

TEST_F(RemoteResourceObjectTest, CacheStateIsUnreadyAfterStartCaching)
{
    object->startCaching();

    ASSERT_EQ(CacheState::UNREADY, object->getCacheState());
}

TEST_F(RemoteResourceObjectTest, CacheStateIsReadyAfterCacheUpdated)
{
    mocks.ExpectCallFunc(cacheUpdatedCallback).
                Do([this](const RCSResourceAttributes&){ Proceed(); });

    object->startCaching(cacheUpdatedCallback);
    Wait();

    ASSERT_EQ(CacheState::READY, object->getCacheState());
}

TEST_F(RemoteResourceObjectTest, IsCachedAvailableReturnsTrueWhenCacheIsReady)
{
    mocks.ExpectCallFunc(cacheUpdatedCallback).
                Do([this](const RCSResourceAttributes&){ Proceed(); });

    object->startCaching(cacheUpdatedCallback);
    Wait();

    ASSERT_TRUE(object->isCachedAvailable());
}

TEST_F(RemoteResourceObjectTest, DISABLED_CacheUpdatedCallbackBeCalledWheneverCacheUpdated)
{
    mocks.OnCallFunc(cacheUpdatedCallback).
            Do([this](const RCSResourceAttributes&){ Proceed(); });
    object->startCaching(cacheUpdatedCallback);
    Wait();

    mocks.ExpectCallFunc(cacheUpdatedCallback).
            Do([this](const RCSResourceAttributes&){ Proceed(); });

    server->setAttribute(ATTR_KEY, ATTR_VALUE + 1);

    Wait();
}

TEST_F(RemoteResourceObjectTest, DISABLED_CacheUpdatedCallbackBeCalledWithUpdatedAttributes)
{
    constexpr int newValue = ATTR_VALUE + 1;

    mocks.OnCallFunc(cacheUpdatedCallback).
            Do([this](const RCSResourceAttributes&){ Proceed(); });
    object->startCaching(cacheUpdatedCallback);
    Wait();

    mocks.ExpectCallFunc(cacheUpdatedCallback).
            Match([this](const RCSResourceAttributes& attrs){
                return attrs.at(ATTR_KEY) == newValue;
            }).
            Do([this](const RCSResourceAttributes&){ Proceed(); });

    server->setAttribute(ATTR_KEY, newValue);

    Wait();
}

TEST_F(RemoteResourceObjectTest, GetCachedAttributesThrowsIfCachingIsNotStarted)
{
    ASSERT_THROW(object->getCachedAttributes(), RCSBadRequestException);
}

TEST_F(RemoteResourceObjectTest, CachedAttributesHasSameAttributesWithServer)
{
    mocks.OnCallFunc(cacheUpdatedCallback).
            Do([this](const RCSResourceAttributes&){ Proceed(); });
    object->startCaching(cacheUpdatedCallback);
    Wait();

    RCSResourceObject::LockGuard lock{ server };

    ASSERT_EQ(object->getCachedAttributes(), server->getAttributes());
}

TEST_F(RemoteResourceObjectTest, GetCachedAttributeThrowsIfCachingIsNotStarted)
{
    ASSERT_THROW(object->getCachedAttribute(ATTR_KEY), RCSBadRequestException);
}

TEST_F(RemoteResourceObjectTest, GetCachedAttributeThrowsIfKeyIsInvalid)
{
    mocks.OnCallFunc(cacheUpdatedCallback).
            Do([this](const RCSResourceAttributes&){ Proceed(); });
    object->startCaching(cacheUpdatedCallback);
    Wait();

    ASSERT_THROW(object->getCachedAttribute(""), RCSInvalidKeyException);
}

TEST_F(RemoteResourceObjectTest, HasSameUriWithServer)
{
    EXPECT_EQ(RESOURCEURI, object->getUri());
}

TEST_F(RemoteResourceObjectTest, HasSameTypeWithServer)
{
    EXPECT_EQ(RESOURCETYPE, object->getTypes()[0]);
}

TEST_F(RemoteResourceObjectTest, HasSameInterfaceWithServer)
{
    EXPECT_EQ(RESOURCEINTERFACE, object->getInterfaces()[0]);
}

