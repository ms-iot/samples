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

#ifndef SIMULATOR_RESOURCE_JNI_H_
#define SIMULATOR_RESOURCE_JNI_H_

#include <jni.h>
#include "simulator_resource_server.h"

class JniSimulatorResource
{
    public:
        JniSimulatorResource(SimulatorResourceServerSP &resource);

        static jobject toJava(JNIEnv *env, jlong resource);
        void setResourceInfo(JNIEnv *env, jobject jobj);
        static SimulatorResourceServerSP getJniSimulatorResourceSP(JNIEnv *env, jobject thiz);
    private:
        SimulatorResourceServerSP m_sharedResource;
};


#ifdef __cplusplus
extern "C" {

JNIEXPORT jobject JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_getModel
(JNIEnv *, jobject);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeInteger
(JNIEnv *, jobject, jstring, jint);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeDouble
(JNIEnv *, jobject, jstring, jdouble);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeBoolean
(JNIEnv *, jobject, jstring, jboolean);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeString
(JNIEnv *, jobject, jstring, jstring);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeInteger
(JNIEnv *, jobject, jstring, jint);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeDouble
(JNIEnv *, jobject, jstring, jdouble);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeBoolean
(JNIEnv *, jobject, jstring, jboolean);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeString
(JNIEnv *, jobject, jstring, jstring);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeFromAllowedValues
(JNIEnv *, jobject, jstring, jint);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setRange
(JNIEnv *, jobject, jstring, jint, jint);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setAllowedValuesInteger
(JNIEnv *, jobject, jstring, jobject);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setAllowedValuesDouble
(JNIEnv *, jobject, jstring, jobject);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setAllowedValuesString
(JNIEnv *, jobject, jstring, jobject);

JNIEXPORT jint JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_startResourceAutomation
(JNIEnv *, jobject, jint, jobject);

JNIEXPORT jint JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_startAttributeAutomation
(JNIEnv *, jobject, jstring, jint, jobject);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_stopAutomation
(JNIEnv *, jobject, jint);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_removeAttribute
(JNIEnv *, jobject, jstring);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setObserverCallback
(JNIEnv *env, jobject object, jobject observer);

JNIEXPORT jobjectArray JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_getObserversList
(JNIEnv *env, jobject object);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_notifyObserver
(JNIEnv *env, jobject object, jint jId);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_notifyAllObservers
(JNIEnv *env, jobject object);

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_dispose
(JNIEnv *, jobject);

}
#endif
#endif //SIMULATOR_RESOURCE_JNI_H_
