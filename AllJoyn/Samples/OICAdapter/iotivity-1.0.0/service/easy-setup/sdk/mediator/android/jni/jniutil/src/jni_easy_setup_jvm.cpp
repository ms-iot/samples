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
#include "jni_easy_setup_jvm.h"
#include <string>
#include "jni_easy_setup.h"

/**
 * @class   JClassMap
 * @brief   This class provides functions for initializing the Java class path and Java class.
 *
 */
class JClassMap {
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
     * @brief constructor
     */
    JClassMap(const char *path) :
            classRef(NULL) {
        szClassPath = path;
    }
};

/**
 * @class   JObjectMap
 * @brief   This class provides functins for initializing the Java Class path and Java Class
 * Object.
 *
 */
class JObjectMap {
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
    JObjectMap(const char *path) :
            object(NULL) {
        szClassPath = path;
    }
};

static JClassMap gJClassMapArray[] = { JClassMap(
        EASY_SETUP_SERVICE_NATIVE_API_CLASS_PATH), JClassMap(
        EASY_SETUP_SERVICE_CALLBACK_NATIVE_API_CLASS_PATH) };

static JObjectMap gJObjectMapArray[] = { JObjectMap(
        EASY_SETUP_SERVICE_CALLBACK_NATIVE_API_CLASS_PATH) };

static JNINativeMethod gEasySetupMethodTable[] = {
    { "InitEasySetup", "()V", (void *) JNIInitEasySetup },
    { "TerminateEasySetup", "()V", (void *) JNITerminateEasySetup },
    { "ProvisionEnrollee", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V",
                                (void *) JNIProvisionEnrollee },
    { "StopEnrolleeProvisioning", "(I)V", (void *) JNIStopEnrolleeProvisioning }, };

static int gEasySetupMethodTableSize = sizeof(gEasySetupMethodTable)
        / sizeof(gEasySetupMethodTable[0]);

int InitializeJClassMapArray(JNIEnv *env) {
    LOGI("InitializeJClassMapArray: Enter");

    unsigned int nLen = sizeof(gJClassMapArray) / sizeof(JClassMap);

    for (unsigned int i = 0; i < nLen; i++) {
        jclass classRef = env->FindClass(gJClassMapArray[i].szClassPath);
        if (NULL == classRef) {
            LOGE("FindClass failed for [%s]", gJClassMapArray[i].szClassPath);
            return -1;
        }
        gJClassMapArray[i].classRef = (jclass) env->NewGlobalRef(classRef);
        env->DeleteLocalRef(classRef);
    }

    LOGI("InitializeJClassMapArray: Exit");
    return 0;
}

jclass GetJClass(const char *szClassPath) {
    unsigned int nLen = sizeof(gJClassMapArray) / sizeof(JClassMap);

    jclass classRef = NULL;

    for (unsigned int i = 0; i < nLen; i++) {
        if (0 == strcmp(gJClassMapArray[i].szClassPath, szClassPath)) {
            classRef = gJClassMapArray[i].classRef;
            break;
        }
    }

    return classRef;
}

void DeleteClassMapArray(JNIEnv *env) {
    LOGI("DeleteClassMapArray: Enter");

    unsigned int nLen = sizeof(gJClassMapArray) / sizeof(JClassMap);
    for (unsigned int i = 0; i < nLen; i++) {
        if (NULL != gJClassMapArray[i].classRef) {
            env->DeleteGlobalRef(gJClassMapArray[i].classRef);
            gJClassMapArray[i].classRef = NULL;
        }
    }

    LOGI("DeleteClassMapArray: Exit");
}

int InitializeJObjectMapArray(JNIEnv *env) {
    LOGI("InitializeJObjectMapArray: Enter");

    unsigned int nLen = sizeof(gJObjectMapArray) / sizeof(JObjectMap);

    for (unsigned int i = 0; i < nLen; i++) {
        jclass classRef = env->FindClass(gJObjectMapArray[i].szClassPath);
        if (NULL == classRef) {
            LOGE("InitializeJObjectMapArray: FindClass failed for [%s]",
                    gJObjectMapArray[i].szClassPath);
            return -1;
        }

        std::string methodSignature = "()L";
        methodSignature.append(gJObjectMapArray[i].szClassPath);
        methodSignature.append(";");

        // Get the object form "getInstance"
        jmethodID methodid = env->GetStaticMethodID(classRef, "getInstance",
                methodSignature.c_str());
        if (NULL == methodid) {
            LOGE("InitializeJObjectMapArray: GetStaticMethodID failed for [%s]",
                    gJObjectMapArray[i].szClassPath);
            return -1;
        }

        // Get the singleton object
        jobject objectRef = (jobject) env->CallStaticObjectMethod(classRef,
                methodid);
        if (NULL == objectRef) {
            LOGE(
                    "InitializeJObjectMapArray: CallStaticObjectMethod failed for [%s]",
                    gJObjectMapArray[i].szClassPath);
            return -1;
        }

        gJObjectMapArray[i].object = (jobject) env->NewGlobalRef(objectRef);
        env->DeleteLocalRef(classRef);
    }

    LOGI("InitializeJObjectMapArray: Exit");
    return 0;
}

jobject GetJObjectInstance(const char *szClassPath) {
    unsigned int nLen = sizeof(gJObjectMapArray) / sizeof(JObjectMap);

    jobject object = NULL;

    for (unsigned int i = 0; i < nLen; i++) {
        if (0 == strcmp(gJObjectMapArray[i].szClassPath, szClassPath)) {
            object = gJObjectMapArray[i].object;
            break;
        }
    }

    return object;
}

void DeleteObjectMapArray(JNIEnv *env) {
    LOGI("DeleteObjectMapArray: Enter");

    unsigned int nLen = sizeof(gJObjectMapArray) / sizeof(JObjectMap);
    for (unsigned int i = 0; i < nLen; i++) {
        if (NULL != gJObjectMapArray[i].object) {
            env->DeleteGlobalRef(gJObjectMapArray[i].object);
            gJObjectMapArray[i].object = NULL;
        }
    }

    LOGI("DeleteObjectMapArray: Exit");
}

JavaVM *EasySetupJVM::m_jvm = NULL;
std::mutex EasySetupJVM::m_currentThreadMutex;
JNIEnv *EasySetupJVM::getEnv() {
    std::unique_lock < std::mutex > scoped_lock(m_currentThreadMutex);

    if (NULL == m_jvm) {
        LOGE("Failed to get JVM");
        return NULL;
    }

    JNIEnv *env = NULL;
    jint ret = m_jvm->GetEnv((void **) &env, JNI_CURRENT_VERSION);
    switch (ret) {
    case JNI_OK:
        return env;
    case JNI_EDETACHED:
        if (0 > m_jvm->AttachCurrentThread(&env, NULL)) {
            LOGE("Failed to attach current thread to env");
            return NULL;
        }
        return env;
    case JNI_EVERSION:
        LOGE("JNI version not supported");
        return NULL;
    default:
        LOGE("Failed to get the environment");
        return NULL;
    }
}

void EasySetupJVM::releaseEnv() {
    std::unique_lock < std::mutex > scoped_lock(m_currentThreadMutex);

    if (0 == m_jvm) {
        LOGE("Failed to release JVM");
        return;
    }

    m_jvm->DetachCurrentThread();
}

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("JNI_OnLoad: Enter");

    if (!vm) {
        LOGE("JNI_OnLoad: vm is invalid");
        return JNI_ERR;
    }

    JNIEnv *env = NULL;
    if (JNI_OK != vm->GetEnv((void **) &env, JNI_CURRENT_VERSION)) {
        LOGE("JNI_OnLoad: Version check is failed!");
        return JNI_ERR;
    }

    if (0 != InitializeJClassMapArray(env)) {
        LOGE("JNI_OnLoad: Initialize JClass Array failed!");
        return JNI_ERR;
    }

    if (0 != InitializeJObjectMapArray(env)) {
        LOGE("JNI_OnLoad: Initialize JObject Array failed!");
        return JNI_ERR;
    }

    jclass easySetupClassRef = GetJClass(
            EASY_SETUP_SERVICE_NATIVE_API_CLASS_PATH);
    if (NULL == easySetupClassRef) {
        LOGE("JNI_OnLoad: GetJClass gEasySetupClass failed !");
        return JNI_ERR;
    }
    env->RegisterNatives(easySetupClassRef, gEasySetupMethodTable,
            gEasySetupMethodTableSize);

    EasySetupJVM::m_jvm = vm;

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

