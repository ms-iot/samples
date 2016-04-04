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

/**
  * @file
  * This file contains the utility functions for conversions from java to CPP
  * and viceversa.
  */

#ifndef JNI_THINGS_MANAGER_UTILS_H_
#define JNI_THINGS_MANAGER_UTILS_H_

#include <vector>
#include <map>
#include "JniOcResource.h"
#include "jni_things_manager_jvm.h"

/**
  * Utility function for converting a Java Vector of Strings to CPP Vector of Strings.
  *
  * @param env           - Default JNI Environment pointer
  * @param jVectorString - Java Vector of Strings
  */
std::vector<std::string> convertStringVector(JNIEnv *env, jobject jVectorString);

/**
  * Utility function for converting a Hash Map of Strings to CPP Map of Strings.
  *
  * @param env        - Default JNI Environment pointer
  * @param jMapString - Java Map of Strings
  */
std::map<std::string, std::string> convertStringMap(JNIEnv *env, jobject jMapString);

/**
  * Utility function for converting ocResource Handle to java.
  * @param env            - Default JNI Environment pointer
  * @param resourceHandle -  Resource Handle
  */
jobject ocResourceHandleToJava(JNIEnv *env, jlong resourceHandle);

/**
  * Utility function for converting native ocResource to java ocResource.
  * @param env      - Default JNI Environment pointer
  * @param resource - Native Resource
  */
jobject OcResourceToJava(JNIEnv *env, jlong resource);

/**
  * Utility function for converting native ocHeaderOption to java ocHeaderOption.
  * @param env          - Default JNI Environment pointer
  * @param headerOption - Native headerOption
  */
jobject OcHeaderOptionToJava(JNIEnv *env, OC::HeaderOption::OCHeaderOption headerOption);

/**
  * Utility function for converting native ocRepresentation  to java ocRepresentation.
  * @param env              - Default JNI Environment pointer
  * @param ocRepresentation - Native ocRepresentation
  */
jobject OcRepresentationToJava(JNIEnv *env, jlong ocRepresentation);

#endif //JNI_THINGS_MANAGER_UTILS_H_

