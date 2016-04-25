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

#include "JniOcResourceIdentifier.h"

JniOcResourceIdentifier::JniOcResourceIdentifier(OC::OCResourceIdentifier resourceIdentifier)
    : m_resourceIdentifier(resourceIdentifier)
{}

JniOcResourceIdentifier::~JniOcResourceIdentifier()
{
    LOGD("~JniOcResourceIdentifier()");
}

OC::OCResourceIdentifier JniOcResourceIdentifier::getOCResourceIdentifier()
{
    return this->m_resourceIdentifier;
}

JniOcResourceIdentifier* JniOcResourceIdentifier::getJniOcResourceIdentifierPtr
(JNIEnv *env, jobject thiz)
{
    JniOcResourceIdentifier *identifier = GetHandle<JniOcResourceIdentifier>(env, thiz);
    if (env->ExceptionCheck())
    {
        LOGE("Failed to get native handle from OcResourceIdentifier");
    }
    if (!identifier)
    {
        ThrowOcException(JNI_NO_NATIVE_POINTER, "");
    }
    return identifier;
}

/*
* Class:     org_iotivity_base_OcResourceIdentifier
* Method:    equalsN
* Signature: (Lorg/iotivity/base/OcResourceIdentifier;)Z
*/
JNIEXPORT jboolean JNICALL Java_org_iotivity_base_OcResourceIdentifier_equalsN
(JNIEnv *env, jobject jThiz, jobject jOther)
{
    JniOcResourceIdentifier *thiz = JniOcResourceIdentifier::getJniOcResourceIdentifierPtr(env, jThiz);
    if (!thiz) return false;

    JniOcResourceIdentifier *other = JniOcResourceIdentifier::getJniOcResourceIdentifierPtr(env, jOther);
    if (!other) return false;

    if (thiz->getOCResourceIdentifier() == other->getOCResourceIdentifier())
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
* Class:     org_iotivity_base_OcResourceIdentifier
* Method:    dispose
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_org_iotivity_base_OcResourceIdentifier_dispose
(JNIEnv *env, jobject thiz)
{
    LOGD("JniOcResourceIdentifier_dispose");
    JniOcResourceIdentifier *identifier = JniOcResourceIdentifier::getJniOcResourceIdentifierPtr(env, thiz);
    delete identifier;
}