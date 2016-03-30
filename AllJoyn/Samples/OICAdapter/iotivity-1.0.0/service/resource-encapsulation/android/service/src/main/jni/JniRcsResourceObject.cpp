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

#include "JniRcsResourceObject.h"

#include "JniRcsObject.h"
#include "JniRcsResourceAttributes.h"
#include "JavaClasses.h"
#include "JavaExceptions.h"
#include "JavaGlobalRef.h"
#include "JniRcsValue.h"
#include "Log.h"
#include "ScopedEnv.h"
#include "Verify.h"

#include "RCSResourceObject.h"
#include "RCSResponse.h"
#include "RequestHandler.h"

#define LOG_TAG "JNI-RCSResourceObject"

#define CLS_NAME_RCS_RESOURCE_OBJECT PACKAGE_NAME "/server/RcsResourceObject"
#define CLS_NAME_RCS_REQUEST PACKAGE_NAME "/server/RcsRequest"

#define CLS_NAME_AUTO_NOTIFY_POLICY CLS_NAME_RCS_RESOURCE_OBJECT "$AutoNotifyPolicy"
#define CLS_NAME_SET_REQUEST_HANDLER_POLICY CLS_NAME_RCS_RESOURCE_OBJECT "$SetRequestHandlerPolicy"

#define CLS_NAME_GET_REQUEST_HANDLER CLS_NAME_RCS_RESOURCE_OBJECT "$GetRequestHandler"
#define CLS_NAME_SET_REQUEST_HANDLER CLS_NAME_RCS_RESOURCE_OBJECT "$SetRequestHandler"
#define CLS_NAME_ON_ATTRIBUTE_UPDATESD_LISTENER \
    CLS_NAME_RCS_RESOURCE_OBJECT "$OnAttributeUpdatedListener"

#define CLS_NAME_RCS_RESPONSE PACKAGE_NAME "/server/RcsResponse"

#define CLS_NAME_RCS_GET_RESPONSE PACKAGE_NAME "/server/RcsGetResponse"
#define CLS_NAME_RCS_SET_RESPONSE PACKAGE_NAME "/server/RcsSetResponse"
#define CLS_NAME_RCS_SET_RESPONSE_ACCEPTANCE_METHOD CLS_NAME_RCS_SET_RESPONSE "$AcceptanceMethod"

using namespace OIC::Service;

namespace
{
    jclass g_cls_RCSRequest;
    jclass g_cls_RCSResourceObject;

    jmethodID g_ctor_RCSRequest;
    jmethodID g_ctor_RCSResourceObject;

    jmethodID g_method_GetRequestHandler_onGetRequested;
    jmethodID g_method_SetRequestHandler_onSetRequested;
    jmethodID g_method_OnAttributeUpdatedListener_onAttributeUpdated;

    jfieldID g_field_RCSResponse_mErrorCode;
    jfieldID g_field_RCSResponse_mAttrs;
    jfieldID g_field_RCSSetResponse_mAcceptanceMethod;

    jobject g_obj_RCSSetResponse_AcceptanceMethod_DEFAULT;
    jobject g_obj_RCSSetResponse_AcceptanceMethod_ACCEPT;
    jobject g_obj_RCSSetResponse_AcceptanceMethod_IGNORE;

    jobject g_obj_AutoNotifyPolicy_NEVER;
    jobject g_obj_AutoNotifyPolicy_ALWAYS;
    jobject g_obj_AutoNotifyPolicy_UPDATED;

    jobject g_obj_SetRequestHandlerPolicy_NEVER;
    jobject g_obj_SetRequestHandlerPolicy_ACCEPT;

    inline RCSResourceObject::Ptr& getResource(JNIEnv* env, jobject obj)
    {
        return getNativeHandleAs< RCSResourceObject::Ptr >(env, obj);
    }

    inline RCSResourceObject::AutoNotifyPolicy convertAutoNotifyPolicy(JNIEnv* env, jobject obj)
    {
        if (env->IsSameObject(g_obj_AutoNotifyPolicy_NEVER, obj))
        {
            return RCSResourceObject::AutoNotifyPolicy::NEVER;
        }

        if (env->IsSameObject(g_obj_AutoNotifyPolicy_ALWAYS, obj))
        {
            return RCSResourceObject::AutoNotifyPolicy::ALWAYS;
        }

        if (env->IsSameObject(g_obj_AutoNotifyPolicy_UPDATED, obj))
        {
            return RCSResourceObject::AutoNotifyPolicy::UPDATED;
        }

        throwRCSException(env, "Failed to convert AutoNotifyPolicy");
        return {};
    }

    inline jobject convertAutoNotifyPolicy(JNIEnv* env, RCSResourceObject::AutoNotifyPolicy policy)
    {
        switch(policy)
        {
            case RCSResourceObject::AutoNotifyPolicy::NEVER: return g_obj_AutoNotifyPolicy_NEVER;
            case RCSResourceObject::AutoNotifyPolicy::ALWAYS: return g_obj_AutoNotifyPolicy_ALWAYS;
            case RCSResourceObject::AutoNotifyPolicy::UPDATED: return g_obj_AutoNotifyPolicy_UPDATED;
        }

        throwRCSException(env, "Failed to convert AutoNotifyPolicy");
        return {};
    }

    inline RCSResourceObject::SetRequestHandlerPolicy convertSetRequestHandlerPolicy(JNIEnv* env,
            jobject obj)
    {
        if (env->IsSameObject(g_obj_SetRequestHandlerPolicy_NEVER, obj))
        {
            return RCSResourceObject::SetRequestHandlerPolicy::NEVER;
        }

        if (env->IsSameObject(g_obj_SetRequestHandlerPolicy_ACCEPT, obj))
        {
            return RCSResourceObject::SetRequestHandlerPolicy::ACCEPTANCE;
        }

        throwRCSException(env, "Failed to convert SetRequestHandlerPolicy");
        return {};
    }

    inline jobject convertSetRequestHandlerPolicy(JNIEnv* env,
            RCSResourceObject::SetRequestHandlerPolicy policy)
    {
        switch(policy)
        {
            case RCSResourceObject::SetRequestHandlerPolicy::NEVER:
                return g_obj_SetRequestHandlerPolicy_NEVER;
            case RCSResourceObject::SetRequestHandlerPolicy::ACCEPTANCE:
                return g_obj_SetRequestHandlerPolicy_ACCEPT;
        }

        throwRCSException(env, "Failed to convert SetRequestHandlerPolicy");
        return {};
    }


    template< typename ENV >
    inline jobject getAttrsObj(ENV& env, jobject responseObj)
    {
        return env->GetObjectField(responseObj, g_field_RCSResponse_mAttrs);
    }

    template< typename ENV >
    inline int getErrorCode(ENV& env, jobject responseObj)
    {
        return env->GetIntField(responseObj, g_field_RCSResponse_mErrorCode);
    }

    template< typename ENV >
    inline RCSSetResponse::AcceptanceMethod getAcceptanceMethod(ENV& env, jobject responseObj)
    {
        auto methodObj = env->GetObjectField(responseObj, g_field_RCSSetResponse_mAcceptanceMethod);

        if (env->IsSameObject(methodObj, g_obj_RCSSetResponse_AcceptanceMethod_IGNORE))
        {
            return RCSSetResponse::AcceptanceMethod::IGNORE;
        }

        if (env->IsSameObject(methodObj, g_obj_RCSSetResponse_AcceptanceMethod_ACCEPT))
        {
            return RCSSetResponse::AcceptanceMethod::ACCEPT;
        }

        return RCSSetResponse::AcceptanceMethod::DEFAULT;
    }

    inline jobject callHandler(ScopedEnvWrapper& env, jobject listener, jmethodID methodId,
            const RCSRequest& request, const RCSResourceAttributes& attrs)
    {
        auto requestObj = env->NewObject(g_cls_RCSRequest, g_ctor_RCSRequest,
                env->NewStringUTF(request.getResourceUri().c_str()));
        auto attrsObj = newAttributesObject(env.get(), attrs);

        return env->CallObjectMethod(listener, methodId, requestObj, attrsObj);
    }

    template< typename RESPONSE >
    inline RESPONSE createResponse(ScopedEnvWrapper& env, jobject responseObj)
    {
        auto errorCode = getErrorCode(env, responseObj);
        auto responseAttrsObj = getAttrsObj(env, responseObj);

        if (responseAttrsObj)
        {
            return RESPONSE::create(toNativeAttributes(env.get(), responseAttrsObj), errorCode);
        }

        return RESPONSE::create(errorCode);
    }

    RCSGetResponse onGetRequest(const RCSRequest& request, const RCSResourceAttributes& attrs,
            const JavaGlobalRef& listener)
    {
        ScopedEnvWrapper env;
        EXPECT_RET(env, "env is null!", RCSGetResponse::create(-1));

        try
        {
            auto responseObj = callHandler(env, listener, g_method_GetRequestHandler_onGetRequested,
                    request, attrs);

            return createResponse< RCSGetResponse >(env, responseObj);
        }
        catch (const JavaException&)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        return RCSGetResponse::create({ }, -1);
    }

    RCSSetResponse onSetRequest(const RCSRequest& request, const RCSResourceAttributes& attrs,
             const JavaGlobalRef& listener)
    {
        ScopedEnvWrapper env;
        EXPECT_RET(env, "env is null!", RCSSetResponse::create(-1));

        try
        {
            auto responseObj = callHandler(env, listener, g_method_SetRequestHandler_onSetRequested,
                               request, attrs);

            auto acceptanceMethod = getAcceptanceMethod(env, responseObj);

            return createResponse< RCSSetResponse >(env, responseObj).
                    setAcceptanceMethod(acceptanceMethod);
        }
        catch (const JavaException&)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        return RCSSetResponse::create(-1);
    }


    void onAttributeUpdated(const RCSResourceAttributes::Value& oldVal,
            const RCSResourceAttributes::Value& newVal, const JavaGlobalRef& listener)
    {
        ScopedEnvWrapper env;
        EXPECT(env, "env is null!");

        try
        {
            env->CallVoidMethod(listener, g_method_OnAttributeUpdatedListener_onAttributeUpdated,
                    newRCSValueObject(env.get(), oldVal), newRCSValueObject(env.get(), newVal));
        }
        catch (const JavaException&)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }

    void initRCSResponse(JNIEnvWrapper* env)
    {
        auto clsRCSResponse = env->FindClass(CLS_NAME_RCS_RESPONSE);

        g_field_RCSResponse_mErrorCode = env->GetFieldID(clsRCSResponse, "mErrorCode", "I");

        g_field_RCSResponse_mAttrs = env->GetFieldID(clsRCSResponse, "mAttrs",
                AS_SIG(CLS_NAME_RESOURCEATTRIBUTES));

        auto clsRCSSetResponse = env->FindClass(CLS_NAME_RCS_SET_RESPONSE);

        g_field_RCSSetResponse_mAcceptanceMethod = env->GetFieldID(clsRCSSetResponse,
                "mAcceptanceMethod", AS_SIG(CLS_NAME_RCS_SET_RESPONSE_ACCEPTANCE_METHOD));

        auto clsAcceptanceMethod = env->FindClass(CLS_NAME_RCS_SET_RESPONSE_ACCEPTANCE_METHOD);

        g_obj_RCSSetResponse_AcceptanceMethod_DEFAULT = env->NewGlobalRef(
                env->GetStaticObjectField(clsAcceptanceMethod, "DEFAULT",
                        AS_SIG(CLS_NAME_RCS_SET_RESPONSE_ACCEPTANCE_METHOD)));

        g_obj_RCSSetResponse_AcceptanceMethod_ACCEPT = env->NewGlobalRef(
                env->GetStaticObjectField(clsAcceptanceMethod, "ACCEPT",
                        AS_SIG(CLS_NAME_RCS_SET_RESPONSE_ACCEPTANCE_METHOD)));

        g_obj_RCSSetResponse_AcceptanceMethod_IGNORE = env->NewGlobalRef(
                env->GetStaticObjectField(clsAcceptanceMethod, "IGNORE",
                        AS_SIG(CLS_NAME_RCS_SET_RESPONSE_ACCEPTANCE_METHOD)));
    }
}

void initRCSResourceObject(JNIEnvWrapper* env)
{
    g_cls_RCSResourceObject = env->FindClassAsGlobalRef(CLS_NAME_RCS_RESOURCE_OBJECT);

    g_ctor_RCSResourceObject = env->GetMethodID(g_cls_RCSResourceObject, "<init>", "()V");

    g_cls_RCSRequest = env->FindClassAsGlobalRef(CLS_NAME_RCS_REQUEST);

    g_ctor_RCSRequest = env->GetMethodID(g_cls_RCSRequest, "<init>",
            "(" AS_SIG(CLS_NAME_STRING) ")V");

    auto clsGetRequestHandler = env->FindClass(CLS_NAME_GET_REQUEST_HANDLER);

    g_method_GetRequestHandler_onGetRequested = env->GetMethodID(clsGetRequestHandler,
            "onGetRequested",
            "(" AS_SIG(CLS_NAME_RCS_REQUEST) AS_SIG(CLS_NAME_RESOURCEATTRIBUTES) ")"
                AS_SIG(CLS_NAME_RCS_GET_RESPONSE));

    auto clsSetRequestHandler = env->FindClass(CLS_NAME_SET_REQUEST_HANDLER);

    g_method_SetRequestHandler_onSetRequested = env->GetMethodID(clsSetRequestHandler,
            "onSetRequested",
            "(" AS_SIG(CLS_NAME_RCS_REQUEST) AS_SIG(CLS_NAME_RESOURCEATTRIBUTES) ")"
                AS_SIG(CLS_NAME_RCS_SET_RESPONSE));

    auto clsOnAttributeUpdatedListener = env->FindClass(CLS_NAME_ON_ATTRIBUTE_UPDATESD_LISTENER);

    g_method_OnAttributeUpdatedListener_onAttributeUpdated = env->GetMethodID(
            clsOnAttributeUpdatedListener, "onAttributeUpdated",
            "(" AS_SIG(CLS_NAME_VALUE) AS_SIG(CLS_NAME_VALUE) ")V");

    auto clsAutoNotifyPolicy = env->FindClass(CLS_NAME_AUTO_NOTIFY_POLICY);

    g_obj_AutoNotifyPolicy_NEVER = env->NewGlobalRef(
            env->GetStaticObjectField(clsAutoNotifyPolicy, "NEVER",
                    AS_SIG(CLS_NAME_AUTO_NOTIFY_POLICY)));

    g_obj_AutoNotifyPolicy_ALWAYS = env->NewGlobalRef(
            env->GetStaticObjectField(clsAutoNotifyPolicy, "ALWAYS",
                    AS_SIG(CLS_NAME_AUTO_NOTIFY_POLICY)));


    g_obj_AutoNotifyPolicy_UPDATED = env->NewGlobalRef(
            env->GetStaticObjectField(clsAutoNotifyPolicy, "UPDATED",
                    AS_SIG(CLS_NAME_AUTO_NOTIFY_POLICY)));

    auto clsSetRequestHandlerPolicy = env->FindClass(CLS_NAME_SET_REQUEST_HANDLER_POLICY);

    g_obj_SetRequestHandlerPolicy_NEVER = env->NewGlobalRef(
            env->GetStaticObjectField(clsSetRequestHandlerPolicy, "NEVER",
                    AS_SIG(CLS_NAME_SET_REQUEST_HANDLER_POLICY)));

    g_obj_SetRequestHandlerPolicy_ACCEPT = env->NewGlobalRef(
            env->GetStaticObjectField(clsSetRequestHandlerPolicy, "ACCEPT",
                    AS_SIG(CLS_NAME_SET_REQUEST_HANDLER_POLICY)));

    initRCSResponse(env);
}

void clearRCSResourceObject(JNIEnvWrapper* env)
{
    env->DeleteGlobalRef(g_cls_RCSRequest);

    env->DeleteGlobalRef(g_obj_RCSSetResponse_AcceptanceMethod_DEFAULT);
    env->DeleteGlobalRef(g_obj_RCSSetResponse_AcceptanceMethod_ACCEPT);
    env->DeleteGlobalRef(g_obj_RCSSetResponse_AcceptanceMethod_IGNORE);

    env->DeleteGlobalRef(g_obj_AutoNotifyPolicy_NEVER);
    env->DeleteGlobalRef(g_obj_AutoNotifyPolicy_ALWAYS);
    env->DeleteGlobalRef(g_obj_AutoNotifyPolicy_UPDATED);

    env->DeleteGlobalRef(g_obj_SetRequestHandlerPolicy_NEVER);
    env->DeleteGlobalRef(g_obj_SetRequestHandlerPolicy_ACCEPT);
}

JNIEXPORT jint JNICALL
Java_org_iotivity_service_server_RcsResponse_nativeGetDefaultErrorCode
(JNIEnv*, jclass)
{
    return RequestHandler::DEFAULT_ERROR_CODE;
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeBuild
(JNIEnv* env, jclass, jstring uriObj, jstring typeObj, jstring interfaceObj, jboolean isObservable,
        jboolean isDiscoverable, jobject attrsObj)
{
    LOGI("nativeBuild");

    EXPECT_RET_DEF(uriObj, "uri is null.");

    auto uri = toStdString(env, uriObj);
    auto type = toStdString(env, typeObj);
    auto interface = toStdString(env, interfaceObj);

    RCSResourceAttributes attrs;
    if (attrsObj)
    {
        attrs = toNativeAttributes(env, attrsObj);
        VERIFY_NO_EXC_RET_DEF(env);
    }

    try
    {
        auto resource = RCSResourceObject::Builder(uri, type,interface).
                setDiscoverable(isDiscoverable). setObservable(isObservable).setAttributes(attrs).
                build();

        auto resourceObj = env->NewObject(g_cls_RCSResourceObject, g_ctor_RCSResourceObject);
        VERIFY_NO_EXC_RET_DEF(env);

        setSafeNativeHandle< decltype(resource) >(env, resourceObj, resource);

        return resourceObj;
    }
    catch (const RCSPlatformException& e)
    {
        LOGE("%s", e.what());
        throwPlatformException(env, e);
    }

    return nullptr;
}


JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetAttribute
(JNIEnv* env, jobject obj, jstring keyObj, jobject valueObj)
{
    LOGD("nativeSetAttributeInteger");

    EXPECT(keyObj, "key is null.");
    EXPECT(valueObj, "value is null.");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    res->setAttribute(toStdString(env, keyObj), toNativeAttrsValue(env, valueObj));
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetAttributeValue
(JNIEnv *env, jobject obj, jstring keyObj)
{
    LOGD("nativeGetAttributeValue");

    EXPECT_RET_DEF(keyObj, "key is null.");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    try
    {
        auto valueObj = newRCSValueObject(env, res->getAttributeValue(toStdString(env, keyObj)));
        VERIFY_NO_EXC_RET_DEF(env);

        return valueObj;
    }
    catch(const RCSInvalidKeyException& e)
    {
        return nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeRemoveAttribute
(JNIEnv* env, jobject obj, jstring keyObj)
{
    LOGD("nativeRemoveAttribute");

    EXPECT_RET_DEF(keyObj, "key is null.");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->removeAttribute(toStdString(env, keyObj));
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeContainsAttribute
(JNIEnv* env, jobject obj, jstring keyObj)
{
    LOGD("nativeContainsAttribute");

    EXPECT_RET_DEF(keyObj, "key is null.");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->containsAttribute(toStdString(env, keyObj));
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetAttributes
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetAttributes");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    RCSResourceObject::LockGuard lock{ res };
    return newAttributesObject(env, res->getAttributes());
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeIsObservable
(JNIEnv* env, jobject obj)
{
    LOGD("nativeIsObservable");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->isObservable();
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeIsDiscoverable
(JNIEnv *env, jobject obj)
{
    LOGD("nativeIsDiscoverable");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->isDiscoverable();
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetGetRequestHandler
(JNIEnv *env, jobject obj, jobject listenerObj)
{
    LOGD("nativeSetGetRequestHandler");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    res->setGetRequestHandler(std::bind(onGetRequest, std::placeholders::_1, std::placeholders::_2,
            JavaGlobalRef{ env, listenerObj }));
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetSetRequestHandler
(JNIEnv* env, jobject obj, jobject listenerObj)
{
    LOGD("nativeSetSetRequestHandler");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    res->setSetRequestHandler(std::bind(onSetRequest, std::placeholders::_1, std::placeholders::_2,
           JavaGlobalRef{ env, listenerObj }));
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeAddAttributeUpdatedListener
(JNIEnv* env, jobject obj, jstring keyObj, jobject listenerObj)
{
    LOGD("nativeAddAttributeUpdatedListener");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    auto key = toStdString(env, keyObj);
    VERIFY_NO_EXC(env);

    res->addAttributeUpdatedListener(std::move(key), std::bind(onAttributeUpdated,
            std::placeholders::_1, std::placeholders::_2, JavaGlobalRef{ env, listenerObj }));
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeRemoveAttributeUpdatedListener
(JNIEnv* env, jobject obj, jstring keyObj)
{
    LOGD("nativeAddAttributeUpdatedListener");

    EXPECT_RET_DEF(keyObj, "key is null");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    auto key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->removeAttributeUpdatedListener(key);
}

JNIEXPORT void JNICALL Java_org_iotivity_service_server_RcsResourceObject_nativeNotify
(JNIEnv* env, jobject obj)
{
    LOGD("nativeNotify");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    try
    {
        res->notify();
    }
    catch (const RCSPlatformException& e) {
        throwPlatformException(env, e);
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetAutoNotifyPolicy
(JNIEnv* env, jobject obj, jobject policyObj)
{
    LOGD("nativeSetAutoNotifyPolicy");

    EXPECT(policyObj, "policyObj is null");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    auto policy = convertAutoNotifyPolicy(env, policyObj);
    VERIFY_NO_EXC(env);

    res->setAutoNotifyPolicy(policy);
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetAutoNotifyPolicy
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetAutoNotifyPolicy");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return convertAutoNotifyPolicy(env, res->getAutoNotifyPolicy());
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeSetSetRequestHandlerPolicy
(JNIEnv* env, jobject obj, jobject policyObj)
{
    LOGD("nativeSetSetRequestHandlerPolicy");

    EXPECT(policyObj, "policyObj is null");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC(env);

    auto policy = convertSetRequestHandlerPolicy(env, policyObj);
    VERIFY_NO_EXC(env);

    res->setSetRequestHandlerPolicy(policy);
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsResourceObject_nativeGetSetRequestHandlerPolicy
(JNIEnv* env, jobject obj)
{
    LOGD("nativeGetSetRequestHandlerPolicy");

    auto res = getResource(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return convertSetRequestHandlerPolicy(env, res->getSetRequestHandlerPolicy());
}
