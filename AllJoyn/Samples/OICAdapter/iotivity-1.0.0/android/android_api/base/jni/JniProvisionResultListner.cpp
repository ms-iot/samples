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
#include "JniProvisionResultListner.h"
#include "JniOcSecureResource.h"
#include "JniSecureUtils.h"

JniProvisionResultListner::JniProvisionResultListner(JNIEnv *env, jobject jListener,
    RemoveCallback removeProvisionResultListener)
{
    m_jwListener = env->NewWeakGlobalRef(jListener);
    m_removeProvisionResultListener = removeProvisionResultListener;
}

JniProvisionResultListner::~JniProvisionResultListner()
{
    LOGI("~JniProvisionResultListner()");
    if (m_jwListener)
    {
        jint ret;
        JNIEnv *env = GetJNIEnv(ret);
        if (NULL == env) return;
        env->DeleteWeakGlobalRef(m_jwListener);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
    }
}

void JniProvisionResultListner::ProvisionResultCallback(PMResultList_t *result, int hasError,
      ListenerFunc func)
{
    jint ret;
    JNIEnv *env = GetJNIEnv(ret);
    if (NULL == env) return;

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


    jobject jResultList = JniSecureUtils::convertProvisionresultVectorToJavaList(env, result);
    if (!jResultList)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }

    std::string calledFunc;
    switch (func)
    {
        case ListenerFunc::OWNERSHIPTRANSFER:
        {
            calledFunc = "doOwnershipTransferListener";
        }
        break;
        case ListenerFunc::PROVISIONACL:
        {
            calledFunc = "provisionAclListener";
        }
        break;
        case ListenerFunc::PROVISIONCREDENTIALS:
        {
            calledFunc = "provisionCredentialsListener";
        }
        break;
        case ListenerFunc::UNLINKDEVICES:
        {
            calledFunc = "unlinkDevicesListener";
        }
        break;
        case ListenerFunc::REMOVEDEVICE:
        {
            calledFunc = "removeDeviceListener";
        }
        break;
        case ListenerFunc::PROVISIONPAIRWISEDEVICES:
        {
            calledFunc = "provisionPairwiseDevicesListener";
        }
        break;
        default:
        {
            checkExAndRemoveListener(env);
            if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        }
        return;
    }

    jmethodID midL = env->GetMethodID(clsL, calledFunc.c_str(), "(Ljava/util/List;I)V");
    if (!midL)
    {
        checkExAndRemoveListener(env);
        if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
        return;
    }
    env->CallVoidMethod(jListener, midL, jResultList, (jint)hasError);
    if (env->ExceptionCheck())
    {
        LOGE("Java exception is thrown");
    }

    checkExAndRemoveListener(env);
    if (JNI_EDETACHED == ret) g_jvm->DetachCurrentThread();
}

void JniProvisionResultListner::checkExAndRemoveListener(JNIEnv* env)
{
    if (env->ExceptionCheck())
    {
        jthrowable ex = env->ExceptionOccurred();
        env->ExceptionClear();
        m_removeProvisionResultListener(env, m_jwListener);
        env->Throw((jthrowable)ex);
    }
    else
    {
        m_removeProvisionResultListener(env, m_jwListener);
    }
}
