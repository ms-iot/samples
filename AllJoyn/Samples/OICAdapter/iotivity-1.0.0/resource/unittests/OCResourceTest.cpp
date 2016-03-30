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
#include <gtest/gtest.h>
#include <string>
#include <map>

namespace OCResourceTest
{
    using namespace OC;
    // Callbacks
    void onObserve(const HeaderOptions, const OCRepresentation&, const int&, const int&)
    {
    }

    void onGetPut(const HeaderOptions&, const OCRepresentation& , const int eCode)
    {
        EXPECT_EQ(eCode, OC_STACK_OK);
    }

    void foundResource(std::shared_ptr<OCResource> )
    {
    }

    void deleteResponse(const HeaderOptions&, const int)
    {
    }

    //Helper method
    OCResource::Ptr ConstructResourceObject(std::string host, std::string uri)
    {
        OCConnectivityType connectivityType = CT_DEFAULT;
        std::vector<std::string> types = {"intel.rpost"};
        std::vector<std::string> ifaces = {DEFAULT_INTERFACE};

        auto ret = OCPlatform::constructResourceObject(host, uri,
                connectivityType, false, types, ifaces);

        if(!ret)
        {
            ADD_FAILURE() << "ConstructResourceObject result was null";
            throw std::runtime_error("ConstructResourceObject result was null");
        }

        return ret;
    }

     //Get Test
    TEST(ConstructResourceTest, ConstructResourceObject)
    {
        EXPECT_ANY_THROW(ConstructResourceObject(std::string(""), std::string("")));
    }

    TEST(ConstructResourceTest, ConstructResourceObjectWithoutCoapScheme)
    {
        EXPECT_ANY_THROW(ConstructResourceObject("//192.168.1.2:5000", "/resource"));
    }

    TEST(ConstructResourceTest, ConstructResourceObjectWithoutPortNumber)
    {
        EXPECT_ANY_THROW(ConstructResourceObject("coap://192.168.1.2", "/resource"));
    }

    TEST(ConstructResourceTest, ConstructResourceObjectInvalidHost)
    {
        EXPECT_ANY_THROW(ConstructResourceObject("192.168.1.2:5000", "/resource"));
    }

    TEST(ConstructResourceTest, ConstructResourceObjectInvalidHost2)
    {
        EXPECT_ANY_THROW(ConstructResourceObject("coap://:5000", "/resource"));
    }

    TEST(ConstructResourceTest, ConstructResourceObjectInvalidUri)
    {
        EXPECT_ANY_THROW(ConstructResourceObject("coap://192.168.1.2:5000", "/"));
    }

    TEST(ConstructResourceTest, ConstructResourceObjectInvalidUri2)
    {
        EXPECT_ANY_THROW(ConstructResourceObject("coap://192.168.1.2:5000", "resource"));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetForValidUri)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->get(OC::QueryParamsMap(), &onGetPut));

    }

    TEST(ResourceGetTest, DISABLED_ResourceGetForBadUri)
    {
        OCResource::Ptr resource = ConstructResourceObject("", "coap://192.168.1.2:5000");
        EXPECT_TRUE(resource != NULL);
        EXPECT_THROW(resource->get(OC::QueryParamsMap(), &onGetPut), OC::OCException);
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithHighQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->get(OC::QueryParamsMap(), &onGetPut, QualityOfService::HighQos));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithLowQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->get(OC::QueryParamsMap(), &onGetPut, QualityOfService::LowQos));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithMidQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->get(OC::QueryParamsMap(), &onGetPut, QualityOfService::MidQos));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithNaQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->get(OC::QueryParamsMap(), &onGetPut, QualityOfService::NaQos));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithRTIFNaQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->get("", DEFAULT_INTERFACE, QueryParamsMap(), &onGetPut,
                        QualityOfService::NaQos));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithNullResourceType)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_ANY_THROW(
                resource->get(nullptr, DEFAULT_INTERFACE, QueryParamsMap(), &onGetPut,
                        QualityOfService::NaQos));
    }


    TEST(ResourceGetTest, DISABLED_ResourceGetWithNullResourceInterface)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_ANY_THROW(
                resource->get("", nullptr, QueryParamsMap(), &onGetPut, QualityOfService::NaQos));
    }

    TEST(ResourceGetTest, DISABLED_ResourceGetWithTypeAndInterface)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->get("", DEFAULT_INTERFACE, QueryParamsMap(), &onGetPut));
    }

    //Post Test
    TEST(ResourcePostTest, DISABLED_ResourcePostValidConfiguration)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->post(rep, query, &onGetPut));
    }

    TEST(ResourcePostTest, DISABLED_ResourcePostWithNaQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->post(rep, query, &onGetPut, QualityOfService::NaQos));
    }

    TEST(ResourcePostTest, DISABLED_ResourcePostWithMidQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->post(rep, query, &onGetPut, QualityOfService::MidQos));
    }

    TEST(ResourcePostTest, DISABLED_ResourcePostWithLowQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->post(rep, query, &onGetPut, QualityOfService::LowQos));
    }

    TEST(ResourcePostTest, DISABLED_ResourcePostWithHighQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->post(rep, query, &onGetPut, QualityOfService::HighQos));
    }

    //Put Test
    TEST(ResourcePutTest, DISABLED_ResourcePutForValid)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->put(rep, query, &onGetPut));
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithNaQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->put(rep, query, &onGetPut, QualityOfService::NaQos));
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithLowQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->put(rep, query, &onGetPut, QualityOfService::LowQos));
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithMidQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->put(rep, query, &onGetPut, QualityOfService::MidQos));
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithHighQos)
    {
        OCRepresentation rep;
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->put(rep, query, &onGetPut, QualityOfService::HighQos));
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithTypeAndInterface)
    {
        OCRepresentation rep;
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->put("", DEFAULT_INTERFACE, rep, QueryParamsMap(), &onGetPut));
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithNullType)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        OCRepresentation rep;
        EXPECT_ANY_THROW(
                resource->put(nullptr, DEFAULT_INTERFACE, rep, QueryParamsMap(), &onGetPut));
        HeaderOptions headerOptions;
        onGetPut(headerOptions, rep, OC_STACK_OK);
    }

    TEST(ResourcePutTest, DISABLED_ResourcePutWithNullInterface)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        OCRepresentation rep;
        EXPECT_ANY_THROW(resource->put("", nullptr, rep, QueryParamsMap(), &onGetPut));
    }

    // Observer Test
    TEST(ResourceObserveTest, DISABLED_ResourceObserveValidUri)
    {
        OCResource::Ptr resource =
                ConstructResourceObject("coap://192.168.1.2:5000", "/Observe");
        EXPECT_TRUE(resource != NULL);
        QueryParamsMap query = {};
        EXPECT_EQ(OC_STACK_OK, resource->observe(ObserveType::ObserveAll, query, &onObserve));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceObserveLoQos)
    {
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/Observe");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::LowQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceObserveNaQos)
    {
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/Observe");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::NaQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceObserveMidQos)
    {
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/Observe");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::MidQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceObserveHighQos)
    {
        QueryParamsMap query = {};
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/Observe");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::HighQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceCancelObserveValidUri)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        QueryParamsMap query = {};
        EXPECT_EQ(OC_STACK_OK, resource->observe(ObserveType::ObserveAll, query, &onObserve));
        EXPECT_EQ(OC_STACK_OK, resource->cancelObserve());
    }

    TEST(ResourceObserveTest, DISABLED_ResourceCancelObserveWithNaQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        QueryParamsMap query = {};
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::NaQos));
        EXPECT_EQ(OC_STACK_OK, resource->cancelObserve(QualityOfService::NaQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceCancelObserveWithLowQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        QueryParamsMap query = {};
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::LowQos));
        EXPECT_EQ(OC_STACK_OK, resource->cancelObserve(QualityOfService::LowQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceCancelObserveWithMidQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
            QueryParamsMap query = {};
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::MidQos));
        EXPECT_EQ(OC_STACK_OK, resource->cancelObserve(QualityOfService::MidQos));
    }

    TEST(ResourceObserveTest, DISABLED_ResourceCancelObserveWithHighQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
            QueryParamsMap query = {};
        EXPECT_EQ(OC_STACK_OK,
                resource->observe(ObserveType::ObserveAll, query, &onObserve,
                        QualityOfService::HighQos));
        EXPECT_EQ(OC_STACK_OK, resource->cancelObserve(QualityOfService::HighQos));
    }

    //DeleteResource
    TEST(DeleteResourceTest, DISABLED_DeleteResourceValidUri)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->deleteResource(&deleteResponse));
    }

    TEST(DeleteResourceTest, DISABLED_DeleteResourceWithLowQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->deleteResource(&deleteResponse, QualityOfService::LowQos));
    }

    TEST(DeleteResourceTest, DISABLED_DeleteResourceWithMidQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->deleteResource(&deleteResponse, QualityOfService::MidQos));
    }

    TEST(DeleteResourceTest, DISABLED_DeleteResourceWithHighQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK,
                resource->deleteResource(&deleteResponse, QualityOfService::HighQos));
    }
    TEST(DeleteResourceTest, DISABLED_DeleteResourceWithNaQos)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_EQ(OC_STACK_OK, resource->deleteResource(&deleteResponse, QualityOfService::NaQos));
    }

    //GetResourceInterfaces Test
    TEST(GetResourceInterfacesTest, GetResourceInterfaces)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->getResourceInterfaces().front() == DEFAULT_INTERFACE);
    }

    //GetResourceTypes Test
    TEST(GetResourceTypesTest, GetResourceTypes)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->getResourceTypes().front() == "intel.rpost");
    }

    // Host Test
    TEST(HostTest, Host)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->host() == "coap://192.168.1.2:5000");
    }

    //Uri Test
    TEST(UriTest, Uri)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->uri() == "/resource");
    }

    //ConnectivityType Test
    TEST(ConnectivityTypeTest, ConnectivityType)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->connectivityType() == CT_DEFAULT);
    }

    //IsObservable Test
    TEST(IsObservableTest, isObservable)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_FALSE(resource->isObservable());
    }

    //SID Test
    TEST(SidTest, sid)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_TRUE(resource->sid() == "");
    }

    //UniqueIdentifier Test
    TEST(UniqueIdentifierTest, uniqueIdentifier)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        std::ostringstream ss;
        ss << resource->uniqueIdentifier();
        EXPECT_TRUE(ss.str() == "/resource");
    }

    // HeaderOptions Test
    TEST(HeaderOptionsTest, SetHeaderOptionsValidInput)
    {
        const uint16_t API_VERSION = 2048;
        const uint16_t TOKEN = 3000;
        HeaderOptions headerOptions;
        HeaderOption::OCHeaderOption apiVersion(API_VERSION, "v.1.0");
        HeaderOption::OCHeaderOption clientToken(TOKEN, "21ae43gf");
        headerOptions.push_back(apiVersion);
        headerOptions.push_back(clientToken);

        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        EXPECT_NO_THROW(resource->setHeaderOptions(headerOptions));
        EXPECT_NO_THROW(resource->unsetHeaderOptions());
    }

    TEST(HeaderOptionsTest, SetHeaderOptionsEmptyInput)
    {
        OCResource::Ptr resource = ConstructResourceObject("coap://192.168.1.2:5000", "/resource");
        EXPECT_TRUE(resource != NULL);
        HeaderOptions headerOptions;
        EXPECT_NO_THROW(resource->setHeaderOptions(headerOptions));
        EXPECT_NO_THROW(resource->unsetHeaderOptions());
    }
}

