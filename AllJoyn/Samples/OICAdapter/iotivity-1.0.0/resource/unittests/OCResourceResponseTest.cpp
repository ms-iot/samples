//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
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
#include <gtest/gtest.h>
#include <OCResourceResponse.h>
#include <ocserverrequest.h>
#include <InProcServerWrapper.h>

namespace PH = std::placeholders;
namespace OCResourceResponseTest
{
    using namespace OC;
    using namespace std;

    TEST(ErrorCodeTest, SetGetErrorCodeValidCode)
    {
        OCResourceResponse response;
        int setCode = 200;
        EXPECT_NO_THROW(response.setErrorCode(setCode));
        EXPECT_EQ(setCode, response.getErrorCode());
    }

    TEST(NewResourceUriTest, SetGetNewResourceUriValidUri)
    {
        OCResourceResponse response;
        std::string uri = "/a/light";
        EXPECT_NO_THROW(response.setNewResourceUri(uri));
        EXPECT_EQ(uri, response.getNewResourceUri());
    }

    TEST(NewResourceUriTest, SetGetNewResourceUriEmpty)
    {
        OCResourceResponse response;
        std::string uri = "/a/light";
        EXPECT_NO_THROW(response.setNewResourceUri(uri));
        EXPECT_NE("", response.getNewResourceUri());
    }

    TEST(ResposeHeaderOptionsTest, SetGetHeaderOptionsValidOption)
    {
        OCResourceResponse response;
        const uint16_t API_VERSION = 2048;
        const std::string FRIDGE_CLIENT_API_VERSION = "v.1.0";
        HeaderOptions headerOptions;
        HeaderOption::OCHeaderOption apiVersion(API_VERSION, FRIDGE_CLIENT_API_VERSION);
        headerOptions.push_back(apiVersion);

        EXPECT_NO_THROW(response.setHeaderOptions(headerOptions));
        EXPECT_FALSE(headerOptions.empty());
        EXPECT_EQ(apiVersion.getOptionID(),
                response.getHeaderOptions()[0].getOptionID());
        EXPECT_EQ(apiVersion.getOptionData(),
                response.getHeaderOptions()[0].getOptionData());
     }

    TEST(ResposeHeaderOptionsTest, SetGetHeaderOptionsEmpty)
    {
        OCResourceResponse response;
        HeaderOptions headerOptions;

        EXPECT_NO_THROW(response.setHeaderOptions(headerOptions));
        EXPECT_TRUE(headerOptions.empty());
    }

    TEST(RequestHandleTest, SetGetRequestHandleValidHandle)
    {
        char query[] = "?rt=core.light";
        char address[] = "127.0.0.1";
        OCResourceResponse response;
        OCServerRequest request;
        request.method = OC_REST_GET;
        strncpy(request.query, query, sizeof(query));
        request.devAddr.flags = OC_DEFAULT_FLAGS;
        request.devAddr.adapter = OC_DEFAULT_ADAPTER;
        strncpy(request.devAddr.addr, address, sizeof(query));
        request.devAddr.port = 5364;
        request.qos = OC_LOW_QOS;
        request.coapID = 0;
        request.delayedResNeeded = 0;

        OCRequestHandle handle = static_cast<OCRequestHandle>(&request);
        EXPECT_EQ(NULL, response.getRequestHandle());
        EXPECT_NO_THROW(response.setRequestHandle(handle));
        EXPECT_NE(static_cast<OCRequestHandle>(NULL), response.getRequestHandle());
    }

    TEST(RequestHandleTest, SetGetRequestHandleNullHandle)
    {
        OCResourceResponse response;
        OCRequestHandle handle = nullptr;

        EXPECT_EQ(NULL, response.getRequestHandle());
        EXPECT_NO_THROW(response.setRequestHandle(handle));
        EXPECT_EQ(NULL, response.getRequestHandle());
    }

    TEST(ResourceHandleTest, SetGetResourceHandleValidHandle)
    {
        OCResourceResponse response;
        OCResourceHandle resHandle;

        std::string resourceURI = "/a/light2";
        std::string resourceTypeName = "core.light";
        std::string resourceInterface = DEFAULT_INTERFACE;
        uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

        EXPECT_EQ(OC_STACK_OK, OCCreateResource(&resHandle, resourceTypeName.c_str(),
                resourceInterface.c_str(), resourceURI.c_str(), nullptr, nullptr,
                resourceProperty));
        EXPECT_EQ(NULL, response.getResourceHandle());
        EXPECT_NO_THROW(response.setResourceHandle(resHandle));
        EXPECT_NE(static_cast<OCResourceHandle>(NULL), response.getResourceHandle());
    }

    TEST(ResourceHandleTest, SetGetResourceHandleNullHandle)
    {
        OCResourceResponse response;
        OCResourceHandle handle = nullptr;

        EXPECT_EQ(NULL, response.getResourceHandle());
        EXPECT_NO_THROW(response.setResourceHandle(handle));
        EXPECT_EQ(NULL, response.getResourceHandle());
    }

    TEST(ResponseResultTest, SetGetResponseResultValidInput)
    {
        OCResourceResponse response;
        OCEntityHandlerResult result = OC_EH_SLOW;
        EXPECT_NO_THROW(response.setResponseResult(result));
        EXPECT_EQ(result, response.getResponseResult());
    }

    TEST(ResourceRepresentation, SetGetResourceRepresentationWithValidRepresentation)
    {
        OCResourceResponse response;
        OCRepresentation lightRepresentation;
        const std::string LIGHT_RESOURCE_KEY = "light";
        lightRepresentation.setValue(LIGHT_RESOURCE_KEY, 0);
        EXPECT_TRUE(response.getResourceRepresentation().emptyData());
        EXPECT_NO_THROW(response.setResourceRepresentation(lightRepresentation));
        EXPECT_FALSE(response.getResourceRepresentation().emptyData());
    }

    TEST(ResourceRepresentation,
            SetGetResourceRepresentationWithRepresentationAndEmptyInterface)
    {
        OCResourceResponse response;
        OCRepresentation lightRepresentation;

        const std::string LIGHT_RESOURCE_KEY = "light";
        lightRepresentation.setValue(LIGHT_RESOURCE_KEY, 0);
        EXPECT_TRUE(response.getResourceRepresentation().emptyData());
        EXPECT_NO_THROW(response.setResourceRepresentation(lightRepresentation, ""));
        EXPECT_FALSE(response.getResourceRepresentation().emptyData());
    }

    TEST(ResourceRepresentation,
            SetGetResourceRepresentationWithRepresentationAndInterface)
    {
        OCResourceResponse response;
        OCRepresentation lightRepresentation;

        const std::string LIGHT_RESOURCE_KEY = "light";
        lightRepresentation.setValue(LIGHT_RESOURCE_KEY, 0);
        EXPECT_TRUE(response.getResourceRepresentation().emptyData());
        EXPECT_NO_THROW(response.setResourceRepresentation(lightRepresentation,
                LINK_INTERFACE));
        EXPECT_FALSE(response.getResourceRepresentation().emptyData());
    }
}
