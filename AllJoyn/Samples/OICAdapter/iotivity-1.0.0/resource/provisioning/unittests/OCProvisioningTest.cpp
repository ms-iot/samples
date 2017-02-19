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

#include <ocstack.h>
#include <oic_malloc.h>
#include <OCApi.h>
#include <OCPlatform_impl.h>
#include <oxmjustworks.h>
#include <oxmrandompin.h>
#include <OCProvisioningManager.h>
#include <gtest/gtest.h>

#define TIMEOUT 5

namespace OCProvisioningTest
{
    using namespace OC;

    void resultCallback(PMResultList_t *result, int hasError)
    {
        (void)result;
        (void)hasError;
    }

    TEST(ProvisionInitTest, TestWithEmptyPath)
    {
        std::string dbPath("");
        EXPECT_EQ(OC_STACK_OK, OCSecure::provisionInit(dbPath));
    }

    TEST(ProvisionInitTest, TestValidPath)
    {
        std::string dbPath("./dbPath");
        EXPECT_EQ(OC_STACK_OK, OCSecure::provisionInit(dbPath));
    }

    TEST(DiscoveryTest, UnownedDevices)
    {
        DeviceList_t list;
        EXPECT_EQ(OC_STACK_OK, OCSecure::discoverUnownedDevices(TIMEOUT, list));
    }

    TEST(DiscoveryTest, UnownedDevicesZeroTimeout)
    {
        DeviceList_t list;
        EXPECT_EQ(OC_STACK_OK, OCSecure::discoverUnownedDevices(0, list));
    }

    TEST(DiscoveryTest, OwnedDevices)
    {
        DeviceList_t list;
        EXPECT_EQ(OC_STACK_OK, OCSecure::discoverOwnedDevices(TIMEOUT, list));
    }

    TEST(DiscoveryTest, OwnedDevicesZeroTimeout)
    {
        DeviceList_t list;
        EXPECT_EQ(OC_STACK_OK, OCSecure::discoverOwnedDevices(0, list));
    }

    TEST(OwnershipTest, SetOwnershipTransferCBDataNull)
    {
        EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSecure::setOwnerTransferCallbackData(
                    OIC_JUST_WORKS, NULL, NULL));
    }

    TEST(OwnershipTest, SetOwnershipTransferCBData)
    {
        OTMCallbackData_t justWorksCBData;
        justWorksCBData.loadSecretCB = LoadSecretJustWorksCallback;
        justWorksCBData.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
        justWorksCBData.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
        justWorksCBData.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;
        EXPECT_EQ(OC_STACK_OK, OCSecure::setOwnerTransferCallbackData(OIC_JUST_WORKS,
                                        &justWorksCBData, NULL));
    }

    TEST(OwnershipTest, SetOwnershipTransferCBDataInvalidType)
    {
        OTMCallbackData_t justWorksCBData;
        justWorksCBData.loadSecretCB = LoadSecretJustWorksCallback;
        justWorksCBData.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
        justWorksCBData.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
        justWorksCBData.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;
        EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSecure::setOwnerTransferCallbackData(OIC_OXM_COUNT,
                                        &justWorksCBData, NULL));
    }

    TEST(OwnershipTest, SetOwnershipTransferCBDataNullInputPin)
    {
        OTMCallbackData_t pinBasedCBData;
        pinBasedCBData.loadSecretCB = InputPinCodeCallback;
        pinBasedCBData.createSecureSessionCB = CreateSecureSessionRandomPinCallbak;
        pinBasedCBData.createSelectOxmPayloadCB = CreatePinBasedSelectOxmPayload;
        pinBasedCBData.createOwnerTransferPayloadCB = CreatePinBasedOwnerTransferPayload;
        OTMSetOwnershipTransferCallbackData(OIC_RANDOM_DEVICE_PIN, &pinBasedCBData);

        EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSecure::setOwnerTransferCallbackData(
                    OIC_RANDOM_DEVICE_PIN, &pinBasedCBData, NULL));
    }

    TEST(OwnershipTest, OwnershipTransferNullCallback)
    {
        OCSecureResource device;
        EXPECT_EQ(OC_STACK_INVALID_PARAM, device.doOwnershipTransfer(nullptr));
    }

    TEST(DeviceInfoTest, DevInfoFromNetwork)
    {
        DeviceList_t owned, unowned;
        EXPECT_EQ(OC_STACK_OK, OCSecure::getDevInfoFromNetwork(TIMEOUT,
                    owned, unowned));
    }

    TEST(SetDisplayPinCBTest, SetDisplayPinCBTestNullCB)
    {
        EXPECT_EQ(OC_STACK_INVALID_PARAM, OCSecure::setDisplayPinCB(nullptr));
    }

    TEST(ProvisionAclTest, ProvisionAclTestNullAcl)
    {
        OCSecureResource device;
        EXPECT_EQ(OC_STACK_INVALID_PARAM, device.provisionACL(nullptr, resultCallback));
    }

    TEST(ProvisionAclTest, ProvisionAclTestNullCallback)
    {
        OCSecureResource device;
        OicSecAcl_t *acl = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
        EXPECT_EQ(OC_STACK_INVALID_PARAM, device.provisionACL(acl, nullptr));
        OICFree(acl);
    }

    TEST(ProvisionAclTest, ProvisionAclTestNullCallbackNUllAcl)
    {
        OCSecureResource device;
        EXPECT_EQ(OC_STACK_INVALID_PARAM, device.provisionACL(nullptr, nullptr));
    }

    TEST(ProvisionCredTest, ProvisionCredTestNullCallback)
    {
        OCSecureResource device, dev2;
        Credential cred;
        EXPECT_EQ(OC_STACK_INVALID_PARAM, device.provisionCredentials(cred, dev2, nullptr));
    }

    TEST(ProvisionPairwiseTest, ProvisionPairwiseTestNullCallback)
    {
        OCSecureResource device, dev2;
        Credential cred;
        OicSecAcl_t *acl1 = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
        OicSecAcl_t *acl2 = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
        EXPECT_EQ(OC_STACK_INVALID_PARAM, device.provisionPairwiseDevices(cred, acl1,
                    dev2, acl2, nullptr));
        OICFree(acl1);
        OICFree(acl2);
    }

}
