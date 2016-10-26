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
#include <pwd.h>
#include <grp.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include "ocstack.h"
#include "ocpayload.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "cJSON.h"
#include "cainterface.h"
#include "secureresourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "srmresourcestrings.h"
#include "aclresource.h"
#include "srmtestcommon.h"
#include "srmutility.h"
#include "logger.h"

using namespace std;

#define TAG  "SRM-ACL-UT"

#ifdef __cplusplus
extern "C" {
#endif

extern char * BinToAclJSON(const OicSecAcl_t * acl);
extern OicSecAcl_t * JSONToAclBin(const char * jsonStr);
extern void DeleteACLList(OicSecAcl_t* acl);
OCStackResult  GetDefaultACL(OicSecAcl_t** defaultAcl);
OCEntityHandlerResult ACLEntityHandler (OCEntityHandlerFlag flag,
                                        OCEntityHandlerRequest * ehRequest);
#ifdef __cplusplus
}
#endif

const char* JSON_FILE_NAME = "oic_unittest.json";
const char* DEFAULT_ACL_JSON_FILE_NAME = "oic_unittest_default_acl.json";
const char* ACL1_JSON_FILE_NAME = "oic_unittest_acl1.json";

#define NUM_ACE_FOR_WILDCARD_IN_ACL1_JSON (2)

// JSON Marshalling Tests
TEST(ACLResourceTest, JSONMarshallingTests)
{
    char *jsonStr1 = ReadFile(ACL1_JSON_FILE_NAME);
    if (jsonStr1)
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

        OicSecAcl_t * acl = JSONToAclBin(jsonStr1);
        EXPECT_TRUE(NULL != acl);

        char * jsonStr2 = BinToAclJSON(acl);
        EXPECT_TRUE(NULL != jsonStr2);

        EXPECT_STREQ(jsonStr1, jsonStr2);

        OICFree(jsonStr1);
        OICFree(jsonStr2);
        DeleteACLList(acl);
    }
}

// Default ACL tests
TEST(ACLResourceTest, GetDefaultACLTests)
{
    // Read default ACL from the file
    char *jsonStr = ReadFile(DEFAULT_ACL_JSON_FILE_NAME);
    if (jsonStr)
    {
        OicSecAcl_t * acl = JSONToAclBin(jsonStr);
        EXPECT_TRUE(NULL != acl);

        // Invoke API to generate default ACL
        OicSecAcl_t * defaultAcl = NULL;
        OCStackResult ret = GetDefaultACL(&defaultAcl);
        EXPECT_TRUE(NULL == defaultAcl);

        EXPECT_TRUE(OC_STACK_ERROR == ret);

        // Verify if the SRM generated default ACL matches with unit test default
        if (acl && defaultAcl)
        {
            EXPECT_TRUE(memcmp(&(acl->subject), &(defaultAcl->subject), sizeof(OicUuid_t)) == 0);
            EXPECT_EQ(acl->resourcesLen, defaultAcl->resourcesLen);
            for (size_t i = 0; i < acl->resourcesLen; i++)
            {
                EXPECT_EQ(strlen(acl->resources[i]), strlen(defaultAcl->resources[i]));
                EXPECT_TRUE(
                        memcmp(acl->resources[i], defaultAcl->resources[i],
                                strlen(acl->resources[i])) == 0);
            }
            EXPECT_EQ(acl->permission, defaultAcl->permission);
        }

        // Perform cleanup
        DeleteACLList(acl);
        DeleteACLList(defaultAcl);
        OICFree(jsonStr);
    }
}


// 'POST' ACL tests
TEST(ACLResourceTest, ACLPostTest)
{
    OCEntityHandlerRequest ehReq =  OCEntityHandlerRequest();

    // Read an ACL from the file
    char *jsonStr = ReadFile(ACL1_JSON_FILE_NAME);
    if (jsonStr)
    {
        static OCPersistentStorage ps = OCPersistentStorage();

        SetPersistentHandler(&ps, true);

        // Create Entity Handler POST request payload
        ehReq.method = OC_REST_POST;
        ehReq.payload = (OCPayload*)OCSecurityPayloadCreate(jsonStr);

        OCEntityHandlerResult ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
        EXPECT_TRUE(OC_EH_ERROR == ehRet);

        // Convert JSON into OicSecAcl_t for verification
        OicSecAcl_t * acl = JSONToAclBin(jsonStr);
        EXPECT_TRUE(NULL != acl);

        // Verify if SRM contains ACL for the subject
        OicSecAcl_t* savePtr = NULL;
        const OicSecAcl_t* subjectAcl = GetACLResourceData(&(acl->subject), &savePtr);
        EXPECT_TRUE(NULL != subjectAcl);

        // Perform cleanup
        DeleteACLList(acl);
        DeInitACLResource();
        OCPayloadDestroy(ehReq.payload);
        OICFree(jsonStr);
    }
}


// GetACLResource tests
TEST(ACLResourceTest, GetACLResourceTests)
{
    // gAcl is a pointer to the the global ACL used by SRM
    extern OicSecAcl_t  *gAcl;

    // Read an ACL from the file
    char *jsonStr = ReadFile(ACL1_JSON_FILE_NAME);
    if (jsonStr)
    {
        gAcl = JSONToAclBin(jsonStr);
        EXPECT_TRUE(NULL != gAcl);

        // Verify that ACL file contains 2 ACE entries for 'WILDCARD' subject
        const OicSecAcl_t* acl = NULL;
        OicSecAcl_t* savePtr = NULL;
        OicUuid_t subject = WILDCARD_SUBJECT_ID;
        int count = 0;

        do
        {
            acl = GetACLResourceData(&subject, &savePtr);
            count = (NULL != acl) ? count + 1 : count;
        } while (acl != NULL);

        EXPECT_EQ(count, NUM_ACE_FOR_WILDCARD_IN_ACL1_JSON);

        /* Perform cleanup */
        DeleteACLList(gAcl);
        gAcl = NULL;
        OICFree(jsonStr);
    }
}

static OCStackResult  populateAcl(OicSecAcl_t *acl,  int numRsrc)
{
    OCStackResult ret = OC_STACK_ERROR;
    memcpy(acl->subject.id, "2222222222222222", sizeof(acl->subject.id));
    acl->resourcesLen = numRsrc;
    acl->resources = (char**)OICCalloc(acl->resourcesLen, sizeof(char*));
    VERIFY_NON_NULL(TAG, acl->resources, ERROR);
    acl->resources[0] = (char*)OICMalloc(strlen("/a/led")+1);
    VERIFY_NON_NULL(TAG, acl->resources[0], ERROR);
    OICStrcpy(acl->resources[0], sizeof(acl->resources[0]), "/a/led");
    if(numRsrc == 2)
    {
        acl->resources[1] = (char*)OICMalloc(strlen("/a/fan")+1);
        VERIFY_NON_NULL(TAG, acl->resources[1], ERROR);
        OICStrcpy(acl->resources[1], sizeof(acl->resources[1]), "/a/fan");
    }
    acl->permission = 6;
    acl->ownersLen = 1;
    acl->owners = (OicUuid_t*)OICCalloc(acl->ownersLen, sizeof(OicUuid_t));
    VERIFY_NON_NULL(TAG, acl->owners, ERROR);
    memcpy(acl->owners->id, "1111111111111111", sizeof(acl->owners->id));

    ret = OC_STACK_OK;
exit:
    return ret;

}

//'DELETE' ACL test
TEST(ACLResourceTest, ACLDeleteWithSingleResourceTest)
{
    OCEntityHandlerRequest ehReq = OCEntityHandlerRequest();
    static OCPersistentStorage ps = OCPersistentStorage();
    char *jsonStr = NULL;
    OicSecAcl_t acl = OicSecAcl_t();
    OicSecAcl_t* savePtr = NULL;
    const OicSecAcl_t* subjectAcl1 = NULL;
    const OicSecAcl_t* subjectAcl2 = NULL;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    char query[] = "sub=MjIyMjIyMjIyMjIyMjIyMg==;rsrc=/a/led";

    SetPersistentHandler(&ps, true);

    //Populate ACL
    VERIFY_SUCCESS(TAG, (OC_STACK_OK == populateAcl(&acl, 1)), ERROR);

    //GET json POST payload
    jsonStr = BinToAclJSON(&acl);
    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    // Create Entity Handler POST request payload
    ehReq.method = OC_REST_POST;
    ehReq.payload = (OCPayload*)OCSecurityPayloadCreate(jsonStr);
    ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_ERROR == ehRet);

    // Verify if SRM contains ACE for the subject
    savePtr = NULL;
    subjectAcl1 = GetACLResourceData(&acl.subject, &savePtr);
    EXPECT_TRUE(NULL != subjectAcl1);

    // Create Entity Handler DELETE request
    ehReq.method = OC_REST_DELETE;
    ehReq.query = (char*)OICMalloc(strlen(query)+1);
    VERIFY_NON_NULL(TAG, ehReq.query, ERROR);
    OICStrcpy(ehReq.query, strlen(query)+1, query);
    ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_ERROR == ehRet);

    // Verify if SRM has deleted ACE for the subject
    savePtr = NULL;
    subjectAcl2 = GetACLResourceData(&acl.subject, &savePtr);
    EXPECT_TRUE(NULL == subjectAcl2);

exit:
    // Perform cleanup
    if(NULL != subjectAcl1)
    {
        DeInitACLResource();
    }
    OCPayloadDestroy(ehReq.payload);
    OICFree(ehReq.query);
    OICFree(jsonStr);

}

TEST(ACLResourceTest, ACLDeleteWithMultiResourceTest)
{
    OCEntityHandlerRequest ehReq = OCEntityHandlerRequest();
    static OCPersistentStorage ps = OCPersistentStorage();
    OicSecAcl_t acl = OicSecAcl_t();
    char *jsonStr = NULL;
    OicSecAcl_t* savePtr = NULL;
    const OicSecAcl_t* subjectAcl1 = NULL;
    const OicSecAcl_t* subjectAcl2 = NULL;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    char query[] = "sub=MjIyMjIyMjIyMjIyMjIyMg==;rsrc=/a/led";

    SetPersistentHandler(&ps, true);

    //Populate ACL
    VERIFY_SUCCESS(TAG, (OC_STACK_OK == populateAcl(&acl, 2)), ERROR);

    //GET json POST payload
    jsonStr = BinToAclJSON(&acl);
    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    // Create Entity Handler POST request payload
    ehReq.method = OC_REST_POST;
    ehReq.payload = (OCPayload*)OCSecurityPayloadCreate(jsonStr);
    ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_ERROR == ehRet);

    // Verify if SRM contains ACE for the subject with two resources
    savePtr = NULL;
    subjectAcl1 = GetACLResourceData(&acl.subject, &savePtr);
    EXPECT_TRUE(NULL != subjectAcl1);
    EXPECT_TRUE(subjectAcl1->resourcesLen == 2);

    // Create Entity Handler DELETE request
    ehReq.method = OC_REST_DELETE;
    ehReq.query = (char*)OICMalloc(strlen(query)+1);
    VERIFY_NON_NULL(TAG, ehReq.query, ERROR);
    OICStrcpy(ehReq.query, strlen(query)+1, query);

    ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_ERROR == ehRet);

    // Verify if SRM contains ACL for the subject but only with one resource
    savePtr = NULL;
    subjectAcl2 = GetACLResourceData(&acl.subject, &savePtr);
    EXPECT_TRUE(NULL != subjectAcl2);
    EXPECT_TRUE(subjectAcl2->resourcesLen == 1);

exit:
    // Perform cleanup
    if(NULL != subjectAcl1)
    {
        DeInitACLResource();
    }
    OCPayloadDestroy(ehReq.payload);
    OICFree(ehReq.query);
    OICFree(jsonStr);
}

//'GET' with query ACL test

TEST(ACLResourceTest, ACLGetWithQueryTest)
{
    OCEntityHandlerRequest ehReq = OCEntityHandlerRequest();
    static OCPersistentStorage ps = OCPersistentStorage();
    OicSecAcl_t acl = OicSecAcl_t();
    char *jsonStr = NULL;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    char query[] = "sub=MjIyMjIyMjIyMjIyMjIyMg==;rsrc=/a/led";

    SetPersistentHandler(&ps, true);

    //Populate ACL
    VERIFY_SUCCESS(TAG, (OC_STACK_OK == populateAcl(&acl, 1)), ERROR);

    //GET json POST payload
    jsonStr = BinToAclJSON(&acl);
    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    //Create Entity Handler POST request payload
    ehReq.method = OC_REST_POST;
    ehReq.payload = (OCPayload*)OCSecurityPayloadCreate(jsonStr);
    ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_ERROR == ehRet);

    //Create Entity Handler GET request wit query
    ehReq.method =  OC_REST_GET;
    ehReq.query = (char*)OICMalloc(strlen(query)+1);
    VERIFY_NON_NULL(TAG, ehReq.query, ERROR);
    OICStrcpy(ehReq.query, strlen(query)+1, query);

    ehRet = ACLEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_OK == ehRet);

exit:
    // Perform cleanup
    OCPayloadDestroy(ehReq.payload);
    OICFree(ehReq.query);
    OICFree(jsonStr);
}
