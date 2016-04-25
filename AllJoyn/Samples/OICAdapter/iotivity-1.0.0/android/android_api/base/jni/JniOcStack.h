/*
* //******************************************************************
* //
* // Copyright 2015 Intel Corporation.
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
#include <jni.h>
#include <android/log.h>
#include "OCApi.h"

#ifndef _Included_org_iotivity_base_ocstack
#define _Included_org_iotivity_base_ocstack

#define TAG "OIC-JNI"

#define JNI_CURRENT_VERSION JNI_VERSION_1_6

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define JNI_EXCEPTION 1000
#define JNI_NO_NATIVE_POINTER 1001
#define JNI_INVALID_VALUE 1002
#define JNI_NO_SUCH_KEY 1003

jobject getOcException(JNIEnv* env, const char* file, const char* functionName, const int line,
    const int code, const char* message);
void throwOcException(JNIEnv* env, jobject ex);
#define GetOcException(code, message) getOcException (env,__FILE__,__func__,__LINE__,code,message)
#define ThrowOcException(code, message) throwOcException (env, GetOcException(code, message))

extern JavaVM* g_jvm;

extern jclass g_cls_Integer;
extern jclass g_cls_int1DArray;
extern jclass g_cls_int2DArray;
extern jclass g_cls_Double;
extern jclass g_cls_double1DArray;
extern jclass g_cls_double2DArray;
extern jclass g_cls_Boolean;
extern jclass g_cls_boolean1DArray;
extern jclass g_cls_boolean2DArray;
extern jclass g_cls_String;
extern jclass g_cls_String1DArray;
extern jclass g_cls_String2DArray;
extern jclass g_cls_LinkedList;
extern jclass g_cls_Map;
extern jclass g_cls_MapEntry;
extern jclass g_cls_Set;
extern jclass g_cls_Iterator;
extern jclass g_cls_HashMap;
extern jclass g_cls_OcException;
extern jclass g_cls_OcResource;
extern jclass g_cls_OcRepresentation;
extern jclass g_cls_OcRepresentation1DArray;
extern jclass g_cls_OcRepresentation2DArray;
extern jclass g_cls_OcResourceRequest;
extern jclass g_cls_OcResourceResponse;
extern jclass g_cls_OcResourceHandle;
extern jclass g_cls_OcPresenceHandle;
extern jclass g_cls_OcRequestHandle;
extern jclass g_cls_OcPresenceStatus;
extern jclass g_cls_OcHeaderOption;
extern jclass g_cls_ObservationInfo;
extern jclass g_cls_OcResourceIdentifier;
extern jclass g_cls_OcProvisionResult;
extern jclass g_cls_OcSecureResource;
extern jclass g_cls_OcOicSecAcl;

extern jmethodID g_mid_Integer_ctor;
extern jmethodID g_mid_Double_ctor;
extern jmethodID g_mid_Boolean_ctor;
extern jmethodID g_mid_LinkedList_ctor;
extern jmethodID g_mid_LinkedList_add_object;
extern jmethodID g_mid_Map_entrySet;
extern jmethodID g_mid_MapEntry_getKey;
extern jmethodID g_mid_MapEntry_getValue;
extern jmethodID g_mid_Set_iterator;
extern jmethodID g_mid_Iterator_hasNext;
extern jmethodID g_mid_Iterator_next;
extern jmethodID g_mid_HashMap_ctor;
extern jmethodID g_mid_HashMap_put;
extern jmethodID g_mid_OcException_ctor;
extern jmethodID g_mid_OcException_setNativeExceptionLocation;
extern jmethodID g_mid_OcResource_ctor;
extern jmethodID g_mid_OcRepresentation_N_ctor;
extern jmethodID g_mid_OcRepresentation_N_ctor_bool;
extern jmethodID g_mid_OcResourceRequest_N_ctor;
extern jmethodID g_mid_OcResourceResponse_N_ctor;
extern jmethodID g_mid_OcResourceHandle_N_ctor;
extern jmethodID g_mid_OcPresenceHandle_N_ctor;
extern jmethodID g_mid_OcRequestHandle_N_ctor;
extern jmethodID g_mid_OcHeaderOption_ctor;
extern jmethodID g_mid_OcHeaderOption_get_id;
extern jmethodID g_mid_OcHeaderOption_get_data;
extern jmethodID g_mid_ObservationInfo_N_ctor;
extern jmethodID g_mid_OcPresenceStatus_get;
extern jmethodID g_mid_OcResourceIdentifier_N_ctor;
extern jmethodID g_mid_OcProvisionResult_ctor;
extern jmethodID g_mid_OcSecureResource_ctor;
extern jmethodID g_mid_OcOicSecAcl_get_subject;
extern jmethodID g_mid_OcOicSecAcl_get_resources_cnt;
extern jmethodID g_mid_OcOicSecAcl_get_resources;
extern jmethodID g_mid_OcOicSecAcl_get_permission;
extern jmethodID g_mid_OcOicSecAcl_get_periods_cnt;
extern jmethodID g_mid_OcOicSecAcl_get_periods;
extern jmethodID g_mid_OcOicSecAcl_get_recurrences;
extern jmethodID g_mid_OcOicSecAcl_get_owners_cnt;
extern jmethodID g_mid_OcOicSecAcl_get_owners;

typedef void(*RemoveListenerCallback)(JNIEnv* env, jobject jListener);

static jfieldID GetHandleField(JNIEnv *env, jobject jobj)
{
    jclass cls = env->GetObjectClass(jobj);
    return env->GetFieldID(cls, "mNativeHandle", "J");
}

template <typename T>
static T *GetHandle(JNIEnv *env, jobject jobj)
{
    jlong handle = env->GetLongField(jobj, GetHandleField(env, jobj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
static void SetHandle(JNIEnv *env, jobject jobj, T *type)
{
    jlong handle = reinterpret_cast<jlong>(type);

    env->SetLongField(jobj, GetHandleField(env, jobj), handle);
}

static JNIEnv* GetJNIEnv(jint& ret)
{
    JNIEnv *env = nullptr;

    ret = g_jvm->GetEnv((void **)&env, JNI_CURRENT_VERSION);
    switch (ret) {
    case JNI_OK:
        return env;
    case JNI_EDETACHED:
        if (g_jvm->AttachCurrentThread(&env, nullptr) < 0)
        {
            LOGE("Failed to get the environment");
            return nullptr;
        }
        else
        {
            return env;
        }

    case JNI_EVERSION:
        LOGE("JNI version not supported");
    default:
        LOGE("Failed to get the environment");
        return nullptr;
    }
}

static void DuplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

#endif // _Included_org_iotivity_base_ocstack