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

#ifndef JNI_RCS_RESOURCE_ATTRIBUTES_H_
#define JNI_RCS_RESOURCE_ATTRIBUTES_H_

#include <jni.h>

namespace OIC
{
    namespace Service
    {
        class RCSResourceAttributes;
    }
}

class JNIEnvWrapper;

void initRCSResourceAttributes(JNIEnvWrapper*);
void clearRCSResourceAttributes(JNIEnvWrapper*);

jobject newAttributesObject(JNIEnv*, const OIC::Service::RCSResourceAttributes&);
jobject newAttributesObject(JNIEnvWrapper*, const OIC::Service::RCSResourceAttributes&);

OIC::Service::RCSResourceAttributes toNativeAttributes(JNIEnv*, jobject);
OIC::Service::RCSResourceAttributes toNativeAttributes(JNIEnvWrapper*, jobject);

void writeNativeAttributesFromMap(JNIEnv*, jobject mapObj,
        OIC::Service::RCSResourceAttributes& targetAttrs);
void writeNativeAttributesFromMap(JNIEnvWrapper*, jobject mapObj,
        OIC::Service::RCSResourceAttributes& targetAttrs);

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeIsEmpty(JNIEnv*, jobject);

JNIEXPORT jint JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeSize(JNIEnv*, jobject);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeRemove(JNIEnv*, jobject, jstring keyObj);

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeClear(JNIEnv*, jobject);

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeContains(JNIEnv*, jobject, jstring keyObj);

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeAddKeys(JNIEnv*, jobject, jstring setObj);

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeExtract(JNIEnv*, jobject, jstring keyObj);

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeExtractAll(JNIEnv*, jobject, jobject);

#ifdef __cplusplus
}
#endif

#endif  //JNI_RCS_RESOURCE_ATTRIBUTES_H_
