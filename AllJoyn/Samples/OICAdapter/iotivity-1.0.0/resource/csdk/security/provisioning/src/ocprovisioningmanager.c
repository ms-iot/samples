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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ocprovisioningmanager.h"
#include "pmutility.h"
#include "ownershiptransfermanager.h"
#include "oic_malloc.h"
#include "logger.h"
#include "secureresourceprovider.h"
#include "provisioningdatabasemanager.h"
#include "credresource.h"
#include "utlist.h"
#include "aclresource.h" //Note: SRM internal header

#define TAG "OCPMAPI"

typedef struct Linkdata Linkdata_t;
struct Linkdata
{
    void *ctx;
    const OCProvisionDev_t *pDev1;
    OicSecAcl_t *pDev1Acl;
    const OCProvisionDev_t *pDev2;
    OicSecAcl_t *pDev2Acl;
    OCProvisionResult_t *resArr;
    int numOfResults;
    int currentCountResults;
    OCProvisionResultCB resultCallback;

};

/**
 * The function is responsible for initializaton of the provisioning manager. It will load
 * provisioning database which have owned device's list and their linked status.
 * TODO: In addition, if there is a device(s) which has not up-to-date credentials, this function will
 * automatically try to update the deivce(s).
 *
 * @param[in] dbPath file path of the sqlite3 db
 *
 * @return OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCInitPM(const char* dbPath)
{
    return PDMInit(dbPath);
}

/**
 * The function is responsible for discovery of device is current subnet. It will list
 * all the device in subnet which are not yet owned. Please call OCInit with OC_CLIENT_SERVER as
 * OCMode.
 *
 * @param[in] timeout Timeout in seconds, value till which function will listen to responses from
 *                    client before returning the list of devices.
 * @param[out] ppList List of candidate devices to be provisioned
 * @return OTM_SUCCESS in case of success and other value otherwise.
 */
OCStackResult OCDiscoverUnownedDevices(unsigned short timeout, OCProvisionDev_t **ppList)
{
    if( ppList == NULL || *ppList != NULL)
    {
        return OC_STACK_INVALID_PARAM;
    }

    return PMDeviceDiscovery(timeout, false, ppList);
}

/**
 * The function is responsible for discovery of owned device is current subnet. It will list
 * all the device in subnet which are owned by calling provisioning client.
 *
 * @param[in] timeout Timeout in seconds, value till which function will listen to responses from
 *                    client before returning the list of devices.
 * @param[out] ppList List of device owned by provisioning tool.
 * @return OTM_SUCCESS in case of success and other value otherwise.
 */
OCStackResult OCDiscoverOwnedDevices(unsigned short timeout, OCProvisionDev_t **ppList)
{
    if( ppList == NULL || *ppList != NULL)
    {
        return OC_STACK_INVALID_PARAM;
    }

    return PMDeviceDiscovery(timeout, true, ppList);
}

/**
 * API to register for particular OxM.
 *
 * @param[in] Ownership transfer method.
 * @param[in] Implementation of callback functions for owership transfer.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCSetOwnerTransferCallbackData(OicSecOxm_t oxm, OTMCallbackData_t* callbackData)
{
    if(NULL == callbackData)
    {
        return OC_STACK_INVALID_PARAM;
    }

    return OTMSetOwnershipTransferCallbackData(oxm, callbackData);
}

OCStackResult OCDoOwnershipTransfer(void* ctx,
                                      OCProvisionDev_t *targetDevices,
                                      OCProvisionResultCB resultCallback)
{
    if( NULL == targetDevices )
    {
        return OC_STACK_INVALID_PARAM;
    }

    return OTMDoOwnershipTransfer(ctx, targetDevices, resultCallback);
}

/**
 * This function deletes memory allocated to linked list created by OCDiscover_XXX_Devices API.
 *
 * @param[in] pList Pointer to OCProvisionDev_t which should be deleted.
 */
void OCDeleteDiscoveredDevices(OCProvisionDev_t *pList)
{
    PMDeleteDeviceList(pList);
}

/**
 * this function sends ACL information to resource.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] selectedDeviceInfo Selected target device.
 * @param[in] acl ACL to provision.
 * @param[in] resultCallback callback provided by API user, callback will be called when provisioning
              request recieves a response from resource server.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionACL(void* ctx, const OCProvisionDev_t *selectedDeviceInfo, OicSecAcl_t *acl,
                             OCProvisionResultCB resultCallback)
{
    return SRPProvisionACL(ctx, selectedDeviceInfo, acl, resultCallback);
}

/**
 * function to provision credential to devices.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] type Type of credentials to be provisioned to the device.
 * @param[in] pDev1 Pointer to OCProvisionDev_t instance,respresenting resource to be provsioned.
   @param[in] pDev2 Pointer to OCProvisionDev_t instance,respresenting resource to be provsioned.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            provisioning request recieves a response from first resource server.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionCredentials(void *ctx, OicSecCredType_t type, size_t keySize,
                                      const OCProvisionDev_t *pDev1,
                                      const OCProvisionDev_t *pDev2,
                                      OCProvisionResultCB resultCallback)
{
    return SRPProvisionCredentials(ctx, type, keySize,
                                      pDev1, pDev2, resultCallback);

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
OCStackResult OCUnlinkDevices(void* ctx,
                              const OCProvisionDev_t* pTargetDev1,
                              const OCProvisionDev_t* pTargetDev2,
                              OCProvisionResultCB resultCallback)
{
    OC_LOG(INFO, TAG, "IN OCUnlinkDevices");
    OCUuidList_t* idList = NULL;
    size_t numOfDev = 0;

    if (!pTargetDev1 || !pTargetDev2 || !resultCallback)
    {
        OC_LOG(ERROR, TAG, "OCUnlinkDevices : NULL parameters");
        return OC_STACK_INVALID_PARAM;
    }

    // Get linked devices with the first device.
    OCStackResult res = PDMGetLinkedDevices(&(pTargetDev1->doxm->deviceID), &idList, &numOfDev);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "OCUnlinkDevices : PDMgetOwnedDevices failed");
        goto error;
    }
    if (1 > numOfDev)
    {
        OC_LOG(DEBUG, TAG, "OCUnlinkDevices : Can not find linked devices");
        res = OC_STACK_INVALID_PARAM; // Input devices are not linked, No request is made
        goto error;
    }

    // Check the linked devices contains the second device. If yes send credential DELETE request.
    OCUuidList_t* curDev = idList;
    while (NULL != curDev)
    {
        if (memcmp(pTargetDev2->doxm->deviceID.id, curDev->dev.id, sizeof(curDev->dev.id)) == 0)
        {
            res = SRPUnlinkDevices(ctx, pTargetDev1, pTargetDev2, resultCallback);
            if (OC_STACK_OK != res)
            {
                OC_LOG(ERROR, TAG, "OCUnlinkDevices : Failed to unlink devices.");
            }
            goto error;
        }
        curDev = curDev->next;
    }
    OC_LOG(DEBUG, TAG, "No matched pair found from provisioning database");
    res = OC_STACK_INVALID_PARAM; // Input devices are not linked, No request is made

error:
    OC_LOG(INFO, TAG, "OUT OCUnlinkDevices");

    PDMDestoryOicUuidLinkList(idList);
    return res;
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
*/
OCStackResult OCRemoveDevice(void* ctx, unsigned short waitTimeForOwnedDeviceDiscovery,
                            const OCProvisionDev_t* pTargetDev,
                            OCProvisionResultCB resultCallback)
{
    OC_LOG(INFO, TAG, "IN OCRemoveDevice");
    OCStackResult res = OC_STACK_ERROR;
    if (!pTargetDev || !resultCallback || 0 == waitTimeForOwnedDeviceDiscovery)
    {
        OC_LOG(INFO, TAG, "OCRemoveDevice : Invalied parameters");
        return OC_STACK_INVALID_PARAM;
    }

    // Send DELETE requests to linked devices
    OCStackResult resReq = OC_STACK_ERROR; // Check that we have to wait callback or not.
    resReq = SRPRemoveDevice(ctx, waitTimeForOwnedDeviceDiscovery, pTargetDev, resultCallback);
    if (OC_STACK_OK != resReq)
    {
        if (OC_STACK_CONTINUE == resReq)
        {
            OC_LOG(DEBUG, TAG, "OCRemoveDevice : Revoked device has no linked device except PT.");
        }
        else
        {
            OC_LOG(ERROR, TAG, "OCRemoveDevice : Failed to invoke SRPRemoveDevice");
            res = resReq;
            goto error;
        }
    }

    // Remove credential of revoked device from SVR database
    const OicSecCred_t *cred = NULL;
    cred = GetCredResourceData(&pTargetDev->doxm->deviceID);
    if (cred == NULL)
    {
        OC_LOG(ERROR, TAG, "OCRemoveDevice : Failed to get credential of remove device.");
        goto error;
    }

    res = RemoveCredential(&cred->subject);
    if (res != OC_STACK_RESOURCE_DELETED)
    {
        OC_LOG(ERROR, TAG, "OCRemoveDevice : Failed to remove credential.");
        goto error;
    }

    // Remove device info from prvisioning database.
    res = PDMDeleteDevice(&pTargetDev->doxm->deviceID);
    if (res != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCRemoveDevice : Failed to delete device in PDM.");
        goto error;
    }

    // Check that we have to wait callback for DELETE request or not
    res = resReq;

error:
    OC_LOG(INFO, TAG, "OUT OCRemoveDevice");
    return res;
}


/**
 * Internal Function to update result in link result array.
 */
static void UpdateLinkResults(Linkdata_t *link, int device, OCStackResult stackresult)
{

    OC_LOG_V(INFO,TAG,"value of link->currentCountResults is %d",link->currentCountResults);
    if (1 == device)
    {
        memcpy(link->resArr[(link->currentCountResults)].deviceId.id, link->pDev1->doxm->deviceID.id,UUID_LENGTH);
    }
    else
    {
        memcpy(link->resArr[(link->currentCountResults)].deviceId.id, link->pDev2->doxm->deviceID.id,UUID_LENGTH);
    }
    link->resArr[(link->currentCountResults)].res = stackresult;
    ++(link->currentCountResults);

}

/**
 * Callback to handle ACL provisioning for device 2.
 */
static void AclProv2CB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{

    if (NULL == ctx)
    {
        OC_LOG(ERROR,TAG,"Context is Null in ACLProv 2");
        return;
    }
    (void)nOfRes;
    Linkdata_t *link = (Linkdata_t*)ctx;
    OCProvisionResultCB resultCallback = link->resultCallback;


    if (hasError)
    {
        UpdateLinkResults(link, 2,arr[0].res);
        OC_LOG(ERROR,TAG,"Error occured while ACL provisioning device 1");
        ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                link->resArr,
                                                true);
        OICFree(link->resArr);
        OICFree(link) ;
        return;
    }
    UpdateLinkResults(link, 2, arr[0].res);
   ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                           link->resArr,
                                           false);
    OICFree(link->resArr);
    OICFree(link);
    return;
}

/**
 * Callback to handle ACL provisioning for device 1
 */
static void AclProv1CB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{

    if (NULL == ctx)
    {
        OC_LOG(ERROR,TAG,"Context is Null in ACLProv1");
        return;
    }
    (void)nOfRes;
    Linkdata_t *link = (Linkdata_t*)ctx;
    OCProvisionResultCB resultCallback = link->resultCallback;

    if (hasError)
    {
        OC_LOG(ERROR,TAG,"Error occured while ACL provisioning device 1");
        UpdateLinkResults(link, 1, arr[0].res);
        ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                link->resArr,
                                                true);
        OICFree(link->resArr);
        OICFree(link);
        return;
    }
    UpdateLinkResults(link, 1, arr[0].res);
    if (NULL != link->pDev2Acl)
    {
        OCStackResult res =  SRPProvisionACL(ctx, link->pDev2, link->pDev2Acl, &AclProv2CB);
        if (OC_STACK_OK!=res)
        {
             UpdateLinkResults(link, 2, res);
             ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                     link->resArr,
                                                     true);

        }
    }
    else
    {
        ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                link->resArr,
                                                false);
        OICFree(link->resArr);
        OICFree(link);
    }

    return;
}

/**
 * Callback to handle credential provisioning.
 */
static void ProvisionCredsCB(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
{
    if (NULL == ctx)
    {
        OC_LOG(ERROR,TAG,"Error occured while credential provisioning");
        return;
    }
    Linkdata_t *link = (Linkdata_t*)ctx;
    OCProvisionResultCB resultCallback = link->resultCallback;
    OC_LOG_V(INFO, TAG, "has error returned %d",hasError);
    UpdateLinkResults(link, 1, arr[0].res);
    UpdateLinkResults(link, 2, arr[1].res);
    if (hasError)
    {
        OC_LOG(ERROR,TAG,"Error occured while credential provisioning");
        ((OCProvisionResultCB)(resultCallback))(link->ctx, nOfRes,
                                                link->resArr,
                                                true);
         OICFree(link->resArr);
         OICFree(link);
         return;
    }
    if (NULL != link->pDev1Acl)
    {

        OCStackResult res =  SRPProvisionACL(ctx, link->pDev1, link->pDev1Acl, &AclProv1CB);
        if (OC_STACK_OK!=res)
        {
             OC_LOG(ERROR, TAG, "Error while provisioning ACL for device 1");
             UpdateLinkResults(link, 1, res);
             ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                     link->resArr,
                                                     true);
              OICFree(link->resArr);
              OICFree(link);
        }
    }
    else if (NULL!=link->pDev2Acl)
    {
        OC_LOG(ERROR, TAG, "ACL for device 1 is NULL");
        OCStackResult res =  SRPProvisionACL(ctx, link->pDev2, link->pDev2Acl, &AclProv2CB);
        if (OC_STACK_OK!=res)
        {
             OC_LOG(ERROR, TAG, "Error while provisioning ACL for device 2");
              UpdateLinkResults(link, 2, res);
             ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                     link->resArr,
                                                     true);
              OICFree(link->resArr);
              OICFree(link);
        }
    }
    else
    {
        OC_LOG(INFO, TAG, "ACLs of both devices are NULL");
        ((OCProvisionResultCB)(resultCallback))(link->ctx, link->currentCountResults,
                                                link->resArr,
                                                false);
        OICFree(link->resArr);
        OICFree(link);
    }
    return;
}
/**
 * function to provision credentials between two devices and ACLs for the devices who act as a server.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] type Type of credentials to be provisioned to the device.
 * @param[in] pDev1 Pointer to OCProvisionDev_t instance,respresenting resource to be provsioned.
 * @param[in] acl ACL for device 1. If this is not required set NULL.
 * @param[in] pDev2 Pointer to OCProvisionDev_t instance,respresenting resource to be provsioned.
 * @param[in] acl ACL for device 2. If this is not required set NULL.
 * @param[in] resultCallback callback provided by API user, callback will be called when
 *            provisioning request recieves a response from first resource server.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionPairwiseDevices(void* ctx, OicSecCredType_t type, size_t keySize,
                                         const OCProvisionDev_t *pDev1, OicSecAcl_t *pDev1Acl,
                                         const OCProvisionDev_t *pDev2, OicSecAcl_t *pDev2Acl,
                                         OCProvisionResultCB resultCallback)
{

    if (!pDev1 || !pDev2 || !resultCallback)
    {
        OC_LOG(ERROR, TAG, "OCProvisionPairwiseDevices : Invalid parameters");
        return OC_STACK_INVALID_PARAM;
    }
    if (!(keySize == OWNER_PSK_LENGTH_128 || keySize == OWNER_PSK_LENGTH_256))
    {
        OC_LOG(INFO, TAG, "OCProvisionPairwiseDevices : Invalid key size");
        return OC_STACK_INVALID_PARAM;
    }

    OC_LOG(DEBUG, TAG, "Checking link in DB");
    bool linkExists = true;
    OCStackResult res = PDMIsLinkExists(&pDev1->doxm->deviceID, &pDev2->doxm->deviceID, &linkExists);
    if(res != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "Internal Error Occured");
        return res;
    }
    if (linkExists)
    {
        OC_LOG(ERROR, TAG, "Link already exists");
        return OC_STACK_INVALID_PARAM;
    }

    int noOfResults = 2; // Initial Value
    if (NULL != pDev1Acl)
    {
        ++noOfResults;
    }
    if (NULL != pDev2Acl)
    {
       ++noOfResults;
    }
    Linkdata_t *link = (Linkdata_t*) OICMalloc(sizeof(Linkdata_t));
    if (!link)
    {
        OC_LOG(ERROR, TAG, "Failed to memory allocation");
        return OC_STACK_NO_MEMORY;
    }
    OC_LOG_V(INFO,TAG, "Maximum no od results %d",noOfResults);

    link->pDev1 = pDev1;
    link->pDev1Acl = pDev1Acl;
    link->pDev2 = pDev2;
    link->pDev2Acl = pDev2Acl;
    link->ctx = ctx;
    // 1 call for each device for credential provisioning. implict call by SRPProvisioning credential
    // 1 call for ACL provisioning for device 1 and 1 call for ACL provisioning for device 2.
    link->numOfResults = noOfResults;
    link->resultCallback = resultCallback;
    link->currentCountResults = 0;
    link->resArr = (OCProvisionResult_t*) OICMalloc(sizeof(OCProvisionResult_t)*noOfResults);
    res = SRPProvisionCredentials(link, type, keySize,
                                     pDev1, pDev2, &ProvisionCredsCB);
    if (res != OC_STACK_OK)
    {
        OICFree(link->resArr);
        OICFree(link);
    }
    return res;

}

OCStackResult OCGetDevInfoFromNetwork(unsigned short waittime,
                                       OCProvisionDev_t** pOwnedDevList,
                                       OCProvisionDev_t** pUnownedDevList)
{
    //TODO will be replaced by more efficient logic
    if (pOwnedDevList == NULL || *pOwnedDevList != NULL || pUnownedDevList == NULL
         || *pUnownedDevList != NULL)
    {
        return OC_STACK_INVALID_PARAM;
    }

    // Code for unowned discovery
    OCProvisionDev_t *unownedDevice = NULL;
    OCStackResult res =  OCDiscoverUnownedDevices(waittime/2, &unownedDevice);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR,TAG, "Error in unowned discovery");
        return res;
    }

    // Code for owned discovery
    OCProvisionDev_t *ownedDevice = NULL;
    res =  OCDiscoverOwnedDevices(waittime/2, &ownedDevice);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR,TAG, "Error in owned discovery");
        PMDeleteDeviceList(unownedDevice);
        return res;
    }

    // Code to get list of all the owned devices.
    OCUuidList_t *uuidList = NULL;
    size_t numOfDevices = 0;
    res =  PDMGetOwnedDevices(&uuidList, &numOfDevices);
    if (OC_STACK_OK != res)
    {
        OC_LOG(ERROR, TAG, "Error while getting info from DB");
        PMDeleteDeviceList(unownedDevice);
        PMDeleteDeviceList(ownedDevice);
        return res;
    }

    // Code to compare devices in owned list and deviceid from DB.
    OCProvisionDev_t* pCurDev = ownedDevice;
    size_t deleteCnt = 0;
    while (pCurDev)
    {
        if(true == PMDeleteFromUUIDList(uuidList, &pCurDev->doxm->deviceID))
        {
            deleteCnt++;
        }
        pCurDev = pCurDev->next;
    }
    // If there is no remaind device in uuidList, we have to assign NULL to prevent free.
    if (deleteCnt == numOfDevices)
    {
        uuidList = NULL;
    }
    // Code to add information of the devices which are currently off in owned list.
    OCUuidList_t *powerOffDeviceList = uuidList;
    while (powerOffDeviceList)
    {
        OCProvisionDev_t *ptr = (OCProvisionDev_t *)OICCalloc(1, sizeof (OCProvisionDev_t));
        if (NULL == ptr)
        {
            OC_LOG(ERROR,TAG,"Fail to allocate memory");
            PMDeleteDeviceList(unownedDevice);
            PMDeleteDeviceList(ownedDevice);
            OCDeleteUuidList(uuidList);
            return OC_STACK_NO_MEMORY;
        }

        ptr->doxm = (OicSecDoxm_t*)OICCalloc(1, sizeof(OicSecDoxm_t));
        if (NULL == ptr->doxm)
        {
            OC_LOG(ERROR,TAG,"Fail to allocate memory");
            PMDeleteDeviceList(unownedDevice);
            PMDeleteDeviceList(ownedDevice);
            OCDeleteUuidList(uuidList);
            OICFree(ptr);
            return OC_STACK_NO_MEMORY;
        }

        memcpy(ptr->doxm->deviceID.id, powerOffDeviceList->dev.id, sizeof(ptr->doxm->deviceID.id));

        ptr->devStatus = DEV_STATUS_OFF;
        LL_PREPEND(ownedDevice, ptr);
        powerOffDeviceList = powerOffDeviceList->next;

    }
    OCDeleteUuidList(uuidList);
    *pOwnedDevList = ownedDevice;
    *pUnownedDevList = unownedDevice;
    return OC_STACK_OK;
}

OCStackResult OCGetLinkedStatus(const OicUuid_t* uuidOfDevice, OCUuidList_t** uuidList,
                                 size_t* numOfDevices)
{
    return PDMGetLinkedDevices(uuidOfDevice, uuidList, numOfDevices);
}

void OCDeleteUuidList(OCUuidList_t* pList)
{
    PDMDestoryOicUuidLinkList(pList);
}

/**
 * This function deletes ACL data.
 *
 * @param pAcl Pointer to OicSecAcl_t structure.
 */
void OCDeleteACLList(OicSecAcl_t* pAcl)
{
    DeleteACLList(pAcl);
}


#ifdef __WITH_X509__
/**
 * this function sends CRL information to resource.
 *
 * @param[in] ctx Application context would be returned in result callback.
 * @param[in] selectedDeviceInfo Selected target device.
 * @param[in] crl CRL to provision.
 * @param[in] resultCallback callback provided by API user, callback will be called when provisioning
              request recieves a response from resource server.
 * @return  OC_STACK_OK in case of success and other value otherwise.
 */
OCStackResult OCProvisionCRL(void* ctx, const OCProvisionDev_t *selectedDeviceInfo, OicSecCrl_t *crl,
                             OCProvisionResultCB resultCallback)
{
    return SRPProvisionCRL(ctx, selectedDeviceInfo, crl, resultCallback);
}
#endif // __WITH_X509__

