/*
* //******************************************************************
* //
* // Copyright 2015 Intel Corporation.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* //
* // Licensed under the Apache License, Version 2.0 (the "License");
* // you may not use this file except in compliance with the License.
* // You may obtain a copy of the License at
* //
* //      http://www.apache.org/licenses/LICENSE-2.0
* //
* // Unless required by applicable law or agreed to in writing, software
* // distributed under the License is distributed on an "AS IS" BASIS,
* // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* // See the License for the specific language governing permissions and
* // limitations under the License.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

#include "JniUtils.h"
#include "JniOcRepresentation.h"

jobject JniUtils::convertStrVectorToJavaStrList(JNIEnv *env, std::vector<std::string> &vector)
{
    jobject jList = env->NewObject(g_cls_LinkedList, g_mid_LinkedList_ctor);
    if (!jList) return nullptr;
    for (size_t i = 0; i < vector.size(); ++i)
    {
        jstring jStr = env->NewStringUTF(vector[i].c_str());
        if (!jStr) return nullptr;
        env->CallBooleanMethod(jList, g_mid_LinkedList_add_object, jStr);
        if (env->ExceptionCheck()) return nullptr;
        env->DeleteLocalRef(jStr);
    }
    return jList;
}

void JniUtils::convertJavaStrArrToStrVector(JNIEnv *env, jobjectArray jStrArr, std::vector<std::string> &vector)
{
    if (!jStrArr) return;

    jsize len = env->GetArrayLength(jStrArr);
    for (jsize i = 0; i < len; ++i)
    {
        jstring jStr = (jstring)env->GetObjectArrayElement(jStrArr, i);
        if (!jStr) return;
        vector.push_back(env->GetStringUTFChars(jStr, nullptr));
        if (env->ExceptionCheck()) return;
        env->DeleteLocalRef(jStr);
    }
}

void JniUtils::convertJavaHeaderOptionsArrToVector(JNIEnv *env, jobjectArray jHeaderOptions,
    OC::HeaderOptions &headerOptions)
{
    if (!jHeaderOptions) return;
    jsize len = env->GetArrayLength(jHeaderOptions);
    for (jsize i = 0; i < len; ++i)
    {
        jobject header = env->GetObjectArrayElement(jHeaderOptions, i);
        if (!header) return;
        jint jId = env->CallIntMethod(header, g_mid_OcHeaderOption_get_id);
        jstring jData = (jstring)env->CallObjectMethod(header, g_mid_OcHeaderOption_get_data);
        OC::HeaderOption::OCHeaderOption hopt(
            static_cast<int>(jId),
            env->GetStringUTFChars(jData, nullptr));

        headerOptions.push_back(hopt);

        if (env->ExceptionCheck()) return;
        env->DeleteLocalRef(header);
        env->DeleteLocalRef(jData);
    }
}

jobject JniUtils::convertHeaderOptionsVectorToJavaList(JNIEnv *env, const OC::HeaderOptions& headerOptions)
{
    jobject jHeaderOptionList = env->NewObject(g_cls_LinkedList, g_mid_LinkedList_ctor);
    if (!jHeaderOptionList) return nullptr;

    for (size_t i = 0; i < headerOptions.size(); ++i)
    {
        jobject jHeaderOption = env->NewObject(
            g_cls_OcHeaderOption,
            g_mid_OcHeaderOption_ctor,
            static_cast<jint>(headerOptions[i].getOptionID()),
            env->NewStringUTF(headerOptions[i].getOptionData().c_str())
            );
        if (!jHeaderOption) return nullptr;

        env->CallBooleanMethod(jHeaderOptionList, g_mid_LinkedList_add_object, jHeaderOption);
        if (env->ExceptionCheck()) return nullptr;
        env->DeleteLocalRef(jHeaderOption);
    }

    return jHeaderOptionList;
}

void JniUtils::convertJavaMapToQueryParamsMap(JNIEnv *env, jobject hashMap, OC::QueryParamsMap &map)
{
    if (!hashMap) return;

    jobject jEntrySet = env->CallObjectMethod(hashMap, g_mid_Map_entrySet);
    jobject jIterator = env->CallObjectMethod(jEntrySet, g_mid_Set_iterator);
    if (!jEntrySet || !jIterator || env->ExceptionCheck()) return;

    while (env->CallBooleanMethod(jIterator, g_mid_Iterator_hasNext))
    {
        jobject jEntry = env->CallObjectMethod(jIterator, g_mid_Iterator_next);
        if (!jEntry) return;
        jstring jKey = (jstring)env->CallObjectMethod(jEntry, g_mid_MapEntry_getKey);
        if (!jKey) return;
        jstring jValue = (jstring)env->CallObjectMethod(jEntry, g_mid_MapEntry_getValue);
        if (!jValue) return;

        map.insert(std::make_pair(env->GetStringUTFChars(jKey, nullptr),
            env->GetStringUTFChars(jValue, nullptr)));

        if (env->ExceptionCheck()) return;
        env->DeleteLocalRef(jEntry);
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    }
}

jobject JniUtils::convertQueryParamsMapToJavaMap(JNIEnv *env, const OC::QueryParamsMap &map)
{
    jobject hashMap = env->NewObject(g_cls_HashMap, g_mid_HashMap_ctor);
    if (!hashMap) return nullptr;

    for (auto it = map.begin(); it != map.end(); ++it)
    {
        std::string key = it->first;
        std::string value = it->second;

        env->CallObjectMethod(hashMap,
            g_mid_HashMap_put,
            env->NewStringUTF(key.c_str()),
            env->NewStringUTF(value.c_str()));
        if (env->ExceptionCheck()) return nullptr;
    }

    return hashMap;
}

void JniUtils::convertJavaRepresentationArrToVector(JNIEnv *env,
    jobjectArray jRepresentationArray,
    std::vector<OC::OCRepresentation>& representationVector)
{
    if (!jRepresentationArray) return;
    jsize len = env->GetArrayLength(jRepresentationArray);

    for (jsize i = 0; i < len; ++i)
    {
        jobject jRep = env->GetObjectArrayElement(jRepresentationArray, i);
        if (!jRep) return;
        OC::OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, jRep);
        representationVector.push_back(*rep);
        if (env->ExceptionCheck()) return;
        env->DeleteLocalRef(jRep);
    }
}

jobjectArray JniUtils::convertRepresentationVectorToJavaArray(JNIEnv *env,
    const std::vector<OC::OCRepresentation>& representationVector)
{
    jsize len = static_cast<jsize>(representationVector.size());
    jobjectArray repArr = env->NewObjectArray(len, g_cls_OcRepresentation, nullptr);
    if (!repArr) return nullptr;
    for (jsize i = 0; i < len; ++i)
    {
        OCRepresentation* rep = new OCRepresentation(representationVector[i]);
        jlong handle = reinterpret_cast<jlong>(rep);
        jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool,
            handle, true);
        if (!jRepresentation)
        {
            delete rep;
            return nullptr;
        }
        env->SetObjectArrayElement(repArr, i, jRepresentation);
        if (env->ExceptionCheck()) return nullptr;
        env->DeleteLocalRef(jRepresentation);
    }

    return repArr;
}