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

#include "jni_group_manager.h"
#include "JniOcResource.h"
#include "JniOcResourceHandle.h"
#include "GroupManager.h"
#include "ActionSet.h"
#include "jni_things_manager_jvm.h"
#include "jni_things_manager_util.h"
#include "jni_group_manager_callbacks.h"
#include "jni_action_set.h"

/**
 * GroupManager static object
 */
static GroupManager g_GroupManager;

JNIEXPORT jint JNICALL JNIGroupManagerFindCandidateResource(JNIEnv *env, jobject interfaceObject,
        jobject jResourceTypes, jint waitSec)
{
    LOGI("JNIGroupManagerFindCandidateResource: Enter");

    if (!jResourceTypes)
    {
        LOGE("JNIGroupManagerFindCandidateResource: jResourceTypes is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;
    try
    {
        ocResult = g_GroupManager.findCandidateResources(convertStringVector(env, jResourceTypes),
                   &GroupManagerCallbacks::onFoundCandidateResource, (int)waitSec);

        if (OC_STACK_OK != ocResult)
        {
            LOGE("JNIGroupManagerFindCandidateResource: findCandidateResources failed!");
            return ocResult;
        }
    }
    catch (InitializeException &e)
    {
        LOGE("JNIGroupManagerFindCandidateResource: Exception occurred! %s, %d", e.reason().c_str(),
             e.code());
        return ocResult;
    }

    LOGI("JNIGroupManagerFindCandidateResource: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIGroupManagerSubscribeCollectionPresence(JNIEnv *env,
        jobject interfaceObject,
        jobject jResource)
{
    LOGI("JNIGroupManagerSubscribeCollectionPresence: Enter");

    if (!jResource)
    {
        LOGE("JNIGroupManagerSubscribeCollectionPresence: jResource is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, jResource);
    if (NULL == jniOcResource)
    {
        LOGE("JNIGroupManagerSubscribeCollectionPresence: Failed to get jni OcResource!");
        return ocResult;
    }

    std::shared_ptr<OCResource> ocResource = jniOcResource->getOCResource();
    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerSubscribeCollectionPresence: Failed to get OCResource object!");
        return ocResult;
    }

    ocResult = g_GroupManager.subscribeCollectionPresence(ocResource,
               &GroupManagerCallbacks::onSubscribePresence);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerSubscribeCollectionPresence: subscribeCollectionPresence failed!");
        return ocResult;
    }

    LOGI("JNIGroupManagerSubscribeCollectionPresence: Exit");
    return ocResult;
}

JNIEXPORT jobject JNICALL JNIGroupManagerBindResourceToGroup(JNIEnv *env, jobject interfaceObject,
        jobject jResource, jobject jCollectionHandle)
{
    LOGI("JNIGroupManagerBindResourceToGroup: Enter");

    if (!jResource || !jCollectionHandle)
    {
        LOGE("JNIGroupManagerBindResourceToGroup: Invalid parameter!");
        return NULL;
    }

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, jResource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerBindResourceToGroup: Failed to get OCResource object!");
        return NULL;
    }

    JniOcResourceHandle *jniOcCollectionHandle = JniOcResourceHandle::getJniOcResourceHandlePtr(env,
            jCollectionHandle);
    if (NULL == jniOcCollectionHandle)
    {
        LOGE("JNIGroupManagerBindResourceToGroup: collection handle is null!");
        return NULL;
    }

    jobject jResourceHandle = NULL;
    try
    {
        OCResourceHandle ocChildHandle = NULL;
        OCResourceHandle ocCollectionHandle = jniOcCollectionHandle->getOCResourceHandle();
        OCStackResult ocResult = g_GroupManager.bindResourceToGroup(ocChildHandle, ocResource,
                                 ocCollectionHandle);
        if (OC_STACK_OK != ocResult)
        {
            LOGE("JNIGroupManagerBindResourceToGroup: bindResourceToGroup failed!");
            return NULL;
        }

        // Convert OCResourceHandle to java type
        JniOcResourceHandle *jniHandle = new JniOcResourceHandle(ocChildHandle);
        jlong handle = reinterpret_cast<jlong>(jniHandle);
        jResourceHandle = env->NewObject(g_cls_OcResourceHandle, g_mid_OcResourceHandle_N_ctor, handle);
        if (env->ExceptionCheck())
        {
            LOGE("JNIGroupManagerBindResourceToGroup: Failed to create OcResourceHandle");
            delete jniHandle;
            return NULL;
        }
    }
    catch (InitializeException &e)
    {
        LOGE("JNIGroupManagerBindResourceToGroup: Exception occurred! %s, %d", e.reason().c_str(),
             e.code());
        return NULL;
    }

    LOGI("JNIGroupManagerBindResourceToGroup: Exit");
    return jResourceHandle;
}

JNIEXPORT jint JNICALL JNIGroupManagerAddActionSet(JNIEnv *env, jobject interfaceObject,
        jobject resource,
        jobject newActionSet)
{
    LOGI("JNIGroupManagerAddActionSet: Entry");

    if ((!resource) || (!newActionSet))
    {
        LOGE("JNIGroupManagerAddActionSet: resource or newActionSet is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerAddActionSet: Failed to get OCResource object!");
        return ocResult;
    }

    JniActionSet *jActionSet = new JniActionSet(env, newActionSet);
    ActionSet *pActionSet = jActionSet->getActionSet(env, newActionSet);
    if (NULL == pActionSet)
    {
        LOGE("JNIGroupManagerAddActionSet: Failed to convert ActionSet!");
        return ocResult;
    }

    ocResult = g_GroupManager.addActionSet(ocResource, pActionSet,
                                           &GroupManagerCallbacks::onPutResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerAddActionSet: addActionSet is failed!");
    }

    delete pActionSet;
    LOGI("JNIGroupManagerAddActionSet: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIGroupManagerExecuteActionSet(JNIEnv *env, jobject interfaceObject,
        jobject resource, jstring jActionSetName)
{
    LOGI("JNIGroupManagerExecuteActionSet: Entry");

    if ((!resource) || (!jActionSetName))
    {
        LOGE("JNIGroupManagerExecuteActionSet: resource or jActionSetName is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerExecuteActionSet: Failed to get OCResource object!");
        return ocResult;
    }

    const char *actionSetNamePointer = env->GetStringUTFChars(jActionSetName, 0);
    if (NULL == actionSetNamePointer)
    {
        LOGE("JNIGroupManagerExecuteActionSet: Failed to convert jstring to char string!");
        return OC_STACK_NO_MEMORY;
    }

    std::string actionSetName(actionSetNamePointer);

    ocResult = g_GroupManager.executeActionSet(ocResource, actionSetName,
               &GroupManagerCallbacks::onPostResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerExecuteActionSet: executeActionSet is failed!");
    }

    env->ReleaseStringUTFChars(jActionSetName, actionSetNamePointer);
    LOGI("JNIGroupManagerExecuteActionSet: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIGroupManagerExecuteActionSetWithDelay(JNIEnv *env,
        jobject interfaceObject,
        jobject resource, jstring jActionSetName, jlong delay)
{
    LOGI("JNIGroupManagerExecuteActionSetWithDelay: Entry");

    if ((!resource) || (!jActionSetName))
    {
        LOGE("JNIGroupManagerExecuteActionSetWithDelay: resource or jActionSetName is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerExecuteActionSetWithDelay: Failed to get OCResource object!");
        return ocResult;
    }

    const char *actionSetNamePointer = env->GetStringUTFChars(jActionSetName, 0);
    if (NULL == actionSetNamePointer)
    {
        LOGE("JNIGroupManagerExecuteActionSetWithDelay: Failed to convert jstring to char string!");
        return OC_STACK_NO_MEMORY;
    }

    std::string actionSetName(actionSetNamePointer);
    if (0 == delay)
    {
        ocResult = g_GroupManager.executeActionSet(ocResource, actionSetName,
                   &GroupManagerCallbacks::onPostResponse);
    }
    else
    {
        ocResult = g_GroupManager.executeActionSet(ocResource, actionSetName,
                   (long int)delay,
                   &GroupManagerCallbacks::onPostResponse);
    }
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerExecuteActionSetWithDelay: executeActionSet is failed!");
    }

    env->ReleaseStringUTFChars(jActionSetName, actionSetNamePointer);
    LOGI("JNIGroupManagerExecuteActionSetWithDelay: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIGroupManagerCancelActionSet(JNIEnv *env, jobject interfaceObject,
        jobject resource, jstring jActionSetName)
{
    LOGI("JNIGroupManagerCancelActionSet: Entry");

    if ((!resource) || (!jActionSetName))
    {
        LOGE("JNIGroupManagerCancelActionSet: resource or jActionSetName is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerCancelActionSet: Failed to get OCResource object!");
        return ocResult;
    }

    const char *actionSetNamePointer = env->GetStringUTFChars(jActionSetName, 0);
    if (NULL == actionSetNamePointer)
    {
        LOGE("JNIGroupManagerCancelActionSet: Failed to get character sequence from jstring!");
        return OC_STACK_NO_MEMORY;
    }

    std::string actionSetName(actionSetNamePointer);

    ocResult = g_GroupManager.cancelActionSet(ocResource, actionSetName,
               &GroupManagerCallbacks::onPostResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerCancelActionSet: cancelActionSet is failed!");
    }

    env->ReleaseStringUTFChars(jActionSetName, actionSetNamePointer);
    LOGI("JNIGroupManagerCancelActionSet: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIGroupManagerGetActionSet(JNIEnv *env, jobject interfaceObject,
        jobject resource,
        jstring jActionSetName)
{
    LOGI("JNIGroupManagerGetActionSet: Entry");

    if ((!resource) || (!jActionSetName))
    {
        LOGE("JNIGroupManagerGetActionSet: resource or jActionSetName is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerGetActionSet: Failed to get OCResource object!");
        return ocResult;
    }

    const char *actionSetNamePointer = env->GetStringUTFChars(jActionSetName, 0);
    std::string actionSetName(actionSetNamePointer);

    ocResult = g_GroupManager.getActionSet(ocResource, actionSetName,
                                           &GroupManagerCallbacks::onGetResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerGetActionSet: getActionSet is failed!");
    }

    env->ReleaseStringUTFChars(jActionSetName, actionSetNamePointer);
    LOGI("JNIGroupManagerGetActionSet: Exit");
    return ocResult;
}

JNIEXPORT jint JNICALL JNIGroupManagerDeleteActionSet(JNIEnv *env, jobject interfaceObject,
        jobject resource,
        jstring jActionSetName)
{
    LOGI("JNIGroupManagerDeleteActionSet: Entry");

    if ((!resource) || (!jActionSetName))
    {
        LOGE("JNIGroupManagerDeleteActionSet: resource or jActionSetName is NULL!");
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ocResult = OC_STACK_ERROR;

    std::shared_ptr<OCResource> ocResource;
    JniOcResource *jniOcResource = JniOcResource::getJniOcResourcePtr(env, resource);
    if (jniOcResource)
    {
        ocResource = jniOcResource->getOCResource();
    }

    if (NULL == ocResource.get())
    {
        LOGE("JNIGroupManagerDeleteActionSet: Failed to get OCResource object!");
        return ocResult;
    }

    const char *actionSetNamePointer = env->GetStringUTFChars(jActionSetName, 0);
    std::string actionSetName(actionSetNamePointer);

    ocResult = g_GroupManager.deleteActionSet(ocResource, actionSetName,
               &GroupManagerCallbacks::onPutResponse);
    if (OC_STACK_OK != ocResult)
    {
        LOGE("JNIGroupManagerDeleteActionSet: deleteActionSet is failed!");
    }

    env->ReleaseStringUTFChars(jActionSetName, actionSetNamePointer);
    LOGI("JNIGroupManagerDeleteActionSet: Exit");
    return ocResult;
}
