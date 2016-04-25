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
#include "JniOnGetListener.h"
#include "JniOcResource.h"
#include "JniOcRepresentation.h"
#include "JniUtils.h"

JniOnGetListener::JniOnGetListener(JNIEnv *env, jobject jListener, JniOcResource* owner)
    : m_ownerResource(owner)
{
    m_jwListener = env->NewWeakGlobalRef(jListener);
}

JniOnGetListener::~JniOnGetListener()
{
    LOGD("~JniOnGetListener");
    if (m_jwListener)
    {
        jint ret;
        JNIEnv *env = GetJNIEnv(ret);
        if (nullptr == env) return;

        env->DeleteWeakGlobalRef(m_jwListener);
        m_jwListener = nullptr;

        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
    }
}

void JniOnGetListener::onGetCallback(const HeaderOptions& headerOptions,
    const OCRepresentation& ocRepresentation, const int eCode)
{
    jint envRet;
    JNIEnv *env = GetJNIEnv(envRet);
    if (nullptr == env) return;

    jobject jListener = env->NewLocalRef(m_jwListener);
    if (!jListener)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return;
    }
    jclass clsL = env->GetObjectClass(jListener);

    if (!clsL)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
        return;
    }

    if (OC_STACK_OK != eCode)
    {
        jobject ex = GetOcException(eCode, "stack error in onGetCallback");
        if (!ex)
        {
            checkExAndRemoveListener(env);
            if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
            return;
        }
        jmethodID midL = env->GetMethodID(clsL, "onGetFailed", "(Ljava/lang/Throwable;)V");
        if (!midL)
        {
            checkExAndRemoveListener(env);
            if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
            return;
        }
        env->CallVoidMethod(jListener, midL, ex);
    }
    else
    {
        jobject jHeaderOptionList = JniUtils::convertHeaderOptionsVectorToJavaList(env, headerOptions);
        if (!jHeaderOptionList)
        {
            checkExAndRemoveListener(env);
            if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
            return;
        }

        OCRepresentation* rep = new OCRepresentation(ocRepresentation);
        jlong handle = reinterpret_cast<jlong>(rep);
        jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool,
            handle, true);
        if (!jRepresentation)
        {
            delete rep;
            checkExAndRemoveListener(env);
            if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
            return;
        }

        jmethodID midL = env->GetMethodID(clsL, "onGetCompleted",
            "(Ljava/util/List;Lorg/iotivity/base/OcRepresentation;)V");
        if (!midL)
        {
            delete rep;
            checkExAndRemoveListener(env);
            if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
            return;
        }
        env->CallVoidMethod(jListener, midL, jHeaderOptionList, jRepresentation);
        if (env->ExceptionCheck())
        {
            LOGE("Java exception is thrown");
            delete rep;
        }
    }

    checkExAndRemoveListener(env);
    if (JNI_EDETACHED == envRet) g_jvm->DetachCurrentThread();
}

void JniOnGetListener::checkExAndRemoveListener(JNIEnv* env)
{
    if (env->ExceptionCheck())
    {
        jthrowable ex = env->ExceptionOccurred();
        env->ExceptionClear();
        m_ownerResource->removeOnGetListener(env, m_jwListener);
        env->Throw((jthrowable)ex);
    }
    else
    {
        m_ownerResource->removeOnGetListener(env, m_jwListener);
    }
}