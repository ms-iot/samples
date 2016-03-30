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
  * This file contains the JGetter class  declarations and its functions required
  * for getting and setting basic data types in C++ and Java.
  */

#ifndef JNI_GETTER_H_
#define JNI_GETTER_H_

#include <string>
#include <jni.h>

/**
 * This class provide utility for get/set basic data types in C++ and Java.
 */
class JGetter
{
    public:
        /**
         * This function is called to get String field from the C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject from which string field is expected
         * @param fieldName
         *           Name of the field to be extracted from JObject
         * @param value
         *         reference to string value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool getJStringField(JNIEnv *env, jobject &object, const char *fieldName,
                                    std::string &value);

        /**
         * This function is called to get Boolean field from the C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject from which boolean field is expected
         * @param fieldName
         *           Name of the field to be extracted from JObject
         * @param value
         *         reference to boolean value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool getJBoolField(JNIEnv *env, jobject &object, const char *fieldName,
                                  bool &value);

        /**
         * This function is called to get Integer field from the C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject from which integer field is expected
         * @param fieldName
         *           Name of the field to be extracted from JObject
         * @param value
         *         reference to integer value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool getJIntField(JNIEnv *env, jobject &object, const char *fieldName, int &value);

        /**
         * This function is called to get Object reference from the C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject from which Object reference is expected
         * @param fieldName
         *           Name of the field to be extracted from JObject
         * @param fieldType
         *           Type of the field to be extracted from JObject
         * @param value
         *         reference to Object reference mentioned in fieldName
         *
         * @return returns true on success and false on failure
         */
        static bool getJObjectField(JNIEnv *env, jobject &object, const char *fieldName,
                                    const char *fieldType, jobject &value);
};
#endif //JNI_GETTER_H_
