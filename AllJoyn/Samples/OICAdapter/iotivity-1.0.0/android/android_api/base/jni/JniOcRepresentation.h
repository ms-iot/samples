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
#include "JniOcStack.h"
#include <AttributeValue.h>

#ifndef _Included_org_iotivity_base_OcRepresentation
#define _Included_org_iotivity_base_OcRepresentation

using namespace OC;

class JniOcRepresentation
{
public:
    static OCRepresentation* getOCRepresentationPtr(JNIEnv *env, jobject thiz);
};

struct JObjectConverter : boost::static_visitor < jobject >
{
    JObjectConverter(JNIEnv *env) : env(env){}

    jobject operator()(const NullType&) const { return nullptr; }
    jobject operator()(const int& val) const
    {
        jobject jobj = env->NewObject(
            g_cls_Integer,
            g_mid_Integer_ctor,
            static_cast<jint>(val));
        return jobj;
    }
    jobject operator()(const double& val) const
    {
        jobject jobj = env->NewObject(
            g_cls_Double,
            g_mid_Double_ctor,
            static_cast<jdouble>(val));
        return jobj;
    }
    jobject operator()(const bool& val) const
    {
        jobject jobj = env->NewObject(
            g_cls_Boolean,
            g_mid_Boolean_ctor,
            static_cast<jboolean>(val));
        return jobj;
    }
    jobject operator()(const std::string& val) const
    {
        jstring jstr = env->NewStringUTF(val.c_str());
        return static_cast<jobject>(jstr);
    }
    jobject operator()(const OC::OCRepresentation& val) const
    {
        OCRepresentation * rep = new OCRepresentation(val);
        jlong handle = reinterpret_cast<jlong>(rep);
        jobject jRepresentation = env->NewObject(
            g_cls_OcRepresentation,
            g_mid_OcRepresentation_N_ctor_bool,
            handle, true);
        if (!jRepresentation)
        {
            delete rep;
        }
        return jRepresentation;
    }

    // Sequences:
    jobject operator()(const std::vector<int>& val) const
    {
        size_t len = val.size();
        jintArray jIntArray = env->NewIntArray(len);
        if (!jIntArray) return nullptr;
        const int* ints = &val[0];
        env->SetIntArrayRegion(jIntArray, 0, len, reinterpret_cast<const jint*>(ints));
        return jIntArray;
    }
    jobject operator()(const std::vector<double>& val) const
    {
        size_t len = val.size();
        jdoubleArray jDoubleArray = env->NewDoubleArray(len);
        if (!jDoubleArray) return nullptr;
        const double* doubles = &val[0];
        env->SetDoubleArrayRegion(jDoubleArray, 0, len, reinterpret_cast<const jdouble*>(doubles));
        return jDoubleArray;
    }
    jobject operator()(const std::vector<bool>& val) const
    {
        size_t len = val.size();
        jbooleanArray jBooleanArray = env->NewBooleanArray(len);
        if (!jBooleanArray) return nullptr;
        jboolean* booleans = new jboolean[len];
        for (size_t i = 0; i < len; ++i) {
            booleans[i] = static_cast<jboolean>(val[i]);
        }
        env->SetBooleanArrayRegion(jBooleanArray, 0, len, booleans);
        if (env->ExceptionCheck()) return nullptr;
        env->ReleaseBooleanArrayElements(jBooleanArray, booleans, 0);
        return jBooleanArray;
    }
    jobject operator()(const std::vector<std::string>& val) const
    {
        size_t len = val.size();
        jobjectArray strArr = env->NewObjectArray(len, g_cls_String, nullptr);
        if (!strArr) return nullptr;
        for (size_t i = 0; i < len; ++i)
        {
            jstring jString = env->NewStringUTF(val[i].c_str());
            env->SetObjectArrayElement(strArr, static_cast<jsize>(i), jString);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jString);
        }
        return strArr;
    }
    jobject operator()(const std::vector<OC::OCRepresentation>& val) const
    {
        jsize len = static_cast<jsize>(val.size());
        jobjectArray repArr = env->NewObjectArray(len, g_cls_OcRepresentation, nullptr);
        if (!repArr) return nullptr;
        for (jsize i = 0; i < len; ++i)
        {
            OCRepresentation* rep = new OCRepresentation(val[i]);
            jlong handle = reinterpret_cast<jlong>(rep);
            jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool,
                handle, true);
            if (!jRepresentation)
            {
                delete rep;
                return nullptr;
            }
            env->SetObjectArrayElement(repArr, i, jRepresentation);
            if (env->ExceptionCheck())
            {
                delete rep;
                return nullptr;
            }
            env->DeleteLocalRef(jRepresentation);
        }
        return repArr;
    }

    // Nested sequences:
    jobject operator()(const std::vector<std::vector<int>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_int1DArray, nullptr);
        if (!jOuterArr)
        {
            return nullptr;
        }
        for (jsize i = 0; i < lenOuter; ++i)
        {
            size_t lenInner = val[i].size();
            jintArray jIntArray = env->NewIntArray(lenInner);
            if (!jIntArray) return nullptr;
            const int* ints = &val[i][0];
            env->SetIntArrayRegion(jIntArray, 0, lenInner, reinterpret_cast<const jint*>(ints));
            if (env->ExceptionCheck()) return nullptr;
            env->SetObjectArrayElement(jOuterArr, i, static_cast<jobject>(jIntArray));
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jIntArray);
        }
        return jOuterArr;
    }
    jobject operator()(const std::vector<std::vector<std::vector<int>>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_int2DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize k = 0; k < lenOuter; ++k)
        {
            jsize lenMiddle = static_cast<jsize>(val[k].size());
            jobjectArray jMiddleArr = env->NewObjectArray(lenMiddle, g_cls_int1DArray, nullptr);
            if (!jMiddleArr) return nullptr;
            for (jsize i = 0; i < lenMiddle; ++i)
            {
                jsize lenInner = static_cast<jsize>(val[k][i].size());
                jintArray jIntArray = env->NewIntArray(lenInner);
                if (!jIntArray) return nullptr;
                const int* ints = &val[k][i][0];
                env->SetIntArrayRegion(jIntArray, 0, lenInner, reinterpret_cast<const jint*>(ints));
                if (env->ExceptionCheck()) return nullptr;
                env->SetObjectArrayElement(jMiddleArr, i, jIntArray);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(jIntArray);
            }
            env->SetObjectArrayElement(jOuterArr, k, jMiddleArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jMiddleArr);
        }
        return jOuterArr;
    }

    jobject operator()(const std::vector<std::vector<double>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_double1DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize i = 0; i < lenOuter; ++i)
        {
            size_t lenInner = val[i].size();
            jdoubleArray jDoubleArray = env->NewDoubleArray(lenInner);
            if (!jDoubleArray) return nullptr;
            const double* doubles = &val[i][0];
            env->SetDoubleArrayRegion(jDoubleArray, 0, lenInner, reinterpret_cast<const jdouble*>(doubles));
            if (env->ExceptionCheck()) return nullptr;
            env->SetObjectArrayElement(jOuterArr, i, jDoubleArray);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jDoubleArray);
        }

        return jOuterArr;
    }
    jobject operator()(const std::vector<std::vector<std::vector<double>>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_double2DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize k = 0; k < lenOuter; ++k)
        {
            jsize lenMiddle = static_cast<jsize>(val[k].size());
            jobjectArray jMiddleArr = env->NewObjectArray(lenMiddle, g_cls_double1DArray, nullptr);
            if (!jMiddleArr) return nullptr;
            for (jsize i = 0; i < lenMiddle; ++i)
            {
                jsize lenInner = static_cast<jsize>(val[k][i].size());
                jdoubleArray jDoubleArray = env->NewDoubleArray(lenInner);
                if (!jDoubleArray) return nullptr;
                const double* doubles = &val[k][i][0];
                env->SetDoubleArrayRegion(jDoubleArray, 0, lenInner, reinterpret_cast<const jdouble*>(doubles));
                if (env->ExceptionCheck()) return nullptr;
                env->SetObjectArrayElement(jMiddleArr, i, jDoubleArray);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(jDoubleArray);
            }
            env->SetObjectArrayElement(jOuterArr, k, jMiddleArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jMiddleArr);
        }
        return jOuterArr;
    }

    jobject operator()(const std::vector<std::vector<bool>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_boolean1DArray, 0);
        if (!jOuterArr) return nullptr;
        for (jsize i = 0; i < lenOuter; ++i)
        {
            size_t lenInner = val[i].size();
            jbooleanArray jBooleanArray = env->NewBooleanArray(lenInner);
            if (!jBooleanArray) return nullptr;
            jboolean* booleans = new jboolean[lenInner];
            for (size_t j = 0; j < lenInner; ++j) {
                booleans[j] = static_cast<jboolean>(val[i][j]);
            }
            env->SetBooleanArrayRegion(jBooleanArray, 0, lenInner, booleans);
            if (env->ExceptionCheck()) return nullptr;
            env->SetObjectArrayElement(jOuterArr, i, jBooleanArray);
            if (env->ExceptionCheck()) return nullptr;
            env->ReleaseBooleanArrayElements(jBooleanArray, booleans, 0);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jBooleanArray);
        }
        return jOuterArr;
    }
    jobject operator()(const std::vector<std::vector<std::vector<bool>>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_boolean2DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize k = 0; k < lenOuter; ++k)
        {
            jsize lenMiddle = static_cast<jsize>(val[k].size());
            jobjectArray jMiddleArr = env->NewObjectArray(lenMiddle, g_cls_boolean1DArray, nullptr);
            if (!jMiddleArr) return nullptr;
            for (jsize i = 0; i < lenMiddle; ++i)
            {
                size_t lenInner = val[k][i].size();
                jbooleanArray jBooleanArray = env->NewBooleanArray(lenInner);
                jboolean* booleans = new jboolean[lenInner];
                for (size_t j = 0; j < lenInner; ++j) {
                    booleans[j] = val[k][i][j];
                }
                env->SetBooleanArrayRegion(jBooleanArray, 0, lenInner, booleans);
                if (env->ExceptionCheck()) return nullptr;
                env->SetObjectArrayElement(jMiddleArr, i, jBooleanArray);
                if (env->ExceptionCheck()) return nullptr;
                env->ReleaseBooleanArrayElements(jBooleanArray, booleans, 0);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(jBooleanArray);
            }
            env->SetObjectArrayElement(jOuterArr, k, jMiddleArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jMiddleArr);
        }
        return jOuterArr;
    }

    jobject operator()(const std::vector<std::vector<std::string>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_String1DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize i = 0; i < lenOuter; ++i)
        {
            jsize lenInner = static_cast<jsize>(val[i].size());
            jobjectArray strArr = env->NewObjectArray(lenInner, g_cls_String, nullptr);
            if (!strArr) return nullptr;
            for (jsize j = 0; j < lenInner; ++j)
            {
                jstring jString = env->NewStringUTF(val[i][j].c_str());
                env->SetObjectArrayElement(strArr, j, jString);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(jString);
            }
            env->SetObjectArrayElement(jOuterArr, i, strArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(strArr);
        }

        return jOuterArr;
    }
    jobject operator()(const std::vector<std::vector<std::vector<std::string>>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_String2DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize k = 0; k < lenOuter; ++k)
        {
            jsize lenMiddle = static_cast<jsize>(val[k].size());
            jobjectArray jMiddleArr = env->NewObjectArray(lenMiddle, g_cls_String1DArray, nullptr);
            if (!jMiddleArr) return nullptr;
            for (jsize i = 0; i < lenMiddle; ++i)
            {
                jsize lenInner = static_cast<jsize>(val[k][i].size());
                jobjectArray strArr = env->NewObjectArray(lenInner, g_cls_String, nullptr);
                if (!strArr) return nullptr;
                for (jsize j = 0; j < lenInner; ++j)
                {
                    jstring jString = env->NewStringUTF(val[k][i][j].c_str());
                    env->SetObjectArrayElement(strArr, j, jString);
                    if (env->ExceptionCheck()) return nullptr;
                    env->DeleteLocalRef(jString);
                }
                env->SetObjectArrayElement(jMiddleArr, i, strArr);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(strArr);
            }
            env->SetObjectArrayElement(jOuterArr, k, jMiddleArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jMiddleArr);
        }
        return jOuterArr;
    }

    jobject operator()(const std::vector<std::vector<OC::OCRepresentation>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_OcRepresentation1DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize i = 0; i < lenOuter; ++i)
        {
            jsize lenInner = static_cast<jsize>(val[i].size());
            jobjectArray repArr = env->NewObjectArray(lenInner, g_cls_OcRepresentation, nullptr);
            if (!repArr) return nullptr;
            for (jsize j = 0; j < lenInner; ++j)
            {
                OCRepresentation* rep = new OCRepresentation(val[i][j]);
                jlong handle = reinterpret_cast<jlong>(rep);
                jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool,
                    handle, true);
                if (!jRepresentation)
                {
                    delete rep;
                    return nullptr;
                }
                env->SetObjectArrayElement(repArr, j, jRepresentation);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(jRepresentation);
            }
            env->SetObjectArrayElement(jOuterArr, i, repArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(repArr);
        }
        return jOuterArr;
    }
    jobject operator()(const std::vector<std::vector<std::vector<OC::OCRepresentation>>>& val) const
    {
        jsize lenOuter = static_cast<jsize>(val.size());
        jobjectArray jOuterArr = env->NewObjectArray(lenOuter, g_cls_OcRepresentation2DArray, nullptr);
        if (!jOuterArr) return nullptr;
        for (jsize k = 0; k < lenOuter; ++k)
        {
            jsize lenMiddle = static_cast<jsize>(val[k].size());
            jobjectArray jMiddleArr = env->NewObjectArray(lenMiddle, g_cls_OcRepresentation1DArray, nullptr);
            if (!jMiddleArr) return nullptr;
            for (jsize i = 0; i < lenMiddle; ++i)
            {
                jsize lenInner = static_cast<jsize>(val[k][i].size());
                jobjectArray repArr = env->NewObjectArray(lenInner, g_cls_OcRepresentation, nullptr);
                if (!repArr) return nullptr;
                for (jsize j = 0; j < lenInner; ++j)
                {
                    OCRepresentation* rep = new OCRepresentation(val[k][i][j]);
                    jlong handle = reinterpret_cast<jlong>(rep);
                    jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool,
                        handle, true);
                    if (!jRepresentation)
                    {
                        delete rep;
                        return nullptr;
                    }
                    env->SetObjectArrayElement(repArr, j, jRepresentation);
                    if (env->ExceptionCheck()) return nullptr;
                    env->DeleteLocalRef(jRepresentation);
                }
                env->SetObjectArrayElement(jMiddleArr, i, repArr);
                if (env->ExceptionCheck()) return nullptr;
                env->DeleteLocalRef(repArr);
            }
            env->SetObjectArrayElement(jOuterArr, k, jMiddleArr);
            if (env->ExceptionCheck()) return nullptr;
            env->DeleteLocalRef(jMiddleArr);
        }
        return jOuterArr;
    }

private:
    JNIEnv *env;
};

#ifdef __cplusplus
extern "C" {
#endif

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    getValueN
    * Signature: (Ljava/lang/String;)Ljava/lang/Object;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcRepresentation_getValueN
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueInteger
    * Signature: (Ljava/lang/String;I)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueInteger
        (JNIEnv *, jobject, jstring, jint);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueDouble
    * Signature: (Ljava/lang/String;D)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDouble
        (JNIEnv *, jobject, jstring, jdouble);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueBoolean
    * Signature: (Ljava/lang/String;Z)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBoolean
        (JNIEnv *, jobject, jstring, jboolean);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueStringN
    * Signature: (Ljava/lang/String;Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueStringN
        (JNIEnv *, jobject, jstring, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueRepresentation
    * Signature: (Ljava/lang/String;Lorg/iotivity/base/OcRepresentation;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentation
        (JNIEnv *, jobject, jstring, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueIntegerArray
    * Signature: (Ljava/lang/String;[I)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueIntegerArray
        (JNIEnv *, jobject, jstring, jintArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueInteger2DArray
    * Signature: (Ljava/lang/String;[[I)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueInteger2DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueInteger3DArray
    * Signature: (Ljava/lang/String;[[[I)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueInteger3DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueDoubleArray
    * Signature: (Ljava/lang/String;[D)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDoubleArray
        (JNIEnv *, jobject, jstring, jdoubleArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueDouble2DArray
    * Signature: (Ljava/lang/String;[[D)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDouble2DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueDouble3DArray
    * Signature: (Ljava/lang/String;[[[D)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueDouble3DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueBooleanArray
    * Signature: (Ljava/lang/String;[Z)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBooleanArray
        (JNIEnv *, jobject, jstring, jbooleanArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueBoolean2DArray
    * Signature: (Ljava/lang/String;[[Z)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBoolean2DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueBoolean3DArray
    * Signature: (Ljava/lang/String;[[[Z)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueBoolean3DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueStringArray
    * Signature: (Ljava/lang/String;[Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueStringArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueString2DArray
    * Signature: (Ljava/lang/String;[[Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueString2DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueString3DArray
    * Signature: (Ljava/lang/String;[[[Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueString3DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueRepresentationArray
    * Signature: (Ljava/lang/String;[Lorg/iotivity/base/OcRepresentation;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentationArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueRepresentation2DArray
    * Signature: (Ljava/lang/String;[[Lorg/iotivity/base/OcRepresentation;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentation2DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setValueRepresentation3DArray
    * Signature: (Ljava/lang/String;[[[Lorg/iotivity/base/OcRepresentation;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setValueRepresentation3DArray
        (JNIEnv *, jobject, jstring, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    addChild
    * Signature: (Lorg/iotivity/base/OcRepresentation;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_addChild
        (JNIEnv *, jobject, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    clearChildren
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_clearChildren
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    getChildrenArray
    * Signature: ()[Lorg/iotivity/base/OcRepresentation;
    */
    JNIEXPORT jobjectArray JNICALL Java_org_iotivity_base_OcRepresentation_getChildrenArray
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    getUri
    * Signature: ()Ljava/lang/String;
    */
    JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcRepresentation_getUri
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setUri
    * Signature: (Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setUri
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    getResourceTypes
    * Signature: ()Ljava/util/List;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcRepresentation_getResourceTypes
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setResourceTypeArray
    * Signature: ([Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setResourceTypeArray
        (JNIEnv *, jobject, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    getResourceInterfaces
    * Signature: ()Ljava/util/List;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcRepresentation_getResourceInterfaces
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setResourceInterfaceArray
    * Signature: ([Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setResourceInterfaceArray
        (JNIEnv *, jobject, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    isEmpty
    * Signature: ()Z
    */
    JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_isEmpty
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    size
    * Signature: ()I
    */
    JNIEXPORT jint JNICALL Java_org_iotivity_base_OcRepresentation_size
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    remove
    * Signature: (Ljava/lang/String;)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_remove
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    hasAttribute
    * Signature: (Ljava/lang/String;)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_hasAttribute
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    setNull
    * Signature: (Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_setNull
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    isNull
    * Signature: (Ljava/lang/String;)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcRepresentation_isNull
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    create
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_create
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcRepresentation
    * Method:    dispose
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRepresentation_dispose
        (JNIEnv *, jobject, jboolean);

#ifdef __cplusplus
}
#endif
#endif
