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
#include "JniOcRepresentation.h"
#include "JniUtils.h"

using namespace OC;

OCRepresentation* JniOcRepresentation::getOCRepresentationPtr(JNIEnv *env, jobject thiz)
{
    OCRepresentation *rep = GetHandle<OCRepresentation>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcRepresentation");
    }
    if (!rep)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return rep;
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    getValueN
* Signature: (Ljava/lang/String;)Ljava/lang/Object;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcRepresentation_getValueN
(JNIEnv *env, jobject thiz, jstring jKey)
{
    LOGD("OcRepresentation_getValue");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "attributeKey cannot be null");
        return nullptr;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return nullptr;

    std::string key = env->GetStringUTFChars(jKey, nullptr);

    AttributeValue attrValue;
    if (!rep->getAttributeValue(key.c_str(), attrValue))
    {
        ThrowOcException(JNI_NO_SUCH_KEY, " attribute key does not exist");
        return nullptr;
    }
    return boost::apply_visitor(JObjectConverter(env), attrValue);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueInteger
* Signature: (Ljava/lang/String;I)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueInteger
(JNIEnv *env, jobject thiz, jstring jKey, jint jValue)
{
    LOGD("OcRepresentation_setValueInteger");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string str = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(str, static_cast<int>(jValue));
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueDouble
* Signature: (Ljava/lang/String;D)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDouble
(JNIEnv *env, jobject thiz, jstring jKey, jdouble jValue)
{
    LOGD("OcRepresentation_setValueDouble");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string str = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(str, static_cast<double>(jValue));
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueBoolean
* Signature: (Ljava/lang/String;Z)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBoolean
(JNIEnv *env, jobject thiz, jstring jKey, jboolean jValue)
{
    LOGD("OcRepresentation_setValueBoolean");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string str = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(str, static_cast<bool>(jValue));
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueStringN
* Signature: (Ljava/lang/String;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueStringN
(JNIEnv *env, jobject thiz, jstring jKey, jstring jValue)
{
    LOGD("OcRepresentation_setValueString");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    std::string value = env->GetStringUTFChars(jValue, nullptr);

    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueRepresentation
* Signature: (Ljava/lang/String;Lorg/iotivity/base/OcRepresentation;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentation
(JNIEnv *env, jobject thiz, jstring jKey, jobject jValue)
{
    LOGD("OcRepresentation_setValueRepresentation");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);

    if (jValue)
    {
        OCRepresentation *value = JniOcRepresentation::getOCRepresentationPtr(env, jValue);
        if (!value) return;
        rep->setValue(key, *value);
    }
    else
    {
        rep->setNULL(key);
    }
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueIntegerArray
* Signature: (Ljava/lang/String;[I)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueIntegerArray
(JNIEnv *env, jobject thiz, jstring jKey, jintArray jValue)
{
    LOGD("OcRepresentation_setValueIntegerArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }

    const jsize len = env->GetArrayLength(jValue);
    jint* ints = env->GetIntArrayElements(jValue, nullptr);

    std::vector<int> value;
    for (jsize i = 0; i < len; ++i)
    {
        value.push_back(static_cast<int>(ints[i]));
    }
    env->ReleaseIntArrayElements(jValue, ints, JNI_ABORT);

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueInteger2DArray
* Signature: (Ljava/lang/String;[[I)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueInteger2DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation__setValueInteger2DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<int>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize j = 0; j < lenOuter; ++j)
    {
        jintArray jInnerArray = static_cast<jintArray>(env->GetObjectArrayElement(jValue, j));
        jint* ints = env->GetIntArrayElements(jInnerArray, nullptr);
        std::vector<int> innerVector;
        const jsize lenInner = env->GetArrayLength(jInnerArray);
        for (jsize i = 0; i < lenInner; ++i)
        {
            innerVector.push_back(static_cast<int>(ints[i]));
        }
        env->ReleaseIntArrayElements(jInnerArray, ints, JNI_ABORT);
        env->DeleteLocalRef(jInnerArray);
        value.push_back(innerVector);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueInteger3DArray
* Signature: (Ljava/lang/String;[[[I)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueInteger3DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueInteger3DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<std::vector<int>>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize k = 0; k < lenOuter; ++k)
    {
        jobjectArray jMiddleArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, k));
        const jsize lenMiddle = env->GetArrayLength(jMiddleArray);
        std::vector<std::vector<int>> middleArray;
        for (jsize j = 0; j < lenMiddle; ++j)
        {
            jintArray jInnerArray = static_cast<jintArray>(env->GetObjectArrayElement(jMiddleArray, j));
            jint* ints = env->GetIntArrayElements(jInnerArray, nullptr);
            std::vector<int> innerVector;
            const jsize lenInner = env->GetArrayLength(jInnerArray);
            for (jsize i = 0; i < lenInner; ++i)
            {
                innerVector.push_back(static_cast<int>(ints[i]));
            }
            env->ReleaseIntArrayElements(jInnerArray, ints, JNI_ABORT);
            env->DeleteLocalRef(jInnerArray);
            middleArray.push_back(innerVector);
        }
        env->DeleteLocalRef(jMiddleArray);
        value.push_back(middleArray);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueDoubleArray
* Signature: (Ljava/lang/String;[D)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDoubleArray
(JNIEnv *env, jobject thiz, jstring jKey, jdoubleArray jValue)
{
    LOGD("OcRepresentation_setValueDoubleArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }

    const jsize len = env->GetArrayLength(jValue);
    jdouble* doubles = env->GetDoubleArrayElements(jValue, nullptr);

    std::vector<double> value;
    for (jsize i = 0; i < len; ++i)
    {
        value.push_back(static_cast<double>(doubles[i]));
    }
    env->ReleaseDoubleArrayElements(jValue, doubles, JNI_ABORT);

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueDouble2DArray
* Signature: (Ljava/lang/String;[[D)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDouble2DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueDouble2DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<double>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize j = 0; j < lenOuter; ++j)
    {
        jdoubleArray jInnerArray = static_cast<jdoubleArray>(env->GetObjectArrayElement(jValue, j));
        jdouble* doubles = env->GetDoubleArrayElements(jInnerArray, nullptr);
        std::vector<double> innerVector;
        const jsize lenInner = env->GetArrayLength(jInnerArray);
        for (jsize i = 0; i < lenInner; ++i)
        {
            innerVector.push_back(static_cast<double>(doubles[i]));
        }
        env->ReleaseDoubleArrayElements(jInnerArray, doubles, JNI_ABORT);
        env->DeleteLocalRef(jInnerArray);
        value.push_back(innerVector);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueDouble3DArray
* Signature: (Ljava/lang/String;[[[D)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDouble3DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueDouble3DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<std::vector<double>>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize k = 0; k < lenOuter; ++k)
    {
        jobjectArray jMiddleArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, k));
        const jsize lenMiddle = env->GetArrayLength(jMiddleArray);
        std::vector<std::vector<double>> middleArray;
        for (jsize j = 0; j < lenMiddle; ++j)
        {
            jdoubleArray jInnerArray = static_cast<jdoubleArray>(env->GetObjectArrayElement(jMiddleArray, j));
            jdouble* doubles = env->GetDoubleArrayElements(jInnerArray, nullptr);
            std::vector<double> innerVector;
            const jsize lenInner = env->GetArrayLength(jInnerArray);
            for (jsize i = 0; i < lenInner; ++i)
            {
                innerVector.push_back(static_cast<double>(doubles[i]));
            }
            env->ReleaseDoubleArrayElements(jInnerArray, doubles, JNI_ABORT);
            env->DeleteLocalRef(jInnerArray);
            middleArray.push_back(innerVector);
        }
        env->DeleteLocalRef(jMiddleArray);
        value.push_back(middleArray);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueBooleanArray
* Signature: (Ljava/lang/String;[Z)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBooleanArray
(JNIEnv *env, jobject thiz, jstring jKey, jbooleanArray jValue)
{
    LOGD("OcRepresentation_setValueBooleanArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }

    const jsize len = env->GetArrayLength(jValue);
    jboolean* booleans = env->GetBooleanArrayElements(jValue, nullptr);

    std::vector<bool> value;
    for (jsize i = 0; i < len; ++i)
    {
        value.push_back(static_cast<bool>(booleans[i]));
    }
    env->ReleaseBooleanArrayElements(jValue, booleans, JNI_ABORT);

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueBoolean2DArray
* Signature: (Ljava/lang/String;[[Z)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBoolean2DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueBoolean2DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<bool>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize j = 0; j < lenOuter; ++j)
    {
        jbooleanArray jInnerArray = static_cast<jbooleanArray>(env->GetObjectArrayElement(jValue, j));
        const jsize lenInner = env->GetArrayLength(jInnerArray);
        jboolean* booleans = env->GetBooleanArrayElements(jInnerArray, nullptr);

        std::vector<bool> innerVector;
        for (jsize i = 0; i < lenInner; ++i)
        {
            innerVector.push_back(static_cast<bool>(booleans[i]));
        }
        env->ReleaseBooleanArrayElements(jInnerArray, booleans, JNI_ABORT);
        env->DeleteLocalRef(jInnerArray);
        value.push_back(innerVector);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueBoolean3DArray
* Signature: (Ljava/lang/String;[[[Z)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBoolean3DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueBoolean3DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<std::vector<bool>>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize k = 0; k < lenOuter; ++k)
    {
        jobjectArray jMiddleArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, k));
        const jsize lenMiddle = env->GetArrayLength(jMiddleArray);
        std::vector<std::vector<bool>> middleArray;
        for (jsize j = 0; j < lenMiddle; ++j)
        {
            jbooleanArray jInnerArray = static_cast<jbooleanArray>(env->GetObjectArrayElement(jMiddleArray, j));
            const jsize lenInner = env->GetArrayLength(jInnerArray);
            jboolean* booleans = env->GetBooleanArrayElements(jInnerArray, nullptr);

            std::vector<bool> innerVector;
            for (jsize i = 0; i < lenInner; ++i)
            {
                innerVector.push_back(static_cast<bool>(booleans[i]));
            }
            env->ReleaseBooleanArrayElements(jInnerArray, booleans, JNI_ABORT);
            env->DeleteLocalRef(jInnerArray);
            middleArray.push_back(innerVector);
        }
        env->DeleteLocalRef(jMiddleArray);
        value.push_back(middleArray);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueStringArray
* Signature: (Ljava/lang/String;[Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueStringArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueStringArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }

    std::vector<std::string> value;
    JniUtils::convertJavaStrArrToStrVector(env, jValue, value);

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueString2DArray
* Signature: (Ljava/lang/String;[[Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueString2DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueString2DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<std::string>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize j = 0; j < lenOuter; ++j)
    {
        jobjectArray jInnerArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, j));
        std::vector<std::string> innerVector;
        JniUtils::convertJavaStrArrToStrVector(env, jInnerArray, innerVector);
        env->DeleteLocalRef(jInnerArray);
        value.push_back(innerVector);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueString3DArray
* Signature: (Ljava/lang/String;[[[Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueString3DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueString3DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<std::vector<std::string>>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize k = 0; k < lenOuter; ++k)
    {
        jobjectArray jMiddleArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, k));
        const jsize lenMiddle = env->GetArrayLength(jMiddleArray);
        std::vector<std::vector<std::string>> middleArray;
        for (jsize j = 0; j < lenMiddle; ++j)
        {
            jobjectArray jInnerArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jMiddleArray, j));
            std::vector<std::string> innerVector;
            JniUtils::convertJavaStrArrToStrVector(env, jInnerArray, innerVector);
            env->DeleteLocalRef(jInnerArray);
            middleArray.push_back(innerVector);
        }
        env->DeleteLocalRef(jMiddleArray);
        value.push_back(middleArray);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueRepresentationArray
* Signature: (Ljava/lang/String;[Lorg/iotivity/base/OcRepresentation;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentationArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueRepresentationArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }

    std::vector<OCRepresentation> value;
    JniUtils::convertJavaRepresentationArrToVector(env, jValue, value);

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueRepresentation2DArray
* Signature: (Ljava/lang/String;[[Lorg/iotivity/base/OcRepresentation;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentation2DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueRepresentation2DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<OCRepresentation>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize j = 0; j < lenOuter; ++j)
    {
        jobjectArray jInnerArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, j));
        std::vector<OCRepresentation> innerVector;
        JniUtils::convertJavaRepresentationArrToVector(env, jInnerArray, innerVector);
        env->DeleteLocalRef(jInnerArray);
        value.push_back(innerVector);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setValueRepresentation3DArray
* Signature: (Ljava/lang/String;[[[Lorg/iotivity/base/OcRepresentation;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentation3DArray
(JNIEnv *env, jobject thiz, jstring jKey, jobjectArray jValue)
{
    LOGD("OcRepresentation_setValueRepresentation3DArray");
    if (!jKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "key cannot be null");
        return;
    }
    std::vector<std::vector<std::vector<OCRepresentation>>> value;
    const jsize lenOuter = env->GetArrayLength(jValue);
    for (jsize k = 0; k < lenOuter; ++k)
    {
        jobjectArray jMiddleArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jValue, k));
        const jsize lenMiddle = env->GetArrayLength(jMiddleArray);
        std::vector<std::vector<OCRepresentation>> middleArray;
        for (jsize j = 0; j < lenMiddle; ++j)
        {
            jobjectArray jInnerArray = static_cast<jobjectArray>(env->GetObjectArrayElement(jMiddleArray, j));
            std::vector<OCRepresentation> innerVector;
            JniUtils::convertJavaRepresentationArrToVector(env, jInnerArray, innerVector);
            env->DeleteLocalRef(jInnerArray);
            middleArray.push_back(innerVector);
        }
        env->DeleteLocalRef(jMiddleArray);
        value.push_back(middleArray);
    }

    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string key = env->GetStringUTFChars(jKey, nullptr);
    rep->setValue(key, value);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    addChild
* Signature: (Lorg/iotivity/base/OcRepresentation;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_addChild
(JNIEnv *env, jobject thiz, jobject jOcRepresentation)
{
    LOGD("OcRepresentation_addChild");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    OCRepresentation *child = JniOcRepresentation::getOCRepresentationPtr(env, jOcRepresentation);
    if (!child) return;

    rep->addChild(*child);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    clearChildren
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_clearChildren
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_clearChildren");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    rep->clearChildren();
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    getChildrenArray
* Signature: ()[Lorg/iotivity/base/OcRepresentation;
*/
JNIEXPORT jobjectArray JNICALL Java_org_iotivity_base_OcRepresentation_getChildrenArray
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_getChildrenArray");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return nullptr;

    return JniUtils::convertRepresentationVectorToJavaArray(env, rep->getChildren());
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    getUri
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcRepresentation_getUri
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_getUri");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return nullptr;

    std::string uri(rep->getUri());
    return env->NewStringUTF(uri.c_str());
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setUri
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setUri
(JNIEnv *env, jobject thiz, jstring jUri)
{
    LOGD("OcRepresentation_setUri");
    if (!jUri)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "uri cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    rep->setUri(env->GetStringUTFChars(jUri, nullptr));
}

JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_hasAttribute
(JNIEnv *env, jobject thiz, jstring jstr)
{
    LOGD("OcRepresentation_hasAttribute");
    if (!jstr)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "attributeKey cannot be null");
        return false;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return false;

    std::string str = env->GetStringUTFChars(jstr, nullptr);
    return rep->hasAttribute(str);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    getResourceTypes
* Signature: ()Ljava/util/List;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcRepresentation_getResourceTypes
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_getResourceTypes");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return nullptr;

    std::vector<std::string> resourceTypes = rep->getResourceTypes();
    return JniUtils::convertStrVectorToJavaStrList(env, resourceTypes);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setResourceTypeArray
* Signature: ([Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setResourceTypeArray
(JNIEnv *env, jobject thiz, jobjectArray jResourceTypeArray)
{
    LOGD("OcRepresentation_setResourceTypeArray");
    if (!jResourceTypeArray)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "resourceTypeList cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::vector<std::string> resourceTypes;
    JniUtils::convertJavaStrArrToStrVector(env, jResourceTypeArray, resourceTypes);
    rep->setResourceTypes(resourceTypes);
}
/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    getResourceInterfaces
* Signature: ()Ljava/util/List;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcRepresentation_getResourceInterfaces
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_getResourceInterfaces");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return nullptr;

    std::vector<std::string> resourceInterfaces = rep->getResourceInterfaces();
    return JniUtils::convertStrVectorToJavaStrList(env, resourceInterfaces);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setResourceInterfaceArray
* Signature: ([Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setResourceInterfaceArray
(JNIEnv *env, jobject thiz, jobjectArray jResourceInterfaceArray)
{
    LOGD("OcRepresentation_setResourceInterfaceArray");
    if (!jResourceInterfaceArray)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "resourceInterfaceList cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::vector<std::string> resourceInterfaces;
    JniUtils::convertJavaStrArrToStrVector(env, jResourceInterfaceArray, resourceInterfaces);
    rep->setResourceInterfaces(resourceInterfaces);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    isEmpty
* Signature: ()Z
*/
JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_isEmpty
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_isEmpty");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return false;

    return static_cast<jboolean>(rep->empty());
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    size
* Signature: ()I
*/
JNIEXPORT jint JNICALL Java_org_iotivity_base_OcRepresentation_size
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_size");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return -1;

    return static_cast<jint>(rep->numberOfAttributes());
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    remove
* Signature: (Ljava/lang/String;)Z
*/
JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_remove
(JNIEnv *env, jobject thiz, jstring jAttributeKey)
{
    LOGD("OcRepresentation_remove");
    if (!jAttributeKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "attributeKey cannot be null");
        return false;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return false;

    std::string attributeKey = env->GetStringUTFChars(jAttributeKey, nullptr);
    return static_cast<jboolean>(rep->erase(attributeKey));
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    setNull
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setNull
(JNIEnv *env, jobject thiz, jstring jAttributeKey)
{
    LOGD("OcRepresentation_setNull");
    if (!jAttributeKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "attributeKey cannot be null");
        return;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return;

    std::string attributeKey = env->GetStringUTFChars(jAttributeKey, nullptr);
    rep->setNULL(attributeKey);
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    isNull
* Signature: (Ljava/lang/String;)Z
*/
JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_isNull
(JNIEnv *env, jobject thiz, jstring jAttributeKey)
{
    LOGD("OcRepresentation_isNull");
    if (!jAttributeKey)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "attributeKey cannot be null");
        return false;
    }
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);
    if (!rep) return false;

    std::string attributeKey = env->GetStringUTFChars(jAttributeKey, nullptr);
    return static_cast<jboolean>(rep->isNULL(attributeKey));
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    create
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_create
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRepresentation_create");
    OCRepresentation *rep = new OCRepresentation();
    SetHandle<OCRepresentation>(env, thiz, rep);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to set native handle for OcRepresentation");
        delete rep;
    }
}

/*
* Class:     org_iotivity_base_OcRepresentation
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_dispose
(JNIEnv *env, jobject thiz, jboolean jNeedsDelete)
{
    LOGD("OcRepresentation_dispose");
    OCRepresentation *rep = JniOcRepresentation::getOCRepresentationPtr(env, thiz);

    if (jNeedsDelete)
    {
        delete rep;
    }
}
