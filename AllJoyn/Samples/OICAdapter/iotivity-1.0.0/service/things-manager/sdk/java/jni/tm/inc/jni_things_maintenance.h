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
 * This file contains the declaration of Things Maintenance  API's
 * for JNI implementation.
 */

#ifndef JNI_THINGS_MAINTENANCE_H_
#define JNI_THINGS_MAINTENANCE_H_

#include <stdio.h>
#include <string.h>

#include <jni.h>
#include <jni_string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * API to let thing(device) reboot.
 * The target thing could be a group of multiple things or a single thing.
 *
 * @param resource       - resource type representing the target group
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIThingsMaintenanceReboot
(JNIEnv *env, jobject interfaceObject, jobject resource);

/**
 * API for factory reset on thing(device).
 * The target thing could be a group of multiple things or a single thing.
 *
 * @param resource       - resource type representing the target group
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIThingsMaintenanceFactoryReset
(JNIEnv *env, jobject interfaceObject, jobject resource);


/**
 * API for showing the list of supported Maintenance units.
 *
 * @return std::string - return value of this API.
 *                       It returns the list in JSON format
 */
JNIEXPORT jstring JNICALL JNIThingsMaintenanceGetListOfSupportedConfigurationUnits
(JNIEnv *env, jobject interfaceObject);

#ifdef __cplusplus
}
#endif
#endif //JNI_THINGS_MAINTENANCE_H_
