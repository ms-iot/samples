/*
* //******************************************************************
* //
* // Copyright 2015 Intel Corporation.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* //
* // Licensed under the Apache License, Version 2.0 (the "License");
* // you may not use this file except in compliance with the License.
* // You may obtain a copy of the License at
* //
* //      http://www.apache.org/licenses/LICENSE-2.0
* //
* // Unless required by applicable law or agreed to in writing, software
* // distributed under the License is distributed on an "AS IS" BASIS,
* // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* // See the License for the specific language governing permissions and
* // limitations under the License.
* //
* //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
#include "JniOcResourceRequest.h"
#include "OCResourceRequest.h"
#include "JniOcResourceHandle.h"
#include "JniOcRequestHandle.h"
#include "JniUtils.h"

using namespace OC;

JniOcResourceRequest::JniOcResourceRequest(const std::shared_ptr<OCResourceRequest> request)
    : m_request(request)
{}

JniOcResourceRequest::~JniOcResourceRequest()
{}

std::string
JniOcResourceRequest::getRequestType()
{
    return m_request->getRequestType();
}

const QueryParamsMap&
JniOcResourceRequest::getQueryParameters() const
{
    return m_request->getQueryParameters();
}

int
JniOcResourceRequest::getRequestHandlerFlag() const
{
    return m_request->getRequestHandlerFlag();
}

const OCRepresentation&
JniOcResourceRequest::getResourceRepresentation() const
{
    return m_request->getResourceRepresentation();
}

const ObservationInfo&
JniOcResourceRequest::getObservationInfo() const
{
    return m_request->getObservationInfo();
}

void
JniOcResourceRequest::setResourceUri(const std::string resourceUri)
{
    m_request->setResourceUri(resourceUri);
}

std::string
JniOcResourceRequest::getResourceUri(void)
{
    return m_request->getResourceUri();
}

const HeaderOptions&
JniOcResourceRequest::getHeaderOptions() const
{
    return m_request->getHeaderOptions();
}

const OCRequestHandle&
JniOcResourceRequest::getRequestHandle() const
{
    return m_request->getRequestHandle();
}

const OCResourceHandle&
JniOcResourceRequest::getResourceHandle() const
{
    return m_request->getResourceHandle();
}

JniOcResourceRequest* JniOcResourceRequest::getJniOcResourceRequestPtr
(JNIEnv *env, jobject thiz)
{
    JniOcResourceRequest *request = GetHandle<JniOcResourceRequest>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcResourceRequest");
    }
    if (!request)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return request;
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getRequestTypeNative
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcResourceRequest_getRequestTypeNative
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getRequestTypeNative");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    std::string requestType = request->getRequestType();
    return env->NewStringUTF(requestType.c_str());
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getQueryParameters
* Signature: ()Ljava/util/Map;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getQueryParameters
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getQueryParameters");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    return JniUtils::convertQueryParamsMapToJavaMap(env, request->getQueryParameters());
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getRequestHandlerFlagNative
* Signature: ()I
*/
JNIEXPORT jint JNICALL Java_org_iotivity_base_OcResourceRequest_getRequestHandlerFlagNative
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getRequestHandlerFlagNative");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return -1;

    return static_cast<jint>(request->getRequestHandlerFlag());
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getResourceRepresentation
* Signature: ()Lorg/iotivity/base/OcRepresentation;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getResourceRepresentation
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getResourceRepresentation");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    OCRepresentation *ocRepresentation = new OCRepresentation(request->getResourceRepresentation());
    if (!ocRepresentation) return nullptr;

    jlong handle = reinterpret_cast<jlong>(ocRepresentation);
    jobject jRepresentation = env->NewObject(g_cls_OcRepresentation, g_mid_OcRepresentation_N_ctor_bool,
        handle, true);
    if (!jRepresentation)
    {
        LOGE("Failed to create OcRepresentation");
        delete ocRepresentation;
    }

    return jRepresentation;
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getObservationInfo
* Signature: ()Lorg/iotivity/base/ObservationInfo;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getObservationInfo
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getObservationInfo");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    ObservationInfo oInfo = request->getObservationInfo();

    jobject jObservationInfo = env->NewObject(g_cls_ObservationInfo, g_mid_ObservationInfo_N_ctor,
        (jint)oInfo.action, (jbyte)oInfo.obsId);

    if (!jObservationInfo)
    {
        LOGE("Failed to create ObservationInfo");
    }

    return jObservationInfo;
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    setResourceUri
* Signature: (Ljava/lang/String);
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceRequest_setResourceUri
(JNIEnv *env, jobject thiz, jstring jUri)
{
    LOGD("OcResourceRequest_setResourceUri");
    if (!jUri)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "URI cannot be null");
        return;
    }
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return;

    request->setResourceUri(env->GetStringUTFChars(jUri, 0));
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getResourceUri
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcResourceRequest_getResourceUri
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getResourceUri");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    std::string requestUri = request->getResourceUri();
    return env->NewStringUTF(requestUri.c_str());
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getHeaderOptions
* Signature: ()Ljava/util/List;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getHeaderOptions
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getHeaderOptions");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    return JniUtils::convertHeaderOptionsVectorToJavaList(env, request->getHeaderOptions());
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getRequestHandle
* Signature: ()Lorg/iotivity/base/OcRequestHandle;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getRequestHandle
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getRequestHandle");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    JniOcRequestHandle* jniHandle = new JniOcRequestHandle(request->getRequestHandle());
    jlong handle = reinterpret_cast<jlong>(jniHandle);
    jobject jRequestHandle = env->NewObject(g_cls_OcRequestHandle, g_mid_OcRequestHandle_N_ctor, handle);
    if (!jRequestHandle)
    {
        LOGE("Failed to create OcRequestHandle");
        delete jniHandle;
    }

    return jRequestHandle;
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    getResourceHandle
* Signature: ()Lorg/iotivity/base/OcResourceHandle;
*/
JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getResourceHandle
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_getResourceHandle");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    if (!request) return nullptr;

    JniOcResourceHandle* jniHandle = new JniOcResourceHandle(
        request->getResourceHandle());
    jlong handle = reinterpret_cast<jlong>(jniHandle);
    jobject jResourceHandle = env->NewObject(g_cls_OcResourceHandle, g_mid_OcResourceHandle_N_ctor, handle);
    if (!jResourceHandle)
    {
        LOGE("Failed to create OcResourceHandle");
        delete jniHandle;
    }

    return jResourceHandle;
}

/*
* Class:     org_iotivity_base_OcResourceRequest
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceRequest_dispose
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceRequest_dispose");
    JniOcResourceRequest *request = JniOcResourceRequest::getJniOcResourceRequestPtr(env, thiz);
    delete request;
}