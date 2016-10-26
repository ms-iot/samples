/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "JniRcsBundleInfo.h"

#include "Log.h"
#include "Verify.h"
#include "JniRcsObject.h"

#include "RCSBundleInfo.h"

#define LOG_TAG "JNI-RCSBundleInfo"

using namespace OIC::Service;

namespace
{
    RCSBundleInfo *getNativeBundleInfo(JNIEnv *env, jobject obj)
    {
        return getNativeHandleAs< RCSBundleInfo * >(env, obj);
    }
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetID
(JNIEnv *env, jobject obj)
{
    LOGI("nativeGetID");

    auto bundleInfo = getNativeBundleInfo(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return newStringObject(env, bundleInfo->getID());
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetPath
(JNIEnv *env, jobject obj)
{
    LOGI("nativeGetPath");

    auto bundleInfo = getNativeBundleInfo(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return newStringObject(env, bundleInfo->getPath());
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetActivatorName
(JNIEnv *env, jobject obj)
{
    LOGI("nativeGetActivatorName");

    auto bundleInfo = getNativeBundleInfo(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return newStringObject(env, bundleInfo->getActivatorName());
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetLibraryPath
(JNIEnv *env, jobject obj)
{
    LOGI("nativeGetLibraryPath");

    auto bundleInfo = getNativeBundleInfo(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return newStringObject(env, bundleInfo->getLibraryPath());
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetVersion
(JNIEnv *env, jobject obj)
{
    LOGI("nativeGetVersion");

    auto bundleInfo = getNativeBundleInfo(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return newStringObject(env, bundleInfo->getVersion());
}

