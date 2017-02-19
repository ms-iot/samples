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
 * This file contains the declaration of Group Manager APIs
 * for JNI implementation.
 */

#ifndef JNI_GROUP_MANAGER_H_
#define JNI_GROUP_MANAGER_H_

#include <stdio.h>
#include <string.h>

#include <jni.h>
#include <jni_string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * API for discoverying candidate resources.
 *
 * @param resourceTypes  - required resource types(called "candidate")
 * @param waitSec        - Delay time in seconds to add before starting to find the resources in network.
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerFindCandidateResource
(JNIEnv *env, jobject interfaceObject, jobject resourceTypes, jint waitSec);

/**
 * API for subscribing child's state.
 *
 * @param resource       - collection resource for subscribing presence of all child resources.
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerSubscribeCollectionPresence
(JNIEnv *env, jobject interfaceObject, jobject resource);


/**
 * API for register and bind resource to group.
 *
 * @param resource         - resource for register and bind to group. It has all data.
 * @param collectionHandle - collection resource handle. It will be added child resource.
 *
 * @return childHandle     - child resource handle.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jobject JNICALL JNIGroupManagerBindResourceToGroup
(JNIEnv *env, jobject interfaceObject, jobject resource, jobject collectionHandle);

/**
 * API for adding an Action Set.
 *
 * @param resource       - resource type representing the target group
 * @param newActionSet   - list of Action Set to be added
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */

JNIEXPORT jint JNICALL JNIGroupManagerAddActionSet
(JNIEnv *env, jobject interfaceObject, jobject resource, jobject newActionSet);

/**
 * API for executing the Action Set.
 *
 * @param resource       - resource type representing the target group
 * @param actionSetName  - Action Set name for executing the Action set
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerExecuteActionSet
(JNIEnv *env, jobject interfaceObject, jobject resource, jstring actionSetName);

/**
 * API for executing the Action Set.
 *
 * @param resource       - resource type representing the target group
 * @param actionSetName  - Action Set name for executing the Action set
 * @param delay          - waiting time for until action set run.
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerExecuteActionSetWithDelay
(JNIEnv *env, jobject interfaceObject, jobject resource, jstring actionSetName, jlong delay);

/**
 * API for cancelling the Action Set.
 *
 * @param resource       - resource type representing the target group
 * @param actionSetName  - Action Set name for cancelling the Action set
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerCancelActionSet
(JNIEnv *env, jobject interfaceObject, jobject resource, jstring actionSetName);


/**
 * API for reading the Action Set.
 *
 * @param resource       - resource type representing the target group
 * @param actionSetName  - Action Set name for reading the Action set
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerGetActionSet
(JNIEnv *env, jobject interfaceObject, jobject resource, jstring actionSetName);

/**
 * API for removing the Action Set.
 *
 * @param resource       - resource type representing the target group
 * @param actionSetName  - Action Set name for removing the Action set
 *
 * @return OCStackResult - return value of this API.
 *                         It returns OC_STACK_OK if success.
 *
 * NOTE: OCStackResult is defined in ocstack.h.
 */
JNIEXPORT jint JNICALL JNIGroupManagerDeleteActionSet
(JNIEnv *env, jobject interfaceObject, jobject resource, jstring actionSetName);

#ifdef __cplusplus
}
#endif
#endif //JNI_GROUP_MANAGER_H_
