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

#include "JniRcsRemoteResourceObject.h"

#include "JniRcsObject.h"
#include "JniRcsResourceAttributes.h"
#include "JavaClasses.h"
#include "JavaExceptions.h"
#include "JavaGlobalRef.h"
#include "Log.h"
#include "ScopedEnv.h"
#include "Verify.h"

#include "RCSRemoteResourceObject.h"

using namespace OIC::Service;

#define LOG_TAG "JNI-RCSRemoteResourceObject"

#define CLS_NAME_RESOURCE_STATE CLS_NAME_REMOTERESOURCEOBJECT "$ResourceState"
#define CLS_NAME_CACHE_STATE CLS_NAME_REMOTERESOURCEOBJECT "$CacheState"

#define CLS_NAME_ON_STATE_CHANGED_LISTENER CLS_NAME_REMOTERESOURCEOBJECT "$OnStateChangedListener"
#define CLS_NAME_ON_CACHE_UPDATED_LISTENER CLS_NAME_REMOTERESOURCEOBJECT "$OnCacheUpdatedListener"
#define CLS_NAME_ON_REMOTE_ATTRIBUTES_RECEIVED_LISTENER \
    CLS_NAME_REMOTERESOURCEOBJECT "$OnRemoteAttributesReceivedListener"

namespace
{
    jclass g_cls_RCSRemoteResourceObject;
    jclass g_cls_ResourceState;

    jmethodID g_ctor_RCSRemoteResourceObject;

    jmethodID g_method_onStateChanged;
    jmethodID g_method_onCacheUpdated;
    jmethodID g_method_onAttributesReceived;

    jobject g_obj_ResourceState_None;
    jobject g_obj_ResourceState_Requested;
    jobject g_obj_ResourceState_Alive;
    jobject g_obj_ResourceState_LostSignal;
    jobject g_obj_ResourceState_Destoryed;

    jobject g_obj_CacheState_None;
    jobject g_obj_CacheState_Unready;
    jobject g_obj_CacheState_Ready;
    jobject g_obj_CacheState_LostSignal;


    inline jobjectArray toJavaStringArray(JNIEnv* env, const std::vector< std::string >& vec)
    {
        jobjectArray arrayObj = env->NewObjectArray(vec.size(), g_cls_String, nullptr);
        if (!arrayObj) return nullptr;
        for (size_t i = 0; i < vec.size(); ++i)
        {
            jstring strObj = env->NewStringUTF(vec[i].c_str());
            VERIFY_NO_EXC_RET_DEF(env);

            env->SetObjectArrayElement(arrayObj, i, strObj);
            VERIFY_NO_EXC_RET_DEF(env);

            env->DeleteLocalRef(strObj);
        }
        return arrayObj;
    }

    template < typename ENV >
    inline jobject convertResourceState(ENV* env, ResourceState state)
    {
        switch (state)
        {
            case ResourceState::NONE: return g_obj_ResourceState_None;
            case ResourceState::REQUESTED: return g_obj_ResourceState_Requested;
            case ResourceState::ALIVE: return g_obj_ResourceState_Alive;
            case ResourceState::LOST_SIGNAL: return g_obj_ResourceState_LostSignal;
            case ResourceState::DESTROYED: return g_obj_ResourceState_Destoryed;
        }

        throwRCSException(env, "Failed to convert ResourceState");
        return { };
    }

    inline jobject convertCacheState(JNIEnv* env, CacheState state)
    {
        switch (state)
        {
            case CacheState::NONE: return g_obj_CacheState_None;
            case CacheState::UNREADY: return g_obj_CacheState_Unready;
            case CacheState::READY: return g_obj_CacheState_Ready;
            case CacheState::LOST_SIGNAL: return g_obj_CacheState_LostSignal;
        }

        throwRCSException(env, "Failed to convert CacheState");
        return { };
    }

    inline RCSRemoteResourceObject::Ptr& getResource(JNIEnv* env, jobject obj) noexcept
    {
        return getNativeHandleAs< RCSRemoteResourceObject::Ptr >(env, obj);
    }

    void onStateChanged(ResourceState newState, const JavaGlobalRef& listener)
    {
        ScopedEnvWrapper env;
        EXPECT(env, "env is null!");

        try
        {
            env->CallVoidMethod(listener, g_method_onStateChanged,
                    convertResourceState(env.get(), newState));
        }
        catch (const JavaException&)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }

    void onCacheUpdated(const RCSResourceAttributes& attrs, const JavaGlobalRef& listener)
    {
        LOGD("onCacheUpdated");

        ScopedEnvWrapper env;
        EXPECT(env, "env is null!");

        try
        {
            env->CallVoidMethod(listener, g_method_onCacheUpdated,
                    newAttributesObject(env.get(), attrs));
        }
        catch (const JavaException&)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }

    void onRemoteAttributesReceived(const RCSResourceAttributes& attrs, int errorCode,
            const JavaGlobalRef& listener)
    {
        ScopedEnvWrapper env;
        EXPECT(env, "env is null!");

        try
        {
            env->CallVoidMethod(listener, g_method_onAttributesReceived,
                    newAttributesObject(env.get(), attrs), errorCode);
        }
        catch (const JavaException&)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
}


void initRCSRemoteResourceObject(JNIEnvWrapper* env)
{
    g_cls_RCSRemoteResourceObject = env->FindClassAsGlobalRef(CLS_NAME_REMOTERESOURCEOBJECT);

    g_ctor_RCSRemoteResourceObject = env->GetConstructorID(g_cls_RCSRemoteResourceObject, "()V");

    auto clsOnStateChangedListener = env->FindClass(CLS_NAME_ON_STATE_CHANGED_LISTENER);
    g_method_onStateChanged = env->GetMethodID(clsOnStateChangedListener, "onStateChanged",
            "(" AS_SIG(CLS_NAME_RESOURCE_STATE) ")V");

    auto clsOnCacheUpdatedListener = env->FindClass(CLS_NAME_ON_CACHE_UPDATED_LISTENER);
    g_method_onCacheUpdated = env->GetMethodID(clsOnCacheUpdatedListener, "onCacheUpdated",
            "(" AS_SIG(CLS_NAME_RESOURCEATTRIBUTES) ")V");

    auto clsOnRemoteAttributesReceivedListener =
            env->FindClass(CLS_NAME_ON_REMOTE_ATTRIBUTES_RECEIVED_LISTENER);
    g_method_onAttributesReceived = env->GetMethodID(clsOnRemoteAttributesReceivedListener,
            "onAttributesReceived", "(" AS_SIG(CLS_NAME_RESOURCEATTRIBUTES) "I)V");

    auto clsResourceState = env->FindClass(CLS_NAME_RESOURCE_STATE);

    g_obj_ResourceState_None = env->NewGlobalRef(env->GetStaticObjectField(clsResourceState,
            "NONE", AS_SIG(CLS_NAME_RESOURCE_STATE)));

    g_obj_ResourceState_Requested = env->NewGlobalRef(env->GetStaticObjectField(clsResourceState,
            "REQUESTED", AS_SIG(CLS_NAME_RESOURCE_STATE)));

    g_obj_ResourceState_Alive = env->NewGlobalRef(env->GetStaticObjectField(clsResourceState,
            "ALIVE", AS_SIG(CLS_NAME_RESOURCE_STATE)));

    g_obj_ResourceState_LostSignal = env->NewGlobalRef(env->GetStaticObjectField(clsResourceState,
            "LOST_SIGNAL", AS_SIG(CLS_NAME_RESOURCE_STATE)));

    g_obj_ResourceState_Destoryed = env->NewGlobalRef(env->GetStaticObjectField(clsResourceState,
            "DESTROYED", AS_SIG(CLS_NAME_RESOURCE_STATE)));

    auto clsCacheState = env->FindClass(CLS_NAME_CACHE_STATE);

    g_obj_CacheState_None = env->NewGlobalRef(env->GetStaticObjectField(clsCacheState,
            "NONE", AS_SIG(CLS_NAME_CACHE_STATE)));

    g_obj_CacheState_Unready = env->NewGlobalRef(env->GetStaticObjectField(clsCacheState,
            "UNREADY", AS_SIG(CLS_NAME_CACHE_STATE)));

    g_obj_CacheState_Ready = env->NewGlobalRef(env->GetStaticObjectField(clsCacheState,
            "READY", AS_SIG(CLS_NAME_CACHE_STATE)));

    g_obj_CacheState_LostSignal = env->NewGlobalRef(env->GetStaticObjectField(clsCacheState,
            "LOST_SIGNAL", AS_SIG(CLS_NAME_CACHE_STATE)));


}

void clearRCSRemoteResourceObject(JNIEnvWrapper* env)
{
    env->DeleteGlobalRef(g_cls_RCSRemoteResourceObject);
    env->DeleteGlobalRef(g_cls_ResourceState);

    env->DeleteGlobalRef(g_obj_ResourceState_None);
    env->DeleteGlobalRef(g_obj_ResourceState_Requested);
    env->DeleteGlobalRef(g_obj_ResourceState_Alive);
    env->DeleteGlobalRef(g_obj_ResourceState_LostSignal);
    env->DeleteGlobalRef(g_obj_ResourceState_Destoryed);

    env->DeleteGlobalRef(g_obj_CacheState_None);
    env->DeleteGlobalRef(g_obj_CacheState_Unready);
    env->DeleteGlobalRef(g_obj_CacheState_Ready);
    env->DeleteGlobalRef(g_obj_CacheState_LostSignal);
}

jobject newRemoteResourceObject(JNIEnvWrapper* env)
{
    return env->NewObject(g_cls_RCSRemoteResourceObject, g_ctor_RCSRemoteResourceObject);
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeIsMonitoring
(JNIEnv* env, jobject obj)
{
    LOGD("nativeIsMonitoring");
    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->isMonitoring();
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeIsCaching
(JNIEnv* env, jobject obj)
{
    LOGD("nativeIsCaching");
    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->isCaching();
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeIsObservable
(JNIEnv* env, jobject obj)
{
    LOGD("nativeIsObservable");
    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->isObservable();
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeStartMonitoring
(JNIEnv* env, jobject obj, jobject listener)
{
    LOGD("nativeStartMonitoring");
    EXPECT(listener, "listener is null.");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    try
    {
        res->startMonitoring(
                std::bind(onStateChanged, std::placeholders::_1, JavaGlobalRef{ env, listener }));
    }
    catch (const RCSBadRequestException& e)
    {
        env->ThrowNew(env->FindClass(EXC_NAME_ILLEGAL_STATE), e.what());
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeStopMonitoring
(JNIEnv* env, jobject obj)
{
    LOGD("nativeStopMonitoring");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    res->stopMonitoring();
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetState
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetState");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return convertResourceState(env, res->getState());
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeStartCaching
(JNIEnv* env, jobject obj, jobject listener)
{
    LOGD("nativeStartCaching");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    try
    {
        if (listener)
        {
            res->startCaching(std::bind(onCacheUpdated,
                    std::placeholders::_1, JavaGlobalRef{ env, listener }));
        }
        else
        {
            res->startCaching();
        }
    }
    catch (const RCSBadRequestException& e)
    {
        env->ThrowNew(env->FindClass(EXC_NAME_ILLEGAL_STATE), e.what());
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeStopCaching
(JNIEnv* env, jobject obj)
{
    LOGD("nativeStopCaching");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    res->stopCaching();
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetCacheState
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetCacheState");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return convertCacheState(env, res->getCacheState());
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeIsCachedAvailable
(JNIEnv* env, jobject obj)
{
    LOGD("nativeIsCachedAvailable");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    try
    {
        return res->isCachedAvailable();
    }
    catch (const RCSBadRequestException& e)
    {
        env->ThrowNew(env->FindClass(EXC_NAME_ILLEGAL_STATE), e.what());
        return false;
    }
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetCachedAttributes
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetCachedAttributes");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    try
    {
        auto attrs = res->getCachedAttributes();
        VERIFY_NO_EXC_RET_DEF(env);

        return newAttributesObject(env, attrs);
    }
    catch (const RCSBadRequestException& e)
    {
        env->ThrowNew(env->FindClass(EXC_NAME_ILLEGAL_STATE), e.what());
        return { };
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetRemoteAttributes
(JNIEnv* env, jobject obj, jobject listener)
{
    LOGD("nativeGetRemoteAttributes");
    EXPECT(listener, "listener is null.");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    try
    {
        res->getRemoteAttributes(std::bind(onRemoteAttributesReceived,
                std::placeholders::_1, std::placeholders::_2, JavaGlobalRef{ env, listener }));
    }
    catch (const RCSPlatformException& e) {
        throwPlatformException(env, e);
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeSetRemoteAttributes
(JNIEnv* env, jobject obj, jobject attrsObj, jobject listener)
{
    LOGD("nativeSetRemoteAttributes");
    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    RCSResourceAttributes attrs = toNativeAttributes(env, attrsObj);
    VERIFY_NO_EXC(env);

    try
    {
        res->setRemoteAttributes(attrs, std::bind(onRemoteAttributesReceived,
                std::placeholders::_1, std::placeholders::_2, JavaGlobalRef{ env, listener }));
    }
    catch (const RCSPlatformException& e) {
        throwPlatformException(env, e);
    }
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetUri
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetUri");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return env->NewStringUTF(res->getUri().c_str());
}

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetAddress
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetAddress");
    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return env->NewStringUTF(res->getAddress().c_str());
}

JNIEXPORT jobjectArray JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetTypes
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetTypes");
    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return toJavaStringArray(env, res->getTypes());
}

JNIEXPORT jobjectArray JNICALL
Java_org_iotivity_service_client_RcsRemoteResourceObject_nativeGetInterfaces
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetInterfaces");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return toJavaStringArray(env, res->getInterfaces());
}

