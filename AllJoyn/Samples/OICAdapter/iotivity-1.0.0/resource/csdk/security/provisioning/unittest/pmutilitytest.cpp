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
#include "pmutility.h"
#include "ocstack.h"
#include "utlist.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

extern OCStackResult AddDevice(OCProvisionDev_t **ppDevicesList, const char* addr,
                               const uint16_t port, OCTransportAdapter adapter,
                               OCConnectivityType connType, OicSecDoxm_t *doxm);
#ifdef __cplusplus
}
#endif

OCProvisionDev_t* gList = NULL;

// List add Tests
TEST(ProvisionListTest, Addition)
{
    OCProvisionDev_t* el = NULL;
    OCStackResult res = OC_STACK_ERROR;
    OicSecDoxm_t* pDoxm = NULL;
    int cnt =0;

    // The first addition
    res = AddDevice(&gList, "10.20.30.40", 5684, OC_DEFAULT_ADAPTER, CT_IP_USE_V4, pDoxm);
    EXPECT_TRUE(OC_STACK_OK == res);
    EXPECT_TRUE(NULL != gList);

    LL_FOREACH(gList,el){ ++cnt; };
    EXPECT_TRUE(1 == cnt);

    // Same node must not be inserted
    res = AddDevice(&gList, "10.20.30.40", 5684, OC_ADAPTER_IP, CT_IP_USE_V4, pDoxm);
    EXPECT_TRUE(OC_STACK_OK == res);
    EXPECT_TRUE(NULL != gList);

    cnt = 0;
    LL_FOREACH(gList,el){ ++cnt; };
    EXPECT_TRUE(1 == cnt);

    // Differnet node must be inserted
    res = AddDevice(&gList, "110.120.130.140", 6789, OC_DEFAULT_ADAPTER, CT_IP_USE_V4, pDoxm);
    EXPECT_TRUE(OC_STACK_OK == res);
    EXPECT_TRUE(NULL != gList);

    cnt = 0;
    LL_FOREACH(gList,el){ ++cnt; };
    EXPECT_TRUE(2 == cnt);
}

// List Delete Tests
TEST(ProvisionListTest, Deletion)
{
    OCProvisionDev_t* el = NULL;
    int cnt =0;

    // Delete whole
    PMDeleteDeviceList(gList);
    gList = NULL;

    LL_FOREACH(gList,el){ ++cnt; };
    EXPECT_TRUE(0 == cnt);
}