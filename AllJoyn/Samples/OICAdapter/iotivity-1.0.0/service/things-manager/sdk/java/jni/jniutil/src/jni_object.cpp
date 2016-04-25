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

#include "jni_object.h"

#define LOG_TAG "TM_JObject"

JObject::JObject(JNIEnv *env) : m_pEnv( env ),
    m_pObject(NULL),
    m_pClazz( NULL ),
    m_fIsNewObject(true)
{
}

JObject::JObject(JNIEnv *env, jobject obj) : m_pEnv(NULL),
    m_pObject(NULL),
    m_pClazz(NULL),
    m_fIsNewObject(false)
{
    if ( NULL == env || NULL == obj)
    {
        return;
    }

    m_pEnv = env;
    m_pObject = obj;
    m_pClazz = m_pEnv->GetObjectClass( obj );
}

JObject::JObject(JNIEnv *env, const char *classPath) : m_pEnv(NULL),
    m_pObject(NULL),
    m_pClazz(NULL),
    m_fIsNewObject(true)
{
    if ( NULL == env || NULL == classPath)
    {
        LOGI("JObject Invalid parameters");
        return;
    }

    m_pEnv = env;
    //m_pClazz = GetJClass( classPath );

    if (NULL == m_pClazz)
    {
        LOGE( "GetJClass failed [%s]" , classPath);
        return;
    }

    jmethodID mid = env->GetMethodID(m_pClazz, "<init>", "()V");
    if (NULL == mid)
    {
        LOGE( "GetMethodID failed [%s]" , classPath);
        return;
    }

    m_pObject = env->NewObject(m_pClazz, mid);
}

JObject::~JObject()
{
    if (m_pEnv)
    {
        if (m_pObject && m_fIsNewObject)
        {
            m_pEnv->DeleteLocalRef( m_pObject );
        }

        if (m_pClazz && !m_fIsNewObject)
        {
            m_pEnv->DeleteLocalRef( m_pClazz );
        }
    }
}

jobject JObject::getObject() const
{
    return m_pObject;
}


void JObject::detachObject()
{
    if (m_fIsNewObject)
    {
        m_fIsNewObject = false;
        m_pClazz = NULL;
    }
}

