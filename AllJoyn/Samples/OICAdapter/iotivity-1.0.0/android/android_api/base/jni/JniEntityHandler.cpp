//******************************************************************
//
// Copyright 2015 Intel Corporation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "JniEntityHandler.h"
#include "JniOcResourceRequest.h"
#include "JniOcResourceResponse.h"
#include "JniUtils.h"

JniEntityHandler::JniEntityHandler(JNIEnv *env, jobject entityHandler)
{
    m_jListener = env->NewGlobalRef(entityHandler);
}

JniEntityHandler::~JniEntityHandler()
{
    LOGD("~JniEntityHandler");
    if (m_jListener)
    {
        jint ret;
        JNIEnv *env = GetJNIEnv(ret);
        if (nullptr == env)
        {
            return;
        }

        env->DeleteGlobalRef(m_jListener);
        m_jListener = nullptr;

        if (JNI_EDETACHED == ret)
        {
            g_jvm->DetachCurrentThread();
        }
    }
}

OCEntityHandlerResult JniEntityHandler::handleEntity(
    const std::shared_ptr<OCResourceRequest> request)
{
    LOGD("JniEntityHandler_handleEntity");
    jint envRet;
    JNIEnv *env = GetJNIEnv(envRet);
    if (nullptr == env)
    {
        return OC_EH_ERROR;
    }

    JniOcResourceRequest* jniResReq = new JniOcResourceRequest(request);
    jlong reqHandle = reinterpret_cast<jlong>(jniResReq);
    jobject jResourceRequest =
        env->NewObject(g_cls_OcResourceRequest,
                       g_mid_OcResourceRequest_N_ctor,
                       reqHandle);
    if (!jResourceRequest)
    {
        LOGE("Failed to create OcResourceRequest");
        delete jniResReq;
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }

    jclass clsL = env->GetObjectClass(m_jListener);
    if (!clsL)
    {
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }
    jmethodID midL = env->GetMethodID(clsL, "handleEntity",
        "(Lorg/iotivity/base/OcResourceRequest;)Lorg/iotivity/base/EntityHandlerResult;");
    if (!midL)
    {
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }
    jobject entityHandlerResult = env->CallObjectMethod(m_jListener, midL, jResourceRequest);
    if (env->ExceptionCheck())
    {
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }
    if (!entityHandlerResult)
    {
        ThrowOcException(JNI_INVALID_VALUE, "EntityHandlerResult cannot be null");
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }
    jclass clsResult = env->GetObjectClass(entityHandlerResult);
    if (!clsResult)
    {
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }
    jmethodID getValue_ID = env->GetMethodID(clsResult, "getValue", "()I");
    if (!getValue_ID)
    {
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return OC_EH_ERROR;
    }
    jint jResult = env->CallIntMethod(entityHandlerResult, getValue_ID);
    if (env->ExceptionCheck())
    {
        LOGE("Java exception is thrown");
        if (JNI_EDETACHED == envRet)
        {
            g_jvm->DetachCurrentThread();
        }

        return OC_EH_ERROR;
    }

    if (JNI_EDETACHED == envRet)
    {
        g_jvm->DetachCurrentThread();
    }

    return JniUtils::getOCEntityHandlerResult(env, static_cast<int>(jResult));
}
