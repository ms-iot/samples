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
#include <unistd.h>

#include "gtest/gtest.h"
#include "HippoMocks/hippomocks.h"

#include "BrokerTypes.h"
#include "PrimitiveResource.h"
#include "ResponseStatement.h"
#include "OCPlatform.h"
#include "DevicePresence.h"
#include "ResourcePresence.h"
#include "UnitTestHelper.h"

using namespace testing;
using namespace OIC::Service;
using namespace OC;

typedef OCStackResult (*subscribePresenceSig1)(OC::OCPlatform::OCPresenceHandle&,
        const std::string&, OCConnectivityType, SubscribeCallback);

class DevicePresenceTest : public TestWithMock
{
public:
    typedef std::function<void(OCStackResult,const unsigned int, const std::string&)> subscribeCallback;
    DevicePresence * instance;
    PrimitiveResource::Ptr pResource;
    BrokerCB cb;
    BrokerID id;

protected:

    void SetUp()
    {
        TestWithMock::SetUp();
        instance = (DevicePresence*)new DevicePresence();
        pResource = PrimitiveResource::Ptr(mocks.Mock< PrimitiveResource >(), [](PrimitiveResource*){});
        cb = ([](BROKER_STATE)->OCStackResult{return OC_STACK_OK;});
        id = 0;
    }

    void TearDown()
    {
        TestWithMock::TearDown();
        pResource.reset();
        id = 0;
        cb = nullptr;
    }

    void MockingFunc()
    {
        mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return(std::string());
        mocks.OnCallFuncOverload(static_cast< subscribePresenceSig1 >(OC::OCPlatform::subscribePresence)).Return(OC_STACK_OK);
    }
};
TEST_F(DevicePresenceTest,timeoutCB_TimeOverWhenIsSubscribe)
{
   MockingFunc();
   instance->initializeDevicePresence(pResource);
   std::cout<<"wait while done timeout device presence\n";
   sleep((BROKER_DEVICE_PRESENCE_TIMEROUT/1000)+1);
   ASSERT_EQ(DEVICE_STATE::LOST_SIGNAL,instance->getDeviceState());
}

TEST_F(DevicePresenceTest,SubscribeCB_NormalHandlingIfMessageOC_STACK_OK)
{
   mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return(std::string());
   mocks.OnCallFuncOverload(static_cast< subscribePresenceSig1 >(OC::OCPlatform::subscribePresence)).Do(
            [](OC::OCPlatform::OCPresenceHandle&,
                    const std::string&, OCConnectivityType, SubscribeCallback callback)->OCStackResult{

        callback(OC_STACK_OK,0,std::string());
        return OC_STACK_OK;

    }).Return(OC_STACK_OK);
   instance->initializeDevicePresence(pResource);
   ASSERT_NE(DEVICE_STATE::LOST_SIGNAL,instance->getDeviceState());
}

TEST_F(DevicePresenceTest,initializeDevicePresence_NormalHandlingIfNormalResource)
{

    MockingFunc();

    ASSERT_NO_THROW(instance->initializeDevicePresence(pResource));

}

TEST_F(DevicePresenceTest,initializeDevicePresence_ErrorHandlingIfAbnormalResource)
{

    MockingFunc();
    mocks.OnCallFuncOverload(static_cast< subscribePresenceSig1 >(OC::OCPlatform::subscribePresence)).Return(OC_STACK_ERROR);

    ASSERT_THROW(instance->initializeDevicePresence(pResource),RCSPlatformException);

}

TEST_F(DevicePresenceTest,addPresenceResource_NormalHandlingIfNormalResource)
{

    ResourcePresence * resource = (ResourcePresence *)new ResourcePresence();
    instance->addPresenceResource(resource);

    ASSERT_FALSE(instance->isEmptyResourcePresence());

}

TEST_F(DevicePresenceTest,isEmptyResourcePresence_NormalHandling)
{

    MockingFunc();

    ASSERT_TRUE(instance->isEmptyResourcePresence());

}

TEST_F(DevicePresenceTest,getAddress_NormalHandling)
{

    MockingFunc();

    instance->initializeDevicePresence(pResource);
    instance->getAddress();

}

TEST_F(DevicePresenceTest,NormalHandlingWhenReceivedCallbackMessage)
{

    MockingFunc();

}
