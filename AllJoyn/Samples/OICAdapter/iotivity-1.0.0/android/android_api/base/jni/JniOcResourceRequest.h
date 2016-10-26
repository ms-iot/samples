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
#include "JniOcStack.h"

#ifndef _Included_org_iotivity_base_OcResourceRequest
#define _Included_org_iotivity_base_OcResourceRequest

using namespace OC;

class JniOcResourceRequest
{
public:
    JniOcResourceRequest(const std::shared_ptr<OCResourceRequest> resourceRequest);

    ~JniOcResourceRequest();

    std::string getRequestType();

    const QueryParamsMap& getQueryParameters() const;

    int getRequestHandlerFlag() const;

    const OCRepresentation& getResourceRepresentation() const;

    const ObservationInfo& getObservationInfo() const;

    void setResourceUri(const std::string resourceUri);

    std::string getResourceUri(void);

    const HeaderOptions& getHeaderOptions() const;

    const OCRequestHandle& getRequestHandle() const;

    const OCResourceHandle& getResourceHandle() const;

    static JniOcResourceRequest* getJniOcResourceRequestPtr
        (JNIEnv *env, jobject thiz);

private:
    std::shared_ptr<OCResourceRequest> m_request;
};

#ifdef __cplusplus
extern "C" {
#endif

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getRequestTypeNative
    * Signature: ()Ljava/lang/String;
    */
    JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcResourceRequest_getRequestTypeNative
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getQueryParameters
    * Signature: ()Ljava/util/Map;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getQueryParameters
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getRequestHandlerFlagNative
    * Signature: ()I
    */
    JNIEXPORT jint JNICALL Java_org_iotivity_base_OcResourceRequest_getRequestHandlerFlagNative
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getResourceRepresentation
    * Signature: ()Lorg/iotivity/base/OcRepresentation;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getResourceRepresentation
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getObservationInfo
    * Signature: ()Lorg/iotivity/base/ObservationInfo;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getObservationInfo
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    setResourceUri
    * Signature: (Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceRequest_setResourceUri
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getResourceUri
    * Signature: ()Ljava/lang/String;
    */
    JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcResourceRequest_getResourceUri
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getHeaderOptions
    * Signature: ()Ljava/util/List;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getHeaderOptions
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getRequestHandle
    * Signature: ()Lorg/iotivity/base/OcRequestHandle;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getRequestHandle
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    getResourceHandle
    * Signature: ()Lorg/iotivity/base/OcResourceHandle;
    */
    JNIEXPORT jobject JNICALL Java_org_iotivity_base_OcResourceRequest_getResourceHandle
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceRequest
    * Method:    dispose
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceRequest_dispose
        (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif