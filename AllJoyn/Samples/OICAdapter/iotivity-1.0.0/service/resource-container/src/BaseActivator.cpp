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
#include "ResourceContainerImpl.h"
#if(JAVA_SUPPORT)
#include <jni.h>
#include "org_iotivity_resourcecontainer_bundle_api_BaseActivator.h"
#include "JavaBundleResource.h"

using namespace OIC::Service;

std::map< string, BundleResource::Ptr > java_resources;

/*
 * Class:     org_iotivity_resourcecontainer_bundle_api_BaseActivator
 * Method:    registerJavaResource
 * Signature: (Lorg/iotivity/resourcecontainer/bundle/api/BundleResource;[Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_resourcecontainer_bundle_api_BaseActivator_registerJavaResource
(JNIEnv *env, jobject obj, jobject bundleResource, jobjectArray attributes, jstring bundleId,
 jstring uri, jstring resourceType, jstring res_name)
{
    const char *str_bundleId = env->GetStringUTFChars(bundleId, 0);
    const char *str_uri = env->GetStringUTFChars(uri, 0);
    const char *str_resourceType = env->GetStringUTFChars(resourceType, 0);
    const char *str_res_name = env->GetStringUTFChars(res_name, 0);

    BundleResource::Ptr javaBundleResource = std::make_shared< JavaBundleResource >
            (env, obj, bundleResource, str_bundleId, attributes);
    ResourceContainerImpl *container = ResourceContainerImpl::getImplInstance();

    javaBundleResource->m_uri = string(str_uri, strlen(str_uri));
    javaBundleResource->m_resourceType = string(str_resourceType, strlen(str_resourceType));
    javaBundleResource->m_name = string(str_res_name, strlen(str_res_name));
    container->registerResource(javaBundleResource);

    java_resources[str_uri] = javaBundleResource;
}

/*
 * Class:     org_iotivity_resourcecontainer_bundle_api_BaseActivator
 * Method:    unregisterJavaResource
 * Signature: (Lorg/iotivity/resourcecontainer/bundle/api/BundleResource;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_resourcecontainer_bundle_api_BaseActivator_unregisterJavaResource
(JNIEnv *env, jobject obj, jobject bundleResource, jstring uri)
{
    (void)obj;
    (void)bundleResource;
    const char *str_uri = env->GetStringUTFChars(uri, 0);

    if (java_resources[str_uri] != NULL)
    {
        ResourceContainerImpl *container = ResourceContainerImpl::getImplInstance();
        container->unregisterResource(java_resources[str_uri]);
        java_resources.erase(str_uri);
    }
}

/*
 * Class:     org_iotivity_resourcecontainer_bundle_api_BaseActivator
 * Method:    getNumberOfConfiguredResources
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_org_iotivity_resourcecontainer_bundle_api_BaseActivator_getNumberOfConfiguredResources(
    JNIEnv *env, jobject obj, jstring bundleId)
{
    (void)obj;
    const char *str_bundleId = env->GetStringUTFChars(bundleId, 0);

    ResourceContainerImpl *container = ResourceContainerImpl::getImplInstance();
    vector< resourceInfo > resourceConfig;
    container->getResourceConfiguration(str_bundleId, &resourceConfig);

    return resourceConfig.size();
}

/*
 * Class:     org_iotivity_resourcecontainer_bundle_api_BaseActivator
 * Method:    getConfiguredResourceParams
 * Signature: (Ljava/lang/String;I)[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL
Java_org_iotivity_resourcecontainer_bundle_api_BaseActivator_getConfiguredResourceParams(
    JNIEnv *env, jobject obj, jstring bundleId, jint resourceId)
{
    (void)obj;
    jobjectArray ret;
    const char *str_bundleId = env->GetStringUTFChars(bundleId, 0);

    ResourceContainerImpl *container = ResourceContainerImpl::getImplInstance();
    vector< resourceInfo > resourceConfig;
    container->getResourceConfiguration(str_bundleId, &resourceConfig);
    resourceInfo conf = resourceConfig[resourceId];
    ret = (jobjectArray) env->NewObjectArray(4, env->FindClass("java/lang/String"),
            env->NewStringUTF(""));

    env->SetObjectArrayElement(ret, 0, env->NewStringUTF(conf.name.c_str()));
    env->SetObjectArrayElement(ret, 1, env->NewStringUTF(conf.uri.c_str()));
    env->SetObjectArrayElement(ret, 2, env->NewStringUTF(conf.resourceType.c_str()));
    env->SetObjectArrayElement(ret, 3, env->NewStringUTF(conf.address.c_str()));
    return ret;
}
#endif