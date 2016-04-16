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

#include "jni_getter.h"

#include <string>

#include "JniOcResource.h"

#define LOG_TAG "TM_JGetter"

bool JGetter::getJStringField(JNIEnv *env, jobject &object, const char *fieldName,
                              std::string &value)
{
    if (NULL == env ||
        NULL == object ||
        NULL == fieldName)
    {
        LOGE("getJStringField invalid parameters");
        return false;
    }

    jclass clazz = env->GetObjectClass( object );
    if (NULL == clazz)
    {
        LOGE("GetObjectClass failed [%s]", fieldName);
        return false;
    }

    jfieldID fieldID = env->GetFieldID( clazz, fieldName, "Ljava/lang/String;" );
    if (0 == fieldID)
    {
        LOGE("GetFieldID failed [%s]", fieldName);
        env->DeleteLocalRef(clazz);
        return false;
    }

    jstring jValue = (jstring)env->GetObjectField( object, fieldID );
    if (NULL == jValue)
    {
        LOGE("GetObjectField failed [%s]", fieldName);
        env->DeleteLocalRef(clazz);
        return false;
    }

    const char *cstr = env->GetStringUTFChars(jValue, 0);
    if (cstr == NULL)
    {
        LOGE("GetStringUTFChars failed");
    }
    else
    {
        value = cstr;
        env->ReleaseStringUTFChars(jValue, cstr);
    }

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jValue);

    return true;
}

bool JGetter::getJBoolField(JNIEnv *env, jobject &object, const char *fieldName, bool &value)
{
    if (NULL == env ||
        NULL == object ||
        NULL == fieldName)
    {
        LOGE("getJBoolField invalid parameters");
        return false;
    }

    jclass clazz = env->GetObjectClass( object );
    if (NULL == clazz)
    {
        LOGE("GetObjectClass failed");
        return false;
    }

    jfieldID fieldID = env->GetFieldID( clazz, fieldName, "Z" );
    if (0 == fieldID)
    {
        LOGE("GetFieldID failed [%s]", fieldName);
        env->DeleteLocalRef( clazz );
        return false;
    }

    value = env->GetBooleanField( object, fieldID );

    env->DeleteLocalRef(clazz);

    return true;
}

bool JGetter::getJIntField(JNIEnv *env, jobject &object, const char *fieldName, int &value)
{
    if (NULL == env ||
        NULL == object ||
        NULL == fieldName)
    {
        LOGE("getJIntField invalid parameters");
        return false;
    }

    jclass clazz = env->GetObjectClass( object );
    if (NULL == clazz)
    {
        LOGE("GetObjectClass failed");
        return false;
    }

    jfieldID fieldID = env->GetFieldID( clazz, fieldName, "I" );
    if (0 == fieldID)
    {
        LOGE("GetFieldID failed [%s]", fieldName);
        env->DeleteLocalRef( clazz );
        return false;
    }

    value = env->GetIntField( object, fieldID );

    env->DeleteLocalRef(clazz);

    return true;
}

bool JGetter::getJObjectField(JNIEnv *env, jobject &object, const char *fieldName,
                              const char *fieldType, jobject &value)
{
    if (NULL == env ||
        NULL == object ||
        NULL == fieldName)
    {
        return false;
    }

    jclass clazz = env->GetObjectClass( object );
    if (NULL == clazz)
    {
        return false;
    }

    jfieldID fieldID = env->GetFieldID( clazz, fieldName, fieldType );
    if (0 == fieldID)
    {
        LOGE("GetFieldID failed [%s][%s]", fieldName, fieldType);
        return false;
    }

    value = env->GetObjectField( object, fieldID );
    if (NULL == value)
    {
        return false;
    }

    env->DeleteLocalRef(clazz);

    return true;
}
