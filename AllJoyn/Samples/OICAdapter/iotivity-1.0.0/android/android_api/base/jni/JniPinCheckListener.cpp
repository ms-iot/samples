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
#include "JniPinCheckListener.h"
#include "oic_string.h"

JniPinCheckListener::JniPinCheckListener(JNIEnv *env, jobject jListener)
{
    m_jListener = env->NewGlobalRef(jListener);
}

JniPinCheckListener::~JniPinCheckListener()
{
    LOGI("~JniPinCheckListener()");
    if (m_jListener)
    {
        jint ret;
        JNIEnv *env = GetJNIEnv(ret);
        if (NULL == env) return;
        env->DeleteGlobalRef(m_jListener);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
    }
}

void JniPinCheckListener::PinCallback(char *pinBuf, size_t bufSize)
{
    jint ret;

    JNIEnv *env = GetJNIEnv(ret);
    if (NULL == env) return;

    jclass clsL = env->GetObjectClass(m_jListener);

    if (!clsL)
    {
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    jmethodID midL = env->GetMethodID(clsL, "pinCallbackListener", "()Ljava/lang/String;");
    if (!midL)
    {
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }
    jstring jpin = (jstring)env->CallObjectMethod(m_jListener, midL);
    if (env->ExceptionCheck())
    {
        LOGE("Java exception is thrown");
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    char *str = (char*)env->GetStringUTFChars(jpin, NULL);
    OICStrcpy(pinBuf, bufSize, str);
    env->ReleaseStringUTFChars(jpin, str);

    if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
}
