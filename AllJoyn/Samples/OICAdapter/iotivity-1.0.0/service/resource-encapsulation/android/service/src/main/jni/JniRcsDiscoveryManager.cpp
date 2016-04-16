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

#include "JniRcsDiscoveryManager.h"

#include "JniRcsObject.h"
#include "JniRcsRemoteResourceObject.h"
#include "JavaClasses.h"
#include "JavaExceptions.h"
#include "JavaGlobalRef.h"
#include "JNIEnvWrapper.h"
#include "Log.h"
#include "ScopedEnv.h"
#include "Verify.h"

#include "RCSDiscoveryManager.h"
#include "RCSAddress.h"
#include "RCSRemoteResourceObject.h"

using namespace OIC::Service;

#define LOG_TAG "JNI-DiscoveryManager"

#define CLS_NAME_DISCOVERY_MANAGER PACKAGE_NAME "/client/RcsDiscoveryManager"

#define CLS_NAME_ON_RESOURCE_DISCOVERED_LISTENER \
    CLS_NAME_DISCOVERY_MANAGER "$OnResourceDiscoveredListener"

#define CLS_NAME_DISCOVERY_TASK CLS_NAME_DISCOVERY_MANAGER "$DiscoveryTask"

namespace
{
    jclass g_cls_DiscoveryaTask;

    jmethodID g_method_onResourceDiscovered;

    jmethodID g_ctor_DiscoveryTask;
}

void initRCSDiscoveryManager(JNIEnvWrapper* env)
{
    auto clsOnResourceDiscoveredListener = env->FindClass(CLS_NAME_ON_RESOURCE_DISCOVERED_LISTENER);

    g_method_onResourceDiscovered = env->GetMethodID(clsOnResourceDiscoveredListener,
            "onResourceDiscovered", "(" AS_SIG(CLS_NAME_REMOTERESOURCEOBJECT) ")V");

    g_cls_DiscoveryaTask = env->FindClassAsGlobalRef(CLS_NAME_DISCOVERY_TASK);

    g_ctor_DiscoveryTask = env->GetConstructorID(g_cls_DiscoveryaTask, "()V");
}

void clearRCSDiscoveryManager(JNIEnvWrapper* env)
{
    env->DeleteGlobalRef(g_cls_DiscoveryaTask);
}

void onResourceDiscovered(RCSRemoteResourceObject::Ptr resource, const JavaGlobalRef& listener)
{
    LOGI("onResourceDiscovered");

    ScopedEnvWrapper env;
    EXPECT(env, "env is null!");

    try
    {
        auto newResourceObj = newRemoteResourceObject(env.get());

        setSafeNativeHandle< RCSRemoteResourceObject::Ptr >(env.get(),
                newResourceObj, std::move(resource));

        env->CallVoidMethod(listener, g_method_onResourceDiscovered, newResourceObj);
    }
    catch (const JavaException&)
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_client_RcsDiscoveryManager_nativeDiscoverResource(
        JNIEnv *env, jclass obj, jstring address, jstring uri, jstring resourceType,
        jobject listener)
{
    LOGI("discoverResource");

    RCSAddress rcsAddress{ address ? RCSAddress::unicast(toStdString(env, address)) :
            RCSAddress::multicast() };

    try
    {
        auto discoveryTask = RCSDiscoveryManager::getInstance()->discoverResourceByType(
            std::move(rcsAddress), toStdString(env, uri), toStdString(env, resourceType),
            std::bind(onResourceDiscovered, std::placeholders::_1, JavaGlobalRef{ env, listener }));

        auto taskObj = env->NewObject(g_cls_DiscoveryaTask, g_ctor_DiscoveryTask);
        VERIFY_NO_EXC_RET_DEF(env);

        setSafeNativeHandle< decltype(discoveryTask) >(env, taskObj, std::move(discoveryTask));

        return taskObj;
    }
    catch (const RCSPlatformException& e) {
        throwPlatformException(env, e);
    }

    return nullptr;
}
