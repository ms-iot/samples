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

#include "jni_group_manager_callbacks.h"
#include "JniOcResource.h"
#include "GroupManager.h"
#include "jni_things_manager_jvm.h"
#include "jni_things_manager_util.h"
#include "jni_string.h"

#define LOG_TAG "GroupManagerCallbacks"

#define METHOD_ONRESOURCE_CALLBACK    "(" TM_JAVA_VECTOR_TYPE")V"
#define METHOD_ONPRESENCE_CALLBACK    "(" TM_JAVA_STRING_TYPE"I)V"
#define METHOD_ONPOST_CALLBACK    "(" TM_JAVA_VECTOR_TYPE TM_SERVICE_OCREPRESENTATION_TYPE"I)V"
#define METHOD_ONPUT_CALLBACK    "(" TM_JAVA_VECTOR_TYPE TM_SERVICE_OCREPRESENTATION_TYPE"I)V"
#define METHOD_ONGET_CALLBACK    "(" TM_JAVA_VECTOR_TYPE TM_SERVICE_OCREPRESENTATION_TYPE"I)V"

void GroupManagerCallbacks::onFoundCandidateResource(
    std::vector< std::shared_ptr<OC::OCResource>> resources)
{
    LOGI("onFoundCandidateResource : Enter");

    if (resources.size() == 0)
    {
        LOGE("onFoundCandidateResource : found resources zero");
        return;
    }

    JNIEnv *env = ThingsManagerJVM::getEnv();
    if (env == NULL)
    {
        LOGE("onFoundCandidateResource : Getting JNIEnv failed");
        return;
    }

    // Get GroupManagerCallback class reference
    jclass groupManagerCallbacks = GetJClass(TM_SERVICE_GROUP_MANAGER_CLASS_PATH);

    if (NULL == groupManagerCallbacks)
    {
        LOGE("onFoundCandidateResource : GetJClass TMServiceCallbackInterface failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    // Get onResourceCallback method reference
    jmethodID method_id = env->GetStaticMethodID(groupManagerCallbacks, "onResourceFoundCallback",
                          METHOD_ONRESOURCE_CALLBACK);
    if (NULL == method_id)
    {
        LOGE("findCandidateResource: onResourceCallback : GetMethodID failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    if ((env)->ExceptionCheck())
    {
        LOGE("findCandidateResource : ExceptionCheck failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jclass vectorCls = env->FindClass(TM_JAVA_VECTOR_CLASS_PATH);
    if (!vectorCls)
    {
        LOGE("findCandidateResource: failed to get %s class reference", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID constr = env->GetMethodID(vectorCls, "<init>", "()V");
    if (!constr)
    {
        LOGE("findCandidateResource: failed to get %s constructor", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jobject vectorObj = env->NewObject(vectorCls, constr);
    if (!vectorObj)
    {
        LOGE("findCandidateResource: failed to create a %s object", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID addElement = env->GetMethodID(vectorCls, "addElement", "(Ljava/lang/Object;)V");
    if (NULL == addElement)
    {
        LOGE("findCandidateResource: failed to create a addElement method");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    // Convert to java OCResource object
    for (int i = 0; i < resources.size(); i++)
    {
        JniOcResource *jniOcResource = new JniOcResource(resources[i]);
        if (!jniOcResource)
        {
            LOGE("findCandidateResource: failed to create a JniOcResource");
            ThingsManagerJVM::releaseEnv();
            return;
        }

        jobject resource = OcResourceToJava(env, reinterpret_cast<jlong>(jniOcResource));
        env->CallVoidMethod(vectorObj, addElement, resource);
    }

    env->CallStaticVoidMethod(groupManagerCallbacks, method_id, vectorObj);

    if ((env)->ExceptionCheck())
    {
        LOGE("findCandidateResource : CallVoidMethod failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    ThingsManagerJVM::releaseEnv();
    LOGI("findCandidateResource : Exit");
}

void GroupManagerCallbacks::onSubscribePresence(std::string resource, OCStackResult result)
{
    LOGI("SubscribePresence : Entry");

    JNIEnv *env = ThingsManagerJVM::getEnv();
    if (env == NULL)
    {
        LOGE("SubscribePresence : Getting JNIEnv failed");
        return;
    }

    // Get GroupManagerCallback class reference
    jclass groupManagerCallbacks = GetJClass(TM_SERVICE_GROUP_MANAGER_CLASS_PATH);

    if (NULL == groupManagerCallbacks)
    {
        LOGE("SubscribePresence : GetJClass TMServiceCallbackInterface failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID method_id = env->GetStaticMethodID(groupManagerCallbacks,
                          "onPresenceCallback",
                          METHOD_ONPRESENCE_CALLBACK);
    if (NULL == method_id)
    {
        LOGE("SubscribePresence : GetMethodID failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    if ((env)->ExceptionCheck())
    {
        LOGE("SubscribePresence : ExceptionCheck failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    JString *jresource = new JString(env, resource);
    if (jresource == NULL)
    {
        LOGE("resource value is invalid");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    env->CallStaticVoidMethod(groupManagerCallbacks, method_id, jresource->getObject(), (jint)result);

    delete jresource;

    if ((env)->ExceptionCheck())
    {
        LOGE("SubscribePresence : CallVoidMethod failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    LOGI("SubscribePresence : Exit");

//METHOD_FAILURE:
    ThingsManagerJVM::releaseEnv();
}

void GroupManagerCallbacks::onPostResponse(const OC::HeaderOptions &headerOptions,
        const OC::OCRepresentation &rep, const int eCode)
{
    LOGI("PostResponse : Enter");

    GroupManagerCallbacks::invokeCallback(headerOptions, rep, eCode, "onPostResponseCallback",
                                          METHOD_ONPOST_CALLBACK);

    LOGI("PostResponse : Exit");
}

void GroupManagerCallbacks::onPutResponse(const OC::HeaderOptions &headerOptions,
        const OC::OCRepresentation &rep, const int eCode)
{
    LOGI("OnPutResponse : Enter");

    GroupManagerCallbacks::invokeCallback(headerOptions, rep, eCode, "onPutResponseCallback",
                                          METHOD_ONPUT_CALLBACK);

    LOGI("OnPutResponse : Exit");
}

void GroupManagerCallbacks::onGetResponse(const OC::HeaderOptions &headerOptions,
        const OC::OCRepresentation &rep, const int eCode)
{
    LOGI("OnGetResponse : Enter");

    GroupManagerCallbacks::invokeCallback(headerOptions, rep, eCode, "onGetResponseCallback",
                                          METHOD_ONGET_CALLBACK);

    LOGI("OnGetResponse : Exit");
}

void GroupManagerCallbacks::invokeCallback(const OC::HeaderOptions &headerOptions,
        const OC::OCRepresentation &rep, const int eCode, const char  *callbackName,
        const char *signature)
{
    LOGI("InvokeCallback : Enter %s", callbackName);

    JNIEnv *env = ThingsManagerJVM::getEnv();
    if (env == NULL)
    {
        LOGE("InvokeCallback : Getting JNIEnv failed");
        return;
    }

    // Get ThingsManagerCallback class reference
    jclass groupManagerCallbacks = GetJClass(TM_SERVICE_GROUP_MANAGER_CLASS_PATH);
    if (NULL == groupManagerCallbacks)
    {
        LOGE("InvokeCallback : GetJClass TMServiceCallbackInterface failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID method_id = env->GetStaticMethodID(groupManagerCallbacks, callbackName, signature);
    if (!method_id)
    {
        LOGE("InvokeCallback : GetMethodID failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    if ((env)->ExceptionCheck())
    {
        LOGE("InvokeCallback : ExceptionCheck failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    // Convert vector<OC:HeaderOption::OCHeaderOption> to java type
    jclass vectorCls = env->FindClass(TM_JAVA_VECTOR_CLASS_PATH);
    if (!vectorCls)
    {
        LOGE("InvokeCallback: failed to get %s class reference", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID constr = env->GetMethodID(vectorCls, "<init>", "()V");
    if (!constr)
    {
        LOGE("InvokeCallback: failed to get %s constructor", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jobject vectorObj = env->NewObject(vectorCls, constr);
    if (!vectorObj)
    {
        LOGE("InvokeCallback: failed to create a %s object", TM_JAVA_VECTOR_CLASS_PATH);
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jmethodID addElement = env->GetMethodID(vectorCls, "addElement", "(Ljava/lang/Object;)V");
    if (NULL == addElement)
    {
        LOGE("InvokeCallback: failed to create a addElement method");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    jobject headerOptionTemp;
    for (int i = 0; i < headerOptions.size(); i++)
    {
        headerOptionTemp = OcHeaderOptionToJava(env, headerOptions[i]);
        env->CallVoidMethod(vectorObj, addElement, headerOptionTemp);
    }

    // Convert OCRepresentation to java type
    jobject jrepresentation = OcRepresentationToJava(env, (jlong) reinterpret_cast<jlong>(&rep));
    if (!jrepresentation)
    {
        LOGE("InvokeCallback : cannot create OCRepresentation class");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    env->CallStaticVoidMethod(groupManagerCallbacks, method_id, vectorObj, jrepresentation,
                              (jint)eCode);

    if ((env)->ExceptionCheck())
    {
        LOGE("InvokeCallback : CallVoidMethod failed");
        ThingsManagerJVM::releaseEnv();
        return;
    }

    ThingsManagerJVM::releaseEnv();
    LOGI("InvokeCallback : Exit %s", callbackName);
}
