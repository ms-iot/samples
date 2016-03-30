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

#include <RCSResourceObject.h>

#include <OCPlatform.h>

using namespace std;
using namespace std::placeholders;

using namespace OIC::Service;
using namespace OC;

typedef OCStackResult (*registerResource)(OCResourceHandle&, string&, const string&, const string&,
                           EntityHandler, uint8_t );

typedef OCStackResult (*NotifyAllObservers)(OCResourceHandle);

constexpr char RESOURCE_URI[]{ "a/test" };
constexpr char RESOURCE_TYPE[]{ "resourcetype" };
constexpr char KEY[]{ "key" };
constexpr int value{ 100 };

TEST(ResourceObjectBuilderCreateTest, ThrowIfUriIsInvalid)
{
    ASSERT_THROW(RCSResourceObject::Builder("", "", "").build(), RCSPlatformException);
}

class ResourceObjectBuilderTest: public TestWithMock
{
protected:
    void SetUp()
    {
        TestWithMock::SetUp();

        mocks.OnCallFuncOverload(static_cast< registerResource >(OCPlatform::registerResource))
                .Return(OC_STACK_OK);
    }
};

TEST_F(ResourceObjectBuilderTest, RegisterResourceWhenCallCreate)
{
    mocks.ExpectCallFuncOverload(static_cast< registerResource >(OCPlatform::registerResource))
            .Return(OC_STACK_OK);

    RCSResourceObject::Builder(RESOURCE_URI, RESOURCE_TYPE, "").build();
}

TEST_F(ResourceObjectBuilderTest, ResourceServerHasPropertiesSetByBuilder)
{
    auto serverResource = RCSResourceObject::Builder(RESOURCE_URI, RESOURCE_TYPE, "").
            setDiscoverable(false).setObservable(true).build();

    EXPECT_FALSE(serverResource->isDiscoverable());
    EXPECT_TRUE(serverResource->isObservable());
}

TEST_F(ResourceObjectBuilderTest, ResourceServerHasAttrsSetByBuilder)
{
    RCSResourceAttributes attrs;
    attrs[KEY] = 100;

    auto serverResource = RCSResourceObject::Builder(RESOURCE_URI, RESOURCE_TYPE, "").
            setAttributes(attrs).build();

    RCSResourceObject::LockGuard lock{ serverResource, RCSResourceObject::AutoNotifyPolicy::NEVER };
    EXPECT_EQ(attrs, serverResource->getAttributes());
}


class ResourceObjectTest: public TestWithMock
{
public:
    RCSResourceObject::Ptr server;

protected:
    void SetUp()
    {
        TestWithMock::SetUp();

        initMocks();

        server = RCSResourceObject::Builder(RESOURCE_URI, RESOURCE_TYPE, "").build();

        initResourceObject();
    }

    virtual void initMocks()
    {
        mocks.OnCallFuncOverload(static_cast< registerResource >(OCPlatform::registerResource)).
                Return(OC_STACK_OK);

        mocks.OnCallFunc(OCPlatform::unregisterResource).Return(OC_STACK_OK);
    }

    virtual void initResourceObject() {
        server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::NEVER);
    }
};

TEST_F(ResourceObjectTest, AccessAttributesWithLock)
{
    {
        RCSResourceObject::LockGuard lock{ server };
        auto& attr = server->getAttributes();
        attr[KEY] = value;
    }

    ASSERT_EQ(value, server->getAttribute<int>(KEY));
}

TEST_F(ResourceObjectTest, ThrowIfTryToAccessAttributesWithoutGuard)
{
    ASSERT_THROW(server->getAttributes(), NoLockException);
}

TEST_F(ResourceObjectTest, SettingAttributesWithinGuardDoesntCauseDeadLock)
{
    {
        RCSResourceObject::LockGuard guard{ server };
        server->setAttribute(KEY, value);
    }

    ASSERT_EQ(value, server->getAttribute<int>(KEY));
}

TEST_F(ResourceObjectTest, SettingNestedAttributesIsSameToGettingNestedAttributes)
{
    RCSResourceAttributes lightAttributes;

    lightAttributes["red"]=50;
    lightAttributes["blue"]=100;
    lightAttributes["green"]=150;

    server->setAttribute(KEY, lightAttributes);

    ASSERT_EQ(lightAttributes, server->getAttribute<RCSResourceAttributes>(KEY));
}

TEST_F(ResourceObjectTest, SettingNestedVectorAttributesIsSameToGettingNestedVectorAttributes)
{
    vector<int> arr11 = {0,1}, arr12 = {4,5}, arr13 ={7,8};
    vector<vector<int>> arr21 = { arr11, arr12 }, arr22 = { arr12, arr13 };
    vector<vector<vector<int>>> arr31={ arr21, arr22 };

    server->setAttribute(KEY, arr31);

    ASSERT_EQ(arr31, server->getAttribute<vector<vector<vector<int>>>>(KEY));
}


class AutoNotifyTest: public ResourceObjectTest
{
protected:
    void initMocks()
    {
        mocks.OnCallFuncOverload(static_cast< NotifyAllObservers >(
                OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);
    }

    virtual void initResourceObject() {
        // intended blank
    }
};

TEST_F(AutoNotifyTest, DefalutAutoNotifyPolicyIsUpdated)
{
    ASSERT_EQ(RCSResourceObject::AutoNotifyPolicy::UPDATED, server->getAutoNotifyPolicy());
}

TEST_F(AutoNotifyTest, AutoNotifyPolicyCanBeSet)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::NEVER);

    ASSERT_EQ(RCSResourceObject::AutoNotifyPolicy::NEVER, server->getAutoNotifyPolicy());
}

TEST_F(AutoNotifyTest, WithUpdatedPolicy_NeverBeNotifiedIfAttributeIsNotChanged)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
    server->setAttribute(KEY, value);

    mocks.NeverCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers));

    server->setAttribute(KEY, value);
}

TEST_F(AutoNotifyTest, WithUpdatedPolicy_WillBeNotifiedIfAttributeIsChanged)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
    server->setAttribute(KEY, value);

    mocks.ExpectCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);

    server->setAttribute(KEY, value + 1);
}

TEST_F(AutoNotifyTest, WithUpdatedPolicy_WillBeNotifiedIfValueIsAdded)
{
    constexpr char newKey[]{ "newKey" };
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);

    mocks.ExpectCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);

    server->setAttribute(newKey, value);
}

TEST_F(AutoNotifyTest, WithNeverPolicy_NeverBeNotifiedEvenIfAttributeIsChanged)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::NEVER);

    mocks.NeverCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers));

    RCSResourceObject::LockGuard lock{ server };
    server->setAttribute(KEY, value);
}

TEST_F(AutoNotifyTest, WithUpdatePolicy_WillBeNotifiedIfAttributeIsDeleted)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);
    server->setAttribute(KEY, value);

    mocks.ExpectCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);

    server->removeAttribute(KEY);
}

class AutoNotifyWithGuardTest: public AutoNotifyTest
{
};

TEST_F(AutoNotifyWithGuardTest, GuardFollowsServerPolicyByDefault)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::UPDATED);

    mocks.ExpectCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);

    RCSResourceObject::LockGuard guard{ server };
    server->setAttribute(KEY, value);
}

TEST_F(AutoNotifyWithGuardTest, GuardCanOverridePolicy)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::ALWAYS);

    mocks.NeverCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers));

    RCSResourceObject::LockGuard guard{ server, RCSResourceObject::AutoNotifyPolicy::NEVER };
    server->getAttributes()[KEY] = value;
}

TEST_F(AutoNotifyWithGuardTest, GuardInvokesNotifyWhenDestroyed)
{
    server->setAutoNotifyPolicy(RCSResourceObject::AutoNotifyPolicy::NEVER);

    mocks.ExpectCallFuncOverload(static_cast< NotifyAllObservers >(
            OC::OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);

    {
        RCSResourceObject::LockGuard guard{ server, RCSResourceObject::AutoNotifyPolicy::ALWAYS };
        server->setAttribute(KEY, value);
    }

    mocks.NeverCallFuncOverload(static_cast< NotifyAllObservers >(
               OC::OCPlatform::notifyAllObservers)).Return(OC_STACK_OK);

    server->setAttribute(KEY, value);
}



class ResourceObjectHandlingRequestTest: public ResourceObjectTest
{
public:
    EntityHandler handler;

    static constexpr OCRequestHandle fakeRequestHandle =
            reinterpret_cast<OCRequestHandle>(0x1234);
    static constexpr OCResourceHandle fakeResourceHandle =
            reinterpret_cast<OCResourceHandle>(0x4321);

public:
    OCResourceRequest::Ptr createRequest(OCMethod method = OC_REST_GET, OCRepresentation ocRep =
            OCRepresentation{})
    {
        auto request = make_shared<OCResourceRequest>();

        OCEntityHandlerRequest ocEntityHandlerRequest;
        memset(&ocEntityHandlerRequest, 0, sizeof(OCEntityHandlerRequest));
        OC::MessageContainer mc;

        mc.addRepresentation(ocRep);

        ocEntityHandlerRequest.requestHandle = fakeRequestHandle;
        ocEntityHandlerRequest.resource = fakeResourceHandle;
        ocEntityHandlerRequest.method = method;
        ocEntityHandlerRequest.payload = reinterpret_cast<OCPayload*>(mc.getPayload());

        formResourceRequest(OC_REQUEST_FLAG, &ocEntityHandlerRequest, request);

        return request;
    }

protected:
    OCStackResult registerResourceFake(OCResourceHandle&, string&, const string&,
            const string&, EntityHandler handler, uint8_t)
    {
        this->handler = handler;
        return OC_STACK_OK;
    }

    void initMocks()
    {
        mocks.OnCallFuncOverload(
            static_cast<registerResource>(OCPlatform::registerResource)).Do(
                    bind(&ResourceObjectHandlingRequestTest::registerResourceFake,
                            this, _1, _2, _3, _4, _5, _6));
        mocks.OnCallFunc(OCPlatform::unregisterResource).Return(OC_STACK_OK);
    }
};

TEST_F(ResourceObjectHandlingRequestTest, CallSendResponseWhenReceiveRequest)
{
    mocks.ExpectCallFunc(OCPlatform::sendResponse).Return(OC_STACK_OK);

    ASSERT_EQ(OC_EH_OK, handler(createRequest()));
}

TEST_F(ResourceObjectHandlingRequestTest, ReturnErrorCodeWhenSendResponseFailed)
{
    mocks.ExpectCallFunc(OCPlatform::sendResponse).Return(OC_STACK_ERROR);

    ASSERT_EQ(OC_EH_ERROR, handler(createRequest()));
}

TEST_F(ResourceObjectHandlingRequestTest, SendResponseWithSameHandlesPassedByRequest)
{
    mocks.ExpectCallFunc(OCPlatform::sendResponse).Match(
            [](const shared_ptr<OCResourceResponse> response)
            {
                return response->getRequestHandle() == fakeRequestHandle &&
                        response->getResourceHandle() == fakeResourceHandle;
            }
    ).Return(OC_STACK_OK);

    ASSERT_EQ(OC_EH_OK, handler(createRequest()));
}

TEST_F(ResourceObjectHandlingRequestTest, SendResponseWithRCSResponseResults)
{
    constexpr int errorCode{ 1999 };

    server->setGetRequestHandler(
            [](const RCSRequest&, RCSResourceAttributes&) -> RCSGetResponse
            {
                return RCSGetResponse::create(errorCode);
            }
    );

    mocks.ExpectCallFunc(OCPlatform::sendResponse).Match(
            [](const shared_ptr<OCResourceResponse> response)
            {
                return response->getErrorCode() == errorCode;
            }
    ).Return(OC_STACK_OK);

    ASSERT_EQ(OC_EH_OK, handler(createRequest()));
}

TEST_F(ResourceObjectHandlingRequestTest, SendSetResponseWithCustomAttrs)
{
    constexpr int errorCode{ 1999 };
    constexpr char value[]{ "value" };

    server->setSetRequestHandler(
            [](const RCSRequest&, RCSResourceAttributes&) -> RCSSetResponse
            {
                RCSResourceAttributes attrs;
                attrs[KEY] = value;
                return RCSSetResponse::create(attrs, errorCode);
            }
    );

    mocks.ExpectCallFunc(OCPlatform::sendResponse).Match(
            [](const shared_ptr<OCResourceResponse> response)
            {
                return value == response->getResourceRepresentation()[KEY].getValue<std::string>()
                        && response->getErrorCode() == errorCode;
            }
    ).Return(OC_STACK_OK);

    ASSERT_EQ(OC_EH_OK, handler(createRequest(OC_REST_PUT)));
}


class SetRequestHandlerPolicyTest: public ResourceObjectHandlingRequestTest
{
public:
    typedef OCStackResult (*SendResponse)(std::shared_ptr<OCResourceResponse>);

public:
    OCRepresentation createOCRepresentation()
    {
        OCRepresentation ocRep;

        vector<string> interface{"oic.if.baseline"};
        vector<string> type{"core.light"};

        ocRep.setUri(RESOURCE_URI);
        ocRep.setResourceInterfaces(interface);
        ocRep.setResourceTypes(type);

        return ocRep;
    }

    void initMocks()
    {
        ResourceObjectHandlingRequestTest::initMocks();
        mocks.OnCallFunc(OCPlatform::sendResponse).Return(OC_STACK_OK);
    }
};

TEST_F(SetRequestHandlerPolicyTest, DefalutSetRequestHandlerPolicyIsNever)
{
    ASSERT_EQ(RCSResourceObject::SetRequestHandlerPolicy::NEVER,
                server->getSetRequestHandlerPolicy());
}

TEST_F(SetRequestHandlerPolicyTest, SetRequestHandlerPolicyCanBeSet)
{
    server->setSetRequestHandlerPolicy(RCSResourceObject::SetRequestHandlerPolicy::ACCEPTANCE);

    ASSERT_EQ(RCSResourceObject::SetRequestHandlerPolicy::ACCEPTANCE,
                server->getSetRequestHandlerPolicy());
}

TEST_F(SetRequestHandlerPolicyTest, WithNeverPolicy_NotAddedIfReceivedNewKeyValuePair)
{
    OCRepresentation ocRep = createOCRepresentation();
    ocRep.setValue("NewKey", value);
    server->setSetRequestHandlerPolicy(RCSResourceObject::SetRequestHandlerPolicy::NEVER);

    handler(createRequest(OC_REST_PUT, ocRep));

    RCSResourceObject::LockGuard guard{ server };
    ASSERT_FALSE((server->getAttributes()).contains("NewKey"));
}

TEST_F(SetRequestHandlerPolicyTest, WithAcceptancePolicy_WillBeAddedIfReceivedNewKeyValuePair)
{
    OCRepresentation ocRep = createOCRepresentation();
    ocRep.setValue("NewKey", value);
    server->setSetRequestHandlerPolicy(RCSResourceObject::SetRequestHandlerPolicy::ACCEPTANCE);

    handler(createRequest(OC_REST_PUT, ocRep));

    RCSResourceObject::LockGuard guard{ server };
    ASSERT_TRUE((server->getAttributes()).contains("NewKey"));
}


class ResourceObjectSynchronizationTest: public ResourceObjectHandlingRequestTest
{
public:

    static void withLock(RCSResourceObject::Ptr serverResource, int count)
    {
        for (int i=0; i<count; ++i)
        {
            RCSResourceObject::LockGuard lock{ serverResource };

            auto& attrs = serverResource->getAttributes();

            attrs[KEY] = attrs[KEY].get<int>() + 1;
        }
    }

    static void withSetter(RCSResourceObject::Ptr serverResource, int count)
    {
        for (int i=0; i<count; ++i)
        {
            RCSResourceObject::LockGuard lock{ serverResource };

            serverResource->setAttribute(KEY, serverResource->getAttribute<int>(KEY) + 1);
        }
    }
};

TEST_F(ResourceObjectSynchronizationTest, MultipleAccessToServerResource)
{
    int expected { 0 };
    vector<thread> threads;

    server->setAttribute(KEY, 0);

    for (int i = 20; i >= 0; --i) {
        int count = 5000 + i * 100;
        threads.push_back(thread { withLock, server, count });
        expected += count;
    }

    for (int i = 20; i >= 0; --i) {
        int count = 5000 + i * 100;
        threads.push_back(thread { withSetter, server, count });
        expected +=count;
    }

    for (auto& t : threads)
    {
        t.join();
    }

    ASSERT_EQ(expected, server->getAttribute<int>(KEY));
}

TEST_F(ResourceObjectSynchronizationTest, MultipleAccessToServerResourceWithRequests)
{
    int expected { 0 };
    vector<thread> threads;

    mocks.OnCallFunc(OCPlatform::sendResponse).Return(OC_STACK_OK);

    server->setAttribute(KEY, 0);

    for (int i = 20; i >= 0; --i) {
        int count = 5000 + i * 100;
        threads.push_back(thread{ withLock, server, count });
        expected += count;
    }

    for (int i = 20; i >= 0; --i) {
        int count = 5000 + i * 100;
        threads.push_back(thread{ withSetter, server, count });
        expected +=count;
    }

    threads.push_back(thread{
        [this]()
        {
            for (int i=0; i<10000; ++i)
            {
                if (i % 5 == 0) handler(createRequest(OC_REST_OBSERVE));
                handler(createRequest((i & 1) ? OC_REST_GET : OC_REST_PUT));
            }
        }
    });

    for (auto& t : threads)
    {
        t.join();
    }

    ASSERT_EQ(expected, server->getAttribute<int>(KEY));
}


class AttributeUpdatedListenerTest: public ResourceObjectHandlingRequestTest
{
public:
    typedef OCStackResult (*SendResponse)(std::shared_ptr<OCResourceResponse>);

public:
    OCRepresentation createOCRepresentation(void)
    {
        OCRepresentation ocRep;

        vector<string> interface{"oic.if.baseline"};
        vector<string> type{"core.light"};

        ocRep.setUri(RESOURCE_URI);
        ocRep.setResourceInterfaces(interface);
        ocRep.setResourceTypes(type);
        ocRep[KEY] = value;

        return ocRep;
    }

    void initMocks()
    {
        ResourceObjectHandlingRequestTest::initMocks();
        mocks.OnCallFunc(OCPlatform::sendResponse).Return(OC_STACK_OK);
    }
};

class FunctionsForAttributeUpdatedListener
{
public:
    virtual void fCalled(const OIC::Service::RCSResourceAttributes::Value&,
        const OIC::Service::RCSResourceAttributes::Value&)=0;
    virtual void fNotCalled(const OIC::Service::RCSResourceAttributes::Value&,
        const OIC::Service::RCSResourceAttributes::Value&)=0;
};

TEST_F(AttributeUpdatedListenerTest, AddListenerRunsAddedFunction)
{
    FunctionsForAttributeUpdatedListener *ptrMock =
        mocks.Mock<FunctionsForAttributeUpdatedListener>();

    server->setAttribute(KEY, 0);

    mocks.ExpectCall(ptrMock, FunctionsForAttributeUpdatedListener::fCalled);

    server->addAttributeUpdatedListener(KEY,
        (std::bind(&FunctionsForAttributeUpdatedListener::fCalled, ptrMock, _1, _2)));

    handler(createRequest(OC_REST_PUT, createOCRepresentation()));
}

TEST_F(AttributeUpdatedListenerTest, AddListenerRunsAccordingToLastAddedFunction)
{
    FunctionsForAttributeUpdatedListener *ptrMock =
        mocks.Mock<FunctionsForAttributeUpdatedListener>();

    string duplicateKEY(KEY);
    server->setAttribute(KEY, 0);

    mocks.ExpectCall(ptrMock, FunctionsForAttributeUpdatedListener::fCalled);
    mocks.NeverCall(ptrMock, FunctionsForAttributeUpdatedListener::fNotCalled);

    server->addAttributeUpdatedListener(duplicateKEY,
        (std::bind(&FunctionsForAttributeUpdatedListener::fNotCalled, ptrMock, _1, _2)));
    server->addAttributeUpdatedListener(KEY,
        (std::bind(&FunctionsForAttributeUpdatedListener::fCalled, ptrMock, _1, _2)));

    handler(createRequest(OC_REST_PUT, createOCRepresentation()));
}

TEST_F(AttributeUpdatedListenerTest, RemoveListenerReturnsTrueIfListenerIsNotAdded)
{
    ASSERT_FALSE(server->removeAttributeUpdatedListener(KEY));
}

TEST_F(AttributeUpdatedListenerTest, RemoveListenerReturnsTrueIfListenerIsAdded)
{
    FunctionsForAttributeUpdatedListener *ptrMock =
        mocks.Mock<FunctionsForAttributeUpdatedListener>();

    server->addAttributeUpdatedListener(KEY,
        (std::bind(&FunctionsForAttributeUpdatedListener::fNotCalled, ptrMock, _1, _2)));

    ASSERT_TRUE(server->removeAttributeUpdatedListener(KEY));
}

TEST_F(AttributeUpdatedListenerTest, RemoveListenerNeverRunsRemovedFunc)
{
    FunctionsForAttributeUpdatedListener *ptrMock =
        mocks.Mock<FunctionsForAttributeUpdatedListener>();

    mocks.NeverCall(ptrMock, FunctionsForAttributeUpdatedListener::fNotCalled);

    server->setAttribute(KEY, 0);
    server->addAttributeUpdatedListener(KEY,
        (std::bind(&FunctionsForAttributeUpdatedListener::fNotCalled, ptrMock, _1, _2)));
    server->removeAttributeUpdatedListener(KEY);

    handler(createRequest(OC_REST_PUT, createOCRepresentation()));
}



