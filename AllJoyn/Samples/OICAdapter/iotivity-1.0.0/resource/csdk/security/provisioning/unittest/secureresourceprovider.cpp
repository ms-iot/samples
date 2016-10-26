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
#include "secureresourceprovider.h"


static OicSecAcl_t acl;
static OCProvisionDev_t pDev1;
static OCProvisionDev_t pDev2;
static OicSecCredType_t credType;

static void provisioningCB (void* UNUSED1, int UNUSED2, OCProvisionResult_t *UNUSED3, bool UNUSED4)
{
    //dummy callback
    (void) UNUSED1;
    (void) UNUSED2;
    (void) UNUSED3;
    (void) UNUSED4;
}

TEST(SRPProvisionACLTest, NullDeviceInfo)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPProvisionACL(NULL, NULL, &acl, &provisioningCB));
}

TEST(SRPProvisionACLTest, NullCallback)
{
    EXPECT_EQ(OC_STACK_INVALID_CALLBACK, SRPProvisionACL(NULL, &pDev1, &acl, NULL));
}

TEST(SRPProvisionACLTest, NullACL)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPProvisionACL(NULL, &pDev1, NULL, &provisioningCB));
}

TEST(SRPProvisionCredentialsTest, NullDevice1)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPProvisionCredentials(NULL, credType,
                                                              OWNER_PSK_LENGTH_128, NULL,
                                                              &pDev2, &provisioningCB));
}

TEST(SRPProvisionCredentialsTest, NullCallback)
{
    EXPECT_EQ(OC_STACK_INVALID_CALLBACK, SRPProvisionCredentials(NULL, credType,
                                                                 OWNER_PSK_LENGTH_128,
                                                                 &pDev1, &pDev2, NULL));
}

TEST(SRPProvisionCredentialsTest, InvalidKeySize)
{
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPProvisionCredentials(NULL, credType,
                                                                0, &pDev1, &pDev2,
                                                                &provisioningCB));
}

TEST(SRPUnlinkDevicesTest, NullDevice1)
{
    OCProvisionDev_t dev2;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPUnlinkDevices(NULL, NULL, &dev2, provisioningCB));
}

TEST(SRPUnlinkDevicesTest, NullDevice2)
{
    OCProvisionDev_t dev1;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPUnlinkDevices(NULL, &dev1, NULL, provisioningCB));
}

TEST(SRPUnlinkDevicesTest, NullCallback)
{
    OCProvisionDev_t dev1;
    OCProvisionDev_t dev2;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPUnlinkDevices(NULL, &dev1, &dev2, NULL));
}

TEST(SRPRemoveDeviceTest, NullTargetDevice)
{
    unsigned short waitTime = 10 ;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPRemoveDevice(NULL, waitTime, NULL, provisioningCB));
}

TEST(SRPRemoveDeviceTest, NullResultCallback)
{
    unsigned short waitTime = 10;
    OCProvisionDev_t dev1;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPRemoveDevice(NULL, waitTime, &dev1, NULL));
}

TEST(SRPRemoveDeviceTest, ZeroWaitTime)
{
    OCProvisionDev_t dev1;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, SRPRemoveDevice(NULL, 0, &dev1, NULL));
}
