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
#include "JniOcRequestHandle.h"

JniOcRequestHandle::JniOcRequestHandle(OCRequestHandle requestHandle) : m_requestHandle(requestHandle)
{}

JniOcRequestHandle::~JniOcRequestHandle()
{
    LOGD("~JniOcRequestHandle()");
}

JniOcRequestHandle* JniOcRequestHandle::getJniOcRequestHandlePtr
(JNIEnv *env, jobject thiz)
{
    JniOcRequestHandle *handle = GetHandle<JniOcRequestHandle>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcRequestHandle");
    }
    if (!handle)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return handle;
}

OCRequestHandle JniOcRequestHandle::getOCRequestHandle()
{
    return this->m_requestHandle;
}

/*
* Class:     org_iotivity_base_OcRequestHandle
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcRequestHandle_dispose
(JNIEnv *env, jobject thiz)
{
    LOGD("OcRequestHandle_dispose");
    JniOcRequestHandle *handle = JniOcRequestHandle::getJniOcRequestHandlePtr(env, thiz);
    delete handle;
}