/*
* //******************************************************************
* //
* // Copyright 2015 Samsung Electronics All Rights Reserved.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* //
* // Licensed under the Apache License, Version 2.0 (the "License");
* // you may not use this file except in compliance with the License.
* // You may obtain a copy of the License at
* //
* //      http://www.apache.org/licenses/LICENSE-2.0
* //
* // Unless required by applicable law or agreed to in writing, software
* // distributed under the License is distributed on an "AS IS" BASIS,
* // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* // See the License for the specific language governing permissions and
* // limitations under the License.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

#include "JniOcSecureResource.h"
#include "JniSecureUtils.h"
namespace PH = std::placeholders;

JniOcSecureResource::JniOcSecureResource(std::shared_ptr<OCSecureResource> device)
    : m_sharedSecureResource(device)
{}

JniOcSecureResource::~JniOcSecureResource()
{
    LOGD("~JniOcSecureResource()");
    m_sharedSecureResource = nullptr;
}

std::string JniOcSecureResource::getIpAddr()
{
    return m_sharedSecureResource->getDevAddr();
}

std::string JniOcSecureResource::getDeviceID()
{
    return m_sharedSecureResource->getDeviceID();
}

int JniOcSecureResource::getDeviceStatus()
{
    return m_sharedSecureResource->getDeviceStatus();
}

bool JniOcSecureResource::getOwnedStatus()
{
    return m_sharedSecureResource->getOwnedStatus();
}

OCSecureResource* JniOcSecureResource::getDevicePtr()
{
    return m_sharedSecureResource.get();
}

JniOcSecureResource* JniOcSecureResource::getJniOcSecureResourcePtr(JNIEnv *env, jobject thiz)
{
    JniOcSecureResource *secureResource = GetHandle<JniOcSecureResource>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcSecureResource");
    }
    if (!secureResource)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return secureResource;
}

JniProvisionResultListner* JniOcSecureResource::AddProvisionResultListener(JNIEnv* env,
        jobject jListener)
{
    JniProvisionResultListner *resultListener = NULL;
    resultMapLock.lock();

    for (auto it = resultMap.begin(); it != resultMap.end(); ++it)
    {
        if (env->IsSameObject(jListener, it->first))
        {
            auto refPair = it->second;
            resultListener = refPair.first;
            refPair.second++;
            it->second = refPair;
            resultMap.insert(*it);
            LOGD("Provision resultListener: ref. count incremented");
            break;
        }
    }
    if (!resultListener)
    {
        resultListener = new JniProvisionResultListner(env, jListener,
            RemoveCallback(std::bind(&JniOcSecureResource::RemoveProvisionResultListener,
                    this, PH::_1, PH::_2)));
        jobject jgListener = env->NewGlobalRef(jListener);

        resultMap.insert(std::pair < jobject, std::pair < JniProvisionResultListner*,
                int >> (jgListener, std::pair<JniProvisionResultListner*, int>(resultListener, 1)));
        LOGD("Provision resultListener: new listener");
    }
    resultMapLock.unlock();
    return resultListener;
}

void JniOcSecureResource::RemoveProvisionResultListener(JNIEnv* env, jobject jListener)
{
    resultMapLock.lock();

    for (auto it = resultMap.begin(); it != resultMap.end(); ++it)
    {
        if (env->IsSameObject(jListener, it->first))
        {
            auto refPair = it->second;
            if (refPair.second > 1)
            {
                refPair.second--;
                it->second = refPair;
                resultMap.insert(*it);
                LOGI("Provision resultListener: ref. count decremented");
            }
            else
            {
                env->DeleteGlobalRef(it->first);
                JniProvisionResultListner* listener = refPair.first;
                delete listener;
                resultMap.erase(it);
                LOGI("Provision resultListener removed");
            }
            break;
        }
    }
    resultMapLock.unlock();
}

OCStackResult JniOcSecureResource::doOwnershipTransfer(JNIEnv* env, jobject jListener)
{
    JniProvisionResultListner *resultListener = AddProvisionResultListener(env, jListener);

    ResultCallBack resultCallback = [resultListener](PMResultList_t *result, int hasError)
    {
        resultListener->ProvisionResultCallback(result, hasError, ListenerFunc::OWNERSHIPTRANSFER);
    };

    return m_sharedSecureResource->doOwnershipTransfer(resultCallback);
}

OCStackResult JniOcSecureResource::getLinkedDevices(JNIEnv *env, UuidList_t &uuidList)
{
    return m_sharedSecureResource->getLinkedDevices(uuidList);
}

OCStackResult JniOcSecureResource::removeDevice(JNIEnv* env, jint timeout, jobject jListener)
{
    JniProvisionResultListner *resultListener = AddProvisionResultListener(env, jListener);

    ResultCallBack resultCallback = [resultListener](PMResultList_t *result, int hasError)
    {
        resultListener->ProvisionResultCallback(result, hasError, ListenerFunc::REMOVEDEVICE);
    };

    return m_sharedSecureResource->removeDevice((int)timeout, resultCallback);
}

OCStackResult JniOcSecureResource::unlinkDevices(JNIEnv* env, jobject _device2, jobject jListener)
{
    JniProvisionResultListner *resultListener = AddProvisionResultListener(env, jListener);

    ResultCallBack resultCallback = [resultListener](PMResultList_t *result, int hasError)
    {
        resultListener->ProvisionResultCallback(result, hasError, ListenerFunc::UNLINKDEVICES);
    };

    JniOcSecureResource *device2 = JniOcSecureResource::getJniOcSecureResourcePtr(env, _device2);
    if (!device2)
    {
        return OC_STACK_ERROR;
    }

    return m_sharedSecureResource->unlinkDevices(*device2->getDevicePtr(), resultCallback);
}

OCStackResult JniOcSecureResource::provisionCredentials(JNIEnv* env, jint type, jint keySize,
        jobject _device2, jobject jListener)
{
    JniProvisionResultListner *resultListener = AddProvisionResultListener(env, jListener);

    ResultCallBack resultCallback = [resultListener](PMResultList_t *result, int hasError)
    {
        resultListener->ProvisionResultCallback(result, hasError, ListenerFunc::PROVISIONCREDENTIALS);
    };

    JniOcSecureResource *device2 = JniOcSecureResource::getJniOcSecureResourcePtr(env, _device2);
    if (!device2)
    {
        return OC_STACK_ERROR;
    }

    Credential cred((OicSecCredType_t)type, keySize);

    return m_sharedSecureResource->provisionCredentials(cred, *device2->getDevicePtr(),
            resultCallback);
}

OCStackResult JniOcSecureResource::provisionACL(JNIEnv* env, jobject _acl, jobject jListener)
{
    OCStackResult ret;
    JniProvisionResultListner *resultListener = AddProvisionResultListener(env, jListener);
    OicSecAcl_t *acl = new OicSecAcl_t;
    acl->next = nullptr;
    if (!acl)
    {
        return OC_STACK_NO_MEMORY;
    }

    if (OC_STACK_OK != JniSecureUtils::convertJavaACLToOCAcl(env, _acl, acl))
    {
        delete acl;
        return OC_STACK_ERROR;
    }

    ResultCallBack resultCallback = [acl, resultListener](PMResultList_t *result, int hasError)
    {
        delete acl;
        resultListener->ProvisionResultCallback(result, hasError, ListenerFunc::PROVISIONACL);
    };
    ret = m_sharedSecureResource->provisionACL(acl, resultCallback);

    if (ret != OC_STACK_OK)
    {
        delete acl;

    }
    return ret;
}

OCStackResult JniOcSecureResource::provisionPairwiseDevices(JNIEnv* env, jint type, jint keySize,
        jobject _acl1, jobject _device2, jobject _acl2, jobject jListener)
{
    OCStackResult ret;

    JniProvisionResultListner *resultListener = AddProvisionResultListener(env, jListener);
    JniOcSecureResource *device2 = JniOcSecureResource::getJniOcSecureResourcePtr(env, _device2);
    if (!device2)
    {
        return OC_STACK_ERROR;
    }

    Credential cred((OicSecCredType_t)type, keySize);

    OicSecAcl_t *acl1 = nullptr;
    OicSecAcl_t *acl2 = nullptr;

    if (_acl1)
    {
        acl1 = new OicSecAcl_t;
        if (!acl1)
        {
            return OC_STACK_NO_MEMORY;
        }

        if (OC_STACK_OK != JniSecureUtils::convertJavaACLToOCAcl(env, _acl1, acl1))
        {
            delete acl1;
            return OC_STACK_ERROR;
        }
        acl1->next = nullptr;
    }

    if (_acl2)
    {
        acl2 = new OicSecAcl_t;
        if (!acl2)
        {
            delete acl1;
            return OC_STACK_NO_MEMORY;
        }

        if (OC_STACK_OK != JniSecureUtils::convertJavaACLToOCAcl(env, _acl2, acl2))
        {
            delete acl2;
            return OC_STACK_ERROR;
        }
        acl2->next = nullptr;
    }

    ResultCallBack resultCallback = [acl1, acl2, resultListener](PMResultList_t *result,
            int hasError)
    {
        delete acl1;
        delete acl2;
        resultListener->ProvisionResultCallback(result, hasError,
                ListenerFunc::PROVISIONPAIRWISEDEVICES);
    };


    ret = m_sharedSecureResource->provisionPairwiseDevices(cred, acl1,
            *device2->getDevicePtr(), acl2, resultCallback);
    if (ret != OC_STACK_OK)
    {
        delete acl1;
        delete acl2;
    }
    return ret;
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    doOwnershipTransfer
 * Signature: (Lorg/iotivity/base/OcSecureResource/doOwnershipTransferListener;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_doOwnershipTransfer
(JNIEnv *env, jobject thiz, jobject jListener)
{
    LOGD("OcSecureResource_doOwnershipTransfer");
    if (!jListener)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "provisionResultListener cannot be null");
        return;
    }

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return;
    }

    try
    {
        OCStackResult result = secureResource->doOwnershipTransfer(env, jListener);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_doOwnershipTransfer");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    removeDevice
 * Signature: (ILorg/iotivity/base/OcSecureResource/removeDevice;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_removeDevice
(JNIEnv *env, jobject thiz, jint timeout, jobject jListener)
{
    LOGD("OcSecureResource_removeDevice");
    if (!jListener)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "provisionResultListener cannot be null");
        return;
    }

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return;
    }

    try
    {
        OCStackResult result = secureResource->removeDevice(env, timeout, jListener);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_removeDevice");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    unlinkDevices
 * Signature: (Lorg/iotivity/base/OcSecureResource/unlinkDevices;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_unlinkDevices
(JNIEnv *env, jobject thiz, jobject device2, jobject jListener)
{
    LOGD("OcSecureResource_unlinkDevices");
    if (!jListener || !device2)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "provisionResultListener or device2 cannot be null");
        return;
    }

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return;
    }

    try
    {
        OCStackResult result = secureResource->unlinkDevices(env, device2, jListener);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_unlinkDevices");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    provisionCredentials1
 * Signature: (Lorg/iotivity/base/OcSecureResource/provisionCredentials;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_provisionCredentials1
(JNIEnv *env, jobject thiz, jint type, jint keySize, jobject device2, jobject jListener)
{
    LOGD("OcSecureResource_provisionCredentials");
    if (!jListener || !device2)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "provisionResultListener or device2 cannot be null");
        return;
    }

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return;
    }

    try
    {
        OCStackResult result = secureResource->provisionCredentials(env, type, keySize, device2,
                jListener);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_provisionCredentials");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    provisionACL
 * Signature: (Lorg/iotivity/base/OcSecureResource/provisionACL;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_provisionACL
(JNIEnv *env, jobject thiz, jobject acl, jobject jListener)
{
    LOGD("OcSecureResource_provisionACL");
    if (!jListener || !acl)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "provisionResultListener or acl cannot be null");
        return;
    }

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return;
    }

    try
    {
        OCStackResult result = secureResource->provisionACL(env, acl, jListener);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_provisionACL");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    provisionPairwiseDevices1
 * Signature: (Lorg/iotivity/base/OcSecureResource/provisionPairwiseDevices;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_provisionPairwiseDevices1
(JNIEnv *env, jobject thiz, jint type, jint keySize, jobject acl1, jobject device2,
        jobject acl2, jobject jListener)
{
    LOGD("OcSecureResource_provisionPairwiseDevices");
    if (!jListener || !device2)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "Invalid Parameters");
        return;
    }

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return;
    }

    try
    {
        OCStackResult result = secureResource->provisionPairwiseDevices(env, type, keySize,
                acl1, device2, acl2, jListener);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_provisionPairwiseDevices");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    getLinkedDevices
 * Signature: ()Ljava/util/List;
 */
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcSecureResource_getLinkedDevices
(JNIEnv *env, jobject thiz)
{
    LOGD("OcSecureResource_getLinkedDevices");
    UuidList_t uuidList;

    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return nullptr;
    }

    try
    {
        OCStackResult result = secureResource->getLinkedDevices(env, uuidList);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcSecureResource_getLinkedDevices");
            return nullptr;
        }
        return JniSecureUtils::convertUUIDVectorToJavaStrList(env, uuidList);
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(e.code(), e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    getIpAddr
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcSecureResource_getIpAddr
  (JNIEnv *env, jobject thiz)
{
    LOGD("OcSecureResource_getIpAddr");
    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return nullptr;
    }

    return env->NewStringUTF(secureResource->getIpAddr().c_str());
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    getDeviceID
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcSecureResource_getDeviceID
  (JNIEnv *env , jobject thiz)
{
    LOGD("OcSecureResource_getDeviceID");
    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return nullptr;
    }

    return env->NewStringUTF(secureResource->getDeviceID().c_str());
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    deviceStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_iotivity_base_OcSecureResource_deviceStatus
  (JNIEnv *env, jobject thiz)
{
    LOGD("OcSecureResource_deviceStatus");
    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return -1;
    }

    return secureResource->getDeviceStatus();
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    ownedStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_iotivity_base_OcSecureResource_ownedStatus
  (JNIEnv *env, jobject thiz)
{
    LOGD("OcSecureResource_ownedStatus");
    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    if (!secureResource)
    {
        return -1;
    }

    return secureResource->getOwnedStatus();
}

/*
 * Class:     org_iotivity_base_OcSecureResource
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcSecureResource_dispose
  (JNIEnv *env, jobject thiz)
{
    LOGD("OcSecureResource_dispose");
    JniOcSecureResource *secureResource = JniOcSecureResource::getJniOcSecureResourcePtr(env, thiz);
    delete secureResource;
}
