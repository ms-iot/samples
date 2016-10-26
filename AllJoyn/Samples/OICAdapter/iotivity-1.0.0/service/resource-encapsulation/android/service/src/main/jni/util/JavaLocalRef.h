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

#ifndef JAVA_LOCAL_REF_H_
#define JAVA_LOCAL_REF_H_

#include <jni.h>
#include <cassert>

template < typename T >
class JavaLocalRef
{
public:
    JavaLocalRef(JNIEnv* env, T obj) noexcept :
        m_env{ env },
        m_obj{ obj }
    {
        assert(env && "JNIEnv is nullptr");
    }

    template< typename ENV >
    JavaLocalRef(ENV* env, T obj) noexcept :
        m_env{ env->get() },
        m_obj{ obj }
    {
        assert(env && "JNIEnv is nullptr");
    }

    ~JavaLocalRef()
    {
        if (m_obj) m_env->DeleteLocalRef(m_obj);
    }

    operator bool() const noexcept { return m_obj; }
    operator T() const noexcept { return m_obj; }

    jobject get() const noexcept { return m_obj; }

    JavaLocalRef(const JavaLocalRef&) = delete;
    JavaLocalRef& operator=(const JavaLocalRef&) = delete;

    JavaLocalRef& operator=(JavaLocalRef&&) = delete;

private:
    JNIEnv* m_env;
    T m_obj;
};

typedef JavaLocalRef< jobject > JavaLocalObject;
typedef JavaLocalRef< jstring > JavaLocalString;
typedef JavaLocalRef< jclass > JavaLocalClass;

#endif // JAVA_LOCAL_REF_H_
