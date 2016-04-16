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

#include "jni_string.h"

#define LOG_TAG "TM_JString"

JString::JString(JNIEnv *env, jstring value) : JObject(env, value)
{
    const char *buff = env->GetStringUTFChars(value, 0);

    m_cstr = buff;

    env->ReleaseStringUTFChars(value, buff);
}

JString::JString(JNIEnv *env, const char *value) : JObject(env)
{
    m_cstr = value;

    if (env)
    {
        m_pObject = env->NewStringUTF( value );
    }
}

JString::JString(JNIEnv *env, const std::string &value) : JObject(env)
{
    m_cstr = value;

    if (env)
    {
        m_pObject = env->NewStringUTF( value.c_str() );
    }
}

JString::~JString()
{
}

bool JString::getValue(std::string &value)
{
    value = m_cstr;
    return true;
}

const char *JString::c_str()
{
    return m_cstr.c_str();
}


