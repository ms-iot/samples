// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "gtest/gtest.h"
#include "ocstack.h"
#include "resourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "srmresourcestrings.h"
#include "doxmresource.h"
#include "ocserverrequest.h"
#include "oic_string.h"
#include "oic_malloc.h"
#include "logger.h"

#define TAG  "SRM-DOXM"

#ifdef __cplusplus
extern "C" {
#endif


//Declare Doxm resource methods for testing
OCStackResult CreateDoxmResource();
OCEntityHandlerResult DoxmEntityHandler (OCEntityHandlerFlag flag,
                OCEntityHandlerRequest * ehRequest);
char * BinToDoxmJSON(const OicSecDoxm_t * doxm);
OicSecDoxm_t * JSONToDoxmBin(const char * jsonStr);
void InitSecDoxmInstance(OicSecDoxm_t * doxm);
OCEntityHandlerResult HandleDoxmPostRequest (const OCEntityHandlerRequest * ehRequest);
void DeleteDoxmBinData(OicSecDoxm_t* doxm);
OCEntityHandlerResult HandleDoxmGetRequest (const OCEntityHandlerRequest * ehRequest);
#ifdef __cplusplus
}
#endif


OicSecDoxm_t * getBinDoxm()
{
    OicSecDoxm_t * doxm = (OicSecDoxm_t*)OICCalloc(1, sizeof(OicSecDoxm_t));
    if(!doxm)
    {
        return NULL;
    }
    doxm->oxmTypeLen =  1;
    doxm->oxmType    = (OicUrn_t *)OICCalloc(doxm->oxmTypeLen, sizeof(char *));
    if(!doxm->oxmType)
    {
        OICFree(doxm);
        return NULL;
    }
    doxm->oxmType[0] = (char*)OICMalloc(strlen(OXM_JUST_WORKS) + 1);
    if(!doxm->oxmType[0])
    {
        OICFree(doxm->oxmType);
        OICFree(doxm);
        return NULL;
    }

    strcpy(doxm->oxmType[0], OXM_JUST_WORKS);
    doxm->oxmLen     = 1;
    doxm->oxm        = (OicSecOxm_t *)OICCalloc(doxm->oxmLen, sizeof(OicSecOxm_t));
    if(!doxm->oxm)
    {
        OICFree(doxm->oxmType[0]);
        OICFree(doxm->oxmType);
        OICFree(doxm);
        return NULL;
    }

    doxm->oxm[0]     = OIC_JUST_WORKS;
    doxm->oxmSel     = OIC_JUST_WORKS;
    doxm->sct        = SYMMETRIC_PAIR_WISE_KEY;
    doxm->owned      = true;
    //TODO: Need more clarification on deviceIDFormat field type.
    //doxm.deviceIDFormat = URN;
    strcpy((char *) doxm->deviceID.id, "deviceId");
    strcpy((char *)doxm->owner.id, "ownersId");
    return doxm;
}

 //InitDoxmResource Tests
TEST(InitDoxmResourceTest, InitDoxmResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, InitDoxmResource());
}

//DeInitDoxmResource Tests
TEST(DeInitDoxmResourceTest, DeInitDoxmResource)
{
    EXPECT_EQ(OC_STACK_ERROR, DeInitDoxmResource());
}

//CreateDoxmResource Tests
TEST(CreateDoxmResourceTest, CreateDoxmResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, CreateDoxmResource());
}

 //DoxmEntityHandler Tests
TEST(DoxmEntityHandlerTest, DoxmEntityHandlerWithDummyRequest)
{
    OCEntityHandlerRequest req;
    EXPECT_EQ(OC_EH_ERROR, DoxmEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, &req));
}

TEST(DoxmEntityHandlerTest, DoxmEntityHandlerWithNULLRequest)
{
    EXPECT_EQ(OC_EH_ERROR, DoxmEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, NULL));
}

TEST(DoxmEntityHandlerTest, DoxmEntityHandlerInvalidFlag)
{
    OCEntityHandlerRequest req;
    EXPECT_EQ(OC_EH_ERROR, DoxmEntityHandler(OCEntityHandlerFlag::OC_OBSERVE_FLAG, &req));
}

TEST(DoxmEntityHandlerTest, DoxmEntityHandlerValidRequest)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, InitDoxmResource());
    char query[] = "oxm=0;owned=false;owner=owner1";
    OCEntityHandlerRequest req = OCEntityHandlerRequest();
    req.method = OC_REST_GET;
    req.query = OICStrdup(query);
    EXPECT_EQ(OC_EH_ERROR, DoxmEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, &req));

    OICFree(req.query);
}

TEST(DoxmEntityHandlerTest, DoxmEntityHandlerDeviceIdQuery)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, InitDoxmResource());
    char query[] = "deviceid=MjIyMjIyMjIyMjIyMjIyMg==";
    OCEntityHandlerRequest req = OCEntityHandlerRequest();
    req.method = OC_REST_GET;
    req.query = OICStrdup(query);
    EXPECT_EQ(OC_EH_ERROR, DoxmEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, &req));

    OICFree(req.query);
}

//BinToDoxmJSON Tests
TEST(BinToDoxmJSONTest, BinToDoxmJSONNullDoxm)
{
    char* value = BinToDoxmJSON(NULL);
    EXPECT_TRUE(value == NULL);
}

TEST(BinToDoxmJSONTest, BinToDoxmJSONValidDoxm)
{
    OicSecDoxm_t * doxm =  getBinDoxm();

    char * json = BinToDoxmJSON(doxm);
    OC_LOG_V(INFO, TAG, "BinToDoxmJSON:%s", json);
    EXPECT_TRUE(json != NULL);

    DeleteDoxmBinData(doxm);
    OICFree(json);
}

//JSONToDoxmBin Tests
TEST(JSONToDoxmBinTest, JSONToDoxmBinValidJSON)
{
    OicSecDoxm_t * doxm1 =  getBinDoxm();
    char * json = BinToDoxmJSON(doxm1);
    EXPECT_TRUE(json != NULL);

    OicSecDoxm_t *doxm2 = JSONToDoxmBin(json);
    EXPECT_TRUE(doxm2 != NULL);

    DeleteDoxmBinData(doxm1);
    DeleteDoxmBinData(doxm2);
    OICFree(json);
}

TEST(JSONToDoxmBinTest, JSONToDoxmBinNullJSON)
{
    OicSecDoxm_t *doxm = JSONToDoxmBin(NULL);
    EXPECT_TRUE(doxm == NULL);
}

#if 0
//HandleDoxmPostRequest Test
TEST(HandleDoxmPostRequestTest, HandleDoxmPostRequestValidInput)
{
    OCEntityHandlerRequest ehRequest = {};
    OCServerRequest svRequest = {};

    OicSecDoxm_t * doxm =  getBinDoxm();

    strcpy(svRequest.addressInfo.IP.ipAddress, "10.10.10.10");
    svRequest.addressInfo.IP.port = 2345;
    svRequest.connectivityType = CA_ETHERNET;

    ehRequest.reqJSONPayload = (unsigned char *) BinToDoxmJSON(doxm);
    ehRequest.requestHandle = (OCRequestHandle) &svRequest;

    EXPECT_EQ(OC_EH_ERROR, HandleDoxmPostRequest(&ehRequest));
    DeleteDoxmBinData(doxm);
    OICFree(ehRequest.reqJSONPayload);
}
#endif
