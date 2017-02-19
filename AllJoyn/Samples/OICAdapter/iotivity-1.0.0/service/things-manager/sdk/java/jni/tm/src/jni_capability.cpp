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

#include "jni_capability.h"
#include "jni_things_manager_jvm.h"
#include "jni_getter.h"
#include "jni_setter.h"

#define JCAPABILITY_STATUS         ("status")
#define JCAPABILITY_CAPABILITY  ("capability")

JniCapability::JniCapability(JNIEnv *env, jobject obj) : JObject(env, obj)
{
}

JniCapability::JniCapability(JNIEnv *env) : JObject(env, TM_SERVICE_CAPABILITY_PATH)
{
}

JniCapability::~JniCapability()
{
}

bool JniCapability::getJniCapabilityValue(std::string &capability)
{
    //Retrieves capability value from JniCapability class object
    return JGetter::getJStringField(m_pEnv, m_pObject, JCAPABILITY_CAPABILITY, capability);
}

bool JniCapability::setJniCapabilityValue(const std::string capability)
{
    //Sets capability value of JniCapability class object
    return JSetter::setJStringField(m_pEnv, m_pObject, JCAPABILITY_CAPABILITY, capability.c_str());
}

bool JniCapability::getJniCapabilityStatus(std::string &status)
{
    //Retrieves status from JniCapability class object
    return JGetter::getJStringField(m_pEnv, m_pObject, JCAPABILITY_STATUS, status);
}

bool JniCapability::setJniCapabilityStatus(const std::string status)
{
    //Sets status of JniCapability class object
    return JSetter::setJStringField(m_pEnv, m_pObject, JCAPABILITY_STATUS, status.c_str());
}
