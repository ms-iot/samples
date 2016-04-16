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

#include "ResourceEncapsulationTestSimulator.h"
#include "RequestObject.h"

using namespace testing;
using namespace OIC::Service;

namespace
{
    void setRequestCB(const RCSResourceAttributes &, RCSResourceAttributes &) { }
}

class RequestObjectTest : public TestWithMock
{
public:
    ResourceEncapsulationTestSimulator::Ptr testObject;
    RCSResourceObject::Ptr server;
    RCSRemoteResourceObject::Ptr remoteObject;

    RCSResourceAttributes attr;

    std::mutex mutexForCondition;
    std::condition_variable responseCon;

protected:

    void SetUp()
    {
        TestWithMock::SetUp();

        testObject = std::make_shared<ResourceEncapsulationTestSimulator>();
        testObject->defaultRunSimulator();
        remoteObject = testObject->getRemoteResource();
    }

    void TearDown()
    {
        TestWithMock::TearDown();
        if(remoteObject)
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

TEST_F(RequestObjectTest, invokeRequestExpectCallwithSetter)
{
   bool isCalled = false;
   RequestObject::Ptr instance = std::make_shared<RequestObject>(setRequestCB);

   mocks.ExpectCallFunc(setRequestCB).Do(
           [this, &isCalled](const RCSResourceAttributes &, RCSResourceAttributes &)
           {
               isCalled = true;
               notifyCondition();
           });

   RCSResourceAttributes att;
   instance->invokeRequest(remoteObject, RequestObject::RequestMethod::Setter, att);

   waitForCondition();

   ASSERT_TRUE(isCalled);
}
