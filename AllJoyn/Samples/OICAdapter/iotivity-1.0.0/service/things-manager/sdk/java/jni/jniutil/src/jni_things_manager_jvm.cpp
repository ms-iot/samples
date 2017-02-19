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

#include <string>

#include "jni_things_manager_jvm.h"
#include "jni_group_manager.h"
#include "jni_things_configuration.h"
#include "jni_things_maintenance.h"
#include "jni_things_manager_util.h"
#include "JniOcResource.h"

#define TM_ERROR_JNI_FOUND_CLASS_FAILED -2005

/**
 * This class provides functions for initializing the Java class path and Java class.
 */
class JClassMap
{
    public:
        /**
         *  Java Class
         */
        jclass classRef;
        /**
         *  Java Class Path
         */
        const char *szClassPath;

        /**
         * constructor
         */
        JClassMap(const char *path)
            : classRef(NULL)
        {
            szClassPath = path;
        }
};

/**
 * This class provides functions for initializing the Java Class path
 *  and Java Class Object.
 */
class JObjectMap
{
    public:
        /**
         *  Java Object
         */
        jobject object;
        /**
         *  Java Class Path
         */
        const char *szClassPath;

        /**
         * @brief constructor
         */
        JObjectMap(const char *path)
            : object(NULL)
        {
            szClassPath = path;
        }
};

static JClassMap gJClassMapArray[] =
{
    JClassMap(TM_SERVICE_GROUP_MANAGER_CLASS_PATH),
    JClassMap(TM_SERVICE_THINGS_CONFIGURATION_CLASS_PATH),
    JClassMap(TM_SERVICE_THINGS_MAINTENANCE_CLASS_PATH),
    JClassMap(TM_SERVICE_OCRESOURCE_PATH),
    JClassMap(TM_SERVICE_OCREPRESENTATION_PATH),
    JClassMap(TM_SERVICE_HEADER_OPTION_PATH),
    JClassMap(TM_SERVICE_ACTIONSET_PATH),
    JClassMap(TM_SERVICE_CAPABILITY_PATH),
    JClassMap(TM_SERVICE_ACTION_PATH),
    JClassMap(TM_SERVICE_PLATFORM_CLASS_PATH),
    JClassMap(TM_SERVICE_PLATFORM_CONFIG_CLASS_PATH),
    JClassMap(TM_SERVICE_OCRESOURCEHANDLE_PATH)
};

static JObjectMap gJObjectMapArray[] =
{
    JObjectMap(TM_SERVICE_THINGS_CONFIGURATION_CLASS_PATH),
    JObjectMap(TM_SERVICE_THINGS_MAINTENANCE_CLASS_PATH)
};

static JNINativeMethod gGroupManagerMethodTable[] =
{
    { "nativeFindCandidateResource", "(Ljava/util/Vector;I)I", (void *) JNIGroupManagerFindCandidateResource},
    { "nativeSubscribeCollectionPresence", "(Lorg/iotivity/base/OcResource;)I", (void *) JNIGroupManagerSubscribeCollectionPresence},
    { "nativeBindResourceToGroup", "(Lorg/iotivity/base/OcResource;Lorg/iotivity/base/OcResourceHandle;)Lorg/iotivity/base/OcResourceHandle;", (void *) JNIGroupManagerBindResourceToGroup},
    { "nativeAddActionSet", "(Lorg/iotivity/base/OcResource;Lorg/iotivity/service/tm/ActionSet;)I", (void *) JNIGroupManagerAddActionSet},
    { "nativeExecuteActionSet", "(Lorg/iotivity/base/OcResource;Ljava/lang/String;)I", (void *) JNIGroupManagerExecuteActionSet},
    { "nativeExecuteActionSetWithDelay", "(Lorg/iotivity/base/OcResource;Ljava/lang/String;J)I", (void *) JNIGroupManagerExecuteActionSetWithDelay},
    { "nativeCancelActionSet", "(Lorg/iotivity/base/OcResource;Ljava/lang/String;)I", (void *) JNIGroupManagerCancelActionSet},
    { "nativeGetActionSet", "(Lorg/iotivity/base/OcResource;Ljava/lang/String;)I", (void *) JNIGroupManagerGetActionSet},
    { "nativeDeleteActionSet", "(Lorg/iotivity/base/OcResource;Ljava/lang/String;)I", (void *) JNIGroupManagerDeleteActionSet}
};

static JNINativeMethod gThingsConfigurationMethodTable[] =
{
    { "nativeUpdateConfigurations", "(Lorg/iotivity/base/OcResource;Ljava/util/Map;)I", (void *) JNIThingsConfigurationUpdateConfigurations},
    { "nativeGetConfigurations", "(Lorg/iotivity/base/OcResource;Ljava/util/Vector;)I", (void *) JNIThingsConfigurationGetConfigurations},
    { "nativeGetListOfSupportedConfigurationUnits", "()Ljava/lang/String;", (void *) JNIThingsConfigurationGetListOfSupportedConfigurationUnits},
    { "nativeDoBootstrap", "()I", (void *) JNIThingsConfigurationDoBootstrap},
};

static JNINativeMethod gThingsMaintenanceMethodTable[] =
{
    { "nativeReboot", "(Lorg/iotivity/base/OcResource;)I", (void *) JNIThingsMaintenanceReboot},
    { "nativeFactoryReset", "(Lorg/iotivity/base/OcResource;)I", (void *) JNIThingsMaintenanceFactoryReset},
    { "nativeGetListOfSupportedMaintenanceUnits", "()Ljava/lang/String;", (void *) JNIThingsMaintenanceGetListOfSupportedConfigurationUnits}

};

static int gGroupManagerMethodTableSize = sizeof(gGroupManagerMethodTable) / sizeof(
            gGroupManagerMethodTable[0]);

static int gThingsConfigurationMethodTableSize = sizeof(gThingsConfigurationMethodTable) / sizeof(
            gThingsConfigurationMethodTable[0]);

static int gThingsMaintenanceMethodTableSize = sizeof(gThingsMaintenanceMethodTable) / sizeof(
            gThingsMaintenanceMethodTable[0]);


int InitializeJClassMapArray(JNIEnv *env)
{
    LOGI("InitializeJClassMapArray: Enter");

    unsigned int nLen = sizeof(gJClassMapArray) / sizeof(JClassMap);

    for (unsigned int i = 0; i < nLen; i++)
    {
        jclass classRef = env->FindClass(gJClassMapArray[i].szClassPath);
        if (NULL == classRef)
        {
            LOGE("FindClass failed for [%s]", gJClassMapArray[i].szClassPath);
            return -1;
        }
        gJClassMapArray[i].classRef = (jclass)env->NewGlobalRef(classRef);
        env->DeleteLocalRef(classRef);
    }

    LOGI("InitializeJClassMapArray: Exit");
    return 0;
}

jclass GetJClass(const char *szClassPath)
{
    unsigned int nLen = sizeof(gJClassMapArray) / sizeof(JClassMap);

    jclass classRef = NULL;

    for (unsigned int i = 0; i < nLen; i++)
    {
        if (0 == strcmp(gJClassMapArray[i].szClassPath, szClassPath))
        {
            classRef = gJClassMapArray[i].classRef;
            break;
        }
    }

    return classRef;
}

void DeleteClassMapArray(JNIEnv *env)
{
    LOGI("DeleteClassMapArray: Enter");

    unsigned int nLen = sizeof(gJClassMapArray) / sizeof(JClassMap);
    for (unsigned int i = 0; i < nLen; i++)
    {
        if (NULL != gJClassMapArray[i].classRef)
        {
            env->DeleteGlobalRef(gJClassMapArray[i].classRef);
            gJClassMapArray[i].classRef = NULL;
        }
    }

    LOGI("DeleteClassMapArray: Exit");
}

int InitializeJObjectMapArray(JNIEnv *env)
{
    LOGI("InitializeJObjectMapArray: Enter");

    unsigned int nLen = sizeof(gJObjectMapArray) / sizeof(JObjectMap);

    for (unsigned int i = 0; i < nLen; i++)
    {
        jclass classRef = env->FindClass(gJObjectMapArray[i].szClassPath);
        if (NULL == classRef)
        {
            LOGE("InitializeJObjectMapArray: FindClass failed for [%s]", gJObjectMapArray[i].szClassPath);
            return -1;
        }

        std::string methodSignature = "()L";
        methodSignature.append(gJObjectMapArray[i].szClassPath);
        methodSignature.append(";");

        // Get the object form "getInstance"
        jmethodID methodid = env->GetStaticMethodID(classRef, "getInstance", methodSignature.c_str());
        if (NULL == methodid)
        {
            LOGE("InitializeJObjectMapArray: GetStaticMethodID failed for [%s]",
                 gJObjectMapArray[i].szClassPath);
            return -1;
        }

        // Get the singleton object
        jobject objectRef = (jobject)env->CallStaticObjectMethod(classRef, methodid);
        if (NULL == objectRef)
        {
            LOGE("InitializeJObjectMapArray: CallStaticObjectMethod failed for [%s]",
                 gJObjectMapArray[i].szClassPath);
            return -1;
        }

        gJObjectMapArray[i].object = (jobject)env->NewGlobalRef(objectRef);
        env->DeleteLocalRef(classRef);
    }

    LOGI("InitializeJObjectMapArray: Exit");
    return 0;
}

jobject GetJObjectInstance(const char *szClassPath)
{
    unsigned int nLen = sizeof(gJObjectMapArray) / sizeof(JObjectMap);

    jobject object = NULL;

    for (unsigned int i = 0; i < nLen; i++)
    {
        if (0 == strcmp(gJObjectMapArray[i].szClassPath, szClassPath))
        {
            object = gJObjectMapArray[i].object;
            break;
        }
    }

    return object;
}

void DeleteObjectMapArray(JNIEnv *env)
{
    LOGI("DeleteObjectMapArray: Enter");

    unsigned int nLen = sizeof(gJObjectMapArray) / sizeof(JObjectMap);
    for (unsigned int i = 0; i < nLen; i++)
    {
        if (NULL != gJObjectMapArray[i].object)
        {
            env->DeleteGlobalRef(gJObjectMapArray[i].object);
            gJObjectMapArray[i].object = NULL;
        }
    }

    LOGI("DeleteObjectMapArray: Exit");
}

JavaVM *ThingsManagerJVM::m_jvm = NULL;
std::mutex ThingsManagerJVM::m_currentThreadMutex;
JNIEnv *ThingsManagerJVM::getEnv()
{
    std::unique_lock<std::mutex> scoped_lock(m_currentThreadMutex);

    if (NULL == m_jvm)
    {
        LOGE("Failed to get JVM");
        return NULL;
    }

    JNIEnv *env = NULL;
    jint ret = m_jvm->GetEnv((void **)&env, JNI_CURRENT_VERSION);
    switch (ret)
    {
        case JNI_OK:
            return env;
        case JNI_EDETACHED:
            if (0 > m_jvm->AttachCurrentThread(&env, NULL))
            {
                LOGE("Failed to attach current thread to env");
                return NULL;
            }
            return env;
        case JNI_EVERSION:
            LOGE("JNI version not supported");
        default:
            LOGE("Failed to get the environment");
            return NULL;
    }
}

void ThingsManagerJVM::releaseEnv()
{
    std::unique_lock<std::mutex> scoped_lock(m_currentThreadMutex);

    if (0 == m_jvm)
    {
        LOGE("Failed to release JVM");
        return;
    }

    m_jvm->DetachCurrentThread();
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    LOGD("JNI_OnLoad: Enter");

    if (!vm)
    {
        LOGE("JNI_OnLoad: vm is invalid");
        return JNI_ERR;
    }

    JNIEnv *env = NULL;
    if (JNI_OK != vm->GetEnv((void **) &env, JNI_CURRENT_VERSION))
    {
        LOGE("JNI_OnLoad: Version check is failed!");
        return JNI_ERR;
    }

    if (0 != InitializeJClassMapArray(env))
    {
        LOGE("JNI_OnLoad: Initialize JClass Array failed!");
        return JNI_ERR;
    }

    if (0 != InitializeJObjectMapArray(env))
    {
        LOGE("JNI_OnLoad: Initialize JObject Array failed!");
        return JNI_ERR;
    }

    // Group Manager
    jclass groupManagerClassRef = GetJClass(TM_SERVICE_GROUP_MANAGER_CLASS_PATH);

    if (NULL == groupManagerClassRef)
    {
        LOGE("JNI_OnLoad: GetJClass gThingsManagerClass failed !");
        return JNI_ERR;
    }

    env->RegisterNatives(groupManagerClassRef, gGroupManagerMethodTable,
                         gGroupManagerMethodTableSize);

    //Things Configuration
    jclass thingsConfigurationClassRef = GetJClass(TM_SERVICE_THINGS_CONFIGURATION_CLASS_PATH);

    if (NULL == thingsConfigurationClassRef)
    {
        LOGE("JNI_OnLoad: GetJClass gThingsManagerClass failed !");
        return JNI_ERR;
    }

    env->RegisterNatives(thingsConfigurationClassRef, gThingsConfigurationMethodTable,
                         gThingsConfigurationMethodTableSize);

    //Things Maintenance
    jclass thingsMaintenanceClassRef = GetJClass(TM_SERVICE_THINGS_MAINTENANCE_CLASS_PATH);

    if (NULL == thingsMaintenanceClassRef)
    {
        LOGE("JNI_OnLoad: GetJClass gThingsManagerClass failed !");
        return JNI_ERR;
    }

    env->RegisterNatives(thingsMaintenanceClassRef, gThingsMaintenanceMethodTable,
                         gThingsMaintenanceMethodTableSize);

    ThingsManagerJVM::m_jvm = vm;

    LOGI("JNI_OnLoad: Exit");
    return JNI_CURRENT_VERSION;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
    LOGD("JNI_OnUnload: Enter");

    JNIEnv *env = NULL;
    if (JNI_OK != vm->GetEnv((void **)&env, JNI_CURRENT_VERSION))
    {
        LOGE("JNI_OnLoad: Version check is failed!");
        return;
    }

    // delete all class references
    DeleteClassMapArray(env);

    // delete all jobject
    DeleteObjectMapArray(env);

    LOGD("JNI_OnUnload: Exit");
}

#ifdef __cplusplus
}
#endif
