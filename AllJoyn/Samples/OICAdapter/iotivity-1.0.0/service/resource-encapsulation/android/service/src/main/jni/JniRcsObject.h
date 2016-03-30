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

#ifndef JNI_RCS_OBJECT_H_
#define JNI_RCS_OBJECT_H_

#include <jni.h>

#include <memory>

#include "JavaClasses.h"
#include "JNIEnvWrapper.h"

extern jfieldID g_field_mNativeHandle;

void initRCSObject(JNIEnvWrapper*);
void clearRCSObject(JNIEnvWrapper*);

namespace Detail
{
    struct BaseHandleHolder
    {
        virtual ~BaseHandleHolder() {}
    };

    template< typename T >
    struct HandleHolder: public BaseHandleHolder
    {
        HandleHolder(T* ptr) : m_ptr { ptr } {}

        virtual ~HandleHolder() { delete m_ptr; }

        T* m_ptr;
    };

    template< typename ENV >
    void* getNativeHandle(ENV* env, jobject obj)
    {
        return reinterpret_cast< void* >(env->GetLongField(obj, g_field_mNativeHandle));
    }
}

template< typename ENV >
bool hasNativeHandle(ENV* env, jobject obj)
{
    return Detail::getNativeHandle(env, obj) != nullptr;
}

template< typename T, typename ENV, typename ...PARAMS >
inline void setSafeNativeHandle(ENV* env, jobject obj, PARAMS&&... params)
{
    static_assert(!std::is_array< T >::value, "Array is not supported!");

    std::unique_ptr< Detail::HandleHolder< T > > p(
            new Detail::HandleHolder< T >{ new T{ std::forward< PARAMS >(params)... } });

    env->SetLongField(obj, g_field_mNativeHandle, reinterpret_cast< jlong >(p.get()));

    if (env->ExceptionCheck()) return;

    p.release();
}

template< typename ENV >
void releaseNativeHandle(ENV* env, jobject obj)
{
    auto handleHolder = reinterpret_cast< Detail::BaseHandleHolder* >(
            env->GetLongField(obj, g_field_mNativeHandle));

    delete handleHolder;

    env->SetLongField(obj, g_field_mNativeHandle, 0);
}


template< typename T >
inline T& getNativeHandleAs(JNIEnv* env, jobject obj)
{
    auto handleHolder = static_cast< Detail::HandleHolder< T >* >(Detail::getNativeHandle(env, obj));

    if (!handleHolder)
    {
        env->ThrowNew(env->FindClass(EXC_NAME_ILLEGAL_STATE), "Internal handle is null!");
    }

   return *handleHolder->m_ptr;
}

template< typename T >
inline T& getNativeHandleAs(JNIEnvWrapper* env, jobject obj)
{
    getNativeHandleAs< T >(env->get(), obj);
    if (env->ExceptionCheck()) throw JavaException();
}


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsObject_nativeDispose(JNIEnv*, jobject);

#ifdef __cplusplus
}
#endif

#endif // JNI_RCS_OBJECT_H_
