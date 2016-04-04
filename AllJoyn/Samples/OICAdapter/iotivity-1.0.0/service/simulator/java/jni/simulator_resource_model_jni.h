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

#ifndef SIMULATOR_RESOURCE_MODEL_JNI_H_
#define SIMULATOR_RESOURCE_MODEL_JNI_H_

#include <jni.h>
#include "simulator_resource_model.h"

class JSimulatorResourceModel
{
    public:
        JSimulatorResourceModel(SimulatorResourceModel resModel);
        JSimulatorResourceModel(SimulatorResourceModelSP resModel);

        static jobject toJava(JNIEnv *env, jlong nativeHandle);
        static void toJava(JNIEnv *env, jobject thiz, jlong nativeHandle);
        static bool getResourceModel(JNIEnv *env, jobject thiz, SimulatorResourceModel &resModel);
        static SimulatorResourceModelSP getResourceModelPtr(JNIEnv *env, jobject thiz);

    private:
        SimulatorResourceModel m_resourceModel;
        SimulatorResourceModelSP m_resModelPtr;
};


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_create
(JNIEnv *, jobject);

JNIEXPORT jint JNICALL
Java_org_oic_simulator_SimulatorResourceModel_size
(JNIEnv *, jobject);

JNIEXPORT jobject JNICALL
Java_org_oic_simulator_SimulatorResourceModel_getAttributes
(JNIEnv *, jobject);

JNIEXPORT jobject JNICALL
Java_org_oic_simulator_SimulatorResourceModel_getAttribute
(JNIEnv *, jobject, jstring);

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeInt
(JNIEnv *, jobject, jstring, jint);

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeDouble
(JNIEnv *, jobject, jstring, jdouble);

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeBoolean
(JNIEnv *, jobject, jstring, jboolean);

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeString
(JNIEnv *, jobject, jstring, jstring);

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_dispose
(JNIEnv *, jobject);


#ifdef __cplusplus
}
#endif

#endif
