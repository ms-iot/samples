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
#include "simulator_jni_utils.h"
#include "simulator_common_jni.h"

extern SimulatorClassRefs gSimulatorClassRefs;

void throwSimulatorException(JNIEnv *env, SimulatorResult errCode, const char *errMessage)
{
    jobject ex = env->NewObject(gSimulatorClassRefs.classSimulatorException,
                                gSimulatorClassRefs.classSimulatorExceptionCtor, errCode,
                                env->NewStringUTF(errMessage));
    if (!ex)
    {
        return;
    }
    env->Throw((jthrowable)ex);
}

void throwInvalidArgsException(JNIEnv *env, SimulatorResult errCode, const char *errMessage)
{
    jobject ex = env->NewObject(gSimulatorClassRefs.classInvalidArgsException,
                                gSimulatorClassRefs.classInvalidArgsExceptionCtor, errCode,
                                env->NewStringUTF(errMessage));
    if (!ex)
    {
        return;
    }
    env->Throw((jthrowable)ex);
}

void throwNoSupportException(JNIEnv *env, SimulatorResult errCode, const char *errMessage)
{
    jobject ex = env->NewObject(gSimulatorClassRefs.classNoSupportException,
                                gSimulatorClassRefs.classNoSupportExceptionCtor, errCode,
                                env->NewStringUTF(errMessage));
    if (!ex)
    {
        return;
    }
    env->Throw((jthrowable)ex);
}

void throwOperationInProgressException(JNIEnv *env, SimulatorResult errCode, const char *errMessage)
{
    jobject ex = env->NewObject(gSimulatorClassRefs.classOperationInProgressException,
                                gSimulatorClassRefs.classOperationInProgressExceptionCtor, errCode,
                                env->NewStringUTF(errMessage));
    if (!ex)
    {
        return;
    }
    env->Throw((jthrowable)ex);
}
