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

#include "resource_attributes_jni.h"
#include "simulator_resource_model.h"
#include "simulator_common_jni.h"
#include "simulator_error_codes.h"
#include <climits>

extern SimulatorClassRefs gSimulatorClassRefs;

class attribute_value_visitor : public boost::static_visitor<jobject>
{
    public:
        attribute_value_visitor(JNIEnv *env) : m_Env(env) {}

        // Integer type value conversion
        jobject operator ()(const int &value) const
        {
            jobject result = m_Env->NewObject(gSimulatorClassRefs.classInteger,
                                              gSimulatorClassRefs.classIntegerCtor, value);
            return result;
        }

        // Double type value conversion
        jobject operator ()(const double &value) const
        {
            jobject result = m_Env->NewObject(gSimulatorClassRefs.classDouble,
                                              gSimulatorClassRefs.classDoubleCtor, value);
            return result;
        }

        // String type value conversion
        jobject operator ()(const std::string &value) const
        {
            jstring str = m_Env->NewStringUTF(value.c_str());
            return static_cast<jobject>(str);
        }

#if 0
        // Boolean type value conversion
        jobject operator ()(const bool &value) const
        {
            jobject result = m_Env->NewObject(gSimulatorClassRefs.classBoolean,
                                              gSimulatorClassRefs.classBooleanCtor, value);
            return result;
        }

        // SimulatorResourceModel::Attribute type value conversion
        jobject operator ()(const SimulatorResourceModel::Attribute &value) const
        {
            JResourceAttributeConverter converter(value);
            return converter.toJava();
        }
#endif

    private:
        JNIEnv *m_Env;
};

jobject JResourceAttributeConverter::toJava(JNIEnv *env)
{
    // Create an object of ResourceAttribute java class
    jobject jattributeObj = (jobject) env->NewObject(gSimulatorClassRefs.classResourceAttribute,
                            gSimulatorClassRefs.classResourceAttributeCtor);
    if (env->ExceptionCheck() || !jattributeObj)
    {
        return nullptr;
    }

    // Set attribute name
    if (!setName(env, jattributeObj))
    {
        return nullptr;
    }

    // Set types
    if (!setType(env, jattributeObj))
    {
        return nullptr;
    }

    // Set value
    if (!setValue(env, jattributeObj))
    {
        return nullptr;
    }

    // Set Range
    if (!setRange(env, jattributeObj))
    {
        return nullptr;
    }

    // Set Allowed values
    if (!setAllowedValues(env, jattributeObj))
    {
        return nullptr;
    }

    return jattributeObj;
}

bool JResourceAttributeConverter::setName(JNIEnv *env, jobject &jattributeObj)
{
    // Get field reference to "ResourceAttribute::m_name"
    static jfieldID fidName = env->GetFieldID(gSimulatorClassRefs.classResourceAttribute, "m_name",
                              "Ljava/lang/String;");
    if (!fidName)
    {
        return false;
    }

    // Set the attribute name
    std::string name = m_attribute.getName();
    jstring jname = env->NewStringUTF(name.c_str());
    env->SetObjectField(jattributeObj, fidName, jname);
    if (env->ExceptionCheck())
    {
        return false;
    }

    return true;
}

bool JResourceAttributeConverter::setType(JNIEnv *env, jobject &jattributeObj)
{
    // Get class refs to ResourceAttribute::Type class
    static jclass clsType = env->FindClass("org/oic/simulator/ResourceAttribute$Type");
    if (!clsType)
    {
        return false;
    }

    // Get method ref to static method to ResourceAttribute::Type::getType
    static jmethodID midGetType = env->GetStaticMethodID(clsType, "getType",
                                  "(I)Lorg/oic/simulator/ResourceAttribute$Type;");
    if (!midGetType)
    {
        return false;
    }

    // Get field reference to "ResourceAttribute::m_type"
    static jfieldID fidType = env->GetFieldID(gSimulatorClassRefs.classResourceAttribute,
                              "m_type", "Lorg/oic/simulator/ResourceAttribute$Type;");
    if (!fidType)
    {
        return false;
    }

    int type = static_cast<int>(m_attribute.getValueType());
    jobject jtype = env->CallStaticObjectMethod(clsType, midGetType, type);
    if (env->ExceptionCheck())
    {
        return false;
    }

    env->SetObjectField(jattributeObj, fidType, jtype);
    if (env->ExceptionCheck())
    {
        return false;
    }

    return true;
}

bool JResourceAttributeConverter::setValue(JNIEnv *env, jobject &jattributeObj)
{
    // Get field reference to "ResourceAttribute::m_value"
    static jfieldID fidValue = env->GetFieldID(gSimulatorClassRefs.classResourceAttribute,
                               "m_value", "Ljava/lang/Object;");
    if (!fidValue)
    {
        return false;
    }
    jobject jvalue = boost::apply_visitor(attribute_value_visitor(env), m_attribute.getValue());
    env->SetObjectField(jattributeObj, fidValue, jvalue);
    return true;
}

bool JResourceAttributeConverter::setRange(JNIEnv *env, jobject &jattributeObj)
{
    int min = INT_MIN;
    int max = INT_MAX;
    m_attribute.getRange(min, max);
    if (INT_MIN == min || INT_MAX == max)
    {
        return true;
    }
    env->CallVoidMethod(jattributeObj, gSimulatorClassRefs.classResourceAttributeSetRange, min, max);
    if (env->ExceptionCheck())
    {
        return false;
    }

    return true;
}

bool JResourceAttributeConverter::setAllowedValues(JNIEnv *env, jobject &jattributeObj)
{
    // Get field reference to "ResourceAttribute::m_AllowedValues"
    static jfieldID fidAllowedValues = env->GetFieldID(gSimulatorClassRefs.classResourceAttribute,
                                       "m_AllowedValues", "Ljava/lang/Object;");
    if (!fidAllowedValues)
    {
        return false;
    }

    jobjectArray jallowedValues = env->NewObjectArray(m_attribute.getAllowedValuesSize(),
                                  gSimulatorClassRefs.classObject, NULL);
    if (!jallowedValues)
    {
        return false;
    }

    int index = 0;
    for (auto & value : m_attribute.getAllowedValues())
    {
        jobject jvalue = boost::apply_visitor(attribute_value_visitor(env), value);
        env->SetObjectArrayElement(jallowedValues, index++, jvalue);
    }

    env->SetObjectField(jattributeObj, fidAllowedValues, jallowedValues);
    if (env->ExceptionCheck())
    {
        return false;
    }

    return true;
}