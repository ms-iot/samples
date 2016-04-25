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
 * This file contains the declaration of Things Configuration  API's
 * for JNI implementation.
 */

#ifndef JNI_THINGS_CONFIGURATION_H_
#define JNI_THINGS_CONFIGURATION_H_

#include <stdio.h>
#include <string.h>

#include <jni.h>
#include <jni_string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * API for updating configuration value of multiple things of a target group
 * or a single thing.
 * Before using the below function, a developer should acquire a resource pointer of
 * (collection) resource that he wants to send a request by calling findResource() function
 * provided in OCPlatform. And he should also notice a "Configuration Name" term which
 * represents a nickname of a target attribute of a resource that he wants to update.
 * The base motivation to introduce the term is to avoid a usage of URI to access a resource
 * from a developer. Thus, a developer should know which configuration names are supported
 * by Things Configuration class and what the configuration name means.
 * To get a list of supported configuration names,  use getListOfSupportedConfigurationUnits()
 * function, which provides the list in JSON format.
 *
 * @param resource - resource pointer representing the target group or the single thing.
 * @param configurations - ConfigurationUnit: a nickname of attribute of target resource
 *                         (e.g., installedlocation, currency, (IP)address)
 *                         Value : a value to be updated
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIThingsConfigurationUpdateConfigurations
(JNIEnv *env, jobject interfaceObject, jobject resource, jobject configurations);

/**
 * API for getting configuration value of multiple things of a target group
 * or a single thing.
 *
 * @param resource - resource pointer representing the target group or the single thing.
 * @param configurations - ConfigurationUnit: a nickname of attribute of target resource.
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIThingsConfigurationGetConfigurations
(JNIEnv *env, jobject interfaceObject, jobject resource, jobject configurations);

/**
 * API for showing the list of supported configuration units (configurable parameters).
 *
 * @return std::string - return value of this API.
 *                       It returns the list in JSON format
 */
JNIEXPORT jstring JNICALL JNIThingsConfigurationGetListOfSupportedConfigurationUnits
(JNIEnv *env, jobject interfaceObject);

/**
 * API for boostrapping system configuration parameters from a bootstrap server.
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIThingsConfigurationDoBootstrap
(JNIEnv *env, jobject interfaceObject);

#ifdef __cplusplus
}
#endif
#endif //JNI_THINGS_CONFIGURATION_H_
