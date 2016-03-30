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

#include <RCSResponse.h>
#include <RCSResourceObject.h>

#include <RequestHandler.h>
#include <ResourceAttributesConverter.h>

#include <OCPlatform.h>

using namespace std;

using namespace OIC::Service;
using namespace OC;

typedef OCStackResult (*registerResourceSig)(OCResourceHandle&,
                       string&,
                       const string&,
                       const string&,
                       EntityHandler,
                       uint8_t );

static constexpr char KEY[] = "key";


void EXPECT_RESPONSE(shared_ptr< OCResourceResponse > ocResponse, int errorCode,
        const RCSResourceAttributes& attrs)
{
    EXPECT_EQ(ocResponse->getErrorCode(), errorCode);
    EXPECT_EQ(ResourceAttributesConverter::fromOCRepresentation(
                    ocResponse->getResourceRepresentation()), attrs);
}


class RCSResponseTest: public TestWithMock
{
public:
    template< typename T >
    shared_ptr< OCResourceResponse > buildResponse(const T& response)
    {
        RCSResourceObject::Ptr server =
                RCSResourceObject::Builder("a/test", "", "").build();

        return response.getHandler()->buildResponse(*server);
    }

protected:
    void SetUp()
    {
        TestWithMock::SetUp();

        mocks.OnCallFuncOverload(static_cast< registerResourceSig >(OCPlatform::registerResource))
                .Return(OC_STACK_OK);

        mocks.OnCallFunc(OCPlatform::unregisterResource).Return(OC_STACK_OK);
    }
};

TEST_F(RCSResponseTest, GetDefaultActionHasEmptyAttrs)
{
    EXPECT_RESPONSE(buildResponse(RCSGetResponse::defaultAction()),
            RequestHandler::DEFAULT_ERROR_CODE, RCSResourceAttributes());
}

TEST_F(RCSResponseTest, GetResponseHasResultsPassedCodes)
{
    constexpr int errorCode{ -10 };

    EXPECT_RESPONSE(buildResponse(RCSGetResponse::create(errorCode)),
            errorCode, RCSResourceAttributes());
}

TEST_F(RCSResponseTest, GetResponseHasAttrsAndResultsPassedCodes)
{
    constexpr int errorCode{ -10 };

    RCSResourceAttributes attrs;
    attrs[KEY] = 100;

    EXPECT_RESPONSE(buildResponse(RCSGetResponse::create(attrs, errorCode)), errorCode, attrs);
}

TEST_F(RCSResponseTest, GetResponseCanMoveAttrs)
{
    constexpr int errorCode{ -10 };

    RCSResourceAttributes attrs;
    attrs[KEY] = 100;

    RCSResourceAttributes attrsClone;
    attrsClone[KEY] = 100;

    EXPECT_RESPONSE(
            buildResponse(RCSGetResponse::create(std::move(attrs), errorCode)),
            errorCode, attrsClone);

    EXPECT_TRUE(attrs.empty());
}

TEST_F(RCSResponseTest, SetDefaultActionHasEmptyAttrs)
{
    EXPECT_RESPONSE(buildResponse(RCSSetResponse::defaultAction()),
            RequestHandler::DEFAULT_ERROR_CODE, RCSResourceAttributes());
}

TEST_F(RCSResponseTest, SetResponseHasResultsPassedCodes)
{
    constexpr int errorCode{ -10 };

    EXPECT_RESPONSE(buildResponse(RCSSetResponse::create(errorCode)),
            errorCode, RCSResourceAttributes());
}

TEST_F(RCSResponseTest, SetResponseHasAttrsAndResultsPassedCodes)
{
    constexpr int errorCode{ -10 };

    RCSResourceAttributes attrs;
    attrs[KEY] = 100;

    EXPECT_RESPONSE(buildResponse(RCSSetResponse::create(attrs, errorCode)),
            errorCode, attrs);
}

TEST_F(RCSResponseTest, SetResponseCanMoveAttrs)
{
    constexpr int errorCode{ -10 };

    RCSResourceAttributes attrs;
    attrs[KEY] = 100;

    RCSResourceAttributes attrsClone;
    attrsClone[KEY] = 100;

    EXPECT_RESPONSE(buildResponse(RCSSetResponse::create(std::move(attrs), errorCode)),
            errorCode, attrsClone);

    EXPECT_TRUE(attrs.empty());
}


TEST_F(RCSResponseTest, DefaultSetResponseHasDefaultMethod)
{
    EXPECT_EQ(RCSSetResponse::AcceptanceMethod::DEFAULT,
            RCSSetResponse::defaultAction().getAcceptanceMethod());
}

TEST_F(RCSResponseTest, AcceptSetResponseHasAcceptMethod)
{
    EXPECT_EQ(RCSSetResponse::AcceptanceMethod::ACCEPT,
            RCSSetResponse::accept().getAcceptanceMethod());
}

TEST_F(RCSResponseTest, IgnoreSetResponseHasIgnoreMethod)
{
    EXPECT_EQ(RCSSetResponse::AcceptanceMethod::IGNORE,
            RCSSetResponse::ignore().getAcceptanceMethod());
}

TEST_F(RCSResponseTest, SetResponseHasMethodSetBySetter)
{
    RCSSetResponse::AcceptanceMethod method = RCSSetResponse::AcceptanceMethod::ACCEPT;
    RCSSetResponse response =
            RCSSetResponse::defaultAction().setAcceptanceMethod(method);

    EXPECT_EQ(method, response.getAcceptanceMethod());
}
