/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "ocprovisioningmanager.h"
#include "oxmjustworks.h"
#include "oxmrandompin.h"
#include "securevirtualresourcetypes.h"
#include "srmutility.h"
#include "pmtypes.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

// declaration(s) for provisioning client using C-level provisioning API
// user input definition for main loop on provisioning client
#define _10_DISCOV_ALL_DEVS_    10
#define _11_DISCOV_UNOWN_DEVS_  11
#define _12_DISCOV_OWN_DEVS_    12
#define _20_REGIST_DEVS_        20
#define _30_PROVIS_PAIR_DEVS_   30
#define _31_PROVIS_CRED_        31
#define _32_PROVIS_ACL_         32
#define _33_CHECK_LINK_STATUS_  33
#define _40_UNLINK_PAIR_DEVS_   40
#define _50_REMOVE_SELEC_DEV_   50
#define _99_EXIT_PRVN_CLT_      99

#define ACL_RESRC_MAX_NUM   16
#define ACL_RESRC_MAX_LEN   128
#define ACL_PEMISN_CNT      5
#define DISCOVERY_TIMEOUT   10  // 10 sec
#define CALLBACK_TIMEOUT    60  // 1 min
#define TAG "provisioningclient"

static const char* ACL_PEMISN[5] = {"CREATE", "READ", "WRITE", "DELETE", "NOTIFY"};
static const char* SVR_DB_FILE_NAME = "oic_svr_db_client.json";
        // '_' for separaing from the same constant variable in |srmresourcestrings.c|
static const char* PRVN_DB_FILE_NAME = "oic_prvn_mng.db";
// |g_ctx| means provision manager application context and
// the following, includes |un/own_list|, could be variables, which |g_ctx| has,
// for accessing all function(s) for these, they are declared on global domain
static const char* g_ctx = "Provision Manager Client Application Context";
static char* g_svr_fname;
static char* g_prvn_fname;
static OCProvisionDev_t* g_own_list;
static OCProvisionDev_t* g_unown_list;
static int g_own_cnt;
static int g_unown_cnt;
static bool g_doneCB;

// function declaration(s) for calling them before implementing
static OicSecAcl_t* createAcl(const int);
static OCProvisionDev_t* getDevInst(const OCProvisionDev_t*, const int);
static int printDevList(const OCProvisionDev_t*);
static size_t printUuidList(const OCUuidList_t*);
static int printResultList(const OCProvisionResult_t*, const int);
static void printUuid(const OicUuid_t*);
static FILE* fopen_prvnMng(const char*, const char*);
static int waitCallbackRet(void);
static int selectTwoDiffNum(int*, int*, const int, const char*);

// callback function(s) for provisioning client using C-level provisioning API
static void ownershipTransferCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if(!hasError)
    {
        OC_LOG_V(INFO, TAG, "Ownership Transfer SUCCEEDED - ctx: %s", (char*) ctx);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Ownership Transfer FAILED - ctx: %s", (char*) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
    g_doneCB = true;
}

static void provisionPairwiseCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if(!hasError)
    {
        OC_LOG_V(INFO, TAG, "Provision Pairwise SUCCEEDED - ctx: %s", (char*) ctx);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Provision Pairwise FAILED - ctx: %s", (char*) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
    g_doneCB = true;
}

static void provisionCredCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if(!hasError)
    {
        OC_LOG_V(INFO, TAG, "Provision Credential SUCCEEDED - ctx: %s", (char*) ctx);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Provision Credential FAILED - ctx: %s", (char*) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
    g_doneCB = true;
}

static void provisionAclCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if(!hasError)
    {
        OC_LOG_V(INFO, TAG, "Provision ACL SUCCEEDED - ctx: %s", (char*) ctx);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Provision ACL FAILED - ctx: %s", (char*) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
    g_doneCB = true;
}

static void unlinkDevicesCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if(!hasError)
    {
        OC_LOG_V(INFO, TAG, "Unlink Devices SUCCEEDED - ctx: %s", (char*) ctx);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Unlink Devices FAILED - ctx: %s", (char*) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
    g_doneCB = true;
}

static void removeDeviceCB(void* ctx, int nOfRes, OCProvisionResult_t* arr, bool hasError)
{
    if(!hasError)
    {
        OC_LOG_V(INFO, TAG, "Remove Device SUCCEEDED - ctx: %s", (char*) ctx);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Remove Device FAILED - ctx: %s", (char*) ctx);
        printResultList((const OCProvisionResult_t*) arr, nOfRes);
    }
    g_doneCB = true;
}

static void inputPinCB(char* pin, size_t len)
{
    if(!pin || OXM_RANDOM_PIN_SIZE>=len)
    {
        OC_LOG(ERROR, TAG, "inputPinCB invalid parameters");
        return;
    }

    printf("   > INPUT PIN: ");
    for(int ret=0; 1!=ret; )
    {
        ret = scanf("%8s", pin);
        for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                    // '0x20<=code' is character region
    }
}

// function(s) for provisioning client using C-level provisioning API
static int initProvisionClient(void)
{
    // initialize persistent storage for SVR DB
    static OCPersistentStorage pstStr =
    {
        .open = fopen_prvnMng,
        .read = fread,
        .write = fwrite,
        .close = fclose,
        .unlink = unlink
    };
    if(OC_STACK_OK != OCRegisterPersistentStorageHandler(&pstStr))
    {
        OC_LOG(ERROR, TAG, "OCRegisterPersistentStorageHandler error");
        return -1;
    }

    // initialize OC stack and provisioning manager
    if(OC_STACK_OK != OCInit(NULL, 0, OC_CLIENT_SERVER))
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return -1;
    }

    if (access(PRVN_DB_FILE_NAME, F_OK) != -1)
    {
        printf("************************************************************\n");
        printf("************Provisioning DB file already exists.************\n");
        printf("************************************************************\n");
    }
    else
    {
        printf("*************************************************************\n");
        printf("************No provisioning DB file, creating new************\n");
        printf("*************************************************************\n");
    }

    if(OC_STACK_OK != OCInitPM(PRVN_DB_FILE_NAME))
    {
        OC_LOG(ERROR, TAG, "OC_PM init error");
        return -1;
    }

    // register callback function(s) to each OxM
    OTMCallbackData_t otmcb =
    {
        .loadSecretCB = LoadSecretJustWorksCallback,
        .createSecureSessionCB = CreateSecureSessionJustWorksCallback,
        .createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload,
        .createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload
    };
    if(OC_STACK_OK != OCSetOwnerTransferCallbackData(OIC_JUST_WORKS, &otmcb))
    {
        OC_LOG(ERROR, TAG, "OCSetOwnerTransferCallbackData error: OIC_JUST_WORKS");
        return -1;
    }
    otmcb.loadSecretCB = InputPinCodeCallback;
    otmcb.createSecureSessionCB = CreateSecureSessionRandomPinCallbak;
    otmcb.createSelectOxmPayloadCB = CreatePinBasedSelectOxmPayload;
    otmcb.createOwnerTransferPayloadCB = CreatePinBasedOwnerTransferPayload;
    if(OC_STACK_OK != OCSetOwnerTransferCallbackData(OIC_RANDOM_DEVICE_PIN, &otmcb))
    {
        OC_LOG(ERROR, TAG, "OCSetOwnerTransferCallbackData error: OIC_RANDOM_DEVICE_PIN");
        return -1;
    }
    SetInputPinCB(inputPinCB);

    return 0;
}

static int discoverAllDevices(void)
{
    // delete un/owned device lists before updating them
    if(g_own_list)
    {
        OCDeleteDiscoveredDevices(g_own_list);
        g_own_list = NULL;
    }
    if(g_unown_list)
    {
        OCDeleteDiscoveredDevices(g_unown_list);
        g_unown_list = NULL;
    }

    // call |OCGetDevInfoFromNetwork| API actually
    printf("   Discovering All Un/Owned Devices on Network..\n");
    if(OC_STACK_OK != OCGetDevInfoFromNetwork(DISCOVERY_TIMEOUT, &g_own_list, &g_unown_list))
    {
        OC_LOG(ERROR, TAG, "OCGetDevInfoFromNetwork API error");
        return -1;
    }

    // display the discovered un/owned lists
    printf("   > Discovered Owned Devices\n");
    g_own_cnt = printDevList(g_own_list);
    printf("   > Discovered Unowned Devices\n");
    g_unown_cnt = printDevList(g_unown_list);

    return 0;
}


static int discoverUnownedDevices(void)
{
    // delete unowned device list before updating it
    if(g_unown_list)
    {
        OCDeleteDiscoveredDevices(g_unown_list);
        g_unown_list = NULL;
    }

    // call |OCDiscoverUnownedDevices| API actually
    printf("   Discovering Only Unowned Devices on Network..\n");
    if(OC_STACK_OK != OCDiscoverUnownedDevices(DISCOVERY_TIMEOUT, &g_unown_list))
    {
        OC_LOG(ERROR, TAG, "OCDiscoverUnownedDevices API error");
        return -1;
    }

    // display the discovered unowned list
    printf("   > Discovered Unowned Devices\n");
    g_unown_cnt = printDevList(g_unown_list);

    return 0;
}

static int discoverOwnedDevices(void)
{
    // delete owned device list before updating it
    if(g_own_list)
    {
        OCDeleteDiscoveredDevices(g_own_list);
        g_own_list = NULL;
    }

    // call |OCDiscoverOwnedDevices| API actually
    printf("   Discovering Only Owned Devices on Network..\n");
    if(OC_STACK_OK != OCDiscoverOwnedDevices(DISCOVERY_TIMEOUT, &g_own_list))
    {
        OC_LOG(ERROR, TAG, "OCDiscoverOwnedDevices API error");
        return -1;
    }

    // display the discovered owned list
    printf("   > Discovered Owned Devices\n");
    g_own_cnt = printDevList(g_own_list);

    return 0;
}

static int registerDevices(void)
{
    // check |unown_list| for registering devices
    if(!g_unown_list || 0>=g_unown_cnt)
    {
        printf("   > Unowned Device List, to Register Devices, is Empty\n");
        printf("   > Please Discover Unowned Devices first, with [10|11] Menu\n");
        return 0;  // normal case
    }

    // call |OCDoOwnershipTransfer| API actually
    // calling this API with callback actually acts like blocking
    // for error checking, the return value saved and printed
    g_doneCB = false;
    printf("   Registering All Discovered Unowned Devices..\n");
    OCStackResult rst = OCDoOwnershipTransfer((void*) g_ctx, g_unown_list, ownershipTransferCB);
    if(OC_STACK_OK != rst)
    {
        OC_LOG_V(ERROR, TAG, "OCDoOwnershipTransfer API error: %d", rst);
        return -1;
    }
    if(waitCallbackRet())  // input |g_doneCB| flag implicitly
    {
        OC_LOG(ERROR, TAG, "OCProvisionCredentials callback error");
        return -1;
    }

    // display the registered result
    printf("   > Registered Discovered Unowned Devices\n");
    printf("   > Please Discover Owned Devices for the Registered Result, with [10|12] Menu\n");

    return 0;
}

static int provisionPairwise(void)
{
    // check |own_list| for provisioning pairwise devices
    if(!g_own_list || 2>g_own_cnt)
    {
        printf("   > Owned Device List, to Provision the Pairwise, is Empty\n");
        printf("   > Please Register Unowned Devices first, with [20] Menu\n");
        return 0;  // normal case
    }

    // select two devices for provisioning pairwise devices
    int dev_num[2] = {0};
    if(selectTwoDiffNum(&(dev_num[0]), &(dev_num[1]), g_own_cnt, "for Linking Devices"))
    {
        OC_LOG(ERROR, TAG, "selectTwoDiffNum error return");
        return -1;  // not need to 'goto' |ERROR| before allocating |acl|
    }

    // create ACL(s) for each selected device
    OicSecAcl_t* acl[2] = {0};
    for(int i=0; 2>i; ++i)
    {
        acl[i] = createAcl(dev_num[i]);
        if(!acl[i])
        {
            OC_LOG(ERROR, TAG, "createAcl error return");
            goto PVPWS_ERROR;
        }
    }

    // call |OCProvisionPairwiseDevices| API actually
    // calling this API with callback actually acts like blocking
    // for error checking, the return value saved and printed
    g_doneCB = false;
    printf("   Provisioning Selected Pairwise Devices..\n");
    OCStackResult rst =
            OCProvisionPairwiseDevices((void*) g_ctx,
                    SYMMETRIC_PAIR_WISE_KEY, OWNER_PSK_LENGTH_128,
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num[0]), acl[0],
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num[1]), acl[1],
                    provisionPairwiseCB);
    if(OC_STACK_OK != rst)
    {
        OC_LOG_V(ERROR, TAG, "OCProvisionPairwiseDevices API error: %d", rst);
        goto PVPWS_ERROR;
    }
    if(waitCallbackRet())  // input |g_doneCB| flag implicitly
    {
        OC_LOG(ERROR, TAG, "OCProvisionCredentials callback error");
        goto PVPWS_ERROR;
    }
    OCDeleteACLList(acl[0]);
    OCDeleteACLList(acl[1]);

    // display the pairwise-provisioned result
    printf("   > Provisioned Selected Pairwise Devices\n");
    printf("   > Please Check Device's Status for the Linked Result, with [33] Menu\n");

    return 0;

PVPWS_ERROR:
    OCDeleteACLList(acl[0]);
    OCDeleteACLList(acl[1]);
    return -1;
}

static int provisionCred(void)
{
    // check |own_list| for provisioning pairwise credentials
    if(!g_own_list || 2>g_own_cnt)
    {
        printf("   > Owned Device List, to Provision Credentials, is Empty\n");
        printf("   > Please Register Unowned Devices first, with [20] Menu\n");
        return 0;  // normal case
    }

    // select two devices for provisioning pairwise credentials
    int dev_num[2] = {0};
    if(selectTwoDiffNum(&(dev_num[0]), &(dev_num[1]), g_own_cnt, "for Linking CRED(s)"))
    {
        OC_LOG(ERROR, TAG, "selectTwoDiffNum error return");
        return -1;
    }

    printf("   Select PSK length..\n");
    printf("   1 - 128bit(Default)\n");
    printf("   2 - 256bit\n");
    int sizeOption = 0;

    for(int ret=0; 1!=ret; )
    {
         ret = scanf("%d",&sizeOption);
         for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                    // '0x20<=code' is character region
    }
    size_t size = 0;

    switch(sizeOption)
    {
        case 1:
        {
            size = OWNER_PSK_LENGTH_128;
            break;
        }
        case 2:
        {
            size = OWNER_PSK_LENGTH_256;
            break;
        }
        default:
        {
            size = OWNER_PSK_LENGTH_128;
            break;
        }
    }


    // call |OCProvisionCredentials| API actually
    // calling this API with callback actually acts like blocking
    // for error checking, the return value saved and printed
    g_doneCB = false;
    printf("   Provisioning Selected Pairwise Credentials..\n");
    OCStackResult rst =
            OCProvisionCredentials((void*) g_ctx,
                    SYMMETRIC_PAIR_WISE_KEY, size,
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num[0]),
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num[1]),
                    provisionCredCB);
    if(OC_STACK_OK != rst)
    {
        OC_LOG_V(ERROR, TAG, "OCProvisionCredentials API error: %d", rst);
        return -1;
    }
    if(waitCallbackRet())  // input |g_doneCB| flag implicitly
    {
        OC_LOG(ERROR, TAG, "OCProvisionCredentials callback error");
        return -1;
    }

    // display the CRED-provisioned result
    printf("   > Provisioned Selected Pairwise Crendentials\n");
    printf("   > Please Check Device's Status for the Linked Result, with [33] Menu\n");

    return 0;
}

static int provisionAcl(void)
{
    // check |own_list| for provisioning access control list
    if(!g_own_list || 1>g_own_cnt)
    {
        printf("   > Owned Device List, to Provision ACL, is Empty\n");
        printf("   > Please Register Unowned Devices first, with [20] Menu\n");
        return 0;  // normal case
    }

    // select device for provisioning access control list
    int dev_num = 0;
    for( ; ; )
    {
        printf("   > Enter Device Number, for Provisioning ACL: ");
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &dev_num);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        if(0<dev_num && g_own_cnt>=dev_num)
        {
            break;
        }
        printf("     Entered Wrong Number. Please Enter Again\n");
    }

    // create ACL for selected device
    OicSecAcl_t* acl = NULL;
    acl = createAcl(dev_num);
    if(!acl)
    {
        OC_LOG(ERROR, TAG, "createAcl error return");
        goto PVACL_ERROR;
    }

    // call |OCProvisionACL| API actually
    // calling this API with callback actually acts like blocking
    // for error checking, the return value saved and printed
    g_doneCB = false;
    printf("   Provisioning Selected ACL..\n");
    OCStackResult rst =
            OCProvisionACL((void*) g_ctx,
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num),
                    acl, provisionAclCB);
    if(OC_STACK_OK != rst)
    {
        OC_LOG_V(ERROR, TAG, "OCProvisionACL API error: %d", rst);
        goto PVACL_ERROR;
    }
    if(waitCallbackRet())  // input |g_doneCB| flag implicitly
    {
        OC_LOG(ERROR, TAG, "OCProvisionCredentials callback error");
        goto PVACL_ERROR;
    }
    OCDeleteACLList(acl);  // after here |acl| points nothing

    // display the ACL-provisioned result
    printf("   > Provisioned Selected ACL\n");

    return 0;

PVACL_ERROR:
    OCDeleteACLList(acl);  // after here |acl| points nothing
    return -1;
}

static int checkLinkedStatus(void)
{
    // check |own_list| for checking selected link status on PRVN DB
    if(!g_own_list || 1>g_own_cnt)
    {
        printf("   > Owned Device List, to Check Linked Status on PRVN DB, is Empty\n");
        printf("   > Please Register Unowned Devices first, with [20] Menu\n");
        return 0;  // normal case
    }

    // select device for checking selected link status on PRVN DB
    int dev_num = 0;
    for( ; ; )
    {
        printf("   > Enter Device Number, for Checking Linked Status on PRVN DB: ");
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &dev_num);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        if(0<dev_num && g_own_cnt>=dev_num)
        {
            break;
        }
        printf("     Entered Wrong Number. Please Enter Again\n");
    }

    // call |OCGetLinkedStatus| API actually
    printf("   Checking Selected Link Status on PRVN DB..\n");
    OCUuidList_t* dvid_lst = NULL;
    size_t dvid_cnt = 0;
    if(OC_STACK_OK !=
            OCGetLinkedStatus(
                    &getDevInst((const OCProvisionDev_t*) g_own_list, dev_num)->doxm->deviceID,
                    &dvid_lst, &dvid_cnt))  // allow empty list
    {
        OC_LOG(ERROR, TAG, "OCGetLinkedStatus API error");
        goto CKLST_ERROR;
    }

    // display the linked status result
    printf("   > Checked Selected Link Status on PRVN DB\n");
    if(!dvid_lst || !dvid_cnt)  // |size_t| is unsigned
    {
        printf("     Linked Device List is Empty..\n");
        return 0;  // normal case
    }
    if(dvid_cnt != printUuidList((const OCUuidList_t*) dvid_lst))
    {
        OC_LOG(ERROR, TAG, "printUuidList error return");
        goto CKLST_ERROR;
    }
    OCDeleteUuidList(dvid_lst);

    return 0;

CKLST_ERROR:
    OCDeleteUuidList(dvid_lst);
    return -1;
}

static int unlinkPairwise(void)
{
    // check |own_list| for unlinking pairwise devices
    if(!g_own_list || 2>g_own_cnt)
    {
        printf("   > Owned Device List, to Unlink the Pairwise, is Empty\n");
        printf("   > Please Register Unowned Devices first, with [20] Menu\n");
        return 0;  // normal case
    }

    // select two devices for unlinking pairwise devices
    int dev_num[2] = {0};
    if(selectTwoDiffNum(&(dev_num[0]), &(dev_num[1]), g_own_cnt, "for Unlinking Devices"))
    {
        OC_LOG(ERROR, TAG, "selectTwoDiffNum error return");
        return -1;
    }

    // call |OCUnlinkDevices| API actually
    // calling this API with callback actually acts like blocking
    // for error checking, the return value saved and printed
    g_doneCB = false;
    printf("   Unlinking Selected Pairwise Devices..\n");
    OCStackResult rst =
            OCUnlinkDevices((void*) g_ctx,
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num[0]),
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num[1]),
                    unlinkDevicesCB);
    if(OC_STACK_OK != rst)
    {
        OC_LOG_V(ERROR, TAG, "OCUnlinkDevices API error: %d", rst);
        return -1;
    }
    if(waitCallbackRet())  // input |g_doneCB| flag implicitly
    {
        OC_LOG(ERROR, TAG, "OCProvisionCredentials callback error");
        return -1;
    }

    // display the pairwise-unlinked result
    printf("   > Unlinked Selected Pairwise Devices\n");
    printf("   > Please Check Device's Status for the Unlinked Result, with [33] Menu\n");

    return 0;
}

static int removeDevice(void)
{
    // check |own_list| for removing device
    if(!g_own_list || 1>g_own_cnt)
    {
        printf("   > Owned Device List, to Remove Device, is Empty\n");
        printf("   > Please Register Unowned Devices first, with [20] Menu\n");
        return 0;  // normal case
    }

    // select device for removing it
    int dev_num = 0;
    for( ; ; )
    {
        printf("   > Enter Device Number, for Removing Device: ");
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &dev_num);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        if(0<dev_num && g_own_cnt>=dev_num)
        {
            break;
        }
        printf("     Entered Wrong Number. Please Enter Again\n");
    }

    // call |OCRemoveDevice| API actually
    // calling this API with callback actually acts like blocking
    // for error checking, the return value saved and printed
    g_doneCB = false;
    printf("   Removing Selected Owned Device..\n");
    OCStackResult rst =
            OCRemoveDevice((void*) g_ctx, DISCOVERY_TIMEOUT,
                    getDevInst((const OCProvisionDev_t*) g_own_list, dev_num), removeDeviceCB);
    if(OC_STACK_OK != rst)
    {
        OC_LOG_V(ERROR, TAG, "OCRemoveDevice API error: %d", rst);
        return -1;
    }
    if(waitCallbackRet())  // input |g_doneCB| flag implicitly
    {
        OC_LOG(ERROR, TAG, "OCProvisionCredentials callback error");
        return -1;
    }

    // display the removed result
    printf("   > Removed Selected Owned Device\n");
    printf("   > Please Discover Owned Devices for the Registered Result, with [10|12] Menu\n");

    return 0;
}

static OicSecAcl_t* createAcl(const int dev_num)
{
    if(0>=dev_num || g_own_cnt<dev_num)
    {
        OC_LOG(ERROR, TAG, "createAcl invalid parameters");
        return NULL;  // not need to 'goto' |ERROR| before allocating |acl|
    }

    // allocate memory for |acl| struct
    printf("   **** Create ACL for the Selected Device[%d]\n", dev_num);
    OicSecAcl_t* acl = (OicSecAcl_t*) OICCalloc(1, sizeof(OicSecAcl_t));
    if(!acl)
    {
        OC_LOG(ERROR, TAG, "createAcl: OICCalloc error return");
        return NULL;  // not need to 'goto' |ERROR| before allocating |acl|
    }

    // enter |subject| device number
    int num = 0;
    for( ; ; )
    {
        printf("   > [A] Enter Subject Device Number: ");
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &num);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        if(num && g_own_cnt>=num && dev_num!=num)
        {
            break;
        }
        printf("     Entered Wrong Number. Please Enter Again\n");
    }
    memcpy(&acl->subject,
            &getDevInst((const OCProvisionDev_t*) g_own_list, num)->doxm->deviceID,
            UUID_LENGTH);  // not need |*sizeof(uint8_t)|

    // enter number of |resources| in 'accessed' device
    for( ; ; )
    {
        printf("   > [B] Enter Number of Accessed Resources (under 16): ");
                // '16' is |ACL_RESRC_MAX_NUM|
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%zu", &acl->resourcesLen);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        if(acl->resourcesLen && ACL_RESRC_MAX_NUM>=acl->resourcesLen)
                // |acl->resourcesLen| is unsigned
        {
            break;
        }
        printf("     Entered Wrong Number. Please Enter under 16 Again\n");
                // '16' is |ACL_RESRC_MAX_NUM|
    }

    // enter actually each 'accessed' |resources| name
    printf("         Enter Each Accessed Resource Name (each under 128 char)\n");
            // '128' is ACL_RESRC_MAX_LEN
    num = acl->resourcesLen;
    acl->resources = (char**) OICCalloc(num, sizeof(char*));
    if(!acl->resources)
    {
        OC_LOG(ERROR, TAG, "createAcl: OICCalloc error return");
        goto CRACL_ERROR;
    }
    char rsrc_in[ACL_RESRC_MAX_LEN+1] = {0};  // '1' for null termination
    for(int i=0; num>i; ++i)
    {
        printf("         Enter Accessed Resource[%d] Name: ", i+1);
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%128s", rsrc_in);  // '128' is ACL_RESRC_MAX_LEN
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        size_t len = strlen(rsrc_in)+1;  // '1' for null termination
        char* rsrc = (char*) OICCalloc(len, sizeof(char));
        if(!rsrc)
        {
            OC_LOG(ERROR, TAG, "createAcl: OICCalloc error return");
            goto CRACL_ERROR;
        }
        OICStrcpy(rsrc, len, rsrc_in);
        acl->resources[i] = rsrc;  // after here, |rsrc| points nothing
    }

    // enter |permission| for this access
    printf("   > [C] Enter Permission for This Access\n");
    uint16_t pmsn = PERMISSION_FULL_CONTROL;  // default full permission
    uint16_t pmsn_msk = PERMISSION_CREATE;  // default permission mask
    for(int i=0; ACL_PEMISN_CNT>i; ++i)
    {
        char ans = 0;
        for( ; ; )
        {
            printf("         Enter %s Permission (y/n): ", ACL_PEMISN[i]);
            for(int ret=0; 1!=ret; )
            {
                ret = scanf("%c", &ans);
                for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                            // '0x20<=code' is character region
            }
            if('y'==ans || 'Y'==ans || 'n'==ans|| 'N'==ans)
            {
                ans &= ~0x20;  // for masking lower case, 'y/n'
                break;
            }
            printf("         Entered Wrong Answer. Please Enter 'y/n' Again\n");
        }
        if('N' == ans)  // masked lower case, 'n'
        {
            pmsn -= pmsn_msk;
        }
        pmsn_msk <<= 1;
    }
    acl->permission = pmsn;

    // enter |owner| device number
    for( ; ; )
    {
        printf("   > [D] Enter Owner Device Number: ");
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &num);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        if(num && g_own_cnt>=num)
        {
            break;
        }
        printf("         Entered Wrong Number. Please Enter Again\n");
    }
    acl->ownersLen = 1;
    acl->owners = (OicUuid_t*) OICCalloc(1, sizeof(OicUuid_t));
    if(!acl->owners)
    {
        OC_LOG(ERROR, TAG, "createAcl: OICCalloc error return");
        goto CRACL_ERROR;
    }
    memcpy(acl->owners,
            &getDevInst((const OCProvisionDev_t*) g_own_list, num)->doxm->deviceID,
            UUID_LENGTH);  // not need |*sizeof(uint8_t)|
    printf("\n");

    return acl;

CRACL_ERROR:
    OCDeleteACLList(acl);  // after here |acl| points nothing
    return NULL;
}

static OCProvisionDev_t* getDevInst(const OCProvisionDev_t* dev_lst, const int dev_num)
{
    if(!dev_lst || 0>=dev_num)
    {
        printf("     Device List is Empty..\n");
        return NULL;
    }

    OCProvisionDev_t* lst = (OCProvisionDev_t*) dev_lst;
    for(int i=0; lst; )
    {
        if(dev_num == ++i)
        {
            return lst;
        }
        lst = lst->next;
    }

    return NULL;  // in here |lst| is always |NULL|
}

static int printDevList(const OCProvisionDev_t* dev_lst)
{
    if(!dev_lst)
    {
        printf("     Device List is Empty..\n\n");
        return 0;
    }

    OCProvisionDev_t* lst = (OCProvisionDev_t*) dev_lst;
    int lst_cnt = 0;
    for( ; lst; )
    {
        printf("     [%d] ", ++lst_cnt);
        printUuid((const OicUuid_t*) &lst->doxm->deviceID);
        printf("\n");
        lst = lst->next;
    }
    printf("\n");

    return lst_cnt;
}

static size_t printUuidList(const OCUuidList_t* uid_lst)
{
    if(!uid_lst)
    {
        printf("     Device List is Empty..\n\n");
        return 0;
    }

    OCUuidList_t* lst = (OCUuidList_t*) uid_lst;
    size_t lst_cnt = 0;
    for( ; lst; )
    {
        printf("     [%zu] ", ++lst_cnt);
        printUuid((const OicUuid_t*) &lst->dev);
        printf("\n");
        lst = lst->next;
    }
    printf("\n");

    return lst_cnt;
}

static int printResultList(const OCProvisionResult_t* rslt_lst, const int rslt_cnt)
{
    if(!rslt_lst || 0>=rslt_cnt)
    {
        printf("     Device List is Empty..\n\n");
        return 0;
    }

    int lst_cnt = 0;
    for( ; rslt_cnt>lst_cnt; ++lst_cnt)
    {
        printf("     [%d] ", lst_cnt+1);
        printUuid((const OicUuid_t*) &rslt_lst[lst_cnt].deviceId);
        printf(" - result: %d\n", rslt_lst[lst_cnt].res);
    }
    printf("\n");

    return lst_cnt;
}

static void printUuid(const OicUuid_t* uid)
{
    for(int i=0; i<UUID_LENGTH; )
    {
        printf("%02X", (*uid).id[i++]);
        if(i==4 || i==6 || i==8 || i==10)  // canonical format for UUID has '8-4-4-4-12'
        {
            printf("-");
        }
    }
}

static FILE* fopen_prvnMng(const char* path, const char* mode)
{
    (void)path;  // unused |path| parameter

    // input |g_svr_db_fname| internally by force, not using |path| parameter
    // because |OCPersistentStorage::open| is called |OCPersistentStorage| internally
    // with its own |SVR_DB_FILE_NAME|
    return fopen(SVR_DB_FILE_NAME, mode);
}

static int waitCallbackRet(void)
{
    for(int i=0; !g_doneCB && CALLBACK_TIMEOUT>i; ++i)
    {
        sleep(1);
        if(OC_STACK_OK != OCProcess())
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return -1;
        }
    }

    return 0;
}

static int selectTwoDiffNum(int* a, int* b, const int max, const char* str)
{
    if(!a || !b || 2>=max || !str)
    {
        return -1;
    }

    for( ; ; )
    {
        for(int i=0; 2>i; ++i)
        {
            int* num = 0==i?a:b;
            for( ; ; )
            {
                printf("   > Enter Device[%d] Number, %s: ", i+1, str);
                for(int ret=0; 1!=ret; )
                {
                    ret = scanf("%d", num);
                    for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                                // '0x20<=code' is character region
                }
                if(0<*num && max>=*num)
                {
                    break;
                }
                printf("     Entered Wrong Number. Please Enter Again\n");
            }
        }
        if(*a != *b)
        {
            printf("\n");
            return 0;
        }
    }

    return -1;
}

static void printMenu(void)
{
    printf("************************************************************\n");
    printf("****** OIC Provisioning Client with using C-level API ******\n");
    printf("************************************************************\n\n");

    printf("** [A] DISCOVER DEVICES ON NETWORK\n");
    printf("** 10. Discover All Un/Owned Devices on Network\n");
    printf("** 11. Discover Only Unowned Devices on Network\n");
    printf("** 12. Discover Only Owned Devices on Network\n\n");

    printf("** [B] REGISTER/OWN ALL DISCOVERED UNOWNED DEVICES\n");
    printf("** 20. Register/Own All Discovered Unowned Devices\n\n");

    printf("** [C] PROVISION/LINK PAIRWISE THINGS\n");
    printf("** 30. Provision/Link Pairwise Things\n");
    printf("** 31. Provision Credentials for Pairwise Things\n");
    printf("** 32. Provision the Selected Access Control List(ACL)\n");
    printf("** 33. Check Linked Status of the Selected Device on PRVN DB\n\n");

    printf("** [D] UNLINK PAIRWISE THINGS\n");
    printf("** 40. Unlink Pairwise Things\n\n");

    printf("** [E] REMOVE THE SELECTED DEVICE\n");
    printf("** 50. Remove the Selected Device\n\n");

    printf("** [F] EXIT PROVISIONING CLIENT\n");
    printf("** 99. Exit Provisionong Client\n\n");

    printf("************************************************************\n\n");
}

#if 0 // Code for enabling path configuration for PDB and SVR DBf
static void printUsage(void)
{
    printf("\n");
    printf("OIC Provisioning Client with using C-level API\n");
    printf("Usage: provisioningclient [option]...\n");
    printf("\n");
    printf("  -h                           print help for this provisioning client\n");
    printf("  -p=[prvn_db_file_path/name]  input PRVN DB file path and name\n");
    printf("                               if not exists, will load default DB file\n");
    printf("                               (default: |oic_prvn_mng.db| on working dir)\n");
    printf("                               (ex. -p=oic_prvn_mng.db)\n");
    printf("  -s=[svr_db_file_path/name]   input SVR DB file path and name\n");
    printf("                               if not exists, will load default DB file\n");
    printf("                               (default: |oic_svr_db_client.json| on working dir)\n");
    printf("                               (ex. -s=oic_svr_db_client.json)\n");
    printf("\n");
}
#endif

// main function for provisioning client using C-level provisioning API
int main()
{
    // initialize provisioning client
    if(initProvisionClient())
    {
        OC_LOG(ERROR, TAG, "ProvisionClient init error");
        goto PMCLT_ERROR;
    }

    // main loop for provisioning manager
    int mn_num = 0;
    for( ; ; )
    {
        printf("\n");
        printMenu();
        printf(">> Enter Menu Number: ");
        for(int ret=0; 1!=ret; )
        {
            ret = scanf("%d", &mn_num);
            for( ; 0x20<=getchar(); );  // for removing overflow garbages
                                        // '0x20<=code' is character region
        }
        printf("\n");
        switch(mn_num)
        {
        case _10_DISCOV_ALL_DEVS_:
            if(discoverAllDevices())
            {
                OC_LOG(ERROR, TAG, "_10_DISCOV_ALL_DEVS_: error");
            }
            break;
        case _11_DISCOV_UNOWN_DEVS_:
            if(discoverUnownedDevices())
            {
                OC_LOG(ERROR, TAG, "_11_DISCOV_UNOWN_DEVS_: error");
            }
            break;
        case _12_DISCOV_OWN_DEVS_:
            if(discoverOwnedDevices())
            {
                OC_LOG(ERROR, TAG, "_12_DISCOV_OWN_DEVS_: error");
            }
            break;
        case _20_REGIST_DEVS_:
            if(registerDevices())
            {
                OC_LOG(ERROR, TAG, "_20_REGIST_DEVS_: error");
            }
            break;
        case _30_PROVIS_PAIR_DEVS_:
            if(provisionPairwise())
            {
                OC_LOG(ERROR, TAG, "_30_PROVIS_PAIR_DEVS_: error");
            }
            break;
        case _31_PROVIS_CRED_:
            if(provisionCred())
            {
                OC_LOG(ERROR, TAG, "_31_PROVIS_CRED_: error");
            }
            break;
        case _32_PROVIS_ACL_:
            if(provisionAcl())
            {
                OC_LOG(ERROR, TAG, "_32_PROVIS_ACL_: error");
            }
            break;
        case _33_CHECK_LINK_STATUS_:
            if(checkLinkedStatus())
            {
                OC_LOG(ERROR, TAG, "_33_CHECK_LINK_STATUS_: error");
            }
            break;
        case _40_UNLINK_PAIR_DEVS_:
            if(unlinkPairwise())
            {
                OC_LOG(ERROR, TAG, "_40_UNLINK_PAIR_DEVS_: error");
            }
            break;
        case _50_REMOVE_SELEC_DEV_:
            if(removeDevice())
            {
                OC_LOG(ERROR, TAG, "_50_REMOVE_SELEC_DEV_: error");
            }
            break;
        case _99_EXIT_PRVN_CLT_:
            goto PMCLT_ERROR;
        default:
            printf(">> Entered Wrong Number. Please Enter Again\n\n");
            break;
        }
    }

PMCLT_ERROR:
    if(OC_STACK_OK != OCStop())
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }
    OCDeleteDiscoveredDevices(g_own_list);  // after here |g_own_list| points nothing
    OCDeleteDiscoveredDevices(g_unown_list);  // after here |g_unown_list| points nothing

    if(g_svr_fname)
    {
        OICFree(g_svr_fname);  // after here |g_svr_fname| points nothing
    }
    if(g_prvn_fname)
    {
        OICFree(g_prvn_fname);  // after here |g_prvn_fname| points nothing
    }
    return 0;  // always return normal case
}

#ifdef __cplusplus
}
#endif //__cplusplus