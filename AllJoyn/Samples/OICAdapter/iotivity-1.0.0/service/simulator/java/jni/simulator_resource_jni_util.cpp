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
#include "simulator_resource_jni_util.h"
#include "simulator_common_jni.h"

extern SimulatorClassRefs gSimulatorClassRefs;

std::vector<int> convertIntegerVector(JNIEnv *env, jobject jVectorInt)
{
    std::vector<int> vectorInt;

    jclass vectorClass = env->FindClass("java/util/Vector");
    if (!vectorClass)
    {
        return vectorInt;
    }

    jmethodID size = env->GetMethodID(vectorClass, "size", "()I");
    if (NULL == size)
    {
        return vectorInt;
    }

    jmethodID get = env->GetMethodID(vectorClass, "get", "(I)""Ljava/lang/Object;");
    if (NULL == get)
    {
        return vectorInt;
    }

    jint jSize = env->CallIntMethod(jVectorInt, size);
    int sizeOfVector = jSize;

    for (int index = 0; index < sizeOfVector; index++)
    {
        jint jIndex = index;
        jint jValue = env->CallIntMethod(jVectorInt, get, jIndex);
        vectorInt.push_back((int)jValue);
    }

    return vectorInt;
}

std::vector<double> convertDoubleVector(JNIEnv *env, jobject jVectorDouble)
{
    std::vector<double> vectorDouble;

    jclass vectorClass = env->FindClass("java/util/Vector");
    if (!vectorClass)
    {
        return vectorDouble;
    }

    jmethodID size = env->GetMethodID(vectorClass, "size", "()I");
    if (NULL == size)
    {
        return vectorDouble;
    }

    jmethodID get = env->GetMethodID(vectorClass, "get", "(I)""Ljava/lang/Object;");
    if (NULL == get)
    {
        return vectorDouble;
    }

    jint jSize = env->CallIntMethod(jVectorDouble, size);
    int sizeOfVector = jSize;

    for (int index = 0; index < sizeOfVector; index++)
    {
        jint jIndex = index;
        jdouble jValue = env->CallDoubleMethod(jVectorDouble, get, jIndex);
        vectorDouble.push_back((double)jValue);
    }

    return vectorDouble;
}

std::vector<std::string> convertStringVector(JNIEnv *env, jobject jVectorString)
{
    std::vector<std::string> vectorString;

    jclass vectorClass = env->FindClass("java/util/Vector");
    if (!vectorClass)
    {
        return vectorString;
    }

    jmethodID size = env->GetMethodID(vectorClass, "size", "()I");
    if (NULL == size)
    {
        return vectorString;
    }

    jmethodID get = env->GetMethodID(vectorClass, "get", "(I)""Ljava/lang/Object;");
    if (NULL == get)
    {
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

void convertJavaMapToQueryParamsMap(JNIEnv *env, jobject hashMap,
                                    std::map<std::string, std::string> &queryParams)
{
    if (!hashMap) return;

    jobject jEntrySet = env->CallObjectMethod(hashMap, gSimulatorClassRefs.classMapEntrySet);
    jobject jIterator = env->CallObjectMethod(jEntrySet, gSimulatorClassRefs.classIteratorId);
    if (!jEntrySet || !jIterator || env->ExceptionCheck()) return;

    while (env->CallBooleanMethod(jIterator, gSimulatorClassRefs.classHasNextId))
    {
        jobject jEntry = env->CallObjectMethod(jIterator, gSimulatorClassRefs.classNextId);
        if (!jEntry) return;
        jstring jKey = (jstring)env->CallObjectMethod(jEntry, gSimulatorClassRefs.classMapGetKey);
        if (!jKey) return;
        jstring jValue = (jstring)env->CallObjectMethod(jEntry, gSimulatorClassRefs.classMapGetValue);
        if (!jValue) return;

        queryParams.insert(std::make_pair(env->GetStringUTFChars(jKey, NULL),
                                          env->GetStringUTFChars(jValue, NULL)));

        if (env->ExceptionCheck()) return;
        env->DeleteLocalRef(jEntry);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    }
}

jobject convertHashMapToJavaMap(JNIEnv *env,
                                const std::map<std::string, uint8_t> &observersList)
{
    if (observersList.empty())
    {
        return NULL;
    }

    jobject jObserverListMap = env->NewObject(gSimulatorClassRefs.classHashMap,
                               gSimulatorClassRefs.classHashMapCtor);

    for (auto it = observersList.begin(); it != observersList.end(); ++it)
    {
        jstring key = (*env).NewStringUTF( (*it).first.c_str() );
        jint value = (*it).second;
        env->CallObjectMethod(jObserverListMap, gSimulatorClassRefs.classHashMapPut, key, value);
    }

    return jObserverListMap;
}

jobject convertStringVectorToJavaList(JNIEnv *env, std::vector<std::string> &vector)
{
    jobject jList = env->NewObject(gSimulatorClassRefs.classLinkedList,
                                   gSimulatorClassRefs.classLinkedListCtor);
    if (!jList) return nullptr;
    for (size_t i = 0; i < vector.size(); ++i)
    {
        jstring jStr = env->NewStringUTF(vector[i].c_str());
        if (!jStr) return nullptr;
        env->CallBooleanMethod(jList, gSimulatorClassRefs.classLinkedListAddObject, jStr);
        if (env->ExceptionCheck()) return nullptr;
        env->DeleteLocalRef(jStr);
    }
    return jList;
}


