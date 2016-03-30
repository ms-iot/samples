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
  * This file contains the JSetter class declarations and and its functions required
  * to set data types in C++ object from Java object.
  */

#ifndef JNI_SETTER_H_
#define JNI_SETTER_H_

#include <jni.h>

/**
 * This class provide utility to set data types in C++ object from Java object.
 */
class JSetter
{
    public:

        /**
         * This function is called to set Integer field in to C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject to which integer field will be set.
         * @param fieldName
         *           Name of the field to be set in JObject
         * @param value
         *         integer value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool setJIntField(JNIEnv *env, jobject &object, const char *fieldName, int value);

        /**
         * This function is called to set Long field in to C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject to which Long field will be set.
         * @param fieldName
         *           Name of the field to be set in JObject
         * @param value
         *         Long value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool setJLongField(JNIEnv *env, jobject &object, const char *fieldName,
                                  jlong value);

        /**
         * This function is called to Set Boolean field to C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject to which boolean field has to be set
         * @param fieldName
         *           Name of the field to be set in JObject
         * @param value
         *         boolean value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool setJBoolField(JNIEnv *env, jobject &object, const char *fieldName, bool value);
        /**
         * This function is called to Set String field from the C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject in which string value has to be set
         * @param fieldName
         *           Name of the field to be set in JObject
         * @param value
         *         string value mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool setJStringField(JNIEnv *env, jobject &object, const char *fieldName,
                                    const char *value);

        /**
         * This function is called to set Object reference in C++ object.
         *
         * @param env
         *           JNI Environment reference
         * @param object
         *           JObject to which Object reference is to be set.
         * @param fieldName
         *           Name of the field to be set in JObject
         * @param fieldType
         *           Type of the field to be set in JObject
         * @param value
         *         value of Object mentioned in fieldName
         *
         * @return returns true on success and false on failer.
         */
        static bool setJObjectField(JNIEnv *env, jobject &object, const char *fieldName,
                                    const char *fieldType, const jobject value);
};

#endif //JNI_SETTER_H_
