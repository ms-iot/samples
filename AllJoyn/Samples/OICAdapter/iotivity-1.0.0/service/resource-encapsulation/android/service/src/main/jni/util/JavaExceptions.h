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

#ifndef JAVA_EXCEPTIONS_H_
#define JAVA_EXCEPTIONS_H_

#include <jni.h>

#include "JavaClasses.h"

namespace OIC
{
    namespace Service
    {
        class RCSPlatformException;
    }
}

class JNIEnvWrapper;

void initJavaExceptions(JNIEnvWrapper*);
void clearJavaExceptions(JNIEnvWrapper*);

void throwPlatformException(JNIEnv*, const OIC::Service::RCSPlatformException&);

template < typename ENV >
void throwRCSException(ENV* env, const char* msg)
{
    env->ThrowNew(env->FindClass(EXC_NAME_RCS), msg);
}


#endif // JAVA_EXCEPTIONS_H_
