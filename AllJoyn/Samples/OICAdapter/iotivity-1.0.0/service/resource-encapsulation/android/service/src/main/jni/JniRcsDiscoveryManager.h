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

#ifndef JNI_RCS_DISCOVERY_MANAGER_H_
#define JNI_RCS_DISCOVERY_MANAGER_H_

#include <jni.h>

class JNIEnvWrapper;

void initRCSDiscoveryManager(JNIEnvWrapper*);
void clearRCSDiscoveryManager(JNIEnvWrapper*);

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_client_RcsDiscoveryManager_nativeDiscoverResource
(JNIEnv*, jclass, jstring address, jstring relativeURI, jstring resourceType, jobject listener);

#ifdef __cplusplus
}
#endif
#endif //JNI_RCS_DISCOVERY_MANAGER_H_
