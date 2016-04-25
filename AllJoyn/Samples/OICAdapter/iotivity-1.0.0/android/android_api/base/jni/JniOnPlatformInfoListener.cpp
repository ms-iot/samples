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
#include "JniOnPlatformInfoListener.h"
#include "JniOcRepresentation.h"

JniOnPlatformInfoListener::JniOnPlatformInfoListener(JNIEnv *env, jobject jListener,
    RemoveListenerCallback removeListenerCallback)
{
    m_jwListener = env->NewWeakGlobalRef(jListener);
    m_removeListenerCallback = removeListenerCallback;
}

JniOnPlatformInfoListener::~JniOnPlatformInfoListener()
{
    LOGI("~JniOnPlatformInfoListener");
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

void JniOnPlatformInfoListener::foundPlatformCallback(const OC::OCRepresentation& ocRepresentation)
{
    jint ret;
    JNIEnv *env = GetJNIEnv(ret);
    if (nullptr == env) return;

    jobject jListener = env->NewLocalRef(m_jwListener);
    if (!jListener)
    {
        LOGI("Java onPlatformInfoListener object is already destroyed, quiting");
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
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
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    jclass clsL = env->GetObjectClass(jListener);
    if (!clsL)
    {
        delete rep;
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }
    jmethodID midL = env->GetMethodID(clsL, "onPlatformFound", "(Lorg/iotivity/base/OcRepresentation;)V");
    if (!midL)
    {
        delete rep;
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    env->CallVoidMethod(jListener, midL, jRepresentation);
    if (env->ExceptionCheck())
    {
        LOGE("Java exception is thrown");
        delete rep;
        checkExAndRemoveListener(env);
    }

    if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
}

void JniOnPlatformInfoListener::checkExAndRemoveListener(JNIEnv* env)
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

