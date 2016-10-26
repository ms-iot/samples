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

#include "JniRcsResourceContainer.h"
#include "JavaClasses.h"
#include "JavaExceptions.h"
#include "JNIEnvWrapper.h"
#include "Log.h"

#define LOG_TAG "JNI-Main"

#define JNI_CURRENT_VERSION JNI_VERSION_1_6

JavaVM *g_jvm;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    LOGI("JNI_OnLoad");
    JNIEnv *env;
    g_jvm = vm;

    if (g_jvm->GetEnv((void **)&env, JNI_CURRENT_VERSION) != JNI_OK)
    {
        LOGE("Failed to get the environment using GetEnv()");
        return JNI_ERR;
    }

    JNIEnvWrapper envWrapper { env };

    try
    {
        initJavaClasses(&envWrapper);
        initJavaExceptions(&envWrapper);
        initRCSObject(&envWrapper);
        initRCSResourceContainer(&envWrapper);
    }
    catch (const JavaException &)
    {
        if (env->ExceptionCheck()) env->ExceptionDescribe();
        return JNI_ERR;
    }

    return JNI_CURRENT_VERSION;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
    LOGI("JNI_OnUnload");
    JNIEnv *env;

    if (g_jvm->GetEnv((void **)&env, JNI_CURRENT_VERSION) != JNI_OK)
    {
        LOGE("Failed to get the environment using GetEnv()");
        return;
    }

    JNIEnvWrapper envWrapper { env };

    try
    {
        clearRCSResourceContainer(&envWrapper);
        clearRCSObject(&envWrapper);
        clearJavaExceptions(&envWrapper);
        clearJavaClasses(&envWrapper);
    }
    catch (const JavaException &)
    {
    }
}
