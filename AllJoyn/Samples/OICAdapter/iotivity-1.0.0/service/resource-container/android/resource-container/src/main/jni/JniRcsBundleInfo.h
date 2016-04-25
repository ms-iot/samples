/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/** @file   jni_re_rcs_bundle_info.h
 *
 * @brief This file contains the JniBundleInfo class
 *               & declaration of RCSBundleInfo APIs for JNI implementation
 */

#ifndef JNI_RCS_BUNDLE_INFO_H_
#define JNI_RCS_BUNDLE_INFO_H_

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetID
(JNIEnv *, jobject);

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetPath
(JNIEnv *, jobject);

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetActivatorName
(JNIEnv *, jobject);

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetLibraryPath
(JNIEnv *, jobject);

JNIEXPORT jstring JNICALL
Java_org_iotivity_service_resourcecontainer_RcsBundleInfo_nativeGetVersion
(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif // JNI_RCS_BUNDLE_INFO_H_

