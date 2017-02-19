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

#include "JniOcProvisioning.h"
#include "JniPinCheckListener.h"
#include "JniDisplayPinListener.h"

using namespace OC;
namespace PH = std::placeholders;

static JniPinCheckListener *jniPinListener = nullptr;
static JniDisplayPinListener *jniDisplayPinListener = nullptr;

void Callback(char *buf, size_t size)
{
    if (jniPinListener)
    {
        jniPinListener->PinCallback(buf, size);
    }
    else
    {
        LOGE("jniPinListener is null");
    }
}

void displayPinCB(char *pinBuf, size_t pinSize)
{
    if (jniDisplayPinListener)
    {
        jniDisplayPinListener->displayPinCallback(pinBuf, pinSize);
    }
    else
    {
        LOGE("DisplayPinListener is null");
    }
}

/*
 * Class:     org_iotivity_base_OcProvisioning
 * Method:    ownershipTransferCDdata
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcProvisioning_ownershipTransferCBdata
  (JNIEnv *env, jobject thiz, jint OxmType, jobject jListener)
{
    LOGD("OcProvisioning_ownershipTransferCBdata");
    OCStackResult result = OC_STACK_ERROR;

    try
    {
        OTMCallbackData_t CBData = {0};
        if (OIC_JUST_WORKS == (OicSecOxm_t)OxmType)
        {
            CBData.loadSecretCB = LoadSecretJustWorksCallback;
            CBData.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
            CBData.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
            CBData.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;

            result = OCSecure::setOwnerTransferCallbackData((OicSecOxm_t)OxmType,
                    &CBData, NULL);
        }
        if (OIC_RANDOM_DEVICE_PIN == (OicSecOxm_t)OxmType)
        {
            if (jListener)
            {
                delete jniPinListener;
                jniPinListener = new JniPinCheckListener(env, jListener);
                CBData.loadSecretCB = InputPinCodeCallback;
                CBData.createSecureSessionCB = CreateSecureSessionRandomPinCallbak;
                CBData.createSelectOxmPayloadCB = CreatePinBasedSelectOxmPayload;
                CBData.createOwnerTransferPayloadCB = CreatePinBasedOwnerTransferPayload;
                result = OCSecure::setOwnerTransferCallbackData((OicSecOxm_t)OxmType,
                        &CBData, Callback);
            }
            else
            {
                result = OC_STACK_ERROR;
            }
        }

        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "OcProvisioning_ownershipTransferCDdata");
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
* Class:     org_iotivity_base_OcProvisioning
* Method:    discoverUnownedDevices
* Signature: (I)[Lorg/iotivity/base/OcSecureResource;
*/
JNIEXPORT jobjectArray JNICALL Java_org_iotivity_base_OcProvisioning_discoverUnownedDevices1
  (JNIEnv *env, jclass clazz, jint timeout)
{
    LOGI("OcProvisioning_discoverUnownedDevices");
    DeviceList_t list;

    try
    {
        OCStackResult result = OCSecure::discoverUnownedDevices((unsigned short)timeout, list);

        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "Failed to discover Unowned devices");
            return nullptr;
        }

        return JniSecureUtils::convertDeviceVectorToJavaArray(env, list);
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(OC_STACK_ERROR, e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcProvisioning
 * Method:    provisionInit
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcProvisioning_provisionInit
  (JNIEnv *env, jclass calzz, jstring jdbPath)
{
    LOGI("OcProvisioning_provisionInit");
    char *dbpath;

    if (!jdbPath)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "SVR db path cannot be null");
        return;
    }

    try
    {
        dbpath = (char*)env->GetStringUTFChars(jdbPath, NULL);
        OCStackResult result = OCSecure::provisionInit(env->GetStringUTFChars(jdbPath, NULL));

        if (OC_STACK_OK != result)
        {
            env->ReleaseStringUTFChars(jdbPath, (const char*)dbpath);
            ThrowOcException(result, "Failed to Init Provisioning Manager");
            return;
        }
        env->ReleaseStringUTFChars(jdbPath, (const char*)dbpath);
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(OC_STACK_ERROR, e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcProvisioning
 * Method:    discoverOwnedDevices
 * Signature: (I)[Lorg/iotivity/base/OcSecureResource;
 */
JNIEXPORT jobjectArray JNICALL Java_org_iotivity_base_OcProvisioning_discoverOwnedDevices1
  (JNIEnv *env, jclass clazz , jint timeout)
{
    LOGI("OcProvisioning_discoverOwnedDevices");
    DeviceList_t list;

    try
    {
        OCStackResult result = OCSecure::discoverOwnedDevices((unsigned short)timeout, list);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "Failed to discover Owned devices");
            return nullptr;
        }

        return JniSecureUtils::convertDeviceVectorToJavaArray(env, list);
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(OC_STACK_ERROR, e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcProvisioning
 * Method:    getDevicestatusLists
 * Signature: (I)[Lorg/iotivity/base/OcSecureResource;
 */
JNIEXPORT jobjectArray JNICALL Java_org_iotivity_base_OcProvisioning_getDeviceStatusList1
  (JNIEnv *env, jclass clazz, jint timeout)
{
    LOGI("OcProvisioning_getDeviceStatusList");
    DeviceList_t  ownedDevList, unownedDevList;

    try
    {
        OCStackResult result = OCSecure::getDevInfoFromNetwork((unsigned short)timeout,
                ownedDevList, unownedDevList);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "Failed to get Device Status List");
            return nullptr;
        }
        ownedDevList.insert(ownedDevList.end(), unownedDevList.begin(), unownedDevList.end());
        return JniSecureUtils::convertDeviceVectorToJavaArray(env, ownedDevList);
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(OC_STACK_ERROR, e.reason().c_str());
    }
}

/*
 * Class:     org_iotivity_base_OcProvisioning
 * Method:    setDisplayPinListener
 * Signature: (Lorg/iotivity/base/OcProvisioning/DisplayPinListener;)V
 */
JNIEXPORT void JNICALL Java_org_iotivity_base_OcProvisioning_setDisplayPinListener
  (JNIEnv *env, jclass thiz, jobject jListener)
{

    LOGI("OcProvisioning_setDisplayPinListener");

    if (!jListener)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "displayPinListener can't be null");
        return;
    }
    delete jniDisplayPinListener;
    jniDisplayPinListener = new JniDisplayPinListener(env, jListener);

    try
    {
        OCStackResult result = OCSecure::setDisplayPinCB(displayPinCB);
        if (OC_STACK_OK != result)
        {
            ThrowOcException(result, "Failed to set displayPinListener");
            return;
        }
    }
    catch (OCException& e)
    {
        LOGE("%s", e.reason().c_str());
        ThrowOcException(OC_STACK_ERROR, e.reason().c_str());
    }
}
