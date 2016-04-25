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
******************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "oic_malloc.h"
#include "utlist.h"
#include "ocprovisioningmanager.h"
#include "secureresourceprovider.h"
#include "oxmjustworks.h"
#include "oic_string.h"
#include "securevirtualresourcetypes.h"
#include "cacommon.h"
#include "ck_manager.h"
#include "ckm_info.h"
#include "crlresource.h"

#define MAX_URI_LENGTH (64)
#define MAX_PERMISSION_LENGTH (5)
#define CREATE (1)
#define READ (2)
#define UPDATE (4)
#define DELETE (8)
#define NOTIFY (16)
#define DASH '-'
#define PREDEFINED_TIMEOUT (10)
#define MAX_OWNED_DEVICE (10)
#define DATE_LENGTH      (14)
#define TAG  "provisioningclient"

static OicSecAcl_t        *gAcl = NULL;
static OicSecCrl_t        *gCrl = NULL;
static char PROV_TOOL_DB_FILE[] = "oic_svr_db_pt.json";
static const char* PRVN_DB_FILE_NAME = "oic_prvn_mng.db";
static int gOwnershipState = 0;

typedef enum
{
    ownershipDone = 1 << 1,
    finalizeDone = 1 << 2,
    provisionAclDone = 1 << 3,
    provisionCert1Done = 1 << 4,
    provisionCert2Done = 1 << 5,
    provisionCrlDone = 1 << 6
} StateManager;


/**
 * Perform cleanup for ACL
 * @param[in]    ACL
 */
static void deleteACL(OicSecAcl_t *acl)
{
    if (acl)
    {
        /* Clean Resources */
        for (size_t i = 0; i < (acl)->resourcesLen; i++)
        {
            OICFree((acl)->resources[i]);
        }
        OICFree((acl)->resources);

        /* Clean Owners */
        OICFree((acl)->owners);

        /* Clean ACL node itself */
        OICFree((acl));

        acl = NULL;
    }
}

void deleteCrl(OicSecCrl_t *crl)
{
    if (crl)
    {
        //Clean ThisUpdate
        OICFree(crl->ThisUpdate.data);

        //clean CrlData
        OICFree(crl->CrlData.data);

        //Clean crl itself
        OICFree(crl);
    }
}

/**
 * Calculate ACL permission from string to bit
 *
 * @param[in] temp_psm    Input data of ACL permission string
 * @param[in,out] pms    The pointer of ACL permission value
 * @return  0 on success otherwise -1.
 */
static int CalculateAclPermission(const char *temp_pms, uint16_t *pms)
{
    int i = 0;

    if (NULL == temp_pms || NULL == pms)
    {
        return -1;
    }
    *pms = 0;
    while (temp_pms[i] != '\0')
    {
        switch (temp_pms[i])
        {
            case 'C':
                {
                    *pms += CREATE;
                    i++;
                    break;
                }
            case 'R':
                {
                    *pms += READ;
                    i++;
                    break;
                }
            case 'U':
                {
                    *pms += UPDATE;
                    i++;
                    break;
                }
            case 'D':
                {
                    *pms += DELETE;
                    i++;
                    break;
                }
            case 'N':
                {
                    *pms += NOTIFY;
                    i++;
                    break;
                }
            case '_':
                {
                    i++;
                    break;
                }
            default:
                {
                    return -1;
                }
        }
    }
    return 0;
}

/**
 * Get the ACL property from user
 *
 * @param[in]    ACL Datastructure to save user inputs
 * @return  0 on success otherwise -1.
 */
static int InputACL(OicSecAcl_t *acl)
{
    int ret;
    char temp_id [UUID_LENGTH + 4] = {0,};
    char temp_rsc[MAX_URI_LENGTH + 1] = {0,};
    char temp_pms[MAX_PERMISSION_LENGTH + 1] = {0,};
    printf("******************************************************************************\n");
    printf("-Set ACL policy for target device\n");
    printf("******************************************************************************\n");
    //Set Subject.
    printf("-URN identifying the subject\n");
    printf("ex) doorDeviceUUID00 (16 Numbers except to '-')\n");
    printf("Subject : ");
    char *ptr = NULL;
    ret = scanf("%19ms", &ptr);
    if(1==ret)
    {
        OICStrcpy(temp_id, sizeof(temp_id), ptr);
        OICFree(ptr);
    }
    else
    {
         printf("Error while input\n");
         return -1;
    }
    int j = 0;
    for (int i = 0; temp_id[i] != '\0'; i++)
    {
        if (DASH != temp_id[i])
        {
            if(j>UUID_LENGTH)
            {
                printf("Invalid input\n");
                return -1;
            }
            acl->subject.id[j++] = temp_id[i];
        }
    }

    //Set Resource.
    printf("Num. of Resource : \n");
    ret = scanf("%zu", &acl->resourcesLen);
    printf("-URI of resource\n");
    printf("ex) /a/light (Max_URI_Length: 64 Byte )\n");
    acl->resources = (char **)OICCalloc(acl->resourcesLen, sizeof(char *));
    if (NULL == acl->resources)
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
        return -1;
    }
    for (size_t i = 0; i < acl->resourcesLen; i++)
    {
        printf("[%zu]Resource : ", i + 1);
        char *ptr_tempRsc = NULL;
        ret = scanf("%64ms", &ptr_tempRsc);
        if (1==ret)
        {
            OICStrcpy(temp_rsc, sizeof(temp_rsc), ptr_tempRsc);
            OICFree(ptr_tempRsc);
        }
        else
        {
            printf("Error while input\n");
            return -1;
        }
        acl->resources[i] = OICStrdup(temp_rsc);

        if (NULL == acl->resources[i])
        {
            OC_LOG(ERROR, TAG, "Error while memory allocation");
            return -1;
        }
    }
    // Set Permission
    do
    {
        printf("-Set the permission(C,R,U,D,N)\n");
        printf("ex) CRUDN, CRU_N,..(5 Charaters)\n");
        printf("Permission : ");
        char *ptr_temp_pms = NULL;
        ret = scanf("%5ms", &ptr_temp_pms);
        if(1 == ret)
        {
            OICStrcpy(temp_pms, sizeof(temp_pms), ptr_temp_pms);
            OICFree(ptr_temp_pms);

        }
        else
        {
            printf("Error while input\n");
            return -1;
        }
    }
    while (0 != CalculateAclPermission(temp_pms, &(acl->permission)) );
    // Set Rowner
    printf("Num. of Rowner : ");
    ret = scanf("%zu", &acl->ownersLen);
    printf("-URN identifying the rowner\n");
    printf("ex) lightDeviceUUID0 (16 Numbers except to '-')\n");
    acl->owners = (OicUuid_t *)OICCalloc(acl->ownersLen, sizeof(OicUuid_t));
    if (NULL == acl->owners)
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
        return -1;
    }
    for (size_t i = 0; i < acl->ownersLen; i++)
    {
        printf("[%zu]Rowner : ", i + 1);
        char *ptr_temp_id = NULL;
        ret = scanf("%19ms", &ptr_temp_id);
        if (1 == ret)
        {
            OICStrcpy(temp_id, sizeof(temp_id), ptr_temp_id);
            OICFree(ptr_temp_id);
        }
        else
        {
            printf("Error while input\n");
            return -1;
        }
        j = 0;
        for (int k = 0; temp_id[k] != '\0'; k++)
        {
            if (DASH != temp_id[k])
            {
                acl->owners[i].id[j++] = temp_id[k];
            }
        }
    }
    return 0;
}


//FILE *client_fopen(const char *path, const char *mode)
FILE *client_fopen(const char* UNUSED_PARAM , const char *mode)
{
    (void)UNUSED_PARAM;
    return fopen(PROV_TOOL_DB_FILE, mode);
}

void PrintfResult(const char* procName, void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{
    printf("-----------------------------------------------------------\n");
    if(!hasError)
    {
        printf("%s was successfully done.\n", procName);
    }
    else
    {
        for(int i = 0; i < nOfRes; i++)
        {
            printf("UUID : ");
            for(int j = 0; j < UUID_LENGTH; j++)
            {
                printf("%c", arr[i].deviceId.id[j]);
            }
            printf("\t");
            printf("Result=%d\n", arr[i].res);
        }
    }

    if(ctx)
    {
        printf("Context is %s\n", (char*)ctx);
    }
    printf("-----------------------------------------------------------\n");
}

void ProvisionCertCB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{
    if(!hasError)
    {
        gOwnershipState = 1;
        PrintfResult("Provision Credential", ctx, nOfRes, arr, hasError);
    }
    else printf("Cert provisioning error\n-----------------------------------------");
}

void ProvisionAclCB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{
    if(!hasError)
    {
        gOwnershipState = 1;
        PrintfResult("Provision ACL", ctx, nOfRes, arr, hasError);
    }
}

void ProvisionCrlCB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{
    if(!hasError)
    {
        gOwnershipState = 1;
        PrintfResult("Provision CRL", ctx, nOfRes, arr, hasError);
    }
}



void OwnershipTransferCB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{
    if(!hasError)
    {
        gOwnershipState = 1;
        PrintfResult("Ownership transfer", ctx, nOfRes, arr, hasError);
    }
}

static short IsCKMInfoFileExists()
{
    FILE *ckmInf = fopen(CA_STORAGE_FILE, "r");
    if (NULL != ckmInf)
    {
        fclose(ckmInf);
        return 1;
    }
    return 0;
}

static PKIError InitCA()
{
    FUNCTION_INIT();

    if (IsCKMInfoFileExists())
    {
        CHECK_CALL(InitCKMInfo);
    }
    else
    {
        ByteArray rootName  = BYTE_ARRAY_INITIALIZER;
        ByteArray CAPubKey  = BYTE_ARRAY_INITIALIZER;
        ByteArray CAPrivKey = BYTE_ARRAY_INITIALIZER;
        ByteArray rootCert  = BYTE_ARRAY_INITIALIZER;


        uint8_t rootCertData[ISSUER_MAX_CERT_SIZE];
        uint8_t CAPubKeyData[PUBLIC_KEY_SIZE];
        uint8_t CAPrivKeyData[PRIVATE_KEY_SIZE];
        const char rootNameStr[] = "Sample_Root";

        CAPubKey.data  = CAPubKeyData;
        CAPubKey.len   = PUBLIC_KEY_SIZE;
        CAPrivKey.data = CAPrivKeyData;
        CAPrivKey.len  = PRIVATE_KEY_SIZE;
        rootCert.data  = rootCertData;
        rootCert.len   = ISSUER_MAX_CERT_SIZE;
        rootName.data  = (uint8_t *)rootNameStr;
        rootName.len   = strlen(rootNameStr);

        CHECK_CALL(SetRootName, rootName);
        CHECK_CALL(GenerateCAKeyPair, &CAPrivKey, &CAPubKey);
        CHECK_CALL(SetSerialNumber, 1);
        CHECK_CALL(CKMIssueRootCertificate, NULL, NULL, &rootCert);
        CHECK_CALL(SetCACertificate, &rootCert);
    }

    FUNCTION_CLEAR();
}

static int InputCRL(OicSecCrl_t *crlRes)
{
    FUNCTION_INIT(
            ByteArray crl = BYTE_ARRAY_INITIALIZER;
            );

    const int MAX_Revoked_NUMBER = 9;
    uint8_t uint8ThisUpdateTime[DATE_LENGTH] = "130101000005Z";
    uint32_t revokedNumbers[MAX_Revoked_NUMBER];
    const uint8_t* revocationDates[MAX_Revoked_NUMBER];
   // const uint8_t revocationDatesContent[MAX_Revoked_NUMBER][DATE_LENGTH];
    uint32_t nuberOfRevoked = 0;
    printf("Enter number of Revoked certificates(1..%d)\n", MAX_Revoked_NUMBER);
    scanf("%u", &nuberOfRevoked);

    for (size_t i = 0; i < nuberOfRevoked; ++i)
    {
        printf("Revoked certificate %d:", i);
        printf("Serial number (E. g.: 100):");
        scanf("%u", &revokedNumbers[i]);
        revocationDates[i] = (const uint8_t*)"130101000005Z";
    }

    crl.len = CRL_MIN_SIZE + nuberOfRevoked * (sizeof(CertificateRevocationInfo_t) + 4)/* + 1000*/;
    crl.data = (uint8_t *)OICCalloc(1, crl.len);

    CHECK_CALL(CKMIssueCRL, uint8ThisUpdateTime, nuberOfRevoked, revokedNumbers,
            revocationDates, &crl);
    PRINT_BYTE_ARRAY("CRL:\n",crl);
    CHECK_CALL(SetCertificateRevocationList, &crl);
    crlRes->CrlData = crl;
    crlRes->ThisUpdate.data = uint8ThisUpdateTime;
    crlRes->ThisUpdate.len = DATE_LENGTH;
    crlRes->CrlId = 1;


    FUNCTION_CLEAR(
    //OICFree(crl.data);
            );
}


/**
 * Provisioning client sample using ProvisioningAPI
 */
int main()
{
    OCStackResult res = OC_STACK_OK;

    // Initialize Persistent Storage for SVR database
    OCPersistentStorage ps = { .open = client_fopen,
                               .read = fread,
                               .write = fwrite,
                               .close = fclose,
                               .unlink = unlink};

    OCRegisterPersistentStorageHandler(&ps);

    if (OC_STACK_OK != OCInit(NULL, 0, OC_CLIENT_SERVER))
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        goto error;
    }
    if(OC_STACK_OK != OCInitPM(PRVN_DB_FILE_NAME))
    {
        OC_LOG(ERROR, TAG, "OC_PM init error");
        goto error;
    }

    OCProvisionDev_t* pDeviceList = NULL;
    res = OCDiscoverUnownedDevices(PREDEFINED_TIMEOUT, &pDeviceList);
    if(OC_STACK_OK != res)
    {
        OC_LOG_V(ERROR, TAG, "Failed to PMDeviceDiscovery : %d", res);
        goto error;
    }

    OCProvisionDev_t* pCurDev = pDeviceList;
    int i;
    while(pCurDev !=NULL)
    {
        for(i = 0; i < UUID_LENGTH; i++)
        {
            printf("%c", pCurDev->doxm->deviceID.id[i]);
        }
        printf("\n");
        pCurDev = pCurDev->next;
    }

    //Register callback function to each OxM
    OTMCallbackData_t justWorksCBData = {.loadSecretCB=NULL,
                                         .createSecureSessionCB=NULL,
                                         .createSelectOxmPayloadCB=NULL,
                                         .createOwnerTransferPayloadCB=NULL};
    justWorksCBData.loadSecretCB = LoadSecretJustWorksCallback;
    justWorksCBData.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
    justWorksCBData.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
    justWorksCBData.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;
    OTMSetOwnershipTransferCallbackData(OIC_JUST_WORKS, &justWorksCBData);

    char* myContext = "OTM Context";
    //Perform ownership transfer
    res = OCDoOwnershipTransfer((void*)myContext, pDeviceList, OwnershipTransferCB);
    if(OC_STACK_OK == res)
    {
        OC_LOG(INFO, TAG, "Request for ownership transfer is sent successfully.");
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Failed to OCDoOwnershipTransfer : %d", res);
    }

    gOwnershipState = 0;
    while (gOwnershipState == 0)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            goto error;
        }
        sleep(1);
    }

// Credential & ACL provisioning between two devices.

    OCProvisionDev_t *pOwnedList = NULL;
    OCProvisionDev_t *pOwnedDevices [MAX_OWNED_DEVICE] = {0,};
    int nOwnedDevice = 0;

    res = OCDiscoverOwnedDevices(PREDEFINED_TIMEOUT, &pOwnedList);
    if (OC_STACK_OK == res)
    {
        printf("################## Owned Device List #######################\n");
        while (pOwnedList != NULL)
        {
            nOwnedDevice ++;
            printf(" %d : ", nOwnedDevice);
            for (int i = 0; i < UUID_LENGTH; i++)
            {
                printf("%c", pOwnedList->doxm->deviceID.id[i]);
            }
            printf("\n");
            pOwnedDevices[nOwnedDevice] = pOwnedList;
            pOwnedList = pOwnedList->next;
        }
    }
    else
    {
        OC_LOG(ERROR, TAG, "Error while Owned Device Discovery");
    }

    int Device1 = 0;
    int Device2 = 0;

    printf("Select 2 devices for Credential & ACL provisioning\n");
    printf("Device 1: ");
    scanf("%d", &Device1);
    printf("Device 2: ");
    scanf("%d", &Device2);


    gAcl = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
    if (NULL == gAcl)
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
        goto error;
    }

    if (PKI_SUCCESS != InitCA())
    {
        OC_LOG(ERROR, TAG, "CA init error");
        goto error;
    }


    char *ctx = "DUMMY";

    res = OCProvisionCredentials(ctx, SIGNED_ASYMMETRIC_KEY, 0, pOwnedDevices[Device1],
                                                                NULL, ProvisionCertCB);
    if (OC_STACK_OK != res) OC_LOG_V(ERROR, TAG, "Failed to provision Device 1 : %d", res);
    gOwnershipState = 0;
    while ( gOwnershipState == 0 )
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            goto error;
        }
        sleep(1);
    }

    res = OCProvisionCredentials(ctx, SIGNED_ASYMMETRIC_KEY, 0, pOwnedDevices[Device2],
                                                                NULL, ProvisionCertCB);
    if (OC_STACK_OK != res)
    {
        OC_LOG_V(ERROR, TAG, "Failed to provision Device 2 : %d", res);
    }

    gOwnershipState = 0;
    while (gOwnershipState == 0)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            goto error;
        }
        sleep(1);
    }

    printf("Input ACL for Device2\n");
    if (0 == InputACL(gAcl))
    {
        printf("Success Input ACL\n");
    }
    else
    {
        OC_LOG(ERROR, TAG, "InputACL error");
        goto error;
    }
    res = OCProvisionACL(ctx, pOwnedDevices[Device2], gAcl, &ProvisionAclCB);
    if (OC_STACK_OK != res)
    {
        OC_LOG_V(ERROR, TAG, "Failed to ACL provision Device 2 : %d", res);
    }

    gOwnershipState = 0;
    while (gOwnershipState == 0)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            goto error;
        }
        sleep(1);
    }
    gCrl = (OicSecCrl_t *)OICMalloc(sizeof(OicSecCrl_t));
    if (PKI_SUCCESS != InputCRL(gCrl))
    {
        OC_LOG(ERROR, TAG, "CA init error");
        goto error;
    }

    PRINT_BYTE_ARRAY("gCrl = \n", gCrl->CrlData);

    res = OCProvisionCRL(ctx, pOwnedDevices[Device2], gCrl, &ProvisionCrlCB);
    if (OC_STACK_OK != res) OC_LOG_V(ERROR, TAG, "Failed to CRL provision Device 2 : %d", res);

    gOwnershipState = 0;
    while (gOwnershipState == 0)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            goto error;
        }
        sleep(1);
    }

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
        goto error;
    }

error:
    deleteACL(gAcl);
    OCDeleteDiscoveredDevices(pDeviceList);
    OCDeleteDiscoveredDevices(pOwnedList);

    return 0;
}
