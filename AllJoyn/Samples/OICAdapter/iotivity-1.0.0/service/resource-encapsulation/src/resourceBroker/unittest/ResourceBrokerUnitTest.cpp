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
#include "gtest/gtest.h"
#include "HippoMocks/hippomocks.h"

#include "OCPlatform.h"
#include "PrimitiveResource.h"
#include "ResponseStatement.h"
#include "ResourceBroker.h"
#include "UnitTestHelper.h"

using namespace testing;
using namespace OIC::Service;
using namespace OC;

typedef OCStackResult (*subscribePresenceSig1)(OC::OCPlatform::OCPresenceHandle&,
        const std::string&, OCConnectivityType, SubscribeCallback);

class ResourceBrokerTest : public TestWithMock
{
public:

    ResourceBroker * brokerInstance;
    PrimitiveResource::Ptr pResource;
    BrokerCB cb;
    BrokerID id;

protected:

    void SetUp()
    {
        TestWithMock::SetUp();
        brokerInstance = ResourceBroker::getInstance();
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
        mocks.OnCall(pResource.get(), PrimitiveResource::requestGet);
        mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return(std::string());
        mocks.OnCallFuncOverload(static_cast< subscribePresenceSig1 >(OC::OCPlatform::subscribePresence)).Return(OC_STACK_OK);
    }

};

TEST_F(ResourceBrokerTest,HostResource_ReturnNormalValueIfNormalParams)
{

    MockingFunc();

    BrokerID ret = brokerInstance->hostResource(pResource, cb);
    ASSERT_NE(BrokerID(0), ret);

    brokerInstance->cancelHostResource(ret);

}

TEST_F(ResourceBrokerTest, HostResource_NormalErrorHandlingIfResourceNull)
{

    ASSERT_THROW(brokerInstance->hostResource(nullptr, cb),ResourceBroker::InvalidParameterException);

}

TEST_F(ResourceBrokerTest, HostResource_NormalErrorHandlingIfCbFuncNull)
{

    ASSERT_THROW(brokerInstance->hostResource(pResource,nullptr),ResourceBroker::InvalidParameterException);

}

TEST_F(ResourceBrokerTest,CancelHostResource_NoThrowIfNormalParams)
{

    MockingFunc();

    BrokerID ret;
    ret = brokerInstance->hostResource(pResource,cb);

    ASSERT_NO_THROW(brokerInstance->cancelHostResource(ret));


}

TEST_F(ResourceBrokerTest,CancelHostResource_NormalErrorHandlingIfAbNormalIdZero)
{

    id = 0;
    ASSERT_THROW(brokerInstance->cancelHostResource(id),ResourceBroker::InvalidParameterException);

}

TEST_F(ResourceBrokerTest,CancelHostResource_NormalErrorHandlingIfAbNormalIdOutOfRangeValue)
{

    id = -1;
    ASSERT_THROW(brokerInstance->cancelHostResource(id),ResourceBroker::InvalidParameterException);

}

TEST_F(ResourceBrokerTest,getResourceState_ReturnNormalValueIfNormalId)
{

    MockingFunc();

    BrokerID ret;
    ret = brokerInstance->hostResource(pResource,cb);

    ASSERT_NE(brokerInstance->getResourceState(ret),BROKER_STATE::NONE);

    brokerInstance->cancelHostResource(ret);
    TearDown();

}

TEST_F(ResourceBrokerTest,getResourceState_NormalErrorHandlingIfIdZero)
{

    id = 0;
    ASSERT_THROW(brokerInstance->getResourceState(id),ResourceBroker::InvalidParameterException);

}

TEST_F(ResourceBrokerTest,getResourceState_ReturnNormalValueIfNormalResource)
{

    MockingFunc();

    BrokerID ret;
    ret = brokerInstance->hostResource(pResource,cb);

    ASSERT_NE(brokerInstance->getResourceState(pResource),BROKER_STATE::NONE);

    brokerInstance->cancelHostResource(ret);

}

TEST_F(ResourceBrokerTest,getResourceState_NormalErrorHandlingIfResourceNull)
{

    ASSERT_THROW(brokerInstance->getResourceState((PrimitiveResource::Ptr)nullptr),ResourceBroker::InvalidParameterException);

}

TEST_F(ResourceBrokerTest,getResourceState_NormalErrorHandlingIfAbnormalResource)
{

    MockingFunc();

    PrimitiveResource::Ptr resource[3];
    BrokerID id[3];

    for(int i=0;i!=3;i++)
    {
        resource[i] = PrimitiveResource::Ptr(mocks.Mock< PrimitiveResource >(), [](PrimitiveResource*){});
        mocks.OnCall(resource[i].get(), PrimitiveResource::requestGet);
        mocks.OnCall(resource[i].get(), PrimitiveResource::getHost).Return(std::string());
        mocks.OnCallFuncOverload(static_cast< subscribePresenceSig1 >(OC::OCPlatform::subscribePresence)).Return(OC_STACK_OK);
        id[i] = brokerInstance->hostResource(resource[i],cb);
    }


    EXPECT_EQ(brokerInstance->getResourceState(pResource),BROKER_STATE::NONE);

    for(int i=0;i!=3;i++)
    {
        brokerInstance->cancelHostResource(id[i]);
    }

}

TEST_F(ResourceBrokerTest,getResourceState_NormalErrorHandlingIfAbnormalId)
{

    id = -1;
    ASSERT_THROW(brokerInstance->getResourceState(id),ResourceBroker::InvalidParameterException);

}
