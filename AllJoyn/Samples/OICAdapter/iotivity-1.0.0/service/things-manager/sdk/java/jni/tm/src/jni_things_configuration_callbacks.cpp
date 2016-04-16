/* *****************************************************************
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

#include "jni_things_configuration_callbacks.h"
#include "JniOcResource.h"
#include "ThingsConfiguration.h"
#include "jni_things_manager_jvm.h"
#include "jni_things_manager_util.h"
#include "jni_string.h"

#define LOG_TAG "ThingsConfigurationCallbacks"

#define METHOD_ONUPDATE_CONF_CALLBACK    "(" TM_JAVA_VECTOR_TYPE TM_SERVICE_OCREPRESENTATION_TYPE"I)V"
#define METHOD_ONGET_CONF_CALLBACK    "(" TM_JAVA_VECTOR_TYPE TM_SERVICE_OCREPRESENTATION_TYPE"I)V"
#define METHOD_ONBOOT_STRAP_CALLBACK    "(" TM_JAVA_VECTOR_TYPE TM_SERVICE_OCREPRESENTATION_TYPE"I)V"

void ThingsConfigurationCallbacks::onUpdateConfigurationsResponse(const OC::HeaderOptions
        &headerOptions,
        const OC::OCRepresentation &rep, const int eCode)
{
    LOGI("UpdateConfigurations : Enter");

    ThingsConfigurationCallbacks::invokeCallback(headerOptions, rep, eCode,
            "onUpdateConfigurationsCallback",
            METHOD_ONUPDATE_CONF_CALLBACK);

    LOGI("UpdateConfigurations : Exit");
}

void ThingsConfigurationCallbacks::onGetConfigurationsResponse(const OC::HeaderOptions
        &headerOptions,
        const OC::OCRepresentation &rep, const int eCode)
{
    LOGI("GetConfigurations : Enter");

    ThingsConfigurationCallbacks::invokeCallback(headerOptions, rep, eCode,
            "onGetConfigurationsCallback",
            METHOD_ONGET_CONF_CALLBACK);

    LOGI("GetConfigurations : Exit");
}

void ThingsConfigurationCallbacks::onBootStrapResponse(const OC::HeaderOptions &headerOptions,
        const OC::OCRepresentation &rep, const int eCode)
{
    LOGI("BootStrap : Enter");

    ThingsConfigurationCallbacks::invokeCallback(headerOptions, rep, eCode, "onBootStrapCallback",
            METHOD_ONBOOT_STRAP_CALLBACK);

    LOGI("BootStrap : Exit");
}

void ThingsConfigurationCallbacks::invokeCallback(const OC::HeaderOptions &headerOptions,
        const OC::OCRepresentation &rep, const int eCode, const char  *callbackName,
        const char *signature)
{
    LOGI("InvokeCallback : Enter %s", callbackName);

    JNIEnv *env = ThingsManagerJVM::getEnv();
    if (env == NULL)
    {
        LOGE("InvokeCallback : Getting JNIEnv failed");
        return;
    }

    // Get ThingsManagerCallback class reference
    jclass thingsConfigurationCallbacks = GetJClass(TM_SERVICE_THINGS_CONFIGURATION_CLASS_PATH);
    if (NULL == thingsConfigurationCallbacks)
    {
        LOGE("InvokeCallback : GetJClass TMServiceCallbackInterface failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    // Get the ThingsManagerCallback class instance
    jobject jobjectCallback = GetJObjectInstance(TM_SERVICE_THINGS_CONFIGURATION_CLASS_PATH);
    if (NULL == jobjectCallback)
    {
        LOGE("InvokeCallback: getInstance( %s) failed!", TM_SERVICE_THINGS_CONFIGURATION_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID method_id = env->GetMethodID(thingsConfigurationCallbacks, callbackName, signature);
    if (!method_id)
    {
        LOGE("InvokeCallback : GetMethodID failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    if ((env)->ExceptionCheck())
    {
        LOGE("InvokeCallback : ExceptionCheck failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    // Convert vector<OC:HeaderOption::OCHeaderOption> to java type
    jclass vectorCls = env->FindClass(TM_JAVA_VECTOR_CLASS_PATH);
    if (!vectorCls)
    {
        LOGE("InvokeCallback: failed to get %s class reference", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID constr = env->GetMethodID(vectorCls, "<init>", "()V");
    if (!constr)
    {
        LOGE("InvokeCallback: failed to get %s constructor", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jobject vectorObj = env->NewObject(vectorCls, constr);
    if (!vectorObj)
    {
        LOGE("InvokeCallback: failed to create a %s object", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID addElement = env->GetMethodID(vectorCls, "addElement", "(Ljava/lang/Object;)V");
    if (NULL == addElement)
    {
        LOGE("InvokeCallback: failed to create a addElement method");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jobject headerOptionTemp;
    for (int i = 0; i < headerOptions.size(); i++)
    {
        headerOptionTemp = OcHeaderOptionToJava(env, headerOptions[i]);
        env->CallVoidMethod(vectorObj, addElement, headerOptionTemp);
    }

    // Convert OCRepresentation to java type
    jobject jrepresentation = OcRepresentationToJava(env, (jlong) reinterpret_cast<jlong>(&rep));
    if (!jrepresentation)
    {
        LOGE("InvokeCallback : cannot create OCRepresentation class");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    env->CallVoidMethod(jobjectCallback, method_id, vectorObj, jrepresentation, (jint)eCode);

    if ((env)->ExceptionCheck())
    {
        LOGE("InvokeCallback : CallVoidMethod failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    ThingsManagerJVM::releaseEnv();
    LOGI("InvokeCallback : Exit %s", callbackName);
}


