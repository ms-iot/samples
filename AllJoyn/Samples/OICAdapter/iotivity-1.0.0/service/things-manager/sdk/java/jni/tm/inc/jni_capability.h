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

/**
  * @file
  * This file contains the utility functions for conversions from java to CPP
  * and viceversa.
  */

#ifndef JNI_CAPABILITY_H_
#define JNI_CAPABILITY_H_

#include <string>

#include <ActionSet.h>
#include "jni_object.h"

/**
 * This class provides a set of functions to get and
 * set Capability Class member variables.
 */
class JniCapability : public JObject
{
    public:
        /**
         * constructor
         */
        JniCapability(JNIEnv *env, jobject obj);

        /**
         * constructor
         */
        JniCapability(JNIEnv *env);

        /**
         * destructor
         */
        ~JniCapability();

        /**
         * Retrieves Capability value from JniCapability class object.
         *
         * @param   capability
         *              [OUT] capability value
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool getJniCapabilityValue(std::string &capability);

        /**
         * Sets Capability value of JniCapability class object.
         *
         * @param   capability
         *              [IN] capability value
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool setJniCapabilityValue(const std::string capability);

        /**
         * Retrieves status of JniCapability class object.
         *
         * @param   status
         *              [OUT] status
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool getJniCapabilityStatus(std::string &status);

        /**
         * Sets status of JniCapability class object
         *
         * @param   status
         *              [IN] status
         *
         * @return  Boolean, true on success, otherwise false.
         */
        bool setJniCapabilityStatus(const std::string status);
};
#endif  //JNI_CAPABILITY_H_
