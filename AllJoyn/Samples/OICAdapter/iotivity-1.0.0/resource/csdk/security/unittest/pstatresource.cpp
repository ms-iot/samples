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

#include "gtest/gtest.h"
#include "ocstack.h"
#include "resourcemanager.h"
#include "pstatresource.h"
#include "oic_malloc.h"
#include "cJSON.h"
#include "base64.h"
#include "cainterface.h"
#include "secureresourcemanager.h"
#include "srmtestcommon.h"
#include "ocpayload.h"
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif
//Declare Provision status resource methods for testing
OCStackResult CreatePstatResource();
OCEntityHandlerResult PstatEntityHandler (OCEntityHandlerFlag flag,
                                        OCEntityHandlerRequest * ehRequest);
char * BinToPstatJSON(const OicSecPstat_t * pstat);
OicSecPstat_t * JSONToPstatBin(const char * jsonStr);
const char* UNIT_TEST_JSON_FILE_NAME = "oic_unittest.json";
#ifdef __cplusplus
}
#endif

//InitPstatResource Tests
TEST(InitPstatResourceTest, InitPstatResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM,  InitPstatResource());
}


//DeInitPstatResource Tests
TEST(DeInitPstatResourceTest, DeInitPstatResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, DeInitPstatResource());
}

//CreatePstatResource Tests
TEST(CreatePstatResourceTest, CreatePstatResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM,  CreatePstatResource());
}

//PstatEntityHandler Tests
TEST(PstatEntityHandlerTest, PstatEntityHandlerWithDummyRequest)
{
    OCEntityHandlerRequest req;
    EXPECT_EQ(OC_EH_ERROR, PstatEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, &req));
}

TEST(PstatEntityHandlerTest, PstatEntityHandlerWithPostRequest)
{
    OCEntityHandlerRequest req;
    req.method = OC_REST_POST;
    req.payload = reinterpret_cast<OCPayload*>(
            OCSecurityPayloadCreate("{ \"pstat\": { \"tm\": 0, \"om\": 3 }}"));
    EXPECT_EQ(OC_EH_ERROR, PstatEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, &req));
    OCPayloadDestroy(req.payload);
}

TEST(PstatEntityHandlerTest, PstatEntityHandlerInvalidRequest)
{
    EXPECT_EQ(OC_EH_ERROR, PstatEntityHandler(OCEntityHandlerFlag::OC_OBSERVE_FLAG, NULL));
}

//BinToJSON Tests
TEST(BinToJSONTest, BinToNullJSON)
{
    char* value = BinToPstatJSON(NULL);
    EXPECT_TRUE(value == NULL);
}

TEST(JSONToBinTest, NullJSONToBin)
{
    OicSecPstat_t *pstat1 = JSONToPstatBin(NULL);
    EXPECT_TRUE(pstat1 == NULL);
}

TEST(MarshalingAndUnMarshalingTest, BinToPstatJSONAndJSONToPstatBin)
{
    const char* id = "ZGV2aWNlaWQAAAAAABhanw==";
    OicSecPstat_t pstat;
    pstat.cm = NORMAL;
    pstat.commitHash = 0;
    uint32_t outLen = 0;
    unsigned char base64Buff[sizeof(((OicUuid_t*) 0)->id)] = {};
    EXPECT_EQ(B64_OK, b64Decode(id, strlen(id), base64Buff, sizeof(base64Buff), &outLen));
    memcpy(pstat.deviceID.id, base64Buff, outLen);
    pstat.isOp = true;
    pstat.tm = NORMAL;
    pstat.om = SINGLE_SERVICE_CLIENT_DRIVEN;
    pstat.smLen = 2;
    pstat.sm = (OicSecDpom_t*)OICCalloc(pstat.smLen, sizeof(OicSecDpom_t));
    if(!pstat.sm)
    {
        FAIL() << "Failed to allocate the pstat.sm";
    }
    pstat.sm[0] = SINGLE_SERVICE_CLIENT_DRIVEN;
    pstat.sm[1] = SINGLE_SERVICE_SERVER_DRIVEN;
    char* jsonPstat = BinToPstatJSON(&pstat);
    if(!jsonPstat)
    {
        OICFree(pstat.sm);
        FAIL() << "Failed to convert BinToPstatJSON";
        return;
    }
    printf("BinToJSON Dump:\n%s\n\n", jsonPstat);
    EXPECT_TRUE(jsonPstat != NULL);
    OicSecPstat_t *pstat1 = JSONToPstatBin(jsonPstat);
    EXPECT_TRUE(pstat1 != NULL);
    if(pstat1)
    {
        OICFree(pstat1->sm);
    }
    OICFree(pstat1);
    OICFree(jsonPstat);
    OICFree(pstat.sm);
}

TEST(PstatTests, JSONMarshalliingTests)
{
    char *jsonStr1 = ReadFile(UNIT_TEST_JSON_FILE_NAME);
    if (NULL != jsonStr1)
    {
        cJSON_Minify(jsonStr1);
        /* Workaround : cJSON_Minify does not remove all the unwanted characters
         from the end. Here is an attempt to remove those characters */
        int len = strlen(jsonStr1);
        while (len > 0)
        {
            if (jsonStr1[--len] == '}')
            {
                break;
            }
        }
        jsonStr1[len + 1] = 0;

        OicSecPstat_t* pstat = JSONToPstatBin(jsonStr1);
        EXPECT_TRUE(NULL != pstat);

        char* jsonStr2 = BinToPstatJSON(pstat);
        EXPECT_STRNE(jsonStr1, jsonStr2);

        OICFree(jsonStr1);
        OICFree(jsonStr2);
        OICFree(pstat);
   }
    else
    {
        printf("Please copy %s into unittest folder\n", UNIT_TEST_JSON_FILE_NAME);
    }
}
