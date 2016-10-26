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

#include "JniSecureUtils.h"
#include "JniOcSecureResource.h"
#include "base64.h"

jobject JniSecureUtils::convertProvisionresultVectorToJavaList(JNIEnv *env, const OC::PMResultList_t *result)
{
    jobject jResultList = env->NewObject(g_cls_LinkedList, g_mid_LinkedList_ctor);
    if (!jResultList)
    {
        return nullptr;
    }

    for (size_t i = 0; i < result->size(); ++i)
    {
        jstring jStr = env->NewStringUTF((convertUUIDtoStr(result->at(i).deviceId).c_str()));
        if (!jStr)
        {
            return nullptr;
        }
        jobject jresult = env->NewObject(
                g_cls_OcProvisionResult,
                g_mid_OcProvisionResult_ctor,
                jStr,
                static_cast<jint>(result->at(i).res)
                );
        if (!jresult)
        {
            return nullptr;
        }

        env->CallBooleanMethod(jResultList, g_mid_LinkedList_add_object, jresult);
        if (env->ExceptionCheck())
        {
            return nullptr;
        }
        env->DeleteLocalRef(jresult);
        env->DeleteLocalRef(jStr);
    }
    return jResultList;
}

jobjectArray JniSecureUtils::convertDeviceVectorToJavaArray(JNIEnv *env,
    std::vector<std::shared_ptr<OC::OCSecureResource>> &deviceListVector)
{
    jsize len = static_cast<jsize>(deviceListVector.size());
    jobjectArray devArr = env->NewObjectArray(len, g_cls_OcSecureResource, NULL);

    if (!devArr)
    {
        return nullptr;
    }

    for (jsize i = 0; i < len; ++i)
    {
        JniOcSecureResource *device = new JniOcSecureResource(deviceListVector[i]);
        jobject jDevice = env->NewObject(g_cls_OcSecureResource, g_mid_OcSecureResource_ctor);

        SetHandle<JniOcSecureResource>(env, jDevice, device);
        if (!jDevice)
        {
            return nullptr;
        }

        env->SetObjectArrayElement(devArr, i, jDevice);
        if (env->ExceptionCheck())
        {
            return nullptr;
        }
        env->DeleteLocalRef(jDevice);
    }
    return devArr;
}

std::string JniSecureUtils::convertUUIDtoStr(OicUuid_t uuid)
{
    std::ostringstream deviceId("");
    char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0,};
    uint32_t outLen = 0;
    B64Result b64Ret = B64_OK;

    b64Ret = b64Encode(uuid.id, sizeof(uuid.id), base64Buff,
            sizeof(base64Buff), &outLen);

    deviceId << base64Buff;
    return deviceId.str();
}

void JniSecureUtils::convertStrToUUID(char *str, OicUuid_t &uuid)
{
    unsigned char base64Buff[sizeof(((OicUuid_t*)0)->id)] = {};
    uint32_t outLen = 0;
    B64Result b64Ret = B64_OK;

    b64Ret = b64Decode(str, strlen(str), base64Buff, sizeof(base64Buff), &outLen);
    memcpy(uuid.id, base64Buff, outLen);
}

jobject JniSecureUtils::convertUUIDVectorToJavaStrList(JNIEnv *env, UuidList_t &vector)
{
    jobject jList = env->NewObject(g_cls_LinkedList, g_mid_LinkedList_ctor);
    if (!jList)
    {
        return nullptr;
    }
    for (size_t i = 0; i < vector.size(); ++i)
    {
        jstring jStr = env->NewStringUTF((convertUUIDtoStr(vector[i])).c_str());
        if (!jStr)
        {
            return nullptr;
        }
        env->CallBooleanMethod(jList, g_mid_LinkedList_add_object, jStr);
        if (env->ExceptionCheck())
        {
            return nullptr;
        }
        env->DeleteLocalRef(jStr);
    }
    return jList;
}

OCStackResult JniSecureUtils::convertJavaACLToOCAcl(JNIEnv *env, jobject in, OicSecAcl_t *acl)
{
    jstring jData;
    jvalue args[1];

    jData = (jstring) env->CallObjectMethod(in, g_mid_OcOicSecAcl_get_subject);
    if (!jData || env->ExceptionCheck())
    {
        return OC_STACK_ERROR;
    }

    char *str = (char*) env->GetStringUTFChars(jData, 0);
    convertStrToUUID(str, acl->subject);
    env->ReleaseStringUTFChars(jData, str);

    jint jCount = (jint) env->CallIntMethod(in, g_mid_OcOicSecAcl_get_resources_cnt);
    if (!jCount || env->ExceptionCheck())
    {
        return OC_STACK_ERROR;
    }

    acl->resourcesLen = jCount;
    acl->resources = new char*[jCount];
    for (jint i = 0; i < jCount; ++i)
    {
        args[0].i = i;
        jData = (jstring) env->CallObjectMethodA(in, g_mid_OcOicSecAcl_get_resources, args);
        if (!jData || env->ExceptionCheck())
        {
            return OC_STACK_ERROR;
        }

        acl->resources[i] = (char*) env->GetStringUTFChars(jData, 0);
    }

    jCount = (jint) env->CallIntMethod(in, g_mid_OcOicSecAcl_get_permission);
    if (env->ExceptionCheck())
    {
        return OC_STACK_ERROR;
    }

    acl->permission = jCount;
    jCount = (jint) env->CallIntMethod(in, g_mid_OcOicSecAcl_get_periods_cnt);
    if (env->ExceptionCheck())
    {
        return OC_STACK_ERROR;
    }

    acl->prdRecrLen = jCount;
    acl->periods = new char*[jCount];
    for (jint i = 0; i < jCount; ++i)
    {
        args[0].i = i;
        jData = (jstring) env->CallObjectMethodA(in, g_mid_OcOicSecAcl_get_periods, args);
        if (!jData || env->ExceptionCheck())
        {
            return OC_STACK_ERROR;
        }

        acl->periods[i] = (char*) env->GetStringUTFChars(jData, 0);
    }

    acl->recurrences = new char*[jCount]; //TODO:Period Len and Reccurence len is same
    for (jint i = 0; i < jCount; ++i)
    {
        args[0].i = i;
        jData = (jstring) env->CallObjectMethodA(in, g_mid_OcOicSecAcl_get_recurrences, args);
        if (!jData ||  env->ExceptionCheck())
        {
            return OC_STACK_ERROR;
        }

        acl->recurrences[i] = (char*) env->GetStringUTFChars(jData, 0);
    }

    jCount = (jint) env->CallIntMethod(in, g_mid_OcOicSecAcl_get_owners_cnt);
    if (!jCount ||  env->ExceptionCheck())
    {
        return OC_STACK_ERROR;
    }

    acl->ownersLen = jCount;
    acl->owners = new OicUuid_t[acl->ownersLen];
    if (!acl->owners)
    {
        return OC_STACK_ERROR;
    }

    for (jint i = 0; i < jCount; ++i)
    {
        args[0].i = i;
        jData = (jstring) env->CallObjectMethodA(in, g_mid_OcOicSecAcl_get_owners, args);
        if (!jData ||  env->ExceptionCheck())
        {
            return OC_STACK_ERROR;
        }

        str = (char*) env->GetStringUTFChars(jData, 0);
        convertStrToUUID(str, acl->owners[i]);
        env->ReleaseStringUTFChars(jData, str);
    }
    return OC_STACK_OK;
}
