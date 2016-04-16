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

#include "simulator_platform_info_jni.h"
#include "simulator_common_jni.h"

extern SimulatorClassRefs gSimulatorClassRefs;
jobject JPlatformInfo::toJava(PlatformInfo &platformInfo)
{
    if (!m_env)
        return nullptr;

    jmethodID constr = m_env->GetMethodID(gSimulatorClassRefs.classPlatformInfo, "<init>", "(V)V");
    if (constr)
        return nullptr;

    jobject jPlatformInfo = (jobject) m_env->NewObject(gSimulatorClassRefs.classPlatformInfo, constr);
    if (jPlatformInfo)
        return nullptr;

    setFieldValue(jPlatformInfo, "mPlatformId", platformInfo.getPlatformID());
    setFieldValue(jPlatformInfo, "m_manufacturerName", platformInfo.getManufacturerName());
    setFieldValue(jPlatformInfo, "m_manufacturerUrl", platformInfo.getManufacturerUrl());
    setFieldValue(jPlatformInfo, "m_modelNumber", platformInfo.getModelNumber());
    setFieldValue(jPlatformInfo, "m_dateOfManufacture", platformInfo.getDateOfManfacture());
    setFieldValue(jPlatformInfo, "m_platformVersion", platformInfo.getPlatformVersion());
    setFieldValue(jPlatformInfo, "m_operationSystemVersion", platformInfo.getOSVersion());
    setFieldValue(jPlatformInfo, "m_hardwareVersion", platformInfo.getHardwareVersion());
    setFieldValue(jPlatformInfo, "m_firmwareVersion", platformInfo.getFirmwareVersion());
    setFieldValue(jPlatformInfo, "m_supportUrl", platformInfo.getSupportUrl());
    setFieldValue(jPlatformInfo, "m_systemTime", platformInfo.getSystemTime());

    return jPlatformInfo;
}

PlatformInfo JPlatformInfo::toCPP(jobject jPlatformInfo)
{
    PlatformInfo platformInfo;
    if (!m_env || !jPlatformInfo)
        return platformInfo;

    platformInfo.setPlatformID(getFieldValue(jPlatformInfo, "mPlatformId"));
    platformInfo.setManufacturerName(getFieldValue(jPlatformInfo, "m_manufacturerName"));
    platformInfo.setManufacturerUrl(getFieldValue(jPlatformInfo, "m_manufacturerUrl"));
    platformInfo.setModelNumber(getFieldValue(jPlatformInfo, "m_modelNumber"));
    platformInfo.setDateOfManfacture(getFieldValue(jPlatformInfo, "m_dateOfManufacture"));
    platformInfo.setPlatformVersion(getFieldValue(jPlatformInfo, "m_platformVersion"));
    platformInfo.setOSVersion(getFieldValue(jPlatformInfo, "m_operationSystemVersion"));
    platformInfo.setHardwareVersion(getFieldValue(jPlatformInfo, "m_hardwareVersion"));
    platformInfo.setFirmwareVersion(getFieldValue(jPlatformInfo, "m_firmwareVersion"));
    platformInfo.setSupportUrl(getFieldValue(jPlatformInfo, "m_supportUrl"));
    platformInfo.setSystemTime(getFieldValue(jPlatformInfo, "m_systemTime"));

    return std::move(platformInfo);
}

void JPlatformInfo::setFieldValue(jobject jPlatformInfo, const std::string &fieldName,
                                  const std::string &value)
{
    jfieldID fieldID = m_env->GetFieldID(gSimulatorClassRefs.classPlatformInfo, fieldName.c_str(),
                                         "Ljava/lang/String;");
    jstring valueStr = m_env->NewStringUTF(value.c_str());
    m_env->SetObjectField(jPlatformInfo, fieldID, valueStr);
}

std::string JPlatformInfo::getFieldValue(jobject jPlatformInfo, const std::string &fieldName)
{
    jfieldID fieldID = m_env->GetFieldID(gSimulatorClassRefs.classPlatformInfo, fieldName.c_str(),
                                         "Ljava/lang/String;");
    jstring jvalue = (jstring) m_env->GetObjectField(jPlatformInfo, fieldID);
    const char *valueCStr = m_env->GetStringUTFChars(jvalue, NULL);
    if (valueCStr)
        return std::string(valueCStr);
    return std::string();
}

void JniPlatformInfoListener::onPlatformInfoReceived(PlatformInfo &platformInfo)
{
    // Get the environment
    JNIEnv *env = getEnv();
    if (!env)
        return;

    jobject listener = env->NewLocalRef(m_listener);
    if (!listener)
    {
        releaseEnv();
        return;
    }

    jclass listenerCls = env->GetObjectClass(listener);
    if (!listenerCls)
    {
        releaseEnv();
        return;
    }

    jmethodID listenerMId = env->GetMethodID(listenerCls, "onPlatformFound",
                            "(Lorg/oic/simulator/PlatformInfo;)V");
    if (!listenerMId)
    {
        releaseEnv();
        return;
    }

    // Convert CPP to Java DeviceInfo object
    jobject jPlatformInfo = JPlatformInfo(env).toJava(platformInfo);
    if (!jPlatformInfo)
    {
        releaseEnv();
        return;
    }

    // Invoke java listener with DeviceInfo
    env->CallVoidMethod(listener, listenerMId, jPlatformInfo);
    if (env->ExceptionCheck())
    {
        releaseEnv();
        return;
    }

    releaseEnv();
}

