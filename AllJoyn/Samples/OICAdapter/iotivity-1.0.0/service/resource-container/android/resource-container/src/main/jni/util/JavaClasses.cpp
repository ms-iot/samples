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

#include "JavaClasses.h"

#include "JNIEnvWrapper.h"

jclass g_cls_String;
jclass g_cls_Integer;
jclass g_cls_Double;
jclass g_cls_Boolean;

jclass g_cls_ArrayList;
jclass g_cls_Set;
jclass g_cls_Map;
jclass g_cls_MapEntry;
jclass g_cls_Iterator;

jmethodID g_method_Boolean_booleanValue;
jmethodID g_method_Integer_intValue;
jmethodID g_method_Double_doubleValue;

jmethodID g_method_Collection_add;

jmethodID g_method_Set_iterator;

jmethodID g_method_Map_entrySet;
jmethodID g_method_Map_put;

jmethodID g_method_MapEntry_getKey;
jmethodID g_method_MapEntry_getValue;

jmethodID g_method_Iterator_hasNext;
jmethodID g_method_Iterator_next;

jmethodID g_ctor_Boolean;
jmethodID g_ctor_Integer;
jmethodID g_ctor_Double;

jmethodID g_ctor_ArrayList;

namespace
{
    inline void initPrimitiveTypes(JNIEnvWrapper *env)
    {
        g_cls_Boolean = env->FindClassAsGlobalRef(CLS_NAME_BOOLEAN);
        g_ctor_Boolean = env->GetConstructorID(g_cls_Boolean, "(Z)V");
        g_method_Boolean_booleanValue = env->GetMethodID(g_cls_Boolean, "booleanValue", "()Z");

        g_cls_Integer = env->FindClassAsGlobalRef(CLS_NAME_INTEGER);
        g_ctor_Integer = env->GetConstructorID(g_cls_Integer, "(I)V");
        g_method_Integer_intValue = env->GetMethodID(g_cls_Integer, "intValue", "()I");

        g_cls_Double = env->FindClassAsGlobalRef(CLS_NAME_DOUBLE);
        g_ctor_Double = env->GetConstructorID(g_cls_Double, "(D)V");
        g_method_Double_doubleValue = env->GetMethodID(g_cls_Double, "doubleValue", "()D");

        g_cls_String = env->FindClassAsGlobalRef(CLS_NAME_STRING);
    }
}

void initJavaClasses(JNIEnvWrapper *env)
{
    initPrimitiveTypes(env);

    auto clsCollection = env->FindClass(CLS_NAME_COLLECTION);
    g_method_Collection_add = env->GetMethodID(clsCollection, "add",
                              "(" AS_SIG(CLS_NAME_OBJECT) ")Z");

    g_cls_ArrayList = env->FindClassAsGlobalRef(CLS_NAME_ARRAY_LIST);
    g_ctor_ArrayList = env->GetConstructorID(g_cls_ArrayList, "()V");

    g_cls_Set = env->FindClassAsGlobalRef(CLS_NAME_SET);
    g_method_Set_iterator = env->GetMethodID(g_cls_Set, "iterator", "()" AS_SIG(CLS_NAME_ITERATOR));

    g_cls_Map = env->FindClassAsGlobalRef(CLS_NAME_MAP);
    g_method_Map_entrySet = env->GetMethodID(g_cls_Map, "entrySet", "()" AS_SIG(CLS_NAME_SET));
    g_method_Map_put = env->GetMethodID(g_cls_Map, "put",
                                        "(" AS_SIG(CLS_NAME_OBJECT) AS_SIG(CLS_NAME_OBJECT) ")" AS_SIG(CLS_NAME_OBJECT));

    g_cls_MapEntry = env->FindClassAsGlobalRef(CLS_NAME_MAP_ENTRY);
    g_method_MapEntry_getKey = env->GetMethodID(g_cls_MapEntry, "getKey",
                               "()" AS_SIG(CLS_NAME_OBJECT));
    g_method_MapEntry_getValue = env->GetMethodID(g_cls_MapEntry, "getValue",
                                 "()" AS_SIG(CLS_NAME_OBJECT));

    g_cls_Iterator = env->FindClassAsGlobalRef(CLS_NAME_ITERATOR);
    g_method_Iterator_hasNext = env->GetMethodID(g_cls_Iterator, "hasNext", "()Z");
    g_method_Iterator_next = env->GetMethodID(g_cls_Iterator, "next", "()" AS_SIG(CLS_NAME_OBJECT));
}

void clearJavaClasses(JNIEnvWrapper *env)
{
    env->DeleteGlobalRef(g_cls_Boolean);
    env->DeleteGlobalRef(g_cls_Integer);
    env->DeleteGlobalRef(g_cls_Double);
    env->DeleteGlobalRef(g_cls_String);
    env->DeleteGlobalRef(g_cls_Set);
    env->DeleteGlobalRef(g_cls_Map);
    env->DeleteGlobalRef(g_cls_MapEntry);
    env->DeleteGlobalRef(g_cls_Iterator);
}
