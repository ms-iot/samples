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
#include "JniOnPresenceListener.h"
#include "JniUtils.h"

JniOnPresenceListener::JniOnPresenceListener(JNIEnv *env, jobject jListener,
    RemoveListenerCallback removeListenerCallback)
{
    m_jwListener = env->NewWeakGlobalRef(jListener);
    m_removeListenerCallback = removeListenerCallback;
}

JniOnPresenceListener::~JniOnPresenceListener()
{
    LOGD("~JniOnPresenceListener");
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

void JniOnPresenceListener::onPresenceCallback(OCStackResult result, const unsigned int nonce,
    const std::string& hostAddress)
{
    LOGI("JniOnPresenceListener::onPresenceCallback");
    if (!m_jwListener) return;

    jint ret;
    JNIEnv *env = GetJNIEnv(ret);
    if (nullptr == env) return;

    if (OC_STACK_OK != result && OC_STACK_PRESENCE_STOPPED != result &&
        OC_STACK_PRESENCE_TIMEOUT != result &&  OC_STACK_PRESENCE_DO_NOT_HANDLE != result)
    {
        ThrowOcException(result, "onPresenceCallback: stack failure");
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    std::string enumField = JniUtils::stackResultToStr(result);
    if (enumField.empty())
    {
        ThrowOcException(JNI_INVALID_VALUE, "Unexpected OCStackResult value");
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    jobject jPresenceStatus = env->CallStaticObjectMethod(g_cls_OcPresenceStatus,
        g_mid_OcPresenceStatus_get, env->NewStringUTF(enumField.c_str()));
    if (!jPresenceStatus)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    jobject jListener = env->NewLocalRef(m_jwListener);
    if (!jListener)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    jclass clsL = env->GetObjectClass(jListener);
    if (!clsL)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }
    jmethodID midL = env->GetMethodID(clsL, "onPresence",
        "(Lorg/iotivity/base/OcPresenceStatus;ILjava/lang/String;)V");
    if (!midL)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    env->CallVoidMethod(jListener, midL, jPresenceStatus,
        (jint)nonce, env->NewStringUTF(hostAddress.c_str()));
    if (env->ExceptionCheck())
    {
        LOGE("Java exception is thrown");
        checkExAndRemoveListener(env);
    }
    if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
}

void JniOnPresenceListener::checkExAndRemoveListener(JNIEnv* env)
{
    if (env->ExceptionCheck())
    {
        jthrowable ex = env->ExceptionOccurred();
        env->ExceptionClear();
        m_removeListenerCallback(env, m_jwListener);
        env->Throw((jthrowable)ex);
    }
    else
    {
        m_removeListenerCallback(env, m_jwListener);
    }
}

jweak JniOnPresenceListener::getJWListener()
{
    return this->m_jwListener;
}