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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "ocprovisioningmanager.h"
#include "secureresourceprovider.h"
#include "logger.h"
#include "oic_malloc.h"
#include "aclresource.h"
#include "pstatresource.h"
#include "srmresourcestrings.h"
#include "credresource.h"
#include "doxmresource.h"
#include "credentialgenerator.h"
#include "cainterface.h"
#include "cJSON.h"
#include "pmtypes.h"
#include "pmutility.h"
#include "provisioningdatabasemanager.h"
#include "base64.h"
#include "utlist.h"

#ifdef __WITH_X509__
#include "crlresource.h"
#endif // WITH_X509__

#define TAG "SRPAPI"

/**
 * Macro to verify argument is not equal to NULL.
 * eg: VERIFY_NON_NULL(TAG, ptrData, ERROR,OC_STACK_ERROR);
 */
#define VERIFY_NON_NULL(tag, arg, logLevel, retValue) { if (NULL == (arg)) \
            { OC_LOG((logLevel), tag, #arg " is NULL"); return retValue; } }

/**
 * Macro to verify success of operation.
 * eg: VERIFY_SUCCESS(TAG, OC_STACK_OK == foo(), ERROR, OC_STACK_ERROR);
 */
#define VERIFY_SUCCESS(tag, op, logLevel, retValue) { if (!(op)) \
            {OC_LOG((logLevel), tag, #op " failed!!"); return retValue;} }

/**
 * Structure to carry credential data to callback.
 */
typedef struct CredentialData CredentialData_t;
struct CredentialData
{
    void *ctx;                                  /**< Pointer to user context.**/
    const OCProvisionDev_t *deviceInfo1;        /**< Pointer to OCProvisionDev_t.**/
    const OCProvisionDev_t *deviceInfo2;        /**< Pointer to OCProvisionDev_t.**/
    OicSecCred_t *credInfo;                     /**< Pointer to OicSecCred_t.**/
    OicSecCred_t *credInfoFirst;                /**< Pointer to OicSecCred_t.**/
    OCProvisionResultCB resultCallback;         /**< Pointer to result callback.**/
    OCProvisionResult_t *resArr;                /**< Result array.**/
    int numOfResults;                           /**< Number of results in result array.**/
};

/**
 * Structure to carry ACL provision API data to callback.
 */
typedef struct ACLData ACLData_t;
struct ACLData
{
    void *ctx;                                  /**< Pointer to user context.**/
    const OCProvisionDev_t *deviceInfo;         /**< Pointer to PMDevInfo_t.**/
    OCProvisionResultCB resultCallback;         /**< Pointer to result callback.**/
    OCProvisionResult_t *resArr;                /**< Result array.**/
    int numOfResults;                           /**< Number of results in result array.**/
};

// Enum type index for unlink callback.
typedef enum {
    IDX_FIRST_DEVICE_RES = 0, // index for resulf of the first device
    IDX_SECOND_DEVICE_RES,    // index for result of the second device
    IDX_DB_UPDATE_RES         // index for result of updating provisioning database.
} IdxUnlinkRes_t;

// Structure to carry unlink APIs data to callback.
typedef struct UnlinkData UnlinkData_t;
struct UnlinkData {
    void *ctx;
    OCProvisionDev_t* unlinkDev;             /**< Pointer to OCProvisionDev_t to be unlinked.**/
    OCProvisionResult_t* unlinkRes;          /**< Result array.**/
    OCProvisionResultCB resultCallback;      /**< Pointer to result callback.**/
    int numOfResults;                        /**< Number of results in result array.**/
};

//Example of DELETE cred request -> coaps://0.0.0.0:5684/oic/sec/cred?sub=(BASE64 ENCODED UUID)
const char * SRP_FORM_DELETE_CREDENTIAL = "coaps://[%s]:%d%s?%s=%s";

// Structure to carry remove APIs data to callback.
typedef struct RemoveData RemoveData_t;
struct RemoveData {
    void *ctx;
    OCProvisionDev_t* revokeTargetDev;      /**< Device which is going to be revoked..**/
    OCProvisionDev_t* linkedDevList;        /**< A list of devices which have invalid credential.**/
    OCProvisionResult_t* removeRes;         /**< Result array.**/
    OCProvisionResultCB resultCallback;     /**< Pointer to result callback.**/
    size_t numOfResults;                    /**< Number of results in result array.**/
    size_t sizeOfResArray;
    bool hasError;
};

/**
 * Function prototype
 */
static OCStackResult provisionCredentials(const OicSecCred_t *cred,
        const OCProvisionDev_t *deviceInfo, CredentialData_t *credData,
        OCClientResponseHandler responseHandler);


/**
 * Internal function to update result in result array.
 */
static void registerResultForCredProvisioning(CredentialData_t *credData,
                                              OCStackResult stackresult, int cause)
{

   OC_LOG_V(INFO,TAG,"value of credData->numOfResults is %d",credData->numOfResults);
   if(1 == cause)
   {
       memcpy(credData->resArr[(credData->numOfResults)].deviceId.id,
              credData->deviceInfo1->doxm->deviceID.id,UUID_LENGTH);
   }
   else
   {
       memcpy(credData->resArr[(credData->numOfResults)].deviceId.id,
              credData->deviceInfo2->doxm->deviceID.id,UUID_LENGTH);
   }
   credData->resArr[(credData->numOfResults)].res = stackresult;
   ++(credData->numOfResults);
}

/**
 * Callback handler for handling callback of provisioning device 2.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult provisionCredentialCB2(void *ctx, OCDoHandle UNUSED,
                                                       OCClientResponse *clientResponse)
{
    VERIFY_NON_NULL(TAG, ctx, ERROR, OC_STACK_DELETE_TRANSACTION);
    CredentialData_t *credData = (CredentialData_t *) ctx;
    (void)UNUSED;

    OCProvisionResultCB resultCallback = credData->resultCallback;
    OC_LOG(INFO, TAG, "provisionCredentialCB2 called");
    if (clientResponse)
    {
        if(OC_STACK_RESOURCE_CREATED == clientResponse->result)
        {
            registerResultForCredProvisioning(credData, OC_STACK_RESOURCE_CREATED, 2);
            OCStackResult res =  PDMLinkDevices(&credData->deviceInfo1->doxm->deviceID,
                    &credData->deviceInfo2->doxm->deviceID);
            if (OC_STACK_OK != res)
            {
                OC_LOG(ERROR, TAG, "Error occured on PDMLinkDevices");
                return OC_STACK_DELETE_TRANSACTION;
            }
            OC_LOG(INFO, TAG, "Link created successfully");

            ((OCProvisionResultCB)(resultCallback))(credData->ctx, credData->numOfResults,
                                                    credData->resArr,
                                                    false);
             OICFree(credData->resArr);
             OICFree(credData);
             return OC_STACK_DELETE_TRANSACTION;
        }

    }
    OC_LOG(INFO, TAG, "provisionCredentialCB2 received Null clientResponse");
    registerResultForCredProvisioning(credData, OC_STACK_ERROR, 2);
    ((OCProvisionResultCB)(resultCallback))(credData->ctx, credData->numOfResults,
                                            credData->resArr,
                                            true);
    OICFree(credData->resArr);
    OICFree(credData);
    return OC_STACK_DELETE_TRANSACTION;
}

/**
 * Callback handler for handling callback of provisioning device 1.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult provisionCredentialCB1(void *ctx, OCDoHandle UNUSED,
                                                       OCClientResponse *clientResponse)
{
    VERIFY_NON_NULL(TAG, ctx, ERROR, OC_STACK_DELETE_TRANSACTION);
    (void)UNUSED;
    CredentialData_t* credData = (CredentialData_t*) ctx;
    OICFree(credData->credInfoFirst);
    const OCProvisionDev_t *deviceInfo = credData->deviceInfo2;
    OicSecCred_t *credInfo = credData->credInfo;
    const OCProvisionResultCB resultCallback = credData->resultCallback;
    if (clientResponse)
    {
        if (OC_STACK_RESOURCE_CREATED == clientResponse->result)
        {
            // send credentials to second device
            registerResultForCredProvisioning(credData, OC_STACK_RESOURCE_CREATED,1);
            OCStackResult res = provisionCredentials(credInfo, deviceInfo, credData,
                    provisionCredentialCB2);
            DeleteCredList(credInfo);
            if (OC_STACK_OK != res)
            {
                registerResultForCredProvisioning(credData, res,2);
                ((OCProvisionResultCB)(resultCallback))(credData->ctx, credData->numOfResults,
                                                        credData->resArr,
                                                        true);
                OICFree(credData->resArr);
                OICFree(credData);
                credData = NULL;
            }
        }
        else
        {
            registerResultForCredProvisioning(credData, OC_STACK_ERROR,1);
            ((OCProvisionResultCB)(resultCallback))(credData->ctx, credData->numOfResults,
                                                    credData->resArr,
                                                    true);
            OICFree(credData->resArr);
            OICFree(credData);
            credData = NULL;
        }
    }
    else
    {
        OC_LOG(INFO, TAG, "provisionCredentialCB received Null clientResponse for first device");
        registerResultForCredProvisioning(credData, OC_STACK_ERROR,1);
       ((OCProvisionResultCB)(resultCallback))(credData->ctx, credData->numOfResults,
                                                     credData->resArr,
                                                     true);
        DeleteCredList(credInfo);
        OICFree(credData->resArr);
        OICFree(credData);
        credData = NULL;
    }
    return OC_STACK_DELETE_TRANSACTION;
}



/**
 * Internal function for handling credential generation and sending credential to resource server.
 *
 * @param[in] cred Instance of cred resource.
 * @param[in] deviceInfo information about device to which credential is to be provisioned.
 * @param[in] responseHandler callbak called by OC stack when request API receives response.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
static OCStackResult provisionCredentials(const OicSecCred_t *cred,
        const OCProvisionDev_t *deviceInfo, CredentialData_t *credData,
        OCClientResponseHandler responseHandler)
{
    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = BinToCredJSON(cred);
    if(NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Failed to BinToCredJSON");
        return OC_STACK_NO_MEMORY;
    }

    OC_LOG_V(INFO, TAG, "Credential for provisioning : %s",secPayload->securityData);
    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(true,
                        deviceInfo->endpoint.addr,
                        deviceInfo->securePort,
                        deviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_CRED_URI))
    {
        OC_LOG(ERROR, TAG, "DeviceDiscoveryHandler : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    OCCallbackData cbData = {.context=NULL, .cb=NULL, .cd=NULL};
    cbData.cb = responseHandler;
    cbData.context = (void *) credData;
    cbData.cd = NULL;

    OCDoHandle handle = NULL;
    OCMethod method = OC_REST_POST;
    OCStackResult ret = OCDoResource(&handle, method, query, 0, (OCPayload*)secPayload,
            deviceInfo->connType, OC_HIGH_QOS, &cbData, NULL, 0);
    OC_LOG_V(INFO, TAG, "OCDoResource::Credential provisioning returned : %d",ret);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
        return ret;
    }
    return OC_STACK_OK;
}

#ifdef __WITH_X509__
/**
 * Structure to carry certificate data to callback.
 */
typedef struct CertificateData CertData_t;
struct CertificateData
{
    void *ctx;                                  /**< Pointer to user context.**/
    const OCProvisionDev_t *deviceInfo;        /**< Pointer to OCProvisionDev_t.**/
    OicSecCred_t *credInfo;                     /**< Pointer to OicSecCred_t.**/
    OCProvisionResultCB resultCallback;         /**< Pointer to result callback.**/
    OCProvisionResult_t *resArr;                /**< Result array.**/
    int numOfResults;                           /**< Number of results in result array.**/
};

/**
 * Structure to carry CRL provision API data to callback.
 */
typedef struct CRLData CRLData_t;
struct CRLData
{
    void *ctx;                                  /**< Pointer to user context.**/
    const OCProvisionDev_t *deviceInfo;         /**< Pointer to PMDevInfo_t.**/
    OCProvisionResultCB resultCallback;         /**< Pointer to result callback.**/
    OCProvisionResult_t *resArr;                /**< Result array.**/
    int numOfResults;                           /**< Number of results in result array.**/
};

/**
 * Internal function to update result in result array.
 */
static void registerResultForCertProvisioning(CertData_t *certData,
                                              OCStackResult stackresult)
{

   OC_LOG_V(INFO,TAG,"value of credData->numOfResults is %d",certData->numOfResults);
   memcpy(certData->resArr[(certData->numOfResults)].deviceId.id,
          certData->deviceInfo->doxm->deviceID.id,UUID_LENGTH);
   certData->resArr[(certData->numOfResults)].res = stackresult;
   ++(certData->numOfResults);
}

/**
 * Internal Function to store results in result array during ACL provisioning.
 */
static void registerResultForCRLProvisioning(CRLData_t *crlData,
                                             OCStackResult stackresult)
{
   OC_LOG_V(INFO, TAG, "Inside registerResultForCRLProvisioning crlData->numOfResults is %d\n",
                       crlData->numOfResults);
   memcpy(crlData->resArr[(crlData->numOfResults)].deviceId.id,
          crlData->deviceInfo->doxm->deviceID.id, UUID_LENGTH);
   crlData->resArr[(crlData->numOfResults)].res = stackresult;
   ++(crlData->numOfResults);
}


/**
 * Callback handler of SRPProvisionCRL.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult SRPProvisionCRLCB(void *ctx, OCDoHandle UNUSED,
                                                  OCClientResponse *clientResponse)
{
    OC_LOG_V(INFO, TAG, "Inside SRPProvisionCRLCB.");
    (void)UNUSED;
    VERIFY_NON_NULL(TAG, ctx, ERROR, OC_STACK_DELETE_TRANSACTION);
    CRLData_t *crlData = (CRLData_t*)ctx;
    OCProvisionResultCB resultCallback = crlData->resultCallback;

    if (clientResponse)
    {
        if(OC_STACK_RESOURCE_CREATED == clientResponse->result)
        {
            registerResultForCRLProvisioning(crlData, OC_STACK_RESOURCE_CREATED);
            ((OCProvisionResultCB)(resultCallback))(crlData->ctx, crlData->numOfResults,
                                                    crlData->resArr,
                                                    false);
             OICFree(crlData->resArr);
             OICFree(crlData);
             return OC_STACK_DELETE_TRANSACTION;
        }
    }
    registerResultForCRLProvisioning(crlData, OC_STACK_ERROR);
    ((OCProvisionResultCB)(resultCallback))(crlData->ctx, crlData->numOfResults,
                                            crlData->resArr,
                                            true);
    OC_LOG_V(ERROR, TAG, "SRPProvisionCRLCB received Null clientResponse");
    OICFree(crlData->resArr);
    OICFree(crlData);
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackResult SRPProvisionCRL(void *ctx, const OCProvisionDev_t *selectedDeviceInfo,
        OicSecCrl_t *crl, OCProvisionResultCB resultCallback)
{
    VERIFY_NON_NULL(TAG, selectedDeviceInfo, ERROR,  OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(TAG, crl, ERROR,  OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(TAG, resultCallback, ERROR,  OC_STACK_INVALID_CALLBACK);

    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if (!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }

    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = BinToCrlJSON(crl);
    if (NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Failed to BinToCrlJSON");
        return OC_STACK_NO_MEMORY;
    }
    OC_LOG_V(INFO, TAG, "CRL : %s", secPayload->securityData);

    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(true,
                        selectedDeviceInfo->endpoint.addr,
                        selectedDeviceInfo->securePort,
                        selectedDeviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_CRL_URI))
    {
        OC_LOG(ERROR, TAG, "DeviceDiscoveryHandler : Failed to generate query");
        OICFree(secPayload->securityData);
        OICFree(secPayload);
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    OCCallbackData cbData =  {.context=NULL, .cb=NULL, .cd=NULL};
    cbData.cb = &SRPProvisionCRLCB;
    CRLData_t *crlData = (CRLData_t *) OICCalloc(1, sizeof(CRLData_t));
    if (crlData == NULL)
    {
        OICFree(secPayload->securityData);
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Unable to allocate memory");
        return OC_STACK_NO_MEMORY;
    }

    crlData->deviceInfo = selectedDeviceInfo;
    crlData->resultCallback = resultCallback;
    crlData->numOfResults=0;
    crlData->ctx = ctx;

    crlData->resArr = (OCProvisionResult_t*)OICCalloc(1, sizeof(OCProvisionResult_t));
    if (crlData->resArr == NULL)
    {
        OICFree(secPayload->securityData);
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Unable to allocate memory");
        return OC_STACK_NO_MEMORY;
    }

    cbData.context = (void *)crlData;
    cbData.cd = NULL;
    OCMethod method = OC_REST_POST;
    OCDoHandle handle = NULL;
    OC_LOG(DEBUG, TAG, "Sending CRL info to resource server");

    OCStackResult ret = OCDoResource(&handle, method, query,
            &selectedDeviceInfo->endpoint, (OCPayload*)secPayload,
            selectedDeviceInfo->connType, OC_HIGH_QOS, &cbData, NULL, 0);

    if (ret != OC_STACK_OK)
    {
        OICFree(crlData->resArr);
        OICFree(crlData);
    }

    return ret;
}

/**
 * Internal function for handling credential generation and sending cretificate credential.
 *
 * @param[in] cred Instance of cred resource.
 * @param[in] deviceInfo information about device to which credential is to be provisioned.
 * @param[in] responseHandler callbak called by OC stack when request API receives response.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
static OCStackResult provisionCertCred(const OicSecCred_t *cred,
        const OCProvisionDev_t *deviceInfo, CertData_t *certData,
        OCClientResponseHandler responseHandler)
{
    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = BinToCredJSON(cred);

    if (NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Failed to BinToCredJSON");
        return OC_STACK_NO_MEMORY;
    }

    OC_LOG_V(INFO, TAG, "Credential for provisioning : %s",secPayload->securityData);
    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(true,
                        deviceInfo->endpoint.addr,
                        deviceInfo->securePort,
                        deviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_CRED_URI))
    {
        OC_LOG(ERROR, TAG, "DeviceDiscoveryHandler : Failed to generate query");
        OICFree(secPayload->securityData);
        OICFree(secPayload);
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    OCCallbackData cbData = {.context=NULL, .cb=NULL, .cd=NULL};
    cbData.cb = responseHandler;
    cbData.context = (void *) certData;
    cbData.cd = NULL;

    OCDoHandle handle = NULL;
    OCMethod method = OC_REST_POST;
    OCStackResult ret = OCDoResource(&handle, method, query, 0, (OCPayload*)secPayload,
            deviceInfo->connType, OC_HIGH_QOS, &cbData, NULL, 0);
    OC_LOG_V(INFO, TAG, "OCDoResource::Certificate provisioning returned : %d",ret);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }

    return ret;
}

/**
 * Callback handler for handling callback of certificate provisioning device.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult provisionCertCB(void *ctx, OCDoHandle UNUSED,
                                                       OCClientResponse *clientResponse)
{
    VERIFY_NON_NULL(TAG, ctx, ERROR, OC_STACK_DELETE_TRANSACTION);
    CertData_t *certData = (CertData_t *) ctx;
    (void)UNUSED;

    OCProvisionResultCB resultCallback = certData->resultCallback;
    OC_LOG(INFO, TAG, "provisionCertCred called");
    if (clientResponse)
    {
        if(OC_STACK_RESOURCE_CREATED == clientResponse->result)
        {
            registerResultForCertProvisioning(certData, OC_STACK_RESOURCE_CREATED);
            ((OCProvisionResultCB)(resultCallback))(certData->ctx, certData->numOfResults,
                                                    certData->resArr,
                                                    false);
             OICFree(certData->resArr);
             OICFree(certData);
             return OC_STACK_DELETE_TRANSACTION;
        }

    }
    OC_LOG(INFO, TAG, "provisionCertCredCB received Null clientResponse");
    registerResultForCertProvisioning(certData, OC_STACK_ERROR);
    ((OCProvisionResultCB)(resultCallback))(certData->ctx, certData->numOfResults,
                                            certData->resArr,
                                            true);
    OICFree(certData->resArr);
    OICFree(certData);
    return OC_STACK_DELETE_TRANSACTION;
}
#endif // __WITH_X509__

OCStackResult SRPProvisionCredentials(void *ctx, OicSecCredType_t type, size_t keySize,
                                      const OCProvisionDev_t *pDev1,
                                      const OCProvisionDev_t *pDev2,
                                      OCProvisionResultCB resultCallback)
{
    VERIFY_NON_NULL(TAG, pDev1, ERROR,  OC_STACK_INVALID_PARAM);
    if (SYMMETRIC_PAIR_WISE_KEY == type)
    {
        VERIFY_NON_NULL(TAG, pDev2, ERROR,  OC_STACK_INVALID_PARAM);
    }
    VERIFY_NON_NULL(TAG, resultCallback, ERROR,  OC_STACK_INVALID_CALLBACK);

    if (SYMMETRIC_PAIR_WISE_KEY == type &&
       !(OWNER_PSK_LENGTH_128 == keySize || OWNER_PSK_LENGTH_256 == keySize))
    {
        OC_LOG(INFO, TAG, "Invalid key size");
        return OC_STACK_INVALID_PARAM;
    }

    OC_LOG(INFO, TAG, "In SRPProvisionCredentials");

    if (SYMMETRIC_PAIR_WISE_KEY == type)
    {
        bool linkExisits = true;
        OCStackResult res = PDMIsLinkExists(&pDev1->doxm->deviceID, &pDev2->doxm->deviceID, &linkExisits);

        if (res != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "Internal error occured");
            return res;
        }
        if (linkExisits)
        {
            OC_LOG(ERROR, TAG, "Link already exists");
            return OC_STACK_INVALID_PARAM;
        }
    }

    OicUuid_t provTooldeviceID =   {{0,}};
    if (OC_STACK_OK != GetDoxmDeviceID(&provTooldeviceID))
    {
        OC_LOG(ERROR, TAG, "Error while retrieving provisioning tool's device ID");
        return OC_STACK_ERROR;
    }
    OC_LOG(INFO, TAG, "retrieved deviceid");
    switch (type)
    {
        case SYMMETRIC_PAIR_WISE_KEY:
        {
            const OCProvisionDev_t *firstDevice = pDev1;
            const OCProvisionDev_t *secondDevice = pDev2;

            OicSecCred_t *firstCred = NULL;
            OicSecCred_t *secondCred = NULL;
            OCStackResult res = PMGeneratePairWiseCredentials(type, keySize, &provTooldeviceID,
                    &firstDevice->doxm->deviceID, &secondDevice->doxm->deviceID,
                    &firstCred, &secondCred);
            VERIFY_SUCCESS(TAG, (res==OC_STACK_OK), ERROR, OC_STACK_ERROR);
            OC_LOG(INFO, TAG, "Credentials generated successfully");
            CredentialData_t *credData =
                (CredentialData_t *) OICCalloc(1, sizeof(CredentialData_t));
            if (NULL == credData)
            {
                OICFree(firstCred);
                OICFree(secondCred);
                OC_LOG(ERROR, TAG, "Memory allocation problem");
                return OC_STACK_NO_MEMORY;
            }
            credData->deviceInfo1 = firstDevice;
            credData->deviceInfo2 = secondDevice;
            credData->credInfo = secondCred;
            credData->ctx = ctx;
            credData->credInfoFirst = firstCred;
            credData->numOfResults = 0;
            credData->resultCallback = resultCallback;
            // first call to provision creds to device1.
            // second call to provision creds to device2.
            int noOfRiCalls = 2;
            credData->resArr =
                (OCProvisionResult_t*)OICCalloc(noOfRiCalls, sizeof(OCProvisionResult_t));
            if (NULL == credData->resArr)
            {
                OICFree(firstCred);
                OICFree(secondCred);
                OICFree(credData);
                OC_LOG(ERROR, TAG, "Memory allocation problem");
                return OC_STACK_NO_MEMORY;
            }
            res = provisionCredentials(firstCred, firstDevice, credData, &provisionCredentialCB1);
            if (OC_STACK_OK != res)
            {
                DeleteCredList(firstCred);
                DeleteCredList(secondCred);
                OICFree(credData->resArr);
                OICFree(credData);
            }
            OC_LOG_V(INFO, TAG, "provisionCredentials returned: %d",res);
            VERIFY_SUCCESS(TAG, (res==OC_STACK_OK), ERROR, OC_STACK_ERROR);
            return res;
        }
#ifdef __WITH_X509__
        case SIGNED_ASYMMETRIC_KEY:
        {
            const OCProvisionDev_t *firstDevice = pDev1;
            OicSecCred_t *cred = NULL;
            OCStackResult res = PMGenerateCertificateCredentials(&provTooldeviceID,
                                                                &firstDevice->doxm->deviceID,&cred);
            VERIFY_SUCCESS(TAG, (res==OC_STACK_OK), ERROR, OC_STACK_ERROR);
            OC_LOG(INFO, TAG, "Certificate credentials generated successfully");
            CertData_t *certData = (CertData_t *) OICCalloc(1, sizeof(CertData_t));
            if (NULL == certData)
            {
                OICFree(cred);
                OC_LOG(ERROR, TAG, "Memory allocation problem");
                return OC_STACK_NO_MEMORY;
            }

            certData->deviceInfo = firstDevice;
            certData->ctx = ctx;
            certData->credInfo = cred;
            certData->numOfResults = 0;
            certData->resultCallback = resultCallback;

            certData->resArr = (OCProvisionResult_t*)OICCalloc(1, sizeof(OCProvisionResult_t));
            if (NULL == certData->resArr)
            {
                DeleteCredList(cred);
                OICFree(certData);
                OC_LOG(ERROR, TAG, "Memory allocation problem");
                return OC_STACK_NO_MEMORY;
            }

            res = provisionCertCred(cred, firstDevice, certData, &provisionCertCB);
            if (OC_STACK_OK != res)
            {
                OICFree(certData->resArr);
                OICFree(certData);
            }
            DeleteCredList(cred);
            OC_LOG_V(INFO, TAG, "provisionCertCredentials returned: %d",res);

            return res;
        }
#endif
        default:
        {
            OC_LOG(ERROR, TAG, "Invalid option.");
            return OC_STACK_INVALID_PARAM;
        }
    }
    return OC_STACK_ERROR;
}

/**
 * Internal Function to store results in result array during ACL provisioning.
 */
static void registerResultForACLProvisioning(ACLData_t *aclData,
                                             OCStackResult stackresult)
{
   OC_LOG_V(INFO, TAG, "Inside registerResultForACLProvisioning aclData->numOfResults is %d\n",
                       aclData->numOfResults);
   memcpy(aclData->resArr[(aclData->numOfResults)].deviceId.id,
          aclData->deviceInfo->doxm->deviceID.id, UUID_LENGTH);
   aclData->resArr[(aclData->numOfResults)].res = stackresult;
   ++(aclData->numOfResults);
}

/**
 * Callback handler of SRPProvisionACL.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] UNUSED          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult SRPProvisionACLCB(void *ctx, OCDoHandle UNUSED,
                                                  OCClientResponse *clientResponse)
{
    OC_LOG_V(INFO, TAG, "Inside SRPProvisionACLCB.");
    (void)UNUSED;
    VERIFY_NON_NULL(TAG, ctx, ERROR, OC_STACK_DELETE_TRANSACTION);
    ACLData_t *aclData = (ACLData_t*)ctx;
    OCProvisionResultCB resultCallback = aclData->resultCallback;

    if (clientResponse)
    {
        if(OC_STACK_RESOURCE_CREATED == clientResponse->result)
        {
            registerResultForACLProvisioning(aclData, OC_STACK_RESOURCE_CREATED);
            ((OCProvisionResultCB)(resultCallback))(aclData->ctx, aclData->numOfResults,
                                                    aclData->resArr,
                                                    false);
             OICFree(aclData->resArr);
             OICFree(aclData);
             return OC_STACK_DELETE_TRANSACTION;
        }
    }
    registerResultForACLProvisioning(aclData, OC_STACK_ERROR);
    ((OCProvisionResultCB)(resultCallback))(aclData->ctx, aclData->numOfResults,
                                            aclData->resArr,
                                            true);
    OC_LOG_V(ERROR, TAG, "SRPProvisionACLCB received Null clientResponse");
    OICFree(aclData->resArr);
    OICFree(aclData);
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackResult SRPProvisionACL(void *ctx, const OCProvisionDev_t *selectedDeviceInfo,
        OicSecAcl_t *acl, OCProvisionResultCB resultCallback)
{
    VERIFY_NON_NULL(TAG, selectedDeviceInfo, ERROR,  OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(TAG, acl, ERROR,  OC_STACK_INVALID_PARAM);
    VERIFY_NON_NULL(TAG, resultCallback, ERROR,  OC_STACK_INVALID_CALLBACK);

    OCSecurityPayload* secPayload = (OCSecurityPayload*)OICCalloc(1, sizeof(OCSecurityPayload));
    if(!secPayload)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    secPayload->base.type = PAYLOAD_TYPE_SECURITY;
    secPayload->securityData = BinToAclJSON(acl);
    if(NULL == secPayload->securityData)
    {
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Failed to BinToAclJSON");
        return OC_STACK_NO_MEMORY;
    }
    OC_LOG_V(INFO, TAG, "ACL : %s", secPayload->securityData);

    char query[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    if(!PMGenerateQuery(true,
                        selectedDeviceInfo->endpoint.addr,
                        selectedDeviceInfo->securePort,
                        selectedDeviceInfo->connType,
                        query, sizeof(query), OIC_RSRC_ACL_URI))
    {
        OC_LOG(ERROR, TAG, "DeviceDiscoveryHandler : Failed to generate query");
        return OC_STACK_ERROR;
    }
    OC_LOG_V(DEBUG, TAG, "Query=%s", query);

    OCCallbackData cbData =  {.context=NULL, .cb=NULL, .cd=NULL};
    cbData.cb = &SRPProvisionACLCB;
    ACLData_t *aclData = (ACLData_t *) OICCalloc(1, sizeof(ACLData_t));
    if (aclData == NULL)
    {
        OICFree(secPayload->securityData);
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Unable to allocate memory");
        return OC_STACK_NO_MEMORY;
    }
    aclData->deviceInfo = selectedDeviceInfo;
    aclData->resultCallback = resultCallback;
    aclData->numOfResults=0;
    aclData->ctx = ctx;
    // call to provision ACL to device1.
    int noOfRiCalls = 1;
    aclData->resArr = (OCProvisionResult_t*)OICCalloc(noOfRiCalls, sizeof(OCProvisionResult_t));
    if (aclData->resArr == NULL)
    {
        OICFree(aclData);
        OICFree(secPayload->securityData);
        OICFree(secPayload);
        OC_LOG(ERROR, TAG, "Unable to allocate memory");
        return OC_STACK_NO_MEMORY;
    }
    cbData.context = (void *)aclData;
    cbData.cd = NULL;
    OCMethod method = OC_REST_POST;
    OCDoHandle handle = NULL;
    OC_LOG(DEBUG, TAG, "Sending ACL info to resource server");
    OCStackResult ret = OCDoResource(&handle, method, query,
            &selectedDeviceInfo->endpoint, (OCPayload*)secPayload,
            selectedDeviceInfo->connType, OC_HIGH_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OICFree(aclData->resArr);
        OICFree(aclData);
    }
    VERIFY_SUCCESS(TAG, (OC_STACK_OK == ret), ERROR, OC_STACK_ERROR);
    return OC_STACK_OK;
}

static void DeleteUnlinkData_t(UnlinkData_t *unlinkData)
{
    if (unlinkData)
    {
        OICFree(unlinkData->unlinkDev);
        OICFree(unlinkData->unlinkRes);
        OICFree(unlinkData);
    }
}

static void registerResultForUnlinkDevices(UnlinkData_t *unlinkData, OCStackResult stackresult,
                                           IdxUnlinkRes_t idx)
{
    if (NULL != unlinkData)
    {
        OC_LOG_V(INFO, TAG, "Inside registerResultForUnlinkDevices unlinkData->numOfResults is %d\n",
                            unlinkData->numOfResults);
        OC_LOG_V(INFO, TAG, "Stack result :: %d", stackresult);

        OicUuid_t *pUuid = &unlinkData->unlinkRes[(unlinkData->numOfResults)].deviceId;

        // Set result in the result array according to the position (devNum).
        if (idx != IDX_DB_UPDATE_RES)
        {
            memcpy(pUuid->id, unlinkData->unlinkDev[idx].doxm->deviceID.id, sizeof(pUuid->id));
        }
        else
        {   // When deivce ID is 000... this means it's the result of database update.
            memset(pUuid->id, 0, sizeof(pUuid->id));
        }
        unlinkData->unlinkRes[(unlinkData->numOfResults)].res = stackresult;
        ++(unlinkData->numOfResults);
        OC_LOG (INFO, TAG, "Out registerResultForUnlinkDevices");
    }
}

static OCStackResult SendDeleteCredentialRequest(void* ctx,
                                                 OCClientResponseHandler respHandler,
                                                 const OCProvisionDev_t* revokedDev,
                                                 const OCProvisionDev_t* destDev)
{
    OC_LOG(DEBUG, TAG, "IN SendDeleteCredentialRequest");

    if (NULL == ctx || NULL == respHandler || NULL == revokedDev || NULL == destDev)
    {
        return OC_STACK_INVALID_PARAM;
    }

    char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(revokedDev->doxm->deviceID.id)) + 1] = {};
    uint32_t base64Len = 0;
    if (B64_OK != b64Encode(revokedDev->doxm->deviceID.id, sizeof(revokedDev->doxm->deviceID.id),
                           base64Buff, sizeof(base64Buff), &base64Len))
    {
        OC_LOG(ERROR, TAG, "SendDeleteCredentialRequest : Failed to base64 encoding");
        return OC_STACK_ERROR;
    }

    char reqBuf[MAX_URI_LENGTH + MAX_QUERY_LENGTH] = {0};
    int snRet = 0;
                    //coaps://0.0.0.0:5684/oic/sec/cred?sub=(BASE64 ENCODED UUID)
    snRet = snprintf(reqBuf, sizeof(reqBuf), SRP_FORM_DELETE_CREDENTIAL, destDev->endpoint.addr,
                     destDev->securePort, OIC_RSRC_CRED_URI, OIC_JSON_SUBJECT_NAME, base64Buff);
    if (snRet < 0)
    {
        OC_LOG_V(ERROR, TAG, "SendDeleteCredentialRequest : Error (snprintf) %d\n", snRet);
        return OC_STACK_ERROR;
    }
    else if ((size_t)snRet >= sizeof(reqBuf))
    {
        OC_LOG_V(ERROR, TAG, "SendDeleteCredentialRequest : Truncated (snprintf) %d\n", snRet);
        return OC_STACK_ERROR;
    }

    OCCallbackData cbData;
    memset(&cbData, 0, sizeof(cbData));
    cbData.context = ctx;
    cbData.cb = respHandler;
    cbData.cd = NULL;
    OC_LOG_V(INFO, TAG, "URI: %s",reqBuf);

    OC_LOG(DEBUG, TAG, "Sending remove credential request to resource server");

    OCStackResult ret = OCDoResource(NULL, OC_REST_DELETE, reqBuf,
                                     &destDev->endpoint, NULL,
                                     CT_ADAPTER_IP, OC_HIGH_QOS, &cbData, NULL, 0);
    if (OC_STACK_OK != ret)
    {
        OC_LOG_V(ERROR, TAG, "SendDeleteCredentialRequest : Error in OCDoResource %d", ret);
    }
    OC_LOG(DEBUG, TAG, "OUT SendDeleteCredentialRequest");

    return ret;
}

/**
 * Callback handler of unlink second device.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] handle          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction and
 *          OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult SRPUnlinkDevice2CB(void *unlinkCtx, OCDoHandle handle,
        OCClientResponse *clientResponse)
{
    (void) handle;
    OC_LOG(DEBUG, TAG, "IN SRPUnlinkDevice2CB");
    VERIFY_NON_NULL(TAG, unlinkCtx, ERROR, OC_STACK_DELETE_TRANSACTION);
    UnlinkData_t* unlinkData = (UnlinkData_t*)unlinkCtx;

    if (clientResponse)
    {
        OC_LOG(DEBUG, TAG, "Valid client response for device 2");
        registerResultForUnlinkDevices(unlinkData, clientResponse->result, IDX_SECOND_DEVICE_RES);

        if (OC_STACK_RESOURCE_DELETED == clientResponse->result)
        {
            OC_LOG(DEBUG, TAG, "Credential of device2 revoked");
        }
        else
        {
            OC_LOG(ERROR, TAG, "Unable to delete credential information from device 2");
            unlinkData->resultCallback(unlinkData->ctx,
                                       unlinkData->numOfResults, unlinkData->unlinkRes, true);
            goto error;
        }
    }
    else
    {
        registerResultForUnlinkDevices(unlinkData, OC_STACK_INVALID_REQUEST_HANDLE,
                                       IDX_SECOND_DEVICE_RES);
        unlinkData->resultCallback(unlinkData->ctx,
                                   unlinkData->numOfResults, unlinkData->unlinkRes, true);
        OC_LOG(ERROR, TAG, "SRPUnlinkDevice2CB received Null clientResponse");
        goto error;
    }

    //Update provisioning DB when succes case.
    if (OC_STACK_OK != PDMUnlinkDevices(&unlinkData->unlinkDev[0].doxm->deviceID,
                                       &unlinkData->unlinkDev[1].doxm->deviceID))
    {
        OC_LOG(FATAL, TAG, "All requests are successfully done but update provisioning DB FAILED.");
        registerResultForUnlinkDevices(unlinkData, OC_STACK_INCONSISTENT_DB, IDX_DB_UPDATE_RES);
        unlinkData->resultCallback(unlinkData->ctx,
                                   unlinkData->numOfResults, unlinkData->unlinkRes, true);
        goto error;
    }
    unlinkData->resultCallback(unlinkData->ctx, unlinkData->numOfResults, unlinkData->unlinkRes,
                               false);

error:
    DeleteUnlinkData_t(unlinkData);
    OC_LOG(DEBUG, TAG, "OUT SRPUnlinkDevice2CB");
    return OC_STACK_DELETE_TRANSACTION;

}

/**
 * Callback handler of unlink first device.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] handle          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction and
 *          OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult SRPUnlinkDevice1CB(void *unlinkCtx, OCDoHandle handle,
        OCClientResponse *clientResponse)
{
    OC_LOG_V(INFO, TAG, "Inside SRPUnlinkDevice1CB ");
    VERIFY_NON_NULL(TAG, unlinkCtx, ERROR, OC_STACK_DELETE_TRANSACTION);
    UnlinkData_t* unlinkData = (UnlinkData_t*)unlinkCtx;
    (void) handle;

    if (clientResponse)
    {
        OC_LOG(DEBUG, TAG, "Valid client response for device 1");
        registerResultForUnlinkDevices(unlinkData, clientResponse->result, IDX_FIRST_DEVICE_RES);

        if (OC_STACK_RESOURCE_DELETED == clientResponse->result)
        {
            OC_LOG(DEBUG, TAG, "Credential of device 1 is revoked");

            // Second revocation request to second device.
            OCStackResult res = SendDeleteCredentialRequest((void*)unlinkData, &SRPUnlinkDevice2CB,
                                                    &unlinkData->unlinkDev[0],
                                                    &unlinkData->unlinkDev[1] /*Dest*/);
            OC_LOG_V(DEBUG, TAG, "Credential revocation request device 2, result :: %d",res);
            if (OC_STACK_OK != res)
            {
                 OC_LOG(ERROR, TAG, "Error while sending revocation request for device 2");
                 registerResultForUnlinkDevices(unlinkData, OC_STACK_INVALID_REQUEST_HANDLE,
                                                IDX_SECOND_DEVICE_RES);
                 unlinkData->resultCallback(unlinkData->ctx,
                                            unlinkData->numOfResults, unlinkData->unlinkRes, true);
                 goto error;
            }
            else
            {
                OC_LOG(DEBUG, TAG, "Request for credential revocation successfully sent");
                return OC_STACK_DELETE_TRANSACTION;
            }
        }
        else
        {
            OC_LOG(ERROR, TAG, "Unable to delete credential information from device 1");

            unlinkData->resultCallback(unlinkData->ctx, unlinkData->numOfResults,
                                            unlinkData->unlinkRes, true);
            goto error;
        }
    }
    else
    {
        OC_LOG(DEBUG, TAG, "Invalid response from server");
        registerResultForUnlinkDevices(unlinkData, OC_STACK_INVALID_REQUEST_HANDLE,
                                       IDX_FIRST_DEVICE_RES );
        unlinkData->resultCallback(unlinkData->ctx,
                                   unlinkData->numOfResults, unlinkData->unlinkRes,
                                   true);
        OC_LOG(ERROR, TAG, "SRPUnlinkDevice1CB received Null clientResponse");
    }

error:
    OC_LOG_V(INFO, TAG, "Out SRPUnlinkDevice1CB");
    DeleteUnlinkData_t(unlinkData);
    return OC_STACK_DELETE_TRANSACTION;
}

/*
* Function to unlink devices.
* This function will remove the credential & relationship between the two devices.
*
* @param[in] ctx Application context would be returned in result callback
* @param[in] pTargetDev1 first device information to be unlinked.
* @param[in] pTargetDev2 second device information to be unlinked.
* @param[in] resultCallback callback provided by API user, callback will be called when
*            device unlink is finished.
 * @return  OC_STACK_OK in case of success and other value otherwise.
*/
OCStackResult SRPUnlinkDevices(void* ctx,
                               const OCProvisionDev_t* pTargetDev1,
                               const OCProvisionDev_t* pTargetDev2,
                               OCProvisionResultCB resultCallback)
{
    OC_LOG(INFO, TAG, "IN SRPUnlinkDevices");

    if (!pTargetDev1 || !pTargetDev2 || !resultCallback)
    {
        OC_LOG(INFO, TAG, "SRPUnlinkDevices : NULL parameters");
        return OC_STACK_INVALID_PARAM;
    }
    OC_LOG(INFO, TAG, "Unlinking following devices: ");
    PMPrintOCProvisionDev(pTargetDev1);
    PMPrintOCProvisionDev(pTargetDev2);

    // Mark the link status stale
    OCStackResult res = PDMSetLinkStale(&pTargetDev1->doxm->deviceID, &pTargetDev2->doxm->deviceID);
    if (OC_STACK_OK != res)
    {
        OC_LOG(FATAL, TAG, "unable to update DB. Try again.");
        return res;
    }

    UnlinkData_t* unlinkData = (UnlinkData_t*)OICCalloc(1, sizeof(UnlinkData_t));
    VERIFY_NON_NULL(TAG, unlinkData, ERROR, OC_STACK_NO_MEMORY);

    //Initialize unlink data
    unlinkData->ctx = ctx;
    unlinkData->unlinkDev = (OCProvisionDev_t*)OICCalloc(2, sizeof(OCProvisionDev_t));
    if (NULL == unlinkData->unlinkDev)
    {
        OC_LOG(ERROR, TAG, "Memory allocation failed");
        res = OC_STACK_NO_MEMORY;
        goto error;
    }

    unlinkData->unlinkRes = (OCProvisionResult_t*)OICCalloc(3, sizeof(OCProvisionResult_t));
    if (NULL == unlinkData->unlinkRes)
    {
        OC_LOG(ERROR, TAG, "Memory allocation failed");
        res = OC_STACK_NO_MEMORY;
        goto error;
    }

    memcpy(&unlinkData->unlinkDev[0], pTargetDev1, sizeof(OCProvisionDev_t));
    memcpy(&unlinkData->unlinkDev[1], pTargetDev2, sizeof(OCProvisionDev_t));

    unlinkData->numOfResults = 0;
    unlinkData->resultCallback = resultCallback;

    res = SendDeleteCredentialRequest((void*)unlinkData, &SRPUnlinkDevice1CB,
                                       &unlinkData->unlinkDev[1], &unlinkData->unlinkDev[0]);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "SRPUnlinkDevices : SendDeleteCredentialRequest failed");
        goto error;
    }

    return res;

error:
    OC_LOG(INFO, TAG, "OUT SRPUnlinkDevices");
    DeleteUnlinkData_t(unlinkData);
    return res;
}

static void DeleteRemoveData_t(RemoveData_t* pRemoveData)
{
    if (pRemoveData)
    {
        OICFree(pRemoveData->revokeTargetDev);
        OCDeleteDiscoveredDevices(pRemoveData->linkedDevList);
        OICFree(pRemoveData->removeRes);
        OICFree(pRemoveData);
    }
}

static void registerResultForRemoveDevice(RemoveData_t *removeData, OicUuid_t *pLinkedDevId,
                                          OCStackResult stackresult, bool hasError)
{
    OC_LOG_V(INFO, TAG, "Inside registerResultForRemoveDevice removeData->numOfResults is %d\n",
                         removeData->numOfResults + 1);
    if (pLinkedDevId)
    {
        memcpy(removeData->removeRes[(removeData->numOfResults)].deviceId.id,
               &pLinkedDevId->id, sizeof(pLinkedDevId->id));
    }
    else
    {
        memset(removeData->removeRes[(removeData->numOfResults)].deviceId.id,
               0, sizeof(pLinkedDevId->id) );
    }
    removeData->removeRes[(removeData->numOfResults)].res = stackresult;
    removeData->hasError = hasError;
    ++(removeData->numOfResults);

    // If we get suffcient result from linked devices, we have to call user callback and do free
    if (removeData->sizeOfResArray == removeData->numOfResults)
    {
        removeData->resultCallback(removeData->ctx, removeData->numOfResults, removeData->removeRes,
                                   removeData->hasError);
        DeleteRemoveData_t(removeData);
    }
 }

/**
 * Callback handler of unlink first device.
 *
 * @param[in] ctx             ctx value passed to callback from calling function.
 * @param[in] handle          handle to an invocation
 * @param[in] clientResponse  Response from queries to remote servers.
 * @return  OC_STACK_DELETE_TRANSACTION to delete the transaction
 *          and  OC_STACK_KEEP_TRANSACTION to keep it.
 */
static OCStackApplicationResult SRPRemoveDeviceCB(void *delDevCtx, OCDoHandle handle,
        OCClientResponse *clientResponse)
{
    //Update the delete credential into delete device context
    //Save the deleted status in delDevCtx
    (void)handle;
    OC_LOG_V(INFO, TAG, "Inside SRPRemoveDeviceCB.");
    VERIFY_NON_NULL(TAG, delDevCtx, ERROR, OC_STACK_DELETE_TRANSACTION);
    OCStackResult res = OC_STACK_ERROR;

    RemoveData_t* removeData = (RemoveData_t*)delDevCtx;
    if (clientResponse)
    {
        // If we can get device's UUID from OCClientResponse, it'd be good to use it in here
        // but OCIdentity in OCClientResponse is emtpy now.
        // It seems that we can set identity to CAData_t *cadata in CAPrepareSendData() API
        // but CA doesn't have deviceID yet.
        //
        //TODO: Get OCIdentity from OCClientResponse and use it for 'registerResultForRemoveDevice'
        //      If we can't complete this task, Provisioning Database has always stale link status
        //      when Remove device is called.

        if (OC_STACK_RESOURCE_DELETED == clientResponse->result)
        {
            res = PDMUnlinkDevices(&removeData->revokeTargetDev->doxm->deviceID,
                                   NULL /*TODO: Replace NULL to uuid from OCClientResponse*/);
            if (OC_STACK_OK != res)
            {
                OC_LOG(FATAL, TAG, "PDMSetLinkStale() FAIL: PDB is an obsolete one.");
                registerResultForRemoveDevice(removeData,
                                          NULL /*TODO: Replace NULL to uuid from OCClientResponse*/,
                                          OC_STACK_INCONSISTENT_DB, true);
                return OC_STACK_DELETE_TRANSACTION;
            }
            registerResultForRemoveDevice(removeData,
                                          NULL /*TODO: Replace NULL to uuid from OCClientResponse*/,
                                          OC_STACK_RESOURCE_DELETED, false);
        }
        else
        {
            registerResultForRemoveDevice(removeData,
                                          NULL /*TODO: Replace NULL to uuid from OCClientResponse*/,
                                          clientResponse->result, true);
            OC_LOG(ERROR, TAG, "Unexpected result from DELETE credential request!");
        }
    }
    else
    {
        registerResultForRemoveDevice(removeData, NULL, OC_STACK_ERROR, true);
        OC_LOG(ERROR, TAG, "SRPRemoveDevices received Null clientResponse");
    }

    return OC_STACK_DELETE_TRANSACTION;
}

static OCStackResult GetListofDevToReqDeleteCred(const OCProvisionDev_t* pRevokeTargetDev,
                                                 OCProvisionDev_t* pOwnedDevList,
                                                 OCUuidList_t* pLinkedUuidList,
                                                 OCProvisionDev_t** ppLinkedDevList,
                                                 size_t *numOfLinkedDev)
{
    // pOwnedDevList could be NULL. It means no alived and owned device now.
    if (pRevokeTargetDev == NULL || pLinkedUuidList == NULL ||\
        ppLinkedDevList == NULL || numOfLinkedDev == NULL)
    {
        return OC_STACK_INVALID_PARAM;
    }

    size_t cnt = 0;
    OCUuidList_t *curUuid = NULL, *tmpUuid = NULL;
    LL_FOREACH_SAFE(pLinkedUuidList, curUuid, tmpUuid)
    {
        // Mark the link status stale.
        OCStackResult res = PDMSetLinkStale(&curUuid->dev, &pRevokeTargetDev->doxm->deviceID);
        if (OC_STACK_OK != res)
        {
            OC_LOG(FATAL, TAG, "PDMSetLinkStale() FAIL: PDB is an obsolete one.");
            return OC_STACK_INCONSISTENT_DB;
        }

        if (pOwnedDevList)
        {
            // If this linked device is alive (power-on), add the deivce to the list.
            OCProvisionDev_t *curDev = NULL, *tmpDev = NULL;
            LL_FOREACH_SAFE(pOwnedDevList, curDev, tmpDev)
            {
                if (memcmp(curDev->doxm->deviceID.id, curUuid->dev.id, sizeof(curUuid->dev.id)) == 0)
                {
                    OCProvisionDev_t* targetDev = PMCloneOCProvisionDev(curDev);
                    if (NULL == targetDev)
                    {
                        OC_LOG(ERROR, TAG, "SRPRemoveDevice : Cloning OCProvisionDev_t Failed.");
                        return OC_STACK_NO_MEMORY;
                    }

                    LL_PREPEND(*ppLinkedDevList, targetDev);
                    cnt++;
                    break;
                }
            }
        }
    }
    *numOfLinkedDev = cnt;
    return OC_STACK_OK;
}

/*
* Function to device revocation
* This function will remove credential of target device from all devices in subnet.
*
* @param[in] ctx Application context would be returned in result callback
* @param[in] waitTimeForOwnedDeviceDiscovery Maximum wait time for owned device discovery.(seconds)
* @param[in] pTargetDev Device information to be revoked.
* @param[in] resultCallback callback provided by API user, callback will be called when
*            credential revocation is finished.
* @return  OC_STACK_OK in case of success and other value otherwise.
*          If OC_STACK_OK is returned, the caller of this API should wait for callback.
*          OC_STACK_CONTINUE means operation is success but no request is need to be initiated.
*/
OCStackResult SRPRemoveDevice(void* ctx, unsigned short waitTimeForOwnedDeviceDiscovery,
                             const OCProvisionDev_t* pTargetDev, OCProvisionResultCB resultCallback)
{
    OC_LOG(INFO, TAG, "IN SRPRemoveDevice");

    if (!pTargetDev || !resultCallback || 0 == waitTimeForOwnedDeviceDiscovery)
    {
        OC_LOG(INFO, TAG, "SRPRemoveDevice : NULL parameters");
        return OC_STACK_INVALID_PARAM;
    }

    // Declare variables in here to handle error cases with goto statement.
    OCProvisionDev_t* pOwnedDevList = NULL;
    OCProvisionDev_t* pLinkedDevList = NULL;
    RemoveData_t* removeData = NULL;

    //1. Find all devices that has a credential of the revoked device
    OCUuidList_t* pLinkedUuidList = NULL;
    size_t numOfDevices = 0;
    OCStackResult res = OC_STACK_ERROR;
    res = PDMGetLinkedDevices(&pTargetDev->doxm->deviceID, &pLinkedUuidList, &numOfDevices);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "SRPRemoveDevice : Failed to get linked devices information");
        return res;
    }
    // if there is no related device, we can skip further process.
    if (0 == numOfDevices)
    {
        OC_LOG(DEBUG, TAG, "SRPRemoveDevice : No linked device found.");
        res = OC_STACK_CONTINUE;
        goto error;
    }

    //2. Find owned device from the network
    res = PMDeviceDiscovery(waitTimeForOwnedDeviceDiscovery, true, &pOwnedDevList);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "SRPRemoveDevice : Failed to PMDeviceDiscovery");
        goto error;
    }

    //3. Make a list of devices to send DELETE credential request
    //   by comparing owned devices from provisioning database with mutlicast discovery result.
    size_t numOfLinkedDev = 0;
    res = GetListofDevToReqDeleteCred(pTargetDev, pOwnedDevList, pLinkedUuidList,
                                      &pLinkedDevList, &numOfLinkedDev);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "SRPRemoveDevice : GetListofDevToReqDeleteCred() failed");
        goto error;
    }
    if (0 == numOfLinkedDev) // This case means, there is linked device but it's not alive now.
    {                       // So we don't have to send request message.
        OC_LOG(DEBUG, TAG, "SRPRemoveDevice : No alived & linked device found.");
        res = OC_STACK_CONTINUE;
        goto error;
    }

    // 4. Prepare RemoveData Context data.
    removeData = (RemoveData_t*)OICCalloc(1, sizeof(RemoveData_t));
    if (!removeData)
    {
        OC_LOG(ERROR, TAG, "SRPRemoveDevices : Failed to memory allocation");
        res = OC_STACK_NO_MEMORY;
        goto error;
    }

    removeData->revokeTargetDev = PMCloneOCProvisionDev(pTargetDev);
    if (!removeData->revokeTargetDev)
    {
        OC_LOG(ERROR, TAG, "SRPRemoveDevices : PMCloneOCProvisionDev Failed");
        res = OC_STACK_NO_MEMORY;
        goto error;
    }

    removeData->removeRes =
        (OCProvisionResult_t*)OICCalloc(numOfLinkedDev, sizeof(OCProvisionResult_t));
    if (!removeData->removeRes)
    {
        OC_LOG(ERROR, TAG, "SRPRemoveDevices : Failed to memory allocation");
        res = OC_STACK_NO_MEMORY;
        goto error;
    }

    removeData->ctx = ctx;
    removeData->linkedDevList = pLinkedDevList;
    removeData->resultCallback = resultCallback;
    removeData->numOfResults = 0;
    removeData->sizeOfResArray = numOfLinkedDev;
    removeData->hasError = false;

    // 5. Send DELETE credential request to linked devices.
    OCProvisionDev_t *curDev = NULL, *tmpDev = NULL;
    OCStackResult totalRes = OC_STACK_ERROR;  /* variable for checking request is sent or not */
    LL_FOREACH_SAFE(pLinkedDevList, curDev, tmpDev)
    {
        res = SendDeleteCredentialRequest((void*)removeData, &SRPRemoveDeviceCB,
                                           removeData->revokeTargetDev, curDev);
        if (OC_STACK_OK != res)
        {
            OC_LOG_V(ERROR, TAG, "SRPRemoveDevice : Fail to send the DELETE credential request to\
                     %s:%u", curDev->endpoint.addr, curDev->endpoint.port);
        }
        else
        {
            totalRes = OC_STACK_OK; // This means at least one request is successfully sent.
        }
    }

    PDMDestoryOicUuidLinkList(pLinkedUuidList); //TODO: Modify API name to have unified convention.
    PMDeleteDeviceList(pOwnedDevList);
    OC_LOG(INFO, TAG, "OUT SRPRemoveDevice");

    return totalRes; // Caller of this API should wait callback if totalRes == OC_STACK_OK.

error:
    PDMDestoryOicUuidLinkList(pLinkedUuidList);
    PMDeleteDeviceList(pOwnedDevList);
    PMDeleteDeviceList(pLinkedDevList);
    if (removeData)
    {
        OICFree(removeData->revokeTargetDev);
        OICFree(removeData->removeRes);
        OICFree(removeData);
    }
    OC_LOG(INFO, TAG, "OUT ERROR case SRPRemoveDevice");
    return res;
}
