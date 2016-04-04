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
#include "JniOcResourceResponse.h"
#include "OCResourceResponse.h"
#include "JniOcRepresentation.h"
#include "JniOcRequestHandle.h"
#include "JniOcResourceHandle.h"
#include "JniUtils.h"

using namespace OC;

JniOcResourceResponse::JniOcResourceResponse
(std::shared_ptr<OCResourceResponse> resourceResponse)
: m_response(resourceResponse){}

JniOcResourceResponse::~JniOcResourceResponse()
{}

void JniOcResourceResponse::setErrorCode(const int eCode)
{
    this->m_response->setErrorCode(eCode);
}

std::string JniOcResourceResponse::getNewResourceUri(void)
{
    this->m_response->getNewResourceUri();
}

void
JniOcResourceResponse::setNewResourceUri(const std::string newResourceUri)
{
    this->m_response->setNewResourceUri(newResourceUri);
}

void JniOcResourceResponse::setHeaderOptions(const HeaderOptions& headerOptions)
{
    this->m_response->setHeaderOptions(headerOptions);
}

void JniOcResourceResponse::setRequestHandle(const OCRequestHandle& requestHandle)
{
    this->m_response->setRequestHandle(requestHandle);
}

void JniOcResourceResponse::setResourceHandle(const OCResourceHandle& resourceHandle)
{
    this->m_response->setResourceHandle(resourceHandle);
}

void JniOcResourceResponse::setResponseResult(const OCEntityHandlerResult& responseResult)
{
    this->m_response->setResponseResult(responseResult);
}

void JniOcResourceResponse::setResourceRepresentation(OCRepresentation& rep,
    std::string interfaceStr)
{
    this->m_response->setResourceRepresentation(rep, interfaceStr);
}

void JniOcResourceResponse::setResourceRepresentation(OCRepresentation& rep)
{
    this->m_response->setResourceRepresentation(rep);
}

std::shared_ptr<OCResourceResponse> JniOcResourceResponse::getOCResourceResponse()
{
    return this->m_response;
}

JniOcResourceResponse* JniOcResourceResponse::getJniOcResourceResponsePtr
(JNIEnv *env, jobject thiz)
{
    JniOcResourceResponse *request = GetHandle<JniOcResourceResponse>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from JniOcResourceResponse");
    }
    if (!request)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return request;
}
/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setErrorCode
* Signature: (I)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setErrorCode
(JNIEnv *env, jobject thiz, jint eCode)
{
    LOGD("OcResourceResponse_setErrorCode");
    JniOcResourceResponse *response = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!response) return;

    response->setErrorCode(static_cast<int>(eCode));
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    getNewResourceUri
* Signature: ()Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcResourceResponse_getNewResourceUri
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceResponse_getNewResourceUri");
    JniOcResourceResponse *response = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!response) return nullptr;

    return env->NewStringUTF(response->getNewResourceUri().c_str());
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setNewResourceUri
* Signature: (Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setNewResourceUri
(JNIEnv *env, jobject thiz, jstring jstr)
{
    LOGD("OcResourceResponse_setNewResourceUri");
    JniOcResourceResponse *response = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!response) return;

    response->setNewResourceUri(env->GetStringUTFChars(jstr, 0));
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setHeaderOptions
* Signature: (Ljava/util/List;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setHeaderOptions
(JNIEnv *env, jobject thiz, jobjectArray jHeaderOptions)
{
    LOGD("OcResourceResponse_setHeaderOptions");
    if (!jHeaderOptions)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "headerOptionList cannot be null");
        return;
    }
    JniOcResourceResponse *jniResponse = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!jniResponse) return;

    HeaderOptions headerOptions;
    JniUtils::convertJavaHeaderOptionsArrToVector(env, jHeaderOptions, headerOptions);

    jniResponse->setHeaderOptions(headerOptions);
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setRequestHandle
* Signature: (Lorg/iotivity/base/OcRequestHandle;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setRequestHandle
(JNIEnv *env, jobject thiz, jobject jRequestHandle)
{
    LOGI("OcResourceResponse_setRequestHandle");
    if (!jRequestHandle)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "requestHandle cannot be null");
        return;
    }
    JniOcResourceResponse *jniResponse = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!jniResponse) return;

    JniOcRequestHandle* jniOcRequestHandle = JniOcRequestHandle::getJniOcRequestHandlePtr(env, jRequestHandle);
    if (!jniOcRequestHandle) return;

    jniResponse->setRequestHandle(jniOcRequestHandle->getOCRequestHandle());
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setResourceHandle
* Signature: (Lorg/iotivity/base/OcResourceHandle;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResourceHandle
(JNIEnv *env, jobject thiz, jobject jResourceHandle)
{
    LOGI("OcResourceResponse_setResourceHandle");
    if (!jResourceHandle)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "resourceHandle cannot be null");
        return;
    }
    JniOcResourceResponse *jniResponse = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!jniResponse) return;

    JniOcResourceHandle* jniOcResourceHandle = JniOcResourceHandle::getJniOcResourceHandlePtr(env, jResourceHandle);
    if (!jniOcResourceHandle) return;

    jniResponse->setResourceHandle(jniOcResourceHandle->getOCResourceHandle());
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setResponseResult
* Signature: (I)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResponseResult
(JNIEnv *env, jobject thiz, jint responseResult)
{
    LOGD("OcResourceResponse_setResponseResult");
    JniOcResourceResponse *response = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!response) return;

    response->setResponseResult(JniUtils::getOCEntityHandlerResult(env, static_cast<int>(responseResult)));
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setResourceRepresentation
* Signature: (Lorg/iotivity/base/OcRepresentation;Ljava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResourceRepresentation
(JNIEnv *env, jobject thiz, jobject jRepresentation, jstring jstr)
{
    LOGD("OcResourceResponse_setResourceRepresentation");
    if (!jRepresentation)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "Representation cannot be null");
        return;
    }
    if (!jstr)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "interface cannot be null");
        return;
    }

    JniOcResourceResponse *response = JniOcResourceResponse::getJniOcResourceResponsePtr(env,
        thiz);
    if (!response) return;

    OCRepresentation *representation = JniOcRepresentation::getOCRepresentationPtr(env,
        jRepresentation);
    if (!representation) return;

    response->setResourceRepresentation(*representation, env->GetStringUTFChars(jstr, 0));
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    setResourceRepresentation1
* Signature: (Lorg/iotivity/base/OcRepresentation;)V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResourceRepresentation1
(JNIEnv *env, jobject thiz, jobject jRepresentation)
{
    LOGD("OcResourceResponse_setResourceRepresentation");
    if (!jRepresentation)
    {
        ThrowOcException(OC_STACK_INVALID_PARAM, "Representation cannot be null");
        return;
    }
    JniOcResourceResponse *response = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    if (!response) return;

    OCRepresentation *representation = JniOcRepresentation::getOCRepresentationPtr(env,
        jRepresentation);

    if (representation)
    {
        response->setResourceRepresentation(*representation);
    }
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    create
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_create
(JNIEnv *env, jobject thiz)
{
    LOGI("OcResourceResponse_create");
    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    JniOcResourceResponse* jniResourceResponse = new JniOcResourceResponse(pResponse);
    SetHandle<JniOcResourceResponse>(env, thiz, jniResourceResponse);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to create OcResourceResponse");
        delete jniResourceResponse;
    }
}

/*
* Class:     org_iotivity_base_OcResourceResponse
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_dispose
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceResponse_dispose");
    JniOcResourceResponse *resp = JniOcResourceResponse::getJniOcResourceResponsePtr(env, thiz);
    delete resp;
}