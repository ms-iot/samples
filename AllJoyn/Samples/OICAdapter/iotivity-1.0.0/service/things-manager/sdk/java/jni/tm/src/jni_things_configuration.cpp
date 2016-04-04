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
#include "jni_things_configuration.h"
#include "JniOcResource.h"
#include "JniOcResourceHandle.h"
#include "ThingsConfiguration.h"
#include "ActionSet.h"
#include "jni_things_manager_jvm.h"
#include "jni_things_manager_util.h"
#include "jni_things_configuration_callbacks.h"
#include "jni_action_set.h"

using namespace OC;
using namespace OIC;

/**
 * ThingsConfiguration static object
 */
static ThingsConfiguration g_ThingsConfiguration;

JNIEXPORT jint JNICALL JNIThingsConfigurationUpdateConfigurations(JNIEnv *env,
        jobject interfaceObject,
        jobject resource, jobject configurations)
{
    LOGI("JNIThingsConfigurationUpdateConfigurations: Enter");

    if ((!resource) || (!configurations))
    {
        LOGE("JNIThingsConfigurationUpdateConfigurations: resource or configurations is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIThingsConfigurationUpdateConfigurations: Failed to get OCResource object!");
        return ocResult;
    }

    std::map<std::string, std::string> configurationsMap;
    configurationsMap = convertStringMap(env, configurations);
    ocResult =  g_ThingsConfiguration.updateConfigurations(ocResource, configurationsMap,
                &ThingsConfigurationCallbacks::onUpdateConfigurationsResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIThingsConfigurationUpdateConfigurations: updateConfigurations failed!");
        return ocResult;
    }

    LOGI("JNIThingsConfigurationUpdateConfigurations: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIThingsConfigurationGetConfigurations(JNIEnv *env, jobject interfaceObject,
        jobject resource, jobject configurations)
{
    LOGI("JNIThingsConfigurationGetConfigurations: Enter");

    if ((!resource) || (!configurations))
    {
        LOGE("JNIThingsConfigurationGetConfigurations: resource or configurations is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIThingsConfigurationGetConfigurations: Failed to get OCResource object!");
        return ocResult;
    }

    ocResult = g_ThingsConfiguration.getConfigurations(ocResource,
               (convertStringVector(env, configurations)),
               &ThingsConfigurationCallbacks::onGetConfigurationsResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIThingsConfigurationGetConfigurations: getConfigurations failed!");
        return ocResult;
    }

    LOGI("JNIThingsConfigurationGetConfigurations: Exit");
    return ocResult;
}

JNIEXPORT jstring JNICALL JNIThingsConfigurationGetListOfSupportedConfigurationUnits(JNIEnv *env,
        jobject interfaceObject)
{
    LOGI("JNIThingsConfigurationGetListOfSupportedConfigurationUnits: Enter");

    std::string configListString = g_ThingsConfiguration.getListOfSupportedConfigurationUnits();
    jstring jConfigListString =  env->NewStringUTF(configListString.c_str());

    LOGI("JNIThingsConfigurationGetListOfSupportedConfigurationUnits: Exit");
    return jConfigListString;
}

JNIEXPORT jint JNICALL JNIThingsConfigurationDoBootstrap(JNIEnv *env, jobject interfaceObject)
{
    LOGI("JNIThingsConfigurationDoBootstrap: Enter");

    OCStackResult ocResult = OC_STACK_ERROR;
    try
    {
        ocResult  = g_ThingsConfiguration.doBootstrap(&ThingsConfigurationCallbacks::onBootStrapResponse);
        if (OC_STACK_OK != ocResult)
        {
            LOGE("JNIThingsConfigurationDoBootstrap: doBootstrap failed!");
            return ocResult;
        }
    }
    catch (InitializeException &e)
    {
        LOGE("JNIThingsConfigurationDoBootstrap: Exception occurred! %s, %d", e.reason().c_str(),
             e.code());
        return ocResult;
    }

    LOGI("JNIThingsConfigurationDoBootstrap: Exit");
    return ocResult;
}


