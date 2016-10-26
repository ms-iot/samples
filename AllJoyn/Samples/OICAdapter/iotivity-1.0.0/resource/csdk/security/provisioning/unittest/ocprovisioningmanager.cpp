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
#include "ocprovisioningmanager.h"

static void provisioningCB (void* UNUSED1, int UNUSED2, OCProvisionResult_t *UNUSED3, bool UNUSED4)
{
    //dummy callback
    (void) UNUSED1;
    (void) UNUSED2;
    (void) UNUSED3;
    (void) UNUSED4;
}

TEST(OCUnlinkDevicesTest, NullDevice1)
{
    OCProvisionDev_t dev2;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCUnlinkDevices(NULL, NULL, &dev2, provisioningCB));
}

TEST(OCUnlinkDevicesTest, NullDevice2)
{
    OCProvisionDev_t dev1;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCUnlinkDevices(NULL, &dev1, NULL, provisioningCB));
}

TEST(OCUnlinkDevicesTest, NullCallback)
{
    OCProvisionDev_t dev1;
    OCProvisionDev_t dev2;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCUnlinkDevices(NULL, &dev1, &dev2, NULL));
}

TEST(OCRemoveDeviceTest, NullTargetDevice)
{
    unsigned short waitTime = 10 ;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCRemoveDevice(NULL, waitTime, NULL, provisioningCB));
}

TEST(OCRemoveDeviceTest, NullResultCallback)
{
    unsigned short waitTime = 10;
    OCProvisionDev_t dev1;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCRemoveDevice(NULL, waitTime, &dev1, NULL));
}

TEST(OCRemoveDeviceTest, ZeroWaitTime)
{
    unsigned short waitTime = 0;
    OCProvisionDev_t dev1;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCRemoveDevice(NULL, waitTime, &dev1, NULL));
}

TEST(OCGetDevInfoFromNetworkTest, NullUnOwnedDeviceInfo)
{
    OCProvisionDev_t *ownedList = NULL;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCGetDevInfoFromNetwork(0, &ownedList, NULL));
}

TEST(OCGetDevInfoFromNetworkTest, NullOwnedDeviceInfo)
{
    OCProvisionDev_t *unownedList = NULL;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCGetDevInfoFromNetwork(0, NULL, &unownedList));
}

TEST(OCGetLinkedStatusTest, NULLDeviceID)
{
    OCUuidList_t *list = NULL;
    size_t noOfDevices = 0;
    EXPECT_EQ(OC_STACK_INVALID_PARAM, OCGetLinkedStatus(NULL, &list, &noOfDevices));
}
