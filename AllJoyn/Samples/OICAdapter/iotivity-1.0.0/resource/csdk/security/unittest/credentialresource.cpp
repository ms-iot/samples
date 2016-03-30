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
#include "ocpayload.h"
#include "resourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "credresource.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "srmtestcommon.h"
#include "srmutility.h"
#include "logger.h"

#define TAG "SRM-CRED-UT"

#ifdef __cplusplus
extern "C" {
#endif

//Declare Cred resource methods for testing
OCStackResult CreateCredResource();
OCEntityHandlerResult CredEntityHandler (OCEntityHandlerFlag flag,
                OCEntityHandlerRequest * ehRequest);
char * BinToCredJSON(const OicSecCred_t * pstat);
OicSecCred_t * JSONToCredBin(const char * jsonStr);
void InitSecCredInstance(OicSecCred_t * cred);
void DeleteCredList(OicSecCred_t* cred);
const OicSecCred_t* GetCredResourceData(const OicUuid_t* subject);

#ifdef __cplusplus
}
#endif


OicSecCred_t * getCredList()
{

    OicSecCred_t * cred = NULL;
    size_t sz = 0;

    cred = (OicSecCred_t*)OICCalloc(1, sizeof(OicSecCred_t));
    VERIFY_NON_NULL(TAG, cred, ERROR);
    cred->credId = 1234;
    OICStrcpy((char *)cred->subject.id, sizeof(cred->subject.id), "subject1");

#if 0
    cred->roleIdsLen = 2;
    cred->roleIds = (OicSecRole_t *)OICCalloc(cred->roleIdsLen, sizeof(OicSecRole_t));
    VERIFY_NON_NULL(TAG, cred->roleIds, ERROR);
    OICStrcpy((char *)cred->roleIds[0].id, sizeof(cred->roleIds[0].id), "role11");
    OICStrcpy((char *)cred->roleIds[1].id, sizeof(cred->roleIds[1].id), "role12");

#endif

    cred->credType = SYMMETRIC_PAIR_WISE_KEY;
    cred->privateData.data = (char *)OICCalloc(1, strlen("My private Key11") + 1);
    VERIFY_NON_NULL(TAG, cred->privateData.data, ERROR);
    strcpy(cred->privateData.data, "My private Key11");
    cred->ownersLen = 1;
    cred->owners = (OicUuid_t*)OICCalloc(cred->ownersLen, sizeof(OicUuid_t));
    VERIFY_NON_NULL(TAG, cred->owners, ERROR);
    OICStrcpy((char *)cred->owners[0].id, sizeof(cred->owners[0].id), "ownersId11");

    cred->next = (OicSecCred_t*)OICCalloc(1, sizeof(OicSecCred_t));
    VERIFY_NON_NULL(TAG, cred->next, ERROR);
    cred->next->credId = 5678;
    OICStrcpy((char *)cred->next->subject.id, sizeof(cred->next->subject.id), "subject2");
#if 0
    cred->next->roleIdsLen = 0;
#endif
    cred->next->credType = SYMMETRIC_PAIR_WISE_KEY;
    sz = strlen("My private Key21") + 1;
    cred->next->privateData.data = (char *)OICCalloc(1, sz);
    VERIFY_NON_NULL(TAG, cred->next->privateData.data, ERROR);
    OICStrcpy(cred->next->privateData.data, sz,"My private Key21");
#if 0
    sz = strlen("My Public Key123") + 1
    cred->next->publicData.data = (char *)OICCalloc(1, sz);
    VERIFY_NON_NULL(TAG, cred->next->publicData.data, ERROR);
    OICStrcpy(cred->next->publicData.data, sz,"My Public Key123");
#endif
    cred->next->ownersLen = 2;
    cred->next->owners = (OicUuid_t*)OICCalloc(cred->next->ownersLen, sizeof(OicUuid_t));
    VERIFY_NON_NULL(TAG, cred->next->owners, ERROR);
    OICStrcpy((char *)cred->next->owners[0].id, sizeof(cred->next->owners[0].id), "ownersId21");
    OICStrcpy((char *)cred->next->owners[1].id, sizeof(cred->next->owners[1].id), "ownersId22");

    return cred;

exit:
    if(cred)
    {
        DeleteCredList(cred);
        cred = NULL;
    }
    return cred;
}

static void printCred(const OicSecCred_t * cred)
{
    EXPECT_TRUE(NULL != cred);

    const OicSecCred_t *credTmp1 = NULL;
    for(credTmp1 = cred; credTmp1; credTmp1 = credTmp1->next)
    {
        OC_LOG_V(INFO, TAG, "\ncred->credId = %d", credTmp1->credId);
        OC_LOG_V(INFO, TAG, "cred->subject.id = %s", credTmp1->subject.id);
        OC_LOG_V(INFO, TAG, "cred->credType = %d", credTmp1->credType);
        if(credTmp1->privateData.data)
        {
            OC_LOG_V(INFO, TAG, "cred->privateData.data = %s", credTmp1->privateData.data);
        }
        if(credTmp1->publicData.data)
        {
           OC_LOG_V(INFO, TAG, "cred->publicData.data = %s", credTmp1->publicData.data);
        }
        OC_LOG_V(INFO, TAG, "cred->ownersLen = %zd", credTmp1->ownersLen);
        for(size_t i = 0; i < cred->ownersLen; i++)
        {
            OC_LOG_V(INFO, TAG, "cred->owners[%zd].id = %s", i, credTmp1->owners[i].id);
        }
    }
}

 //InitCredResource Tests
TEST(InitCredResourceTest, InitCredResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, InitCredResource());
}

//DeInitCredResource Tests
TEST(DeInitCredResourceTest, DeInitCredResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, DeInitCredResource());
}

//CreateCredResource Tests
TEST(CreateCredResourceTest, CreateCredResource)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, CreateCredResource());
}

 //CredEntityHandler Tests
TEST(CredEntityHandlerTest, CredEntityHandlerWithDummyRequest)
{
    OCEntityHandlerRequest req;
    EXPECT_EQ(OC_EH_ERROR,
            CredEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, &req));
}

TEST(CredEntityHandlerTest, CredEntityHandlerWithNULLRequest)
{
    EXPECT_EQ(OC_EH_ERROR,
            CredEntityHandler(OCEntityHandlerFlag::OC_REQUEST_FLAG, NULL));
}

TEST(CredEntityHandlerTest, CredEntityHandlerInvalidFlag)
{
    OCEntityHandlerRequest req;
    EXPECT_EQ(OC_EH_ERROR,
            CredEntityHandler(OCEntityHandlerFlag::OC_OBSERVE_FLAG, &req));
}

//Cred DELETE request
TEST(CredEntityHandlerTest, CredEntityHandlerDeleteTest)
{
    OCEntityHandlerRequest ehReq =  OCEntityHandlerRequest();
    static OCPersistentStorage ps =  OCPersistentStorage();
    const OicSecCred_t* subjectCred1 = NULL;
    const OicSecCred_t* subjectCred2 = NULL;
    char *jsonStr = NULL;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    char query[] = "sub=c3ViamVjdDE=";

    SetPersistentHandler(&ps, true);

    OicSecCred_t *cred = getCredList();
    VERIFY_NON_NULL(TAG, cred, ERROR);

    jsonStr = BinToCredJSON(cred);
    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    // Create Entity Handler POST request payload
    ehReq.method = OC_REST_POST;
    ehReq.payload = (OCPayload*)OCSecurityPayloadCreate(jsonStr);
    ehRet = CredEntityHandler(OC_REQUEST_FLAG, &ehReq);
    EXPECT_TRUE(OC_EH_ERROR == ehRet);

    // Verify if SRM contains Credential for the subject
    subjectCred1 = GetCredResourceData(&cred->subject);
    EXPECT_TRUE(NULL != subjectCred1);

   // Create Entity Handler DELETE request
   ehReq.method = OC_REST_DELETE;
   ehReq.query = (char*)OICMalloc(strlen(query)+1);
   VERIFY_NON_NULL(TAG, ehReq.query, ERROR);
   OICStrcpy(ehReq.query, strlen(query)+1, query);

   ehRet = CredEntityHandler(OC_REQUEST_FLAG, &ehReq);
   EXPECT_TRUE(OC_EH_ERROR == ehRet);

   // Verify if SRM has deleted ACE for the subject
   subjectCred2 = GetCredResourceData(&cred->subject);
   EXPECT_TRUE(NULL == subjectCred2);

exit:
   // Perform cleanup
   OICFree(ehReq.query);
   OICFree(jsonStr);
   OCPayloadDestroy(ehReq.payload);
   if(NULL != cred)
   {
       DeInitCredResource();
       DeleteCredList(cred);
   }
}

//BinToCredJSON Tests
TEST(BinToCredJSONTest, BinToCredJSONNullCred)
{
    char* value = BinToCredJSON(NULL);
    EXPECT_TRUE(value == NULL);
}

TEST(BinToCredJSONTest, BinToCredJSONValidCred)
{
    char* json = NULL;
    OicSecCred_t * cred = getCredList();

    json = BinToCredJSON(cred);

    OC_LOG_V(INFO, TAG, "BinToCredJSON:%s\n", json);
    EXPECT_TRUE(json != NULL);
    DeleteCredList(cred);
    OICFree(json);
}

//JSONToCredBin Tests
TEST(JSONToCredBinTest, JSONToCredBinValidJSON)
{
    OicSecCred_t* cred1 = getCredList();
    char* json = BinToCredJSON(cred1);

    EXPECT_TRUE(json != NULL);
    OicSecCred_t *cred2 = JSONToCredBin(json);
    EXPECT_TRUE(cred2 != NULL);
    DeleteCredList(cred1);
    DeleteCredList(cred2);
    OICFree(json);
}

TEST(JSONToCredBinTest, JSONToCredBinNullJSON)
{
    OicSecCred_t *cred = JSONToCredBin(NULL);
    EXPECT_TRUE(cred == NULL);
}

//GetCredResourceData Test
TEST(CredGetResourceDataTest, GetCredResourceDataNULLSubject)
{
    EXPECT_TRUE(NULL == GetCredResourceData(NULL));
}

TEST(CredGenerateCredentialTest, GenerateCredentialValidInput)
{
    OicUuid_t owners[1];
    OICStrcpy((char *)owners[0].id, sizeof(owners[0].id), "ownersId21");

    OicUuid_t subject = {{0}};
    OICStrcpy((char *)subject.id, sizeof(subject.id), "subject11");

    char privateKey[] = "My private Key11";

    OicSecCred_t * cred  = NULL;

    cred = GenerateCredential(&subject, SYMMETRIC_PAIR_WISE_KEY, NULL,
                             privateKey, 1, owners);
    printCred(cred);

    EXPECT_TRUE(NULL != cred);
    DeleteCredList(cred);
}

TEST(GenerateAndAddCredentialTest, GenerateAndAddCredentialValidInput)
{
    OicUuid_t owners[1];
    OICStrcpy((char *)owners[0].id, sizeof(owners[0].id), "ownersId11");

    OicUuid_t subject = {{0}};
    OICStrcpy((char *)subject.id, sizeof(subject.id), "subject11");

    char privateKey[] = "My private Key11";

    OicSecCred_t * cred1  = NULL;
    OicSecCred_t * headCred = NULL;

    cred1 = GenerateCredential(&subject, SYMMETRIC_PAIR_WISE_KEY, NULL,
                                 privateKey, 1, owners);

    EXPECT_EQ(OC_STACK_ERROR, AddCredential(cred1));
    headCred = cred1;

    OICStrcpy((char *)owners[0].id, sizeof(owners[0].id), "ownersId22");
    OICStrcpy((char *)subject.id, sizeof(subject.id), "subject22");
    cred1 = GenerateCredential(&subject, SYMMETRIC_PAIR_WISE_KEY, NULL,
                                     privateKey, 1, owners);
    EXPECT_EQ(OC_STACK_ERROR, AddCredential(cred1));

    OICStrcpy((char *)owners[0].id, sizeof(owners[0].id), "ownersId33");
    OICStrcpy((char *)subject.id, sizeof(subject.id), "subject33");
    cred1 = GenerateCredential(&subject, SYMMETRIC_PAIR_WISE_KEY, NULL,
                                     privateKey, 1, owners);
    EXPECT_EQ(OC_STACK_ERROR, AddCredential(cred1));

    const OicSecCred_t* credList = GetCredResourceData(&headCred->subject);

    printCred(credList);

    DeleteCredList(headCred);

}

#if 0
TEST(CredGetResourceDataTest, GetCredResourceDataValidSubject)
{
    OicSecCred_t* cred = getCredList();
    EXPECT_TRUE(NULL != GetCredResourceData(cred->subject));
}
#endif


