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

#include "simulator_resource_model_jni.h"
#include "simulator_common_jni.h"
#include "resource_attributes_jni.h"
#include "simulator_error_codes.h"
#include "simulator_jni_utils.h"

using namespace std;

extern SimulatorClassRefs gSimulatorClassRefs;

JSimulatorResourceModel::JSimulatorResourceModel(SimulatorResourceModel resModel)
    : m_resourceModel(resModel)
{}

JSimulatorResourceModel::JSimulatorResourceModel(SimulatorResourceModelSP resModel)
    : m_resModelPtr(resModel)
{}

bool JSimulatorResourceModel::getResourceModel(JNIEnv *env, jobject thiz,
        SimulatorResourceModel &resModel)
{
    JSimulatorResourceModel *resource = GetHandle<JSimulatorResourceModel>(env, thiz);
    if (env->ExceptionCheck())
    {
        return false;
    }

    if (nullptr != resource->m_resModelPtr)
        resModel = *(resource->m_resModelPtr.get());
    else
        resModel = resource->m_resourceModel;
    return true;
}

SimulatorResourceModelSP JSimulatorResourceModel::getResourceModelPtr(JNIEnv *env, jobject thiz)
{
    JSimulatorResourceModel *resource = GetHandle<JSimulatorResourceModel>(env, thiz);
    if (env->ExceptionCheck())
    {
        return nullptr;
    }

    if (nullptr != resource->m_resModelPtr)
        return resource->m_resModelPtr;
    return nullptr;
}

jobject JSimulatorResourceModel::toJava(JNIEnv *env, jlong nativeHandle)
{
    jobject resourceObj = (jobject) env->NewObject(gSimulatorClassRefs.classSimulatorResourceModel,
                          gSimulatorClassRefs.classSimulatorResourceModelCtor, nativeHandle);
    if (!resourceObj)
    {
        return NULL;
    }
    return resourceObj;
}

void JSimulatorResourceModel::toJava(JNIEnv *env, jobject thiz, jlong nativeHandle)
{
    if (env && thiz && nativeHandle)
    {
        env->SetLongField(thiz, GetHandleField(env, thiz), nativeHandle);
    }
}

static jobject createHashMap(JNIEnv *env)
{
    jobject mapobj = env->NewObject(gSimulatorClassRefs.classHashMap,
                                    gSimulatorClassRefs.classHashMapCtor);
    return mapobj;
}

static void addEntryToHashMap(JNIEnv *env, jobject mapobj, jobject key, jobject value)
{
    if (!mapobj || !key || !value)
    {
        return;
    }

    env->CallObjectMethod(mapobj, gSimulatorClassRefs.classHashMapPut, key, value);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_create
(JNIEnv *env, jobject thiz)
{
    SimulatorResourceModelSP resModel = std::make_shared<SimulatorResourceModel>();
    JSimulatorResourceModel *jresModel = new JSimulatorResourceModel(resModel);
    JSimulatorResourceModel::toJava(env, thiz, reinterpret_cast<jlong>(jresModel));
}

JNIEXPORT jint JNICALL
Java_org_oic_simulator_SimulatorResourceModel_size
(JNIEnv *env, jobject thiz)
{
    SimulatorResourceModel resourceModel;
    bool result = JSimulatorResourceModel::getResourceModel(env, thiz, resourceModel);
    if (!result)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return SIMULATOR_BAD_OBJECT;
    }

    return resourceModel.size();
}

JNIEXPORT jobject JNICALL
Java_org_oic_simulator_SimulatorResourceModel_getAttributes
(JNIEnv *env, jobject thiz)
{
    SimulatorResourceModel resourceModel;
    bool result = JSimulatorResourceModel::getResourceModel(env, thiz, resourceModel);
    if (!result)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return nullptr;
    }

    map<string, SimulatorResourceModel::Attribute> attributesMap = resourceModel.getAttributes();

    // Create Java HashMap object
    jobject jHashMap = NULL;
    jHashMap = createHashMap(env);
    if (!jHashMap)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "Java map creation failed!");
        return nullptr;
    }

    for (auto & attributeEntry : attributesMap)
    {
        SimulatorResourceModel::Attribute attribute(attributeEntry.second);

        // Create a object of ResourceAttribute java class
        JResourceAttributeConverter converter(attribute);
        jobject jAttribute = converter.toJava(env);

        // Add an entry with attribute.first and javaSimualatorResourceAttribute to the HashMap
        jstring jAttrName = env->NewStringUTF((attributeEntry.first).c_str());
        addEntryToHashMap(env, jHashMap, static_cast<jobject>(jAttrName), jAttribute);
        env->DeleteLocalRef(jAttrName);
    }

    return jHashMap;
}

JNIEXPORT jobject JNICALL
Java_org_oic_simulator_SimulatorResourceModel_getAttribute
(JNIEnv *env, jobject thiz, jstring jAttrName)
{
    if (!jAttrName)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid attribute name!");
        return nullptr;
    }

    const char *attrName = env->GetStringUTFChars(jAttrName, NULL);
    if (!attrName)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return nullptr;
    }

    SimulatorResourceModel resourceModel;
    bool result = JSimulatorResourceModel::getResourceModel(env, thiz, resourceModel);
    if (!result)
    {
        env->ReleaseStringUTFChars(jAttrName, attrName);
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return nullptr;
    }

    SimulatorResourceModel::Attribute attribute;
    bool found = resourceModel.getAttribute(attrName, attribute);
    if (!found)
    {
        env->ReleaseStringUTFChars(jAttrName, attrName);
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Attribute does not exist!");
        return nullptr;
    }

    env->ReleaseStringUTFChars(jAttrName, attrName);

    // Create a object of ResourceAttribute java class
    JResourceAttributeConverter converter(attribute);
    return converter.toJava(env);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeInt
(JNIEnv *env, jobject thiz, jstring jname, jint jvalue)
{
    if (!jname)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid attribute name!");
        return;
    }

    SimulatorResourceModelSP resModelPtr;
    resModelPtr = JSimulatorResourceModel::getResourceModelPtr(env, thiz);
    if (!resModelPtr)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return;
    }

    const char *nameCstr = env->GetStringUTFChars(jname, NULL);
    if (!nameCstr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    std::string attrName(nameCstr);
    int value = static_cast<int>(jvalue);
    resModelPtr->addAttribute(attrName, value);

    // Release created c string
    env->ReleaseStringUTFChars(jname, nameCstr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeDouble
(JNIEnv *env, jobject thiz, jstring jname, jdouble jvalue)
{
    if (!jname)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid attribute name!");
        return;
    }

    SimulatorResourceModelSP resModelPtr;
    resModelPtr = JSimulatorResourceModel::getResourceModelPtr(env, thiz);
    if (!resModelPtr)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return;
    }

    const char *nameCstr = env->GetStringUTFChars(jname, NULL);
    if (!nameCstr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    std::string attrName(nameCstr);
    double value = static_cast<double>(jvalue);
    resModelPtr->addAttribute(attrName, value);

    // Release created c string
    env->ReleaseStringUTFChars(jname, nameCstr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeBoolean
(JNIEnv *env, jobject thiz, jstring jname, jboolean jvalue)
{
    if (!jname)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid attribute name!");
        return;
    }

    SimulatorResourceModelSP resModelPtr;
    resModelPtr = JSimulatorResourceModel::getResourceModelPtr(env, thiz);
    if (!resModelPtr)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return;
    }

    const char *nameCstr = env->GetStringUTFChars(jname, NULL);
    if (!nameCstr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    std::string attrName(nameCstr);
    bool value = static_cast<bool>(jvalue);
    resModelPtr->addAttribute(attrName, value);

    // Release created c string
    env->ReleaseStringUTFChars(jname, nameCstr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_addAttributeString
(JNIEnv *env, jobject thiz, jstring jname, jstring jvalue)
{
    if (!jname)
    {
        throwInvalidArgsException(env, SIMULATOR_INVALID_PARAM, "Invalid attribute name!");
        return;
    }

    SimulatorResourceModelSP resModelPtr;
    resModelPtr = JSimulatorResourceModel::getResourceModelPtr(env, thiz);
    if (!resModelPtr)
    {
        throwSimulatorException(env, SIMULATOR_BAD_OBJECT, "Resource model not found!");
        return;
    }

    const char *nameCstr = env->GetStringUTFChars(jname, NULL);
    if (!nameCstr)
    {
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    const char *valueCstr = env->GetStringUTFChars(jvalue, NULL);
    if (!valueCstr)
    {
        env->ReleaseStringUTFChars(jname, nameCstr);
        throwSimulatorException(env, SIMULATOR_ERROR, "String error!");
        return;
    }

    std::string attrName(nameCstr);
    std::string value(valueCstr);
    resModelPtr->addAttribute(attrName, value);

    // Release created c string
    env->ReleaseStringUTFChars(jname, nameCstr);
    env->ReleaseStringUTFChars(jvalue, valueCstr);
}

JNIEXPORT void JNICALL
Java_org_oic_simulator_SimulatorResourceModel_dispose
(JNIEnv *env, jobject thiz)
{
    JSimulatorResourceModel *resourceModel = GetHandle<JSimulatorResourceModel>(env, thiz);
    delete resourceModel;
}
