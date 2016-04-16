/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * *****************************************************************/
#include "gtest/gtest.h"
#include "oxmjustworks.h"
#include "oxmrandompin.h"
#include "ownershiptransfermanager.h"
#include "ocstack.h"
#include "utlist.h"

using namespace std;

TEST(JustWorksOxMTest, NullParam)
{

    OTMContext_t* otmCtx = NULL;
    OCStackResult res = OC_STACK_ERROR;
    char* payloadRes;

    //LoadSecretJustWorksCallback always returns OC_STACK_OK.
    res = LoadSecretJustWorksCallback(otmCtx);
    EXPECT_TRUE(OC_STACK_OK == res);

    res = CreateSecureSessionJustWorksCallback(otmCtx);
    EXPECT_TRUE(OC_STACK_INVALID_PARAM == res);

    payloadRes = CreateJustWorksSelectOxmPayload(otmCtx);
    EXPECT_TRUE(NULL == payloadRes);

    payloadRes = CreateJustWorksOwnerTransferPayload(otmCtx);
    EXPECT_TRUE(NULL == payloadRes);

    OTMContext_t otmCtx2;
    otmCtx2.selectedDeviceInfo = NULL;

    //LoadSecretJustWorksCallback always returns OC_STACK_OK.
    res = LoadSecretJustWorksCallback(&otmCtx2);
    EXPECT_TRUE(OC_STACK_OK == res);

    res = CreateSecureSessionJustWorksCallback(&otmCtx2);
    EXPECT_TRUE(OC_STACK_INVALID_PARAM == res);

    payloadRes = CreateJustWorksSelectOxmPayload(&otmCtx2);
    EXPECT_TRUE(NULL == payloadRes);

    payloadRes = CreateJustWorksOwnerTransferPayload(&otmCtx2);
    EXPECT_TRUE(NULL == payloadRes);
}

TEST(RandomPinOxMTest, NullParam)
{
    OTMContext_t* otmCtx = NULL;
    OCStackResult res = OC_STACK_ERROR;
    char* payloadRes;

    //LoadSecretJustWorksCallback always returns OC_STACK_OK.
    res = InputPinCodeCallback(otmCtx);
    EXPECT_TRUE(OC_STACK_INVALID_PARAM == res);

    res = CreateSecureSessionRandomPinCallbak(otmCtx);
    EXPECT_TRUE(OC_STACK_INVALID_PARAM == res);

    payloadRes = CreatePinBasedSelectOxmPayload(otmCtx);
    EXPECT_TRUE(NULL == payloadRes);

    payloadRes = CreatePinBasedOwnerTransferPayload(otmCtx);
    EXPECT_TRUE(NULL == payloadRes);

    OTMContext_t otmCtx2;
    otmCtx2.selectedDeviceInfo = NULL;

    //LoadSecretJustWorksCallback always returns OC_STACK_OK.
    res = InputPinCodeCallback(&otmCtx2);
    EXPECT_TRUE(OC_STACK_INVALID_PARAM == res);

    res = CreateSecureSessionRandomPinCallbak(&otmCtx2);
    EXPECT_TRUE(OC_STACK_INVALID_PARAM == res);

    payloadRes = CreatePinBasedSelectOxmPayload(&otmCtx2);
    EXPECT_TRUE(NULL == payloadRes);

    payloadRes = CreatePinBasedOwnerTransferPayload(&otmCtx2);
    EXPECT_TRUE(NULL == payloadRes);
}
