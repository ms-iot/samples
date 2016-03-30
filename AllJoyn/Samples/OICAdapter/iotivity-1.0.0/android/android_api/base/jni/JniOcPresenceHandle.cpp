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
#include "JniOcPresenceHandle.h"
#include "OCPlatform.h"

JniOcPresenceHandle::JniOcPresenceHandle(JniOnPresenceListener* jniListener, OCPresenceHandle presenceHandle)
    : m_jniListener(jniListener), m_presenceHandle(presenceHandle)
{}

JniOcPresenceHandle::~JniOcPresenceHandle()
{
    LOGD("~JniOcPresenceHandle()");

    //delete m_jniListener;
    m_presenceHandle = nullptr;
}

JniOcPresenceHandle* JniOcPresenceHandle::getJniOcPresenceHandlePtr
(JNIEnv *env, jobject thiz)
{
    JniOcPresenceHandle *handle = GetHandle<JniOcPresenceHandle>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcPresenceHandle");
    }
    if (!handle)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return handle;
}

OCPresenceHandle JniOcPresenceHandle::getOCPresenceHandle()
{
    return this->m_presenceHandle;
}

JniOnPresenceListener* JniOcPresenceHandle::getJniOnPresenceListener()
{
    return this->m_jniListener;
}

/*
* Class:     org_iotivity_base_OcPresenceHandle
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcPresenceHandle_dispose
(JNIEnv *env, jobject thiz)
{
    LOGD("OcPresenceHandle_dispose");
    JniOcPresenceHandle *presenceHandle = JniOcPresenceHandle::getJniOcPresenceHandlePtr(env, thiz);
    delete presenceHandle;
}