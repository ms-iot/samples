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

#include "simulator_resource_server_jni.h"
#include "simulator_resource_jni_util.h"
#include "simulator_common_jni.h"
#include "simulator_resource_model_jni.h"
#include "simulator_jni_utils.h"
#include "simulator_logger.h"
#include "simulator_jni_utils.h"

extern SimulatorClassRefs gSimulatorClassRefs;

JniSimulatorResource::JniSimulatorResource(SimulatorResourceServerSP &resource)
    : m_sharedResource(resource) {}

SimulatorResourceServerSP JniSimulatorResource::getJniSimulatorResourceSP(JNIEnv *env,
        jobject thiz)
{
    JniSimulatorResource *resource = GetHandle<JniSimulatorResource>(env, thiz);
    if (env->ExceptionCheck())
    {
        return NULL;
    }
    return resource->m_sharedResource;
}

jobject JniSimulatorResource::toJava(JNIEnv *env, jlong resource)
{
    jobject resourceObj = (jobject) env->NewObject(gSimulatorClassRefs.classSimulatorResource,
                          gSimulatorClassRefs.classSimulatorResourceCtor, resource);
    if (NULL == resourceObj)
    {
        return NULL;
    }
    return resourceObj;
}

void JniSimulatorResource::setResourceInfo(JNIEnv *env, jobject jobj)
{
    if (!env || !jobj)
        return;

    std::string uri = m_sharedResource->getURI();
    std::string resourceType = m_sharedResource->getResourceType();
    std::string name = m_sharedResource->getName();
    std::string interfaceType = m_sharedResource->getInterfaceType();

    jfieldID fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorResource, "resourceURI",
                                       "Ljava/lang/String;");
    jstring jUri = env->NewStringUTF(uri.c_str());
    env->SetObjectField(jobj, fieldID, jUri);

    fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorResource, "resourceName",
                              "Ljava/lang/String;");
    jstring jName = env->NewStringUTF(name.c_str());
    env->SetObjectField(jobj, fieldID, jName);

    fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorResource, "resourceType",
                              "Ljava/lang/String;");
    jstring jResourceType = env->NewStringUTF(resourceType.c_str());
    env->SetObjectField(jobj, fieldID, jResourceType);

    fieldID = env->GetFieldID(gSimulatorClassRefs.classSimulatorResource, "interfaceType",
                              "Ljava/lang/String;");
    jstring jInterfaceType = env->NewStringUTF(interfaceType.c_str());
    env->SetObjectField(jobj, fieldID, jInterfaceType);

    env->DeleteLocalRef(jUri);
    env->DeleteLocalRef(jName);
    env->DeleteLocalRef(jResourceType);
    env->DeleteLocalRef(jInterfaceType);
}

void onAutomationComplete(jweak jlistenerRef, const std::string &uri,
                          const int automationID)
{
    JNIEnv *env = getEnv();
    if (nullptr == env)
        return;

    jobject autoCompleteListener = env->NewLocalRef(jlistenerRef);
    if (!autoCompleteListener)
    {
        releaseEnv();
        return;
    }

    jclass autoCompleteCls = env->GetObjectClass(autoCompleteListener);
    if (!autoCompleteCls)
    {
        releaseEnv();
        return;
    }

    jmethodID autoCompleteMId = env->GetMethodID(autoCompleteCls, "onAutomationComplete",
                                "(Ljava/lang/String;I)V");
    if (!autoCompleteMId)
    {
        releaseEnv();
        return;
    }

    jstring jUri = env->NewStringUTF(uri.c_str());

    env->CallVoidMethod(autoCompleteListener, autoCompleteMId, jUri, automationID);
    if ((env)->ExceptionCheck())
    {
        releaseEnv();
        return;
    }

    env->DeleteLocalRef(jUri);

    releaseEnv();
}

JNIEXPORT jobject JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_getModel
(JNIEnv *env, jobject object)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return nullptr;
    }

    SimulatorResourceModel resModel = resource->getModel();
    JSimulatorResourceModel *model = new JSimulatorResourceModel(resModel);
    jobject jModel = JSimulatorResourceModel::toJava(env, reinterpret_cast<jlong>(model));
    return jModel;
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeInteger
(JNIEnv *env, jobject jobject, jstring jKey, jint jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    SimulatorResourceModel::Attribute att;
    att.setName(str);
    att.setValue(static_cast<int>(jValue));
    resource->addAttribute(att);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeDouble
(JNIEnv *env, jobject jobject, jstring jKey, jdouble jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    SimulatorResourceModel::Attribute att;
    att.setName(str);
    att.setValue(static_cast<double>(jValue));
    resource->addAttribute(att);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeBoolean
(JNIEnv *env, jobject jobject, jstring jKey, jboolean jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    SimulatorResourceModel::Attribute att;
    att.setName(str);
    att.setValue(static_cast<bool>(jValue));
    resource->addAttribute(att);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_addAttributeString
(JNIEnv *env, jobject jobject, jstring jKey, jstring jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string key = env->GetStringUTFChars(jKey, NULL);
    std::string value = env->GetStringUTFChars(jValue, NULL);
    SimulatorResourceModel::Attribute att;
    att.setName(key);
    att.setValue(value);
    resource->addAttribute(att);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeInteger
(JNIEnv *env, jobject jobject, jstring jKey, jint jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    resource->updateAttributeValue(str, static_cast<int>(jValue));
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeDouble
(JNIEnv *env, jobject jobject, jstring jKey, jdouble jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    resource->updateAttributeValue(str, static_cast<double>(jValue));
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeBoolean
(JNIEnv *env, jobject jobject, jstring jKey, jboolean jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    resource->updateAttributeValue(str, static_cast<bool>(jValue));
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeString
(JNIEnv *env, jobject jobject, jstring jKey, jstring jValue)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string key = env->GetStringUTFChars(jKey, NULL);
    std::string value = env->GetStringUTFChars(jValue, NULL);

    resource->updateAttributeValue(key, value);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_updateAttributeFromAllowedValues
(JNIEnv *env, jobject object, jstring attrName, jint index)
{
    if (!attrName || index < 0)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid parameter!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    const char *attrNameCStr = env->GetStringUTFChars(attrName, NULL);
    if (!attrNameCStr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    resource->updateAttributeValue(attrNameCStr, static_cast<int>(index));
    env->ReleaseStringUTFChars(attrName, attrNameCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setRange
(JNIEnv *env, jobject object, jstring attrName, jint min, jint max)
{
    if (!attrName)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid parameter!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    const char *attrNameCStr = env->GetStringUTFChars(attrName, NULL);
    if (!attrNameCStr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    resource->setRange(attrNameCStr, static_cast<int>(min), static_cast<int>(max));
    env->ReleaseStringUTFChars(attrName, attrNameCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setAllowedValuesInteger
(JNIEnv *env, jobject object, jstring jKey, jobject jAllowedValues)
{
    if (!jKey || !jAllowedValues)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid parameter!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    const char *keyCStr = env->GetStringUTFChars(jKey, NULL);
    if (!keyCStr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    resource->setAllowedValues(keyCStr, convertIntegerVector(env, jAllowedValues));
    env->ReleaseStringUTFChars(jKey, keyCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setAllowedValuesDouble
(JNIEnv *env, jobject object, jstring jKey, jobject jAllowedValues)
{
    if (!jKey || !jAllowedValues)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid parameter!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    const char *keyCStr = env->GetStringUTFChars(jKey, NULL);
    if (!keyCStr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    resource->setAllowedValues(keyCStr, convertDoubleVector(env, jAllowedValues));
    env->ReleaseStringUTFChars(jKey, keyCStr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setAllowedValuesString
(JNIEnv *env, jobject object, jstring jKey, jobject jAllowedValues)
{
    if (!jKey || !jAllowedValues)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid parameter!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    const char *keyCStr = env->GetStringUTFChars(jKey, NULL);
    if (!keyCStr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    resource->setAllowedValues(keyCStr, convertStringVector(env, jAllowedValues));
    env->ReleaseStringUTFChars(jKey, keyCStr);
}

JNIEXPORT jint JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_startResourceAutomation
(JNIEnv *env, jobject object, jint automationType, jobject listener)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return SIMULATOR_BAD_OBJECT;
    }

    if (!listener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK,
                                  "Start Resource Automation failed! Callback not set");
        return SIMULATOR_INVALID_CALLBACK;
    }

    jweak jlistenerRef = env->NewWeakGlobalRef(listener);
    updateCompleteCallback callback =  [jlistenerRef](const std::string & uri, const int automationID)
    {
        onAutomationComplete(jlistenerRef, uri, automationID);
    };

    AutomationType type = AutomationType::NORMAL;
    if (1 == automationType)
    {
        type = AutomationType::RECURRENT;
    }

    int automationId = -1;

    try
    {
        automationId = resource->startUpdateAutomation(type, callback);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }
    return automationId;
}

JNIEXPORT jint JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_startAttributeAutomation
(JNIEnv *env, jobject object, jstring attrName, jint automationType, jobject listener)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return SIMULATOR_BAD_OBJECT;
    }

    if (!attrName)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return SIMULATOR_INVALID_PARAM;
    }

    if (!listener)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK,
                                  "Start Attribute Automation failed! Callback not set");
        return SIMULATOR_INVALID_CALLBACK;
    }

    jweak jlistenerRef = env->NewWeakGlobalRef(listener);
    updateCompleteCallback callback =  [jlistenerRef](const std::string & uri, const int automationID)
    {
        onAutomationComplete(jlistenerRef, uri, automationID);
    };

    const char *attrNamePtr = env->GetStringUTFChars(attrName, NULL);

    AutomationType type = AutomationType::NORMAL;
    if (1 == automationType)
    {
        type = AutomationType::RECURRENT;
    }

    int automationId = -1;
    try
    {
        automationId = resource->startUpdateAutomation(attrNamePtr, type, callback);
    }
    catch (InvalidArgsException &e)
    {
        throwInvalidArgsException(env, e.code(), e.what());
        return -1;
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
        return -1;
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
        return -1;
    }
    env->ReleaseStringUTFChars(attrName, attrNamePtr);
    return automationId;
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_stopAutomation
(JNIEnv *env, jobject object, jint automationId)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    try
    {
        resource->stopUpdateAutomation(automationId);
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }

    SIM_LOG(ILogger::INFO, "Automation has been forcibly stopped.")
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_removeAttribute
(JNIEnv *env, jobject jobject, jstring jKey)
{
    if (!jKey)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid Attribute name!");
        return;
    }

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env,
                                         jobject);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    std::string str = env->GetStringUTFChars(jKey, NULL);
    resource->removeAttribute(str);
}

JNIEXPORT jobjectArray JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_getObserversList
(JNIEnv *env, jobject object)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return nullptr;
    }

    std::vector<ObserverInfo> observersList;
    observersList = resource->getObserversList();

    // Construct the object array and send it java layer
    jobjectArray jobserversArray = env->NewObjectArray(observersList.size(),
                                   gSimulatorClassRefs.classObserverInfo, NULL);
    if (jobserversArray)
    {
        for (size_t i = 0; i < observersList.size(); i++)
        {
            jstring jaddress = env->NewStringUTF(observersList[i].address.c_str());
            jobject jobserver = (jobject) env->NewObject(gSimulatorClassRefs.classObserverInfo,
                                gSimulatorClassRefs.classObserverInfoCtor, observersList[i].id,
                                jaddress, observersList[i].port);

            env->SetObjectArrayElement(jobserversArray, i, jobserver);
            env->DeleteLocalRef(jaddress);
        }
    }

    return jobserversArray;
}

void onObserverChange(jweak jlistenerRef, const std::string &uri,
                      ObservationStatus state, const ObserverInfo &observerInfo)
{
    JNIEnv *env = getEnv();
    if (nullptr == env)
        return;

    jobject observerChangeListener = env->NewLocalRef(jlistenerRef);
    if (!observerChangeListener)
    {
        releaseEnv();
        return;
    }

    jclass observerChangeCls = env->GetObjectClass(observerChangeListener);
    if (!observerChangeCls)
    {
        releaseEnv();
        return;
    }

    jmethodID observerChangeMId = env->GetMethodID(observerChangeCls, "onObserverChanged",
                                  "(Ljava/lang/String;ILorg/oic/simulator/serviceprovider/ObserverInfo;)V");
    if (!observerChangeMId)
    {
        releaseEnv();
        return;
    }

    // Convert URI
    jstring jUri = env->NewStringUTF(uri.c_str());

    // Convert state
    jint jstate = (state == ObservationStatus::OBSERVE_REGISTER) ? 0 : 1;

    // Construct the java object of observerinfo
    jstring jaddress = env->NewStringUTF(observerInfo.address.c_str());
    jobject jobserver = (jobject) env->NewObject(gSimulatorClassRefs.classObserverInfo,
                        gSimulatorClassRefs.classObserverInfoCtor, observerInfo.id,
                        jaddress, observerInfo.port);

    env->CallVoidMethod(observerChangeListener, observerChangeMId, jUri, jstate, jobserver);
    env->DeleteLocalRef(jaddress);
    if ((env)->ExceptionCheck())
    {
        releaseEnv();
        return;
    }

    env->DeleteLocalRef(jUri);

    releaseEnv();
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_setObserverCallback
(JNIEnv *env, jobject object, jobject jcallback)
{
    if (!jcallback)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_CALLBACK, "Callback not set");
        return;
    }

    jweak jlistenerRef = env->NewWeakGlobalRef(jcallback);
    SimulatorResourceServer::ObserverCB callback =  [jlistenerRef](const std::string & uri,
            ObservationStatus state, const ObserverInfo & observerInfo)
    {
        onObserverChange(jlistenerRef, uri, state, observerInfo);
    };

    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    resource->setObserverCallback(callback);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_notifyObserver
(JNIEnv *env, jobject object, jint jId)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    try
    {
        resource->notify(jId);
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_notifyAllObservers
(JNIEnv *env, jobject object)
{
    SimulatorResourceServerSP resource = JniSimulatorResource::getJniSimulatorResourceSP(env, object);
    if (!resource)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "No resource!");
        return;
    }

    try
    {
        resource->notifyAll();
    }
    catch (SimulatorException &e)
    {
        throwSimulatorException(env, e.code(), e.what());
    }
    catch (...)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Unknown Exception");
    }
}

JNIEXPORT void JNICALL Java_org_oic_simulator_serviceprovider_SimulatorResourceServer_dispose
(JNIEnv *env, jobject thiz)
{
    JniSimulatorResource *resource = GetHandle<JniSimulatorResource>(env, thiz);
    delete resource;
}

