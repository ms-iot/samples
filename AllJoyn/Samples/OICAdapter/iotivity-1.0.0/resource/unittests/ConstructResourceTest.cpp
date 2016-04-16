//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

#include <OCPlatform.h>
#include <OCApi.h>

//Uncomment the below line for testing with Mocks
//TODO enable these tests (unconditionally) after CA code clean up
//#define WITH_MOCKS
#ifdef WITH_MOCKS

#include "hippomocks.h"
#include "Framework.h"

#define GTEST_DONT_DEFINE_TEST 1

#include <gtest/gtest.h>

namespace ConstructResourceTest
{
    using namespace OC;
    std::vector<std::string> ifaces = {DEFAULT_INTERFACE};
    //using mocks framework
    GTEST_TEST(ConstructResourceObjectTest, ConstructResourceObjectValidReturnValue)
    {
        MockRepository mocks;
        OCResource::Ptr rightdoor = std::shared_ptr<OCResource>();
        OCConnectivityType connectivityType = OC_WIFI;
        mocks.ExpectCallFunc(OCPlatform::constructResourceObject).Return(rightdoor);
        std::vector<std::string> types = {"core.leftdoor"};
        OCResource::Ptr leftdoor = OCPlatform::constructResourceObject("192.168.1.2:5000",
                "a/leftdoor", connectivityType, false, types, ifaces);
        EXPECT_EQ(leftdoor, rightdoor);

    }

    GTEST_TEST(ConstructResourceObjectTest, ConstructResourceObjectInValidReturnValue)
    {
        MockRepository mocks;
        OCResource::Ptr rightdoor = std::shared_ptr<OCResource>();
        OCConnectivityType connectivityType = OC_WIFI;
        mocks.ExpectCallFunc(OCPlatform::constructResourceObject).Return(NULL);
        std::vector<std::string> types = {"core.rightdoor"};
        OCResource::Ptr leftdoor = OCPlatform::constructResourceObject("192.168.1.2:5000",
                "a/rightdoor", connectivityType, false, types, ifaces);
        bool value = (leftdoor == NULL);
        EXPECT_EQ(true, value);
    }
}
#endif

