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
  * This file contains the declaration of JniAction class and
  * its members related to JniAction.
  */

#ifndef JNI_ACTION_H_
#define JNI_ACTION_H_

#include <vector>
#include <string>

#include "jni_object.h"
#include "jni_capability.h"

/**
 * This class provides a set of functions to get and
 * set Action Class member variables.
 */
class JniAction : public JObject
{
    public:
        /**
         * constructor
         */
        JniAction(JNIEnv *env, jobject obj);

        /**
         * constructor
         */
        JniAction(JNIEnv *env);

        /**
         * destructor
         */
        ~JniAction();

        /**
         * Retrieves target value from JniAction class object.
         *
         * @param target - target value
         *
         * @return Boolean, true on success, otherwise false
         */
        bool getTarget(std::string &target);

        /**
         * Sets target value of JniAction class object.
         *
         * @param target - target value
         *
         * @return Boolean, true on success, otherwise false
         */
        bool setTarget(const std::string target);

        /**
         * Retrieves capability values from JniAction class object.
         *
         * @param capabilityList - capability list
         *
         * @return Boolean, true on success, otherwise false
         */
        bool getJniCapabilityValues(std::vector<OIC::Capability *> &capabilityList);

};
#endif //JNI_ACTION_H_
