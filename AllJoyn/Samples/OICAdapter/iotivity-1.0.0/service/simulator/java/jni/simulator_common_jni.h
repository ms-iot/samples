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

#ifndef SIMULATOR_COMMON_JNI_H_
#define SIMULATOR_COMMON_JNI_H_

#include <jni.h>

typedef struct
{
    jclass classObject;
    jclass classInteger;
    jclass classDouble;
    jclass classString;
    jclass classHashMap;
    jclass classVector;
    jclass classMap;
    jclass classMapEntry;
    jclass classSet;
    jclass classIterator;
    jclass classLinkedList;

    jclass classSimulatorResource;
    jclass classSimulatorResourceModel;
    jclass classResourceAttribute;
    jclass classSimulatorRemoteResource;
    jclass classSimulatorCallback;
    jclass classObserverInfo;
    jclass classDeviceInfo;
    jclass classPlatformInfo;
    jclass classSimulatorException;
    jclass classInvalidArgsException;
    jclass classNoSupportException;
    jclass classOperationInProgressException;

    jmethodID classIntegerCtor;
    jmethodID classDoubleCtor;
    jmethodID classHashMapCtor;
    jmethodID classHashMapPut;
    jmethodID classVectorCtor;
    jmethodID classVectorAddElement;
    jmethodID classMapEntrySet;
    jmethodID classMapGetKey;
    jmethodID classMapGetValue;
    jmethodID classIteratorId;
    jmethodID classHasNextId;
    jmethodID classNextId;
    jmethodID classLinkedListCtor;
    jmethodID classLinkedListAddObject;

    jmethodID classSimulatorResourceCtor;
    jmethodID classSimulatorResourceModelCtor;
    jmethodID classResourceAttributeCtor;
    jmethodID classResourceAttributeSetRange;
    jmethodID classSimulatorResourceModelId;
    jmethodID classObserverInfoCtor;
    jmethodID classSimulatorExceptionCtor;
    jmethodID classInvalidArgsExceptionCtor;
    jmethodID classNoSupportExceptionCtor;
    jmethodID classOperationInProgressExceptionCtor;

} SimulatorClassRefs;

static jfieldID GetHandleField(JNIEnv *env, jobject jobj)
{
    jclass cls = env->GetObjectClass(jobj);
    return env->GetFieldID(cls, "nativeHandle", "J");
}

template <typename T>
static T *GetHandle(JNIEnv *env, jobject jobj)
{
    jlong handle = env->GetLongField(jobj, GetHandleField(env, jobj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
static void SetHandle(JNIEnv *env, jobject jobj, T *type)
{
    jlong handle = reinterpret_cast<jlong>(type);

    env->SetLongField(jobj, GetHandleField(env, jobj), handle);
}

JNIEnv *getEnv();
void releaseEnv();

#endif
