/* *****************************************************************
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

#include "jni_things_manager_util.h"
#include "JniOcResource.h"
#include "jni_string.h"

jobject ocResourceHandleToJava(JNIEnv *env, jlong resourceHandle)
{
    jclass resourceClass = GetJClass(TM_SERVICE_OCRESOURCEHANDLE_PATH);
    if (NULL == resourceClass)
    {
        LOGE("ocResourceHandleToJava : failed to find OCResourceHandle java class!");
        return NULL;
    }

    jmethodID constructor = env->GetMethodID(resourceClass, "<init>", "(J)V");
    if (NULL == constructor)
    {
        LOGE("ocResourceHandleToJava: Failed to get constructor method!");
        return NULL;
    }

    jobject resourceObj = (jobject) env->NewObject(resourceClass, constructor, resourceHandle);
    if (NULL == resourceObj)
    {
        LOGE("ocResourceHandleToJava: Failed to create OCResouceHandle java object!");
        return NULL;
    }
    return resourceObj;
}

jobject OcResourceToJava(JNIEnv *env, jlong resource)
{
    jclass resourceClass = GetJClass(TM_SERVICE_OCRESOURCE_PATH);
    if (NULL == resourceClass)
    {
        LOGE("OcResourceToJava : failed to find OCResouce java class!");
        return NULL;
    }

    jmethodID constructor = env->GetMethodID(resourceClass, "<init>", "(J)V");
    if (NULL == constructor)
    {
        LOGE("OcResourceToJava: Failed to get constructor method!");
        return NULL;
    }

    jobject resourceObj = (jobject) env->NewObject(resourceClass, constructor, resource);
    if (NULL == resourceObj)
    {
        LOGE("OcResourceToJava: Failed to create OCResouce java object!");
        return NULL;
    }
    return resourceObj;
}

jobject OcHeaderOptionToJava(JNIEnv *env, OC::HeaderOption::OCHeaderOption headerOption)
{
    JString *optionData = new JString(env, headerOption.getOptionData());
    if (!optionData)
    {
        LOGE("OcHeaderOptionToJava : failed to get JString!");
        return NULL;
    }

    jclass headerOptionClass = GetJClass(TM_SERVICE_HEADER_OPTION_PATH);
    if (NULL == headerOptionClass)
    {
        LOGE("OcHeaderOptionToJava : failed to find OCRepresentation java class!");
        return NULL;
    }

    jmethodID constructor = env->GetMethodID(headerOptionClass, "<init>", "(JLjava/lang/String;)V");
    if (NULL == constructor)
    {
        LOGE("OcHeaderOptionToJava: Failed to get constructor method!");
        return NULL;
    }

    jobject headerOptionObj = (jobject) env->NewObject(headerOptionClass, constructor,
                              (jlong)headerOption.getOptionID(), optionData);
    if (NULL == headerOptionObj)
    {
        LOGE("OcHeaderOptionToJava: Failed to create OCRepresentation java object!");
        return NULL;
    }
    return headerOptionObj;
}

jobject OcRepresentationToJava(JNIEnv *env, jlong ocRepresentation)
{
    jclass ocRepresentationClass = GetJClass(TM_SERVICE_OCREPRESENTATION_PATH);
    if (NULL == ocRepresentationClass)
    {
        LOGE("OcRepresentationToJava : failed to find OCRepresentation java class!");
        return NULL;
    }

    jmethodID constructor = env->GetMethodID(ocRepresentationClass, "<init>", "(J)V");
    if (NULL == constructor)
    {
        LOGE("OcRepresentationToJava: Failed to get constructor method!");
        return NULL;
    }

    jobject ocRepresentationObj = (jobject) env->NewObject(ocRepresentationClass, constructor,
                                  ocRepresentation);
    if (NULL == ocRepresentationObj)
    {
        LOGE("OcRepresentationToJava: Failed to create OCRepresentation java object!");
        return NULL;
    }
    return ocRepresentationObj;
}

std::vector<std::string> convertStringVector(JNIEnv *env, jobject jVectorString)
{
    std::vector<std::string> vectorString;

    jclass vectorClass = env->FindClass(TM_JAVA_VECTOR_CLASS_PATH);
    if (!vectorClass)
    {
        LOGE("convertStringVector: Not able to find class %s", TM_JAVA_VECTOR_CLASS_PATH);
        return vectorString;
    }

    jmethodID size = env->GetMethodID(vectorClass, "size", "()I");
    if (NULL == size)
    {
        LOGE("convertStringVector: Failed to get method id for size");
        return vectorString;
    }

    jmethodID get = env->GetMethodID(vectorClass, "get", "(I)""Ljava/lang/Object;");
    if (NULL == get)
    {
        LOGE("convertStringVector: Failed to get method id for get");
        return vectorString;
    }

    jint jSize = env->CallIntMethod(jVectorString, size);
    int sizeOfVector = jSize;

    for (int index = 0; index < sizeOfVector; index++)
    {
        jint jIndex = index;
        jstring jContactInfoObj = (jstring)env->CallObjectMethod(jVectorString, get, jIndex);
        if (jContactInfoObj == NULL)
        {
            LOGE("convertStringVector: Failed on CallObjectMethod");
            return vectorString;
        }
        const char *buff = env->GetStringUTFChars(jContactInfoObj, 0);
        if (NULL != buff)
        {
            std::string tempString = buff;
            vectorString.push_back(tempString);
        }

        env->ReleaseStringUTFChars(jContactInfoObj, buff);
    }
    return vectorString;
}

std::map<std::string, std::string> convertStringMap(JNIEnv *env, jobject jMapString)
{
    // Get reference to java/util/Map class
    jclass mapClass = env->GetObjectClass(jMapString);
    jmethodID sizeMethodId = env->GetMethodID(mapClass, "size", "()I");
    jmethodID entrySetMethodId = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");

    // Get reference to java/util/Set class
    jclass setClass = env->FindClass("java/util/Set");
    jmethodID iteratorMethodId = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");

    // Get reference to java/util/Iterator class
    jclass iteratorClass = env->FindClass("java/util/Iterator");
    jmethodID hasNextMethodId = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID nextMethodId = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    // Get reference to java/util/Map/Entry class
    jclass mapEntryClass = env->FindClass("java/util/Map$Entry");
    jmethodID getKeyMethodId = env->GetMethodID(mapEntryClass, "getKey", "()Ljava/lang/Object;");
    jmethodID getValueMethodId = env->GetMethodID(mapEntryClass, "getValue", "()Ljava/lang/Object;");

    std::map<std::string, std::string> mapString;
    // Get entry set from map
    jobject entrySet = env->CallObjectMethod(jMapString, entrySetMethodId);
    if (!entrySet)
    {
        LOGE("convertStringMap: Failed to get entrySet");
        return mapString;
    }

    // Get iterator from set
    jobject iterator = env->CallObjectMethod(entrySet, iteratorMethodId);
    if (!iterator)
    {
        LOGE("convertStringMap: Failed to get iterator from set");
        return mapString;
    }

    if (JNI_TRUE == env->CallBooleanMethod(iterator, hasNextMethodId))
    {
        jobject entry = env->CallObjectMethod(iterator, nextMethodId);
        jstring jMapKeyStr = (jstring)env->CallObjectMethod(entry, getKeyMethodId);
        jstring jMapValueStr = (jstring)env->CallObjectMethod(entry, getValueMethodId);

        const char *key = env->GetStringUTFChars(jMapKeyStr, 0);
        const char *value = env->GetStringUTFChars(jMapValueStr, 0);

        if ((NULL != key) && (NULL != value))
        {
            std::string keyStr = key;
            std::string valueStr = value;
            mapString.insert(std::make_pair(keyStr, valueStr));
        }
        env->ReleaseStringUTFChars(jMapKeyStr, key);
        env->ReleaseStringUTFChars(jMapValueStr, value);
    }
    return mapString;
}
