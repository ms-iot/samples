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

#ifndef JNI_RCS_VALUE_H_
#define JNI_RCS_VALUE_H_

#include <jni.h>

#include "RCSResourceAttributes.h"

class JNIEnvWrapper;

void initRCSValue(JNIEnvWrapper*);
void clearRCSValue(JNIEnvWrapper*);

OIC::Service::RCSResourceAttributes::Value toNativeAttrsValue(JNIEnv*, jobject);
OIC::Service::RCSResourceAttributes::Value toNativeAttrsValue(JNIEnvWrapper*, jobject);

jobject newRCSValueObject(JNIEnv*, const OIC::Service::RCSResourceAttributes::Value&);
jobject newRCSValueObject(JNIEnvWrapper*, const OIC::Service::RCSResourceAttributes::Value&);

#endif // JNI_RCS_VALUE_H_
