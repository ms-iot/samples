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

#ifndef JNI_RCS_RESOURCE_OBJECT_H
#define JNI_RCS_RESOURCE_OBJECT_H

#include <jni.h>

class JNIEnvWrapper;

void initRCSResourceObject(JNIEnvWrapper*);
void clearRCSResourceObject(JNIEnvWrapper*);

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL
Java_org_iotivity_service_server_RcsResponse_nativeGetDefaultErrorCode
(JNIEnv*, jclass);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeBuild
(JNIEnv*, jclass, jstring uri, jstring type, jstring interface, jboolean isObservable,
        jboolean isDiscovervable, jobject attrs);

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetAttribute
(JNIEnv*, jobject, jstring key, jobject value);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetAttributeValue
(JNIEnv*, jobject, jstring key);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeRemoveAttribute
(JNIEnv*, jobject, jstring key);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeContainsAttribute
(JNIEnv*, jobject, jstring key);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetAttributes
(JNIEnv*, jobject);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeIsObservable
(JNIEnv*, jobject);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeIsDiscoverable
(JNIEnv*, jobject);

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetGetRequestHandler
(JNIEnv*, jobject, jobject handler);

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetSetRequestHandler
(JNIEnv*, jobject, jobject handler);

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeAddAttributeUpdatedListener
(JNIEnv*, jobject, jstring key, jobject listenr);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeRemoveAttributeUpdatedListener
(JNIEnv*, jobject, jstring key);

JNIEXPORT void JNICALL Java_org_iotivity_service_server_RcsResourceObject_nativeNotify
(JNIEnv*, jobject);

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetAutoNotifyPolicy
(JNIEnv*, jobject, jobject policyObj);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetAutoNotifyPolicy
(JNIEnv*, jobject);

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetSetRequestHandlerPolicy
(JNIEnv*, jobject, jobject policyObj);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetSetRequestHandlerPolicy
(JNIEnv*, jobject);

#ifdef __cplusplus
}
#endif
#endif // JNI_RCS_RESOURCE_OBJECT_H


