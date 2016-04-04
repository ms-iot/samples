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
  * This file contains the declaration of JniActionSet class
  * and its members related to JniActionSet.
  */

#ifndef JNI_ACTIONSET_H_
#define JNI_ACTIONSET_H_

#include <vector>
#include <string>

#include "GroupManager.h"
#include "ActionSet.h"
#include "jni_object.h"
#include "jni_action.h"

using namespace OC;
using namespace OIC;

/**
 * This class provides a set of functions to get and
 * set ActionSet Class member variables.
 */
class JniActionSet : public JObject
{
    public:
        /**
         * constructor
         */
        JniActionSet(JNIEnv *env, jobject obj);

        /**
         * constructor
         */
        JniActionSet(JNIEnv *env);

        /**
         * destructor
         */
        ~JniActionSet();

        /**
         * Retrieves target value from JniActionSet class object.
         *
         * @param name - ActionSet Name
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool getJniActionSetName(std::string &name);

        /**
         * Sets target value of JniActionSet class object
         *
         * @param name - ActionSet Name
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool setJniActionSetName(const std::string name);

        /**
         * Retrieves capability values from JniActionSet class object.
         *
         * @param actionList - List of Actions
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool getJniListOfActions(std::vector<OIC::Action *> &actionList);

        /**
         * Converts actionSet class from java to CPP.
         *
         * @param env           - Default JNI Environment Pointer
         * @param jnewActionSet - action set
         *
         * @return  OIC CPP ActionSet
         */
        ActionSet *getActionSet(JNIEnv *env, jobject jnewActionSet);

        /**
         * Converts Time class from java to CPP.
         *
         * @param env           - Default JNI Environment Pointer
         * @param jnewActionSet - Java action set
         * @param pActionSet    - CPP action set
         *
         * @return  Boolean, true on success, otherwise false
         */
        bool setTimeInfo(JNIEnv *env, jobject jnewActionSet, OIC::ActionSet *pActionSet);
};
#endif  //JNI_ACTIONSET_H_

