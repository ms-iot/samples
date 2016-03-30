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

#include "jni_action_set.h"
#include "jni_things_manager_jvm.h"
#include "jni_getter.h"
#include "jni_setter.h"
#include "JniOcResource.h"

#define JACTIONSET_NAME             ("actionsetName")
#define JACTIONSET_LISTOF_ACTION    ("listOfAction")

JniActionSet::JniActionSet(JNIEnv *env, jobject obj) : JObject(env, obj)
{
}

JniActionSet::JniActionSet(JNIEnv *env) : JObject(env, TM_SERVICE_ACTIONSET_PATH)
{
}

JniActionSet::~JniActionSet()
{
}

bool JniActionSet::getJniActionSetName(std::string &name)
{
    //Retrieves target value from JniActionSet class object
    return JGetter::getJStringField(m_pEnv, m_pObject, JACTIONSET_NAME, name);
}

bool JniActionSet::setJniActionSetName(const std::string name)
{
    //Sets target value of JniActionSet class object
    return JSetter::setJStringField(m_pEnv, m_pObject, JACTIONSET_NAME, name.c_str());
}

bool JniActionSet::getJniListOfActions(std::vector<OIC::Action *> &actionList)
{
    jclass jvectorClass = m_pEnv->FindClass(TM_JAVA_VECTOR_CLASS_PATH);
    jmethodID vectorMethodID = m_pEnv->GetMethodID(jvectorClass, "get", "(I)Ljava/lang/Object;");
    jmethodID vectorSizeMethodID = m_pEnv->GetMethodID(jvectorClass, "size", "()I");
    jobject jAttrList = NULL;
    bool res = JGetter::getJObjectField(m_pEnv, m_pObject,
                                        JACTIONSET_LISTOF_ACTION, TM_JAVA_VECTOR_TYPE, jAttrList);

    if (res == true)
    {
        actionList.clear();
        int attrCount = m_pEnv->CallIntMethod(jAttrList, vectorSizeMethodID);
        for (int i = 0; i < attrCount; i++)
        {
            JniAction *attr_var = new JniAction(m_pEnv,
                                                m_pEnv->CallObjectMethod(jAttrList, vectorMethodID, i));
            OIC::Action *actionName = new OIC::Action();
            attr_var->getTarget(actionName->target);
            attr_var->getJniCapabilityValues(actionName->listOfCapability);
            delete attr_var;
            actionList.push_back(actionName);
        }
    }
    m_pEnv->DeleteLocalRef(jvectorClass);
    return true;
}
ActionSet *JniActionSet::getActionSet(JNIEnv *env, jobject jnewActionSet)
{
    LOGI("getActionSet: Enter");

    if (NULL == jnewActionSet)
    {
        LOGE("getActionSet: jnewActionSet is NULL");
        return NULL;
    }

    std::string actionsetName;
    getJniActionSetName(actionsetName);

    std::vector<Action *> actionVector;
    getJniListOfActions(actionVector);

    OIC::ActionSet *pActionSet = new OIC::ActionSet();
    if (NULL == pActionSet)
    {
        LOGE("getActionSet: Failed to create newActionSet");
        return NULL;
    }

    pActionSet->actionsetName = actionsetName;
    pActionSet->listOfAction = actionVector;
    if (false == setTimeInfo(env, jnewActionSet, pActionSet))
    {
        LOGE("getActionSet: setTimeInfo failed!");
        delete pActionSet;
        return NULL;
    }

    LOGI("getActionSet: Exit");
    return pActionSet;
}

bool JniActionSet::setTimeInfo(JNIEnv *env, jobject jnewActionSet, OIC::ActionSet *pActionSet)
{
    LOGI("setTimeInfo: Entry");
    if (NULL == jnewActionSet)
    {
        LOGE("setTimeInfo: jnewActionSet is NULL");
        return NULL;
    }

    jclass classTime = env->FindClass(TM_SERVICE_TIME_PATH);
    if (NULL == classTime)
    {
        LOGE("FindClass failed for [%s]", TM_SERVICE_TIME_PATH);
        return false;
    }

    jfieldID fieldID = env->GetFieldID(classTime, "mYear", "I");
    int year = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mMonth", "I");
    int month = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mDay", "I");
    int day = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mHour", "I");
    int hour = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mMin", "I");
    int min = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mSec", "I");
    int sec = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mDayOfWeek", "I");
    int wday = env->GetIntField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mDelay", "J");
    long delay = env->GetLongField(jnewActionSet, fieldID);

    fieldID = env->GetFieldID(classTime, "mType", "Lorg/iotivity/service/tm/Time$ActionSetType;");
    jobject actionSetTypeObj = env->GetObjectField(jnewActionSet, fieldID);
    if (NULL == actionSetTypeObj)
    {
        LOGE("setTimeInfo: actionSetTypeObj is NULL");
        return false;
    }

    jclass actionSetTypeClass = env->GetObjectClass(actionSetTypeObj);
    if (NULL == actionSetTypeClass)
    {
        LOGE("setTimeInfo: actionSetTypeClass is NULL");
        return false;
    }

    jmethodID actionSetTypeMethod = env->GetMethodID(actionSetTypeClass, "ordinal", "()I");
    if (NULL == actionSetTypeMethod)
    {
        LOGE("setTimeInfo: actionSetTypeMethod is NULL");
        return false;
    }

    int type = (int) env->CallIntMethod(actionSetTypeObj, actionSetTypeMethod);
    pActionSet->setTime(year, month, day, hour, min, sec, wday);
    pActionSet->type = static_cast<OIC::ACTIONSET_TYPE>(type);
    pActionSet->setDelay(delay);

    LOGI("setTimeInfo: Exit");
    return true;
}
