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
 * @file   simulator_resource_jni_util.h
 *
 * @brief  This file contains the utility functions for conversions from java to CPP
 * and viceversa
 */

#ifndef __SIMULATOR_RESOURCE_JNI_UTIL_H_
#define __SIMULATOR_RESOURCE_JNI_UTIL_H_

#include <jni.h>
#include <iostream>
#include <vector>
#include <map>

#include "simulator_remote_resource.h"

/**
  * Utility function for converting a Java Vector of Strings to CPP Vector of Strings
  *
  * @param env - Default JNI Environment pointer
  * @param jVectorString - Java Vector of Strings
  *
  * @return void
  */
std::vector<int> convertIntegerVector(JNIEnv *env, jobject jVectorInt);

/**
  * Utility function for converting a Java Vector of Strings to CPP Vector of Strings
  *
  * @param env - Default JNI Environment pointer
  * @param jVectorString - Java Vector of Strings
  *
  * @return void
  */
std::vector<double> convertDoubleVector(JNIEnv *env, jobject jVectorDouble);

/**
  * Utility function for converting a Java Vector of Strings to CPP Vector of Strings
  *
  * @param env - Default JNI Environment pointer
  * @param jVectorString - Java Vector of Strings
  *
  * @return void
  */
std::vector<std::string> convertStringVector(JNIEnv *env, jobject jVectorString);

void convertJavaMapToQueryParamsMap(JNIEnv *env, jobject hashMap,
                                    std::map<std::string, std::string> &map);

jobject convertHashMapToJavaMap(JNIEnv *env,
                                const std::map<std::string, uint8_t> &observersList);

jobject convertStringVectorToJavaList(JNIEnv *env, std::vector<std::string> &vector);

#endif //__SIMULATOR_RESOURCE_JNI_UTIL_H_

