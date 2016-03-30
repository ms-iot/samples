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
#include "JniOcResourceHandle.h"

using namespace OC;

JniOcResourceHandle::JniOcResourceHandle(OCResourceHandle resourceHandle)
    : m_resourceHandle(resourceHandle)
{}

JniOcResourceHandle::~JniOcResourceHandle()
{
    LOGD("~JniOcResourceHandle()");
}

JniOcResourceHandle* JniOcResourceHandle::getJniOcResourceHandlePtr
(JNIEnv *env, jobject thiz)
{
    JniOcResourceHandle *handle = GetHandle<JniOcResourceHandle>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcResourceHandle");
    }
    if (!handle)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return handle;
}

OCResourceHandle JniOcResourceHandle::getOCResourceHandle()
{
    return this->m_resourceHandle;
}

/*
* Class:     org_iotivity_base_OcResourceHandle
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceHandle_dispose
(JNIEnv *env, jobject thiz)
{
    LOGD("OcResourceHandle_dispose");
    JniOcResourceHandle *resourceHandle = JniOcResourceHandle::getJniOcResourceHandlePtr(env, thiz);
    delete resourceHandle;
}