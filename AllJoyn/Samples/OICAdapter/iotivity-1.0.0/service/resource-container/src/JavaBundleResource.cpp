//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


#if(JAVA_SUPPORT)
#include "JavaBundleResource.h"

#include <jni.h>
#include <string.h>
#include <iostream>
#include "InternalTypes.h"

using namespace OIC::Service;
using namespace std;


JavaBundleResource::JavaBundleResource()
{

}

void JavaBundleResource::initAttributes()
{

}

JavaBundleResource::JavaBundleResource(JNIEnv *env, jobject obj, jobject bundleResource,
                                       string bundleId, jobjectArray attributes)
{
    (void) obj;
    int stringCount = env->GetArrayLength(attributes);

    for (int i = 0; i < stringCount; i++)
    {
        jstring str = (jstring) env->GetObjectArrayElement(attributes, i);
        const char *rawString = env->GetStringUTFChars(str, 0);
        string s(rawString, strlen(rawString));
        BundleResource::setAttribute(s, "");
    }

    m_bundleId = bundleId;

    this->m_bundleResource = env->NewGlobalRef(bundleResource);

    m_bundleResourceClass = env->GetObjectClass(bundleResource);

    m_attributeSetRequestHandler = env->GetMethodID(m_bundleResourceClass,
            "handleSetAttributeRequest", "(Ljava/lang/String;Ljava/lang/String;)V");

    m_attributeGetRequestHandler = env->GetMethodID(m_bundleResourceClass,
            "handleGetAttributeRequest", "(Ljava/lang/String;)Ljava/lang/String;");

}

JavaBundleResource::~JavaBundleResource()
{

}

RCSResourceAttributes::Value JavaBundleResource::handleGetAttributeRequest(
        const std::string &attributeName)
{
    JavaVM *vm = ResourceContainerImpl::getImplInstance()->getJavaVM(m_bundleId);

    JNIEnv *env;
    int envStat = vm->GetEnv((void **) &env, JNI_VERSION_1_4);

    if (envStat == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread((void **) &env, NULL) != 0)
        {
            OC_LOG_V(ERROR, CONTAINER_TAG,
                    "[JavaBundleResource::handleGetAttributeRequest] Failed to attach ");
        }
    }
    else if (envStat == JNI_EVERSION)
    {
        OC_LOG_V(ERROR, CONTAINER_TAG,
                "[JavaBundleResource::handleGetAttributeRequest] Env: version not supported");
    }

    jstring attrName = env->NewStringUTF(attributeName.c_str());

    jstring returnString = (jstring) env->CallObjectMethod(m_bundleResource,
            m_attributeGetRequestHandler, attrName);

    const char *js = env->GetStringUTFChars(returnString, NULL);
    std::string val(js);
    RCSResourceAttributes::Value newVal = val;
    env->ReleaseStringUTFChars(returnString, js);
    BundleResource::setAttribute(attributeName, newVal.toString());
    return BundleResource::getAttribute(attributeName);
}

void JavaBundleResource::handleSetAttributeRequest(const std::string &attributeName,
                                      RCSResourceAttributes::Value &&value)
{
    JavaVM *vm = ResourceContainerImpl::getImplInstance()->getJavaVM(m_bundleId);

    JNIEnv *env;
    int envStat = vm->GetEnv((void **) &env, JNI_VERSION_1_4);

    if (envStat == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread((void **) &env, NULL) != 0)
        {
            OC_LOG_V(ERROR, CONTAINER_TAG,
                    "[JavaBundleResource::handleSetAttributeRequest] Failed to attach ");
        }
    }
    else if (envStat == JNI_EVERSION)
    {
        OC_LOG_V(ERROR, CONTAINER_TAG,
                "[JavaBundleResource::handleSetAttributeRequest] Env: version not supported ");
    }

    jstring attrName = env->NewStringUTF(attributeName.c_str());
    jstring val = env->NewStringUTF(value.toString().c_str());


    env->CallObjectMethod(m_bundleResource, m_attributeSetRequestHandler, attrName, val);
    BundleResource::setAttribute(attributeName, std::move(value));
}


void JavaBundleResource::handleSetAttributesRequest(RCSResourceAttributes &attrs){
    for (RCSResourceAttributes::iterator it = attrs.begin(); it != attrs.end(); ++it)
    {
        handleSetAttributeRequest(it->key(),std::move(it->value()));
    }
}

RCSResourceAttributes & JavaBundleResource::handleGetAttributesRequest()
{
    std::list<string> attrsNames = getAttributeNames();
    for(std::list<string>::iterator iterator = attrsNames.begin();
            iterator != attrsNames.end(); ++iterator )
    {
        handleGetAttributeRequest(*iterator);
    }
    return BundleResource::getAttributes();
}
#endif
