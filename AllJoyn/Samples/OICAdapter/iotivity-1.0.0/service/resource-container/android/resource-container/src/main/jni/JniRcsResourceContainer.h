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

/** @file   jni_re_rcs_resource_container.h
 *
 *   @brief  This file contains the declaration of RCSResourceContainer
 *    APIs for JNI implementation
 */

#ifndef JNI_RCS_RESOURCE_CONTAINER_H_
#define JNI_RCS_RESOURCE_CONTAINER_H_

#include <jni.h>

class JNIEnvWrapper;

void initRCSResourceContainer(JNIEnvWrapper *);
void clearRCSResourceContainer(JNIEnvWrapper *);

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeStartContainer
(JNIEnv *, jobject, jstring configFile);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeStopContainer
(JNIEnv *, jobject);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeAddBundle
(JNIEnv *, jobject, jstring bundleId, jstring bundleUri, jstring bundlePath, jstring activator,
 jobject params);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeListBundles
(JNIEnv *, jobject);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeRemoveBundle
(JNIEnv *, jobject, jstring bundleId);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeStartBundle
(JNIEnv *, jobject, jstring bundleId);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeStopBundle
(JNIEnv *, jobject, jstring bundleId);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeAddResourceConfig
(JNIEnv *, jobject, jstring bundleId, jstring resourceUri, jobject params);

JNIEXPORT void JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeRemoveResourceConfig
(JNIEnv *, jobject, jstring bundleId, jstring resourceUri);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_resourcecontainer_RcsResourceContainer_nativeListBundleResources
(JNIEnv *, jobject, jstring bundleId);

#ifdef __cplusplus
}
#endif
#endif //JNI_RCS_RESOURCE_CONTAINER_H_
