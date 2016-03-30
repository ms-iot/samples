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

#ifndef RESOURCE_ATTRIBUTE_JNI_H_
#define RESOURCE_ATTRIBUTE_JNI_H_

#include <jni.h>
#include "simulator_resource_model.h"

class JResourceAttributeConverter
{
    public:
        JResourceAttributeConverter(SimulatorResourceModel::Attribute &attribute) {m_attribute = attribute;}
        JResourceAttributeConverter(const JResourceAttributeConverter &) = delete;
        JResourceAttributeConverter &operator=(const JResourceAttributeConverter &) = delete;
        JResourceAttributeConverter(const JResourceAttributeConverter &&) = delete;
        JResourceAttributeConverter &operator=(const JResourceAttributeConverter && ) = delete;
        jobject toJava(JNIEnv *env);

    private:
        bool setName(JNIEnv *env, jobject &jaAttributeObj);
        bool setType(JNIEnv *env, jobject &jaAttributeObj);
        bool setValue(JNIEnv *env, jobject &jaAttributeObj);
        bool setRange(JNIEnv *env, jobject &jaAttributeObj);
        bool setAllowedValues(JNIEnv *env, jobject &jaAttributeObj);

        SimulatorResourceModel::Attribute m_attribute;
};

#endif //SIMULATOR_RESOURCE_ATTRIBUTE_JNI_H_
