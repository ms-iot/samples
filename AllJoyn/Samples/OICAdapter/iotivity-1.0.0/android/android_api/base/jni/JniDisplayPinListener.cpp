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
#include "JniDisplayPinListener.h"
#include "oic_string.h"
#include "oic_malloc.h"

JniDisplayPinListener::JniDisplayPinListener(JNIEnv *env, jobject jListener)
{
    m_jgListener = env->NewGlobalRef(jListener);
}

JniDisplayPinListener::~JniDisplayPinListener()
{
    LOGI("~JniDisplayPinListener()");
    if (m_jgListener)
    {
        jint ret;
        JNIEnv *env = GetJNIEnv(ret);
        if (NULL == env) return;
        env->DeleteGlobalRef(m_jgListener);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
    }
}

void JniDisplayPinListener::displayPinCallback(char *pinBuf, size_t pinSize)
{
    jint ret;
    JNIEnv *env = GetJNIEnv(ret);
    if (NULL == env) return;

    jclass clsL = env->GetObjectClass(m_jgListener);

    if (!clsL)
    {
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    jmethodID midL = env->GetMethodID(clsL, "displayPinListener", "(Ljava/lang/String;)V");
    if (!midL)
    {
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    char *pinStr = (char*)OICMalloc(pinSize + 1);
    if (!pinStr)
    {
        LOGE("malloc failed");
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return ;
    }
    OICStrcpy(pinStr, (pinSize+1), pinBuf);

    env->CallVoidMethod(m_jgListener, midL, env->NewStringUTF(pinStr));
    OICFree(pinStr);

    if (env->ExceptionCheck())
    {
        LOGE("Java exception is thrown");
        env->ExceptionClear();
    }

    if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
}
