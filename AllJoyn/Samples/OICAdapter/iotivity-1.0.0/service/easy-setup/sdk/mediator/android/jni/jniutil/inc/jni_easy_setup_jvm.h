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

/**
 * @file   jni_easy_setup_jvm.h
 *
 * @brief  This file contains the essential declarations and functions required
 *            for JNI implementation
 */

#ifndef __JNI_EASY_SETUP_JVM_H_
#define __JNI_EASY_SETUP_JVM_H_

#include <jni.h>
#include <thread>
#include <mutex>

#define EASY_SETUP_SERVICE_NATIVE_API_CLASS_PATH    "org/iotivity/service/easysetup/core/EasySetupManager"
#define EASY_SETUP_SERVICE_NATIVE_API_CLASS_TYPE    "Lorg/iotivity/service/easysetup/core/EasySetupManager;"
#define EASY_SETUP_SERVICE_CALLBACK_NATIVE_API_CLASS_PATH    "org/iotivity/service/easysetup/core/EasySetupCallbackHandler"
#define EASY_SETUP_SERVICE_CALLBACK_NATIVE_API_CLASS_TYPE    "Lorg/iotivity/service/easysetup/core/EasySetupCallbackHandler;"

#define EASY_SETUP_JAVA_STRING_TYPE "Ljava/lang/String;"
#define EASY_SETUP_JAVA_INTEGER_TYPE "I"

/**
 * @class   EasySetupJVM
 * @brief   This class provides functions related to JNI Environment.
 *
 */
class EasySetupJVM {
public:
    /**
     * @brief destructor
     */
    ~EasySetupJVM() {
    }
    ;

    /**
     * @brief  Get JVM instance
     */
    static JNIEnv *getEnv();

    /**
     * @brief  Release aquired JVM instance
     */
    static void releaseEnv();

public:
    /**
     *  Java VM pointer
     */
    static JavaVM *m_jvm;

private:
    /**
     * @brief constructor
     */
    EasySetupJVM();

    /**
     *  Mutex for thread synchronization
     */
    static std::mutex m_currentThreadMutex;
};

#ifdef __cplusplus
extern "C" {
#endif
jclass GetJClass(const char *szClassPath);
jobject GetJObjectInstance(const char *szClassPath);
#ifdef __cplusplus
}
#endif
#endif //__JNI_EASY_SETUP_JVM_H_

