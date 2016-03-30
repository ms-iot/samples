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

#include "simulator_device_info_jni.h"
#include "simulator_common_jni.h"

extern SimulatorClassRefs gSimulatorClassRefs;
jobject JDeviceInfo::toJava(DeviceInfo &deviceInfo)
{
    if (!m_env)
        return nullptr;

    jmethodID constr = m_env->GetMethodID(gSimulatorClassRefs.classDeviceInfo, "<init>", "(V)V");
    if (constr)
        return nullptr;

    jobject jDeviceInfo = (jobject) m_env->NewObject(gSimulatorClassRefs.classDeviceInfo, constr);
    if (jDeviceInfo)
        return nullptr;

    setFieldValue(jDeviceInfo, "mName", deviceInfo.getName());
    setFieldValue(jDeviceInfo, "mID", deviceInfo.getID());
    setFieldValue(jDeviceInfo, "mSpecVersion", deviceInfo.getSpecVersion());
    setFieldValue(jDeviceInfo, "mDMVVersion", deviceInfo.getDataModelVersion());

    return jDeviceInfo;
}

void JDeviceInfo::setFieldValue(jobject jDeviceInfo, const std::string &fieldName,
                                const std::string &value)
{
    jfieldID fieldID = m_env->GetFieldID(gSimulatorClassRefs.classDeviceInfo, fieldName.c_str(),
                                         "Ljava/lang/String;");
    jstring valueStr = m_env->NewStringUTF(value.c_str());
    m_env->SetObjectField(jDeviceInfo, fieldID, valueStr);
}

void JniDeviceInfoListener::onDeviceInfoReceived(DeviceInfo &deviceInfo)
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

    jmethodID listenerMId = env->GetMethodID(listenerCls, "onDeviceFound",
                            "(Lorg/oic/simulator/DeviceInfo;)V");
    if (!listenerMId)
    {
        releaseEnv();
        return;
    }

    // Convert CPP to Java DeviceInfo object
    jobject jDeviceInfo = JDeviceInfo(env).toJava(deviceInfo);
    if (!jDeviceInfo)
    {
        releaseEnv();
        return;
    }

    // Invoke java listener with DeviceInfo
    env->CallVoidMethod(listener, listenerMId, jDeviceInfo);
    if (env->ExceptionCheck())
    {
        releaseEnv();
        return;
    }

    releaseEnv();
}

