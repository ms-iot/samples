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
#include "OCResourceResponse.h"

#ifndef _Included_org_iotivity_base_OcResourceResponse
#define _Included_org_iotivity_base_OcResourceResponse

using namespace OC;

class JniOcResourceResponse
{
public:
    JniOcResourceResponse(std::shared_ptr<OCResourceResponse> resourceResponse);
    ~JniOcResourceResponse();
    void setErrorCode(const int eCode);
    std::string getNewResourceUri(void);
    void setNewResourceUri(const std::string newResourceUri);
    void setHeaderOptions(const HeaderOptions& headerOptions);
    void setRequestHandle(const OCRequestHandle& requestHandle);
    void setResourceHandle(const OCResourceHandle& resourceHandle);
    void setResponseResult(const OCEntityHandlerResult& responseResult);
    void setResourceRepresentation(OCRepresentation& rep, std::string interfaceStr);
    void setResourceRepresentation(OCRepresentation& rep);
    std::shared_ptr<OCResourceResponse> getOCResourceResponse();
    static JniOcResourceResponse* getJniOcResourceResponsePtr(JNIEnv *env, jobject thiz);
private:
    std::shared_ptr<OCResourceResponse> m_response;
};

#ifdef __cplusplus
extern "C" {
#endif

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setErrorCode
    * Signature: (I)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setErrorCode
        (JNIEnv *, jobject, jint);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    getNewResourceUri
    * Signature: ()Ljava/lang/String;
    */
    JNIEXPORT jstring JNICALL Java_org_iotivity_base_OcResourceResponse_getNewResourceUri
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setNewResourceUri
    * Signature: (Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setNewResourceUri
        (JNIEnv *, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setHeaderOptions
    * Signature: (Ljava/util/List;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setHeaderOptions
        (JNIEnv *, jobject, jobjectArray);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setRequestHandle
    * Signature: (Lorg/iotivity/base/OcRequestHandle;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setRequestHandle
        (JNIEnv *, jobject, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setResourceHandle
    * Signature: (Lorg/iotivity/base/OcResourceHandle;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResourceHandle
        (JNIEnv *, jobject, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setResponseResult
    * Signature: (I)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResponseResult
        (JNIEnv *, jobject, jint);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setResourceRepresentation
    * Signature: (Lorg/iotivity/base/OcRepresentation;Ljava/lang/String;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResourceRepresentation
        (JNIEnv *, jobject, jobject, jstring);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    setResourceRepresentation1
    * Signature: (Lorg/iotivity/base/OcRepresentation;)V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_setResourceRepresentation1
        (JNIEnv *, jobject, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    create
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_create
        (JNIEnv *, jobject);

    /*
    * Class:     org_iotivity_base_OcResourceResponse
    * Method:    dispose
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceResponse_dispose
        (JNIEnv *, jobject);
#ifdef __cplusplus
}
#endif
#endif