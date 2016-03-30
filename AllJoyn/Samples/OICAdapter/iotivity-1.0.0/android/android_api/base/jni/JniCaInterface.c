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
#include <stdio.h>
#include "cainterface.h"
#include "JniCaInterface.h"

#define  LOG_TAG   "JNI_CA_INTERFACE"
#define  LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    LOGI("CaInterface_initialize");
    CANativeJNISetJavaVM(jvm);

    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    return;
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaInterface_initialize
(JNIEnv *env, jclass clazz, jobject context)
{
    LOGI("CaInterface_initialize");

    CANativeJNISetContext(env, context);

    CAResult_t res = CAInitialize();

    if (CA_STATUS_OK != res)
    {
        LOGE("Could not Initialize");
    }
}