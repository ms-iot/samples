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

#include <iostream>

#include "gtest/gtest.h"
#include "HippoMocks/hippomocks.h"

#include "OCPlatform.h"

#include "DevicePresence.h"
#include "DeviceAssociation.h"
#include "ResourcePresence.h"
#include "PrimitiveResource.h"
#include "ResponseStatement.h"
#include "UnitTestHelper.h"

using namespace testing;
using namespace OIC::Service;
using namespace OC;

#define STRING_VALUE "10.242.34.235"

typedef OCStackResult (*subscribePresenceSig1)(OC::OCPlatform::OCPresenceHandle&,
        const std::string&, OCConnectivityType, SubscribeCallback);

class DeviceAssociationTest : public TestWithMock
{
public:

    DeviceAssociation * instance;
    DevicePresencePtr device;
    PrimitiveResource::Ptr pResource;
protected:

    void setMockingFunc()
    {
        mocks.OnCall(pResource.get(), PrimitiveResource::requestGet);
        mocks.OnCall(pResource.get(), PrimitiveResource::getHost).Return(STRING_VALUE);
        mocks.OnCallFuncOverload(static_cast< subscribePresenceSig1 >(OC::OCPlatform::subscribePresence)).Return(OC_STACK_OK);
    }

    void SetAssociationDevice()
    {
        setMockingFunc();
        device->initializeDevicePresence(pResource);
        instance->addDevice(device);
    }

    void SetUp()
    {
        TestWithMock::SetUp();
        instance = DeviceAssociation::getInstance();
        device = (DevicePresencePtr)new DevicePresence();
        pResource = PrimitiveResource::Ptr(mocks.Mock< PrimitiveResource >(), [](PrimitiveResource*){});
    }

    void TearDown()
    {
        TestWithMock::TearDown();
        device.reset();
        pResource.reset();

    }
};

TEST_F(DeviceAssociationTest,findDevice_ReturnNormalValueIfNormalParam)
{

    SetAssociationDevice();
    ASSERT_NE(nullptr,instance->findDevice(pResource->getHost()));


}

TEST_F(DeviceAssociationTest,addDevice_NormalHandlingIfNormalParam)
{

    SetAssociationDevice();
    ASSERT_FALSE(instance->isEmptyDeviceList());
}

TEST_F(DeviceAssociationTest,removeDevice_NormalHandlingIfNormalParam)
{

    SetAssociationDevice();
    instance->removeDevice(device);
    ASSERT_TRUE(instance->isEmptyDeviceList());
}

