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

#include "jni_action.h"
#include "jni_things_manager_jvm.h"
#include "jni_getter.h"
#include "jni_setter.h"

#include <ActionSet.h>

#define JACTION_TARGET              ("target")
#define JACTION_LISTOF_CAPABILITY   ("listOfCapability")

JniAction::JniAction(JNIEnv *env, jobject obj) : JObject(env, obj)
{
}

JniAction::JniAction(JNIEnv *env) : JObject(env, TM_SERVICE_ACTION_PATH)
{
}

JniAction::~JniAction()
{
}

bool JniAction::getTarget(std::string &target)
{
    //Retrieves target value from JniAction class object
    return JGetter::getJStringField(m_pEnv, m_pObject, JACTION_TARGET, target);
}

bool JniAction::setTarget(const std::string target)
{
    //Sets target value of JniAction class object
    return JSetter::setJStringField(m_pEnv, m_pObject, JACTION_TARGET, target.c_str());
}

bool JniAction::getJniCapabilityValues(std::vector<OIC::Capability *> &capabilityList)
{
    jclass jvectorClass = m_pEnv->FindClass(TM_JAVA_VECTOR_CLASS_PATH);
    jmethodID vectorMethodID = m_pEnv->GetMethodID(jvectorClass, "get", "(I)Ljava/lang/Object;");
    jmethodID vectorSizeMethodID = m_pEnv->GetMethodID(jvectorClass, "size", "()I");

    jobject jAttrList = NULL;
    bool res = JGetter::getJObjectField(m_pEnv, m_pObject,
                                        JACTION_LISTOF_CAPABILITY, TM_JAVA_VECTOR_TYPE, jAttrList);

    if (res == true)
    {
        capabilityList.clear();
        int attrCount = m_pEnv->CallIntMethod(jAttrList, vectorSizeMethodID);
        for (int i = 0; i < attrCount; i++)
        {
            JniCapability *attr_var = new JniCapability(m_pEnv,
                    m_pEnv->CallObjectMethod(jAttrList, vectorMethodID, i));

            OIC::Capability *cap = new OIC::Capability();
            attr_var->getJniCapabilityValue(cap->capability);
            attr_var->getJniCapabilityStatus(cap->status);
            delete attr_var;
            capabilityList.push_back(cap);
        }
    }
    m_pEnv->DeleteLocalRef(jvectorClass);
    return res;
}
