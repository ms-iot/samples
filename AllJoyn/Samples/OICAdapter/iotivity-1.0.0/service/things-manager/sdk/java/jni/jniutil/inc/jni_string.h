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
  * This file contains the declaration of JString class and its members related to JString.
  */

#ifndef JNI_STRING_H_
#define JNI_STRING_H_

#include <string>

#include "jni_object.h"

/**
 * This class inherits JObject class and provides a set of functions for JNI String.
 */
class JString : public JObject
{
    public:
        /**
         * constructor
         */
        JString(JNIEnv *env, jstring value);
        /**
         * constructor
         */
        JString(JNIEnv *env, const char *value);
        /**
         * constructor
         */
        JString(JNIEnv *env, const std::string &value);
        /**
         * destructor
         */
        ~JString();

        /**
         * Function to get the string value and set it.
         *
         * @param value - String value to set to a private member variable.
         *
         * @return bool - true on success
         */
        bool getValue(std::string &value);

        /**
         * Function to get the private string value.
         *
         * @return  C String value.
         */
        const char *c_str();

    private:
        std::string m_cstr;
};
#endif //JNI_STRING_H_
