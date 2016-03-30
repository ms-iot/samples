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

#include "ocstack.h"
#include "base64.h"
#include "OCProvisioningManager.h"

namespace OC
{
    OCStackResult OCSecure::provisionInit(const std::string& dbPath)
    {
        OCStackResult result;
        auto cLock = OCPlatform_impl::Instance().csdkLock().lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCInitPM(dbPath.c_str());
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }

        return result;
    }

    OCStackResult OCSecure::discoverUnownedDevices(unsigned short timeout,
            DeviceList_t &list)
    {
        OCStackResult result;
        OCProvisionDev_t *pDevList = nullptr, *pCurDev = nullptr, *tmp = nullptr;
        auto csdkLock = OCPlatform_impl::Instance().csdkLock();
        auto cLock = csdkLock.lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCDiscoverUnownedDevices(timeout, &pDevList);
            if (result == OC_STACK_OK)
            {
                // Create DeviceList of OCSecureResource's
                pCurDev = pDevList;
                while (pCurDev)
                {
                    tmp = pCurDev;
                    list.push_back(std::shared_ptr<OCSecureResource>(
                                new OCSecureResource(csdkLock, pCurDev)));
                    pCurDev = pCurDev->next;
                    tmp->next = nullptr;
                }
            }
            else
            {
                oclog() <<"Unowned device discovery failed!";
            }
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }

        return result;
    }

    OCStackResult OCSecure::discoverOwnedDevices(unsigned short timeout,
            DeviceList_t &list)
    {
        OCStackResult result;
        OCProvisionDev_t *pDevList = nullptr, *pCurDev = nullptr, *tmp = nullptr;
        auto csdkLock = OCPlatform_impl::Instance().csdkLock();
        auto cLock = csdkLock.lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCDiscoverOwnedDevices(timeout, &pDevList);
            if (result == OC_STACK_OK)
            {
                pCurDev = pDevList;
                while (pCurDev)
                {
                    tmp = pCurDev;
                    list.push_back(std::shared_ptr<OCSecureResource>(
                                new OCSecureResource(csdkLock, pCurDev)));
                    pCurDev = pCurDev->next;
                    tmp->next = nullptr;
                }
            }
            else
            {
                oclog() <<"Owned device discovery failed!";
            }
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }

        return result;
    }

    OCStackResult OCSecure::setOwnerTransferCallbackData(OicSecOxm_t oxm,
            OTMCallbackData_t* callbackData, InputPinCallback inputPin)
    {
        if(NULL == callbackData || oxm >= OIC_OXM_COUNT)
        {
            oclog() <<"Invalid callbackData or OXM type";
            return OC_STACK_INVALID_PARAM;
        }

        if((OIC_RANDOM_DEVICE_PIN == oxm) && !inputPin)
        {
            oclog() <<"for OXM type DEVICE_PIN, inputPin callback can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = OCPlatform_impl::Instance().csdkLock().lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCSetOwnerTransferCallbackData(oxm, callbackData);
            if(result == OC_STACK_OK && (OIC_RANDOM_DEVICE_PIN & oxm))
            {
                SetInputPinCB(inputPin);
            }
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }

        return result;

    }

    OCStackResult OCSecure::getDevInfoFromNetwork(unsigned short timeout,
            DeviceList_t &ownedDevList,
            DeviceList_t &unownedDevList)
    {
        OCStackResult result = OC_STACK_OK;
        OCProvisionDev_t *owned = nullptr, *unowned = nullptr, *tmp = nullptr, *dev = nullptr;
        auto csdkLock = OCPlatform_impl::Instance().csdkLock();
        auto cLock = csdkLock.lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);

            result = OCGetDevInfoFromNetwork(timeout, &owned, &unowned);

            if (result == OC_STACK_OK)
            {
                dev = owned;
                while (dev)
                {
                    tmp = dev;
                    ownedDevList.push_back(std::shared_ptr<OCSecureResource>(
                                new OCSecureResource(csdkLock, dev)));
                    dev = dev->next;
                    tmp->next = nullptr;
                }

                dev = unowned;
                while (dev)
                {
                    tmp = dev;
                    unownedDevList.push_back(std::shared_ptr<OCSecureResource>(
                                new OCSecureResource(csdkLock,  dev)));
                    dev = dev->next;
                    tmp->next = nullptr;
                }
            }
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }

        return result;
    }

    OCStackResult OCSecure::setDisplayPinCB(GeneratePinCallback displayPin)
    {
        if(!displayPin)
        {
            oclog() <<"displayPin can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result = OC_STACK_OK;
        auto cLock = OCPlatform_impl::Instance().csdkLock().lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);
            SetGeneratePinCB(displayPin);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }

        return result;
    }

    void OCSecureResource::callbackWrapper(void* ctx, int nOfRes, OCProvisionResult_t *arr, bool hasError)
    {
        PMResultList_t *results = nullptr;
        ProvisionContext* context = static_cast<ProvisionContext*>(ctx);

        results = new PMResultList_t;
        for (int i = 0; i < nOfRes; i++)
        {
            results->push_back(arr[i]);
        }

        std::thread exec(context->callback, results, hasError);
        exec.detach();

        delete context;
    }

    OCSecureResource::OCSecureResource(): m_csdkLock(std::weak_ptr<std::recursive_mutex>()),
                                        devPtr(nullptr)
    {
    }

    OCSecureResource::OCSecureResource(std::weak_ptr<std::recursive_mutex> csdkLock,
            OCProvisionDev_t *dPtr)
        :m_csdkLock(csdkLock), devPtr(dPtr)
    {
    }

    OCSecureResource::~OCSecureResource()
    {
        if(devPtr)
        {
            OCDeleteDiscoveredDevices(devPtr);
        }
    }

    OCStackResult OCSecureResource::doOwnershipTransfer(ResultCallBack resultCallback)
    {
        if(!resultCallback)
        {
            oclog() <<"Result callback can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            ProvisionContext* context = new ProvisionContext(resultCallback);

            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCDoOwnershipTransfer(static_cast<void*>(context),
                    devPtr, &OCSecureResource::callbackWrapper);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    OCStackResult OCSecureResource::provisionACL( const OicSecAcl_t* acl,
            ResultCallBack resultCallback)
    {
        if(!resultCallback || !acl)
        {
            oclog() <<"Result callback or ACL can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            ProvisionContext* context = new ProvisionContext(resultCallback);

            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCProvisionACL(static_cast<void*>(context),
                    devPtr, const_cast<OicSecAcl_t*>(acl),
                    &OCSecureResource::callbackWrapper);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    OCStackResult OCSecureResource::provisionCredentials(const Credential &cred,
            const OCSecureResource &device2, ResultCallBack resultCallback)
    {
        if(!resultCallback)
        {
            oclog() << "Result calback can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            ProvisionContext* context = new ProvisionContext(resultCallback);

            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCProvisionCredentials(static_cast<void*>(context),
                    cred.getCredentialType(),
                    cred.getCredentialKeySize(),
                    devPtr, device2.getDevPtr(),
                    &OCSecureResource::callbackWrapper);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    OCStackResult OCSecureResource::provisionPairwiseDevices(const Credential &cred,
            const OicSecAcl_t* acl1, const OCSecureResource &device2, const OicSecAcl_t* acl2,
            ResultCallBack resultCallback)
    {
        if(!resultCallback)
        {
            oclog() << "Result calback can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            ProvisionContext* context = new ProvisionContext(resultCallback);

            std::lock_guard<std::recursive_mutex> lock(*cLock);
            result = OCProvisionPairwiseDevices(static_cast<void*>(context),
                    cred.getCredentialType(),
                    cred.getCredentialKeySize(),
                    devPtr, const_cast<OicSecAcl_t*>(acl1),
                    device2.getDevPtr(), const_cast<OicSecAcl_t*>(acl2),
                    &OCSecureResource::callbackWrapper);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    OCStackResult OCSecureResource::unlinkDevices(const OCSecureResource &device2,
            ResultCallBack resultCallback)
    {
        if(!resultCallback)
        {
            oclog() << "Result calback can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            ProvisionContext* context = new ProvisionContext(resultCallback);

            std::lock_guard<std::recursive_mutex> lock(*cLock);

            result = OCUnlinkDevices(static_cast<void*>(context),
                    devPtr, device2.getDevPtr(), &OCSecureResource::callbackWrapper);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    OCStackResult OCSecureResource::removeDevice(unsigned short waitTimeForOwnedDeviceDiscovery,
            ResultCallBack resultCallback)
    {
        if(!resultCallback)
        {
            oclog() << "Result calback can't be null";
            return OC_STACK_INVALID_PARAM;
        }

        OCStackResult result;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            ProvisionContext* context = new ProvisionContext(resultCallback);

            std::lock_guard<std::recursive_mutex> lock(*cLock);

            result = OCRemoveDevice(static_cast<void*>(context), waitTimeForOwnedDeviceDiscovery,
                    devPtr, &OCSecureResource::callbackWrapper);
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    OCStackResult OCSecureResource::getLinkedDevices(UuidList_t &uuidList)
    {
        OCStackResult result;
        size_t numOfDevices = -1;
        auto devUuid = devPtr->doxm->deviceID;
        auto cLock = m_csdkLock.lock();

        if(cLock)
        {
            std::lock_guard<std::recursive_mutex> lock(*cLock);

            OCUuidList_t* linkedDevs = nullptr, *tmp = nullptr;
            result = OCGetLinkedStatus(&devUuid, &linkedDevs, &numOfDevices);
            if (result == OC_STACK_OK)
            {
                for (tmp = linkedDevs; tmp; tmp = tmp->next)
                {
                    uuidList.push_back(tmp->dev);
                }
                OCDeleteUuidList(linkedDevs);
            }
        }
        else
        {
            oclog() <<"Mutex not found";
            result = OC_STACK_ERROR;
        }
        return result;
    }

    std::string OCSecureResource::getDeviceID()
    {
        char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0,};
        uint32_t outLen = 0;
        B64Result b64Ret = B64_OK;
        std::ostringstream deviceId("");

        validateSecureResource();
        b64Ret = b64Encode(devPtr->doxm->deviceID.id, sizeof(devPtr->doxm->deviceID.id), base64Buff,
                sizeof(base64Buff), &outLen);

        if (B64_OK == b64Ret)
        {
            deviceId << base64Buff;
        }
        return deviceId.str();
    }

    OCProvisionDev_t* OCSecureResource::getDevPtr() const
    {
        return devPtr;
    }

    std::string OCSecureResource::getDevAddr()
    {
        validateSecureResource();
        std::string ipAddr(devPtr->endpoint.addr);
        return ipAddr;
    }

    int OCSecureResource::getDeviceStatus()
    {
        validateSecureResource();
        return (int)devPtr->devStatus;
    }

    bool OCSecureResource::getOwnedStatus()
    {
        validateSecureResource();
        return devPtr->doxm->owned;
    }

    void OCSecureResource::validateSecureResource()
    {
        if (!devPtr)
        {
            throw OCException("Incomplete secure resource", OC_STACK_RESOURCE_ERROR);
        }
    }
}
