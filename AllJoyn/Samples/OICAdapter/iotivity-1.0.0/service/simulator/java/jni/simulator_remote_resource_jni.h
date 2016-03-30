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

#ifndef SIMULATOR_REMOTE_RESOURCE_JNI_H_
#define SIMULATOR_REMOTE_RESOURCE_JNI_H_

#include <jni.h>
#include "simulator_remote_resource.h"

class JniSimulatorRemoteResource
{
    public:
        JniSimulatorRemoteResource(SimulatorRemoteResourceSP &resource)
            : m_resource(resource) {};
        static SimulatorRemoteResourceSP getResourceHandle(JNIEnv *env, jobject object);
    private:
        SimulatorRemoteResourceSP m_resource;
};

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_startObserve
(JNIEnv *env, jobject thiz, jint observeType, jobject jQueryParamsMap, jobject jListener);

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_stopObserve
(JNIEnv *env, jobject thiz);

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_nativeGet
(JNIEnv *env, jobject thiz, jstring jResourceInterface,
 jobject jQueryParamsMap, jobject jListener);

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_nativePut
(JNIEnv *env, jobject thiz, jstring jResourceInterface,
 jobject jRepresentation, jobject jQueryParamsMap, jobject jListener);

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_nativePost
(JNIEnv *env, jobject thiz, jstring jResourceInterface,
 jobject jRepresentation, jobject jQueryParamsMap, jobject jListener);

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_setConfigInfo
(JNIEnv *env, jobject thiz, jstring jConfigPath);

JNIEXPORT jint JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_startVerification
(JNIEnv *env, jobject thiz, jint jReqType, jobject jListener);

JNIEXPORT void JNICALL
Java_org_oic_simulator_clientcontroller_SimulatorRemoteResource_stopVerification
(JNIEnv *env, jobject thiz, jint jId);

#ifdef __cplusplus
}
#endif

#endif

