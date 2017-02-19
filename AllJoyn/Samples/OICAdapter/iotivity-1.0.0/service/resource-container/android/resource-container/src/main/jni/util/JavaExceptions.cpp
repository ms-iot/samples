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

#include "JavaExceptions.h"

#include "JNIEnvWrapper.h"
#include "Verify.h"

#include "RCSException.h"

namespace
{
    jclass g_cls_PlatformException;

    jmethodID g_ctor_PlatformException;
}

void initJavaExceptions(JNIEnvWrapper *env)
{
    g_cls_PlatformException = env->FindClassAsGlobalRef(EXC_NAME_PLATFORM);
    g_ctor_PlatformException = env->GetConstructorID(g_cls_PlatformException,
                               "(" AS_SIG(CLS_NAME_STRING) "I)V");
}

void clearJavaExceptions(JNIEnvWrapper *env)
{
    env->DeleteGlobalRef(g_cls_PlatformException);
}

void throwPlatformException(JNIEnv *env, const OIC::Service::RCSPlatformException &e)
{
    auto msg = newStringObject(env, e.getReason());
    VERIFY_NO_EXC(env);

    auto exObj = env->NewObject(g_cls_PlatformException, g_ctor_PlatformException,
                                msg, e.getReasonCode());
    VERIFY_NO_EXC(env);

    env->Throw(static_cast< jthrowable >(exObj));
}
