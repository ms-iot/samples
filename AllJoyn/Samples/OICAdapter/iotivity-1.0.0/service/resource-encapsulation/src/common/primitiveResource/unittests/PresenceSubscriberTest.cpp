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

#include <UnitTestHelper.h>

#include <PresenceSubscriber.h>
#include <RCSException.h>

#include <OCPlatform.h>

using namespace OIC::Service;

typedef OCStackResult (*SubscribePresence1)(OC::OCPlatform::OCPresenceHandle&,
        const std::string&, OCConnectivityType, SubscribeCallback);
typedef OCStackResult (*SubscribePresence2)(OC::OCPlatform::OCPresenceHandle&,
        const std::string&, const std::string&, OCConnectivityType, SubscribeCallback);

const std::string HOST{ "host" };
const OCConnectivityType CONTYPE{ };


class PresenceSubscriberNonMemberTest: public TestWithMock
{
public:
    OCDoHandle handle;
};

TEST_F(PresenceSubscriberNonMemberTest, OCPlatformSubscribePresenceWillBeCalled)
{
    mocks.ExpectCallFuncOverload(
            static_cast< SubscribePresence1 >(OC::OCPlatform::subscribePresence))
                        .With(_, HOST,CONTYPE, _).Return(OC_STACK_OK);

    subscribePresence(handle, HOST, CONTYPE, SubscribeCallback());
}

TEST_F(PresenceSubscriberNonMemberTest, SubscribePresenceThrowsIfResultIsNotOK)
{
    mocks.ExpectCallFuncOverload(
            static_cast< SubscribePresence1>(OC::OCPlatform::subscribePresence))
                    .Return(OC_STACK_ERROR);

    ASSERT_THROW(subscribePresence(handle, "", CONTYPE, SubscribeCallback()), RCSPlatformException);
}

TEST_F(PresenceSubscriberNonMemberTest, OCPlatformUnsubscribePresenceWillBeCalled)
{
    mocks.ExpectCallFuncOverload(OC::OCPlatform::unsubscribePresence).Return(OC_STACK_OK);

    unsubscribePresence(handle);
}

TEST_F(PresenceSubscriberNonMemberTest, UnsubscribePresenceThrowIfResultIsNotOK)
{
    mocks.ExpectCallFuncOverload(OC::OCPlatform::unsubscribePresence).Return(OC_STACK_ERROR);

    ASSERT_THROW(unsubscribePresence(handle), RCSPlatformException);
}

class PresenceSubscriberTest: public TestWithMock
{
protected:
    void SetUp() {
        mocks.OnCallFuncOverload(
                static_cast< SubscribePresence1 >(OC::OCPlatform::subscribePresence)).Do(

            [](OC::OCPlatform::OCPresenceHandle& handle, const std::string&,
                    OCConnectivityType, SubscribeCallback) -> OCStackResult
            {
                handle = reinterpret_cast<OC::OCPlatform::OCPresenceHandle>(1);
                return OC_STACK_OK;
            }
        );

        mocks.OnCallFunc(OC::OCPlatform::unsubscribePresence).Return(OC_STACK_OK);
    }

};

TEST_F(PresenceSubscriberTest, IsNotSubscribingWhenCreatedWithDefaultConstructor)
{
    PresenceSubscriber subscriber;
    ASSERT_FALSE(subscriber.isSubscribing());
}

TEST_F(PresenceSubscriberTest, ConstructorCallOCPlatformSubscribe)
{
    mocks.ExpectCallFuncOverload(
            static_cast< SubscribePresence1 >(OC::OCPlatform::subscribePresence))
                     .With(_, HOST, CONTYPE, _).Return(OC_STACK_OK);

    PresenceSubscriber subscriber{ HOST, CONTYPE, SubscribeCallback() };
}

TEST_F(PresenceSubscriberTest, ConstructorWithResourceTypeCallOCPlatformSubscribe)
{
    const std::string resType { "resType" };

    mocks.ExpectCallFuncOverload(
            static_cast< SubscribePresence2 >(OC::OCPlatform::subscribePresence))
                     .With(_, HOST, resType, CONTYPE, _).Return(OC_STACK_OK);

    PresenceSubscriber subscriber{ HOST, resType, CONTYPE, SubscribeCallback() };
}

TEST_F(PresenceSubscriberTest, ConstructorThrowsIfResultIsNotOK)
{
    mocks.ExpectCallFuncOverload(
            static_cast< SubscribePresence1 >(OC::OCPlatform::subscribePresence))
                    .Return(OC_STACK_ERROR);

    ASSERT_THROW(PresenceSubscriber(HOST, CONTYPE, SubscribeCallback()), RCSPlatformException);
}

TEST_F(PresenceSubscriberTest, IsSubscribingIfConstructedWithoutException)
{
    PresenceSubscriber subscriber{ HOST, CONTYPE, SubscribeCallback() };

    ASSERT_TRUE(subscriber.isSubscribing());
}

TEST_F(PresenceSubscriberTest, IsSubscribingOfMovedSubscriberReturnsFalse)
{
    PresenceSubscriber subscriber{ HOST, CONTYPE, SubscribeCallback() };

    PresenceSubscriber newSubscriber{ std::move(subscriber) };

    ASSERT_FALSE(subscriber.isSubscribing());
}

TEST_F(PresenceSubscriberTest, IsSubscribingOfMovedSubscriberWithAssignmentReturnsFalse)
{
    PresenceSubscriber subscriber{ HOST, CONTYPE, SubscribeCallback() };

    PresenceSubscriber newSubscriber;

    newSubscriber = std::move(subscriber);

    ASSERT_FALSE(subscriber.isSubscribing());
}

TEST_F(PresenceSubscriberTest, UnsubscribeWillBeCalledWhenSubscriberIsDestoryed)
{
    mocks.ExpectCallFunc(OC::OCPlatform::unsubscribePresence).Return(OC_STACK_OK);

    PresenceSubscriber subscriber{ HOST, CONTYPE, SubscribeCallback() };
}
