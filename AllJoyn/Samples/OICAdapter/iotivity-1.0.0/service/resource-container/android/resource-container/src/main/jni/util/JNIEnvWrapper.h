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

#ifndef RCS_JNI_ENV_WRAPPER_H_
#define RCS_JNI_ENV_WRAPPER_H_

#include <jni.h>

#include <exception>

#include "JavaLocalRef.h"

class JavaException: public std::exception
{
};

class JNIEnvWrapper
{
    public:
    JNIEnvWrapper() noexcept : m_env { } {}
    JNIEnvWrapper(JNIEnv *env) noexcept : m_env { env } {}

        JNIEnvWrapper &operator=(JNIEnv *env) noexcept
        {
            m_env = env;
            return *this;
        }

        jboolean IsSameObject(jobject lhs, jobject rhs)
        {
            return m_env->IsSameObject(lhs, rhs);
        }

        jclass GetObjectClass(jobject obj)
        {
            auto ret = m_env->GetObjectClass(obj);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jclass FindClass(const char *name)
        {
            auto ret = m_env->FindClass(name);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jclass FindClassAsGlobalRef(const char *name)
        {
            JavaLocalClass cls { m_env, FindClass(name) };

            return NewGlobalRef(cls);
        }

        jobject NewGlobalRef(jobject obj)
        {
            auto ret = m_env->NewGlobalRef(obj);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        template<typename T>
        T NewGlobalRef(T obj)
        {
            auto ret = static_cast< T >(m_env->NewGlobalRef(obj));
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        template<typename T>
        T NewGlobalRef(const JavaLocalRef< T > &obj)
        {
            auto ret = static_cast< T >(m_env->NewGlobalRef(obj));
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        void DeleteGlobalRef(jobject obj)
        {
            m_env->DeleteGlobalRef(obj);
        }

        jobject NewObject(jclass cls, jmethodID ctor, ...)
        {
            va_list args;
            va_start(args, ctor);
            auto ret = m_env->NewObjectV(cls, ctor, args);
            va_end(args);

            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jstring NewStringUTF(const char *str)
        {
            auto ret = m_env->NewStringUTF(str);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        const char *GetStringUTFChars(jstring str, jboolean *isCopy)
        {
            auto ret = m_env->GetStringUTFChars(str, isCopy);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        void ReleaseStringUTFChars(jstring str, const char *chars)
        {
            m_env->ReleaseStringUTFChars(str, chars);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        jmethodID GetConstructorID(jclass cls, const char *sig)
        {
            auto ret = m_env->GetMethodID(cls, "<init>", sig);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jmethodID GetMethodID(jclass cls, const char *name, const char *sig)
        {
            auto ret = m_env->GetMethodID(cls, name, sig);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jmethodID GetStaticMethodID(jclass cls, const char *name, const char *sig)
        {
            auto ret = m_env->GetStaticMethodID(cls, name, sig);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }


        jfieldID GetFieldID(jclass cls, const char *name, const char *sig)
        {
            auto ret = m_env->GetFieldID(cls, name, sig);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jfieldID GetStaticFieldID(jclass cls, const char *name, const char *sig)
        {
            auto ret = m_env->GetStaticFieldID(cls, name, sig);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jobject GetStaticObjectField(jclass cls, jfieldID fieldId)
        {
            auto ret = m_env->GetStaticObjectField(cls, fieldId);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jobject GetStaticObjectField(jclass cls, const char *name, const char *sig)
        {
            return GetStaticObjectField(cls, GetStaticFieldID(cls, name, sig));
        }

        jint GetIntField(jobject obj, jfieldID fieldId)
        {
            auto ret = m_env->GetIntField(obj, fieldId);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jlong GetLongField(jobject obj, jfieldID fieldId)
        {
            auto ret = m_env->GetLongField(obj, fieldId);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        void SetLongField(jobject obj, jfieldID fieldId, jlong val)
        {
            m_env->SetLongField(obj, fieldId, val);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        jobject GetObjectField(jobject obj, jfieldID fieldId)
        {
            auto ret = m_env->GetObjectField(obj, fieldId);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jint CallStaticIntMethod(jclass cls, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args, methodId);
            auto ret = m_env->CallStaticIntMethodV(cls, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jobject CallStaticObjectMethod(jclass cls, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args, methodId);
            auto ret = m_env->CallStaticObjectMethodV(cls, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        void CallVoidMethod(jobject obj, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args, methodId);
            m_env->CallVoidMethodV(obj, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        jboolean CallBooleanMethod(jobject obj, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args, methodId);
            auto ret = m_env->CallBooleanMethodV(obj, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jint CallIntMethod(jobject obj, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args, methodId);
            auto ret = m_env->CallIntMethod(obj, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jdouble CallDoubleMethod(jobject obj, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args , methodId);
            auto ret = m_env->CallDoubleMethod(obj, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }


        jobject CallObjectMethod(jobject obj, jmethodID methodId, ...)
        {
            va_list args;
            va_start(args, methodId);
            auto ret = m_env->CallObjectMethodV(obj, methodId, args);
            va_end(args);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jbooleanArray NewBooleanArray(jsize len)
        {
            auto ret = m_env->NewBooleanArray(len);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jintArray NewIntArray(jsize len)
        {
            auto ret = m_env->NewIntArray(len);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jdoubleArray NewDoubleArray(jsize len)
        {
            auto ret = m_env->NewDoubleArray(len);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jobjectArray NewObjectArray(jsize len, jclass cls, jobject init)
        {
            auto ret = m_env->NewObjectArray(len, cls, init);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jsize GetArrayLength(jarray array)
        {
            auto ret = m_env->GetArrayLength(array);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        jobject GetObjectArrayElement(jobjectArray array, jsize index)
        {
            auto ret = m_env->GetObjectArrayElement(array, index);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        void SetObjectArrayElement(jobjectArray array, jsize index, jobject val)
        {
            m_env->SetObjectArrayElement(array, index, val);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        void SetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, const jboolean *buf)
        {
            m_env->SetBooleanArrayRegion(array, start, len, buf);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        void SetIntArrayRegion(jintArray array, jsize start, jsize len, const jint *buf)
        {
            m_env->SetIntArrayRegion(array, start, len, buf);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        void SetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len, const jdouble *buf)
        {
            m_env->SetDoubleArrayRegion(array, start, len, buf);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        void *GetPrimitiveArrayCritical(jarray array, jboolean *isCopy)
        {
            auto ret = m_env->GetPrimitiveArrayCritical(array, isCopy);
            if (m_env->ExceptionCheck()) throw JavaException();
            return ret;
        }

        void ReleasePrimitiveArrayCritical(jarray array, void *carray, int mode)
        {
            m_env->ReleasePrimitiveArrayCritical(array, carray, mode);
            if (m_env->ExceptionCheck()) throw JavaException();
        }

        void ThrowNew(jclass cls, const char *msg)
        {
            m_env->ThrowNew(cls, msg);
            throw JavaException();
        }

        void ExceptionDescribe() const noexcept
        {
            m_env->ExceptionDescribe();
        }

        void ExceptionClear() noexcept
        {
            m_env->ExceptionClear();
        }

        jboolean ExceptionCheck() const noexcept
        {
            return m_env->ExceptionCheck();
        }

        operator bool() const noexcept
        {
            return m_env != nullptr;
        }

        JNIEnv *get() const
        {
            return m_env;
        }

    private:
        JNIEnv *m_env;
};



#endif // RCS_JNI_ENV_WRAPPER_H_
