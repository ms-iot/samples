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
#include "jni_things_maintenance.h"
#include "JniOcResource.h"
#include "JniOcResourceHandle.h"
#include "ThingsMaintenance.h"
#include "ActionSet.h"
#include "jni_things_manager_jvm.h"
#include "jni_things_manager_util.h"
#include "jni_things_maintenance_callbacks.h"
#include "jni_action_set.h"

using namespace OC;
using namespace OIC;

/**
 * ThingsMaintenance static object
 */
static ThingsMaintenance g_ThingsMaintenance;

JNIEXPORT jint JNICALL JNIThingsMaintenanceReboot(JNIEnv *env, jobject interfaceObject,
        jobject resource)
{
    LOGI("JNIThingsMaintenanceReboot: Enter");

    if (!resource)
    {
        LOGE("JNIThingsMaintenanceReboot: resource is NULL!");
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
        LOGE("JNIThingsMaintenanceReboot: Failed to get OCResource object!");
        return ocResult;
    }

    ocResult = g_ThingsMaintenance.reboot(ocResource, &ThingsMaintenanceCallbacks::onRebootResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIThingsMaintenanceReboot: reboot failed!");
        return ocResult;
    }

    LOGI("JNIThingsMaintenanceReboot: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIThingsMaintenanceFactoryReset(JNIEnv *env, jobject interfaceObject,
        jobject resource)
{
    LOGI("JNIThingsMaintenanceFactoryReset: Enter");

    if (!resource)
    {
        LOGE("JNIThingsMaintenanceFactoryReset: resource is NULL!");
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
        LOGE("JNIThingsMaintenanceFactoryReset: Failed to get OCResource object!");
        return ocResult;
    }

    ocResult = g_ThingsMaintenance.factoryReset(ocResource,
               &ThingsMaintenanceCallbacks::onFactoryResetResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIThingsMaintenanceFactoryReset: factoryReset failed!");
        return ocResult;
    }

    LOGI("JNIThingsMaintenanceFactoryReset: Exit");
    return ocResult;
}

JNIEXPORT jstring JNICALL JNIThingsMaintenanceGetListOfSupportedConfigurationUnits
(JNIEnv *env, jobject interfaceObject)
{
    LOGI("JNIThingsMaintenanceGetListOfSupportedConfigurationUnits: Enter");

    std::string configListString = g_ThingsMaintenance.getListOfSupportedMaintenanceUnits();
    jstring jConfigListString =  env->NewStringUTF(configListString.c_str());

    LOGI("JNIThingsMaintenanceGetListOfSupportedConfigurationUnits: Exit");
    return jConfigListString;
}
