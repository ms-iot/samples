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

#ifndef JAVA_CLASSES_H_
#define JAVA_CLASSES_H_

#include <jni.h>

#include <string>

#define PACKAGE_NAME "org/iotivity/service"

#define CLS_NAME_OBJECT "java/lang/Object"
#define CLS_NAME_STRING "java/lang/String"
#define CLS_NAME_INTEGER "java/lang/Integer"
#define CLS_NAME_DOUBLE "java/lang/Double"
#define CLS_NAME_BOOLEAN "java/lang/Boolean"

#define CLS_NAME_COLLECTION "java/util/Collection"
#define CLS_NAME_ARRAY_LIST "java/util/ArrayList"
#define CLS_NAME_SET "java/util/Set"
#define CLS_NAME_MAP "java/util/Map"
#define CLS_NAME_MAP_ENTRY "java/util/Map$Entry"
#define CLS_NAME_ITERATOR "java/util/Iterator"

#define EXC_NAME_RCS PACKAGE_NAME "/utils/RcsException"
#define EXC_NAME_PLATFORM PACKAGE_NAME "/utils/RcsPlatformException"
#define EXC_NAME_ILLEGAL_STATE PACKAGE_NAME "/utils/RcsIllegalStateException"

#define AS_SIG(CLS_NAME) "L" CLS_NAME ";"

class JNIEnvWrapper;

extern jclass g_cls_Integer;
extern jclass g_cls_Double;
extern jclass g_cls_Boolean;
extern jclass g_cls_String;

extern jclass g_cls_ArrayList;
extern jclass g_cls_Set;
extern jclass g_cls_Map;
extern jclass g_cls_MapEntry;
extern jclass g_cls_Iterator;

extern jmethodID g_method_Boolean_booleanValue;
extern jmethodID g_method_Integer_intValue;
extern jmethodID g_method_Double_doubleValue;

extern jmethodID g_method_Collection_add;

extern jmethodID g_method_Set_iterator;

extern jmethodID g_method_Map_entrySet;
extern jmethodID g_method_Map_put;

extern jmethodID g_method_MapEntry_getKey;
extern jmethodID g_method_MapEntry_getValue;

extern jmethodID g_method_Iterator_hasNext;
extern jmethodID g_method_Iterator_next;

extern jmethodID g_ctor_Boolean;
extern jmethodID g_ctor_Integer;
extern jmethodID g_ctor_Double;

extern jmethodID g_ctor_ArrayList;

void initJavaClasses(JNIEnvWrapper *);
void clearJavaClasses(JNIEnvWrapper *);

template< typename ENV >
inline jobject newBooleanObject(ENV *env, bool value)
{
    return env->NewObject(g_cls_Boolean, g_ctor_Boolean, value);
}

template< typename ENV >
inline jobject newIntegerObject(ENV *env, int value)
{
    return env->NewObject(g_cls_Integer, g_ctor_Integer, value);
}

template< typename ENV >
inline jobject newDoubleObject(ENV *env, double value)
{
    return env->NewObject(g_cls_Double, g_ctor_Double, value);
}

template< typename ENV >
inline jstring newStringObject(ENV *env, const std::string &value)
{
    return env->NewStringUTF(value.c_str());
}

template< typename ENV >
inline jstring newStringObjectCstr(ENV *env, const char *value)
{
    return env->NewStringUTF(value);
}

template< typename ENV >
inline std::string toStdString(ENV *env, jstring obj)
{
    if (!obj) return "";

    auto cstr = env->GetStringUTFChars(obj, nullptr);

    if (!cstr) return "";

    std::string result { cstr };

    env->ReleaseStringUTFChars(obj, cstr);

    return result;
}

template< typename ENV >
inline jobject newArrayList(ENV *env)
{
    return env->NewObject(g_cls_ArrayList, g_ctor_ArrayList);
}

template< typename ENV >
inline bool invoke_Boolean_booleanValue(ENV *env, jobject obj)
{
    return env->CallBooleanMethod(obj, g_method_Boolean_booleanValue);
}

template< typename ENV >
inline int invoke_Integer_intValue(ENV *env, jobject obj)
{
    return env->CallIntMethod(obj, g_method_Integer_intValue);
}

template< typename ENV >
inline double invoke_Double_doubleValue(ENV *env, jobject obj)
{
    return env->CallDoubleMethod(obj, g_method_Double_doubleValue);
}

template< typename ENV >
inline jboolean invoke_Collection_add(ENV *env, jobject collectionObj, jobject valueObj)
{
    return env->CallBooleanMethod(collectionObj, g_method_Collection_add, valueObj);
}

template< typename ENV >
inline jobject invoke_Map_entrySet(ENV *env, jobject mapObj)
{
    return env->CallObjectMethod(mapObj, g_method_Map_entrySet);
}

template< typename ENV >
inline jobject invoke_Map_put(ENV *env, jobject mapObj, jobject keyObj, jobject valueObj)
{
    return env->CallObjectMethod(mapObj, g_method_Map_put, keyObj, valueObj);
}

template< typename ENV >
inline jobject invoke_MapEntry_getKey(ENV *env, jobject entryObj)
{
    return env->CallObjectMethod(entryObj, g_method_MapEntry_getKey);
}

template< typename ENV >
inline jobject invoke_MapEntry_getValue(ENV *env, jobject entryObj)
{
    return env->CallObjectMethod(entryObj, g_method_MapEntry_getValue);
}

template< typename ENV >
inline jobject invoke_Set_iterator(ENV *env, jobject setObj)
{
    return env->CallObjectMethod(setObj, g_method_Set_iterator);
}

template< typename ENV >
inline bool invoke_Iterator_hasNext(ENV *env, jobject iterObj)
{
    return env->CallBooleanMethod(iterObj, g_method_Iterator_hasNext);
}

template< typename ENV >
inline jobject invoke_Iterator_next(ENV *env, jobject iterObj)
{
    return env->CallObjectMethod(iterObj, g_method_Iterator_next);
}

#endif // JAVA_CLASSES_H_
