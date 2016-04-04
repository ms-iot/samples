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

#include "JniRcsObject.h"

#include "JavaClasses.h"
#include "JNIEnvWrapper.h"
#include "Log.h"
#include "Verify.h"

#define LOG_TAG "JNI-RCSObject"

jfieldID g_field_mNativeHandle;

void initRCSObject(JNIEnvWrapper* env)
{
    auto clsRCSObject = env->FindClass(PACKAGE_NAME "/RcsObject");

    g_field_mNativeHandle = env->GetFieldID(clsRCSObject, "mNativeHandle", "J");
}

void clearRCSObject(JNIEnvWrapper* env)
{
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsObject_nativeDispose(JNIEnv* env, jobject obj)
{
    LOGD("release nativeHandle!");
    releaseNativeHandle(env, obj);
}
