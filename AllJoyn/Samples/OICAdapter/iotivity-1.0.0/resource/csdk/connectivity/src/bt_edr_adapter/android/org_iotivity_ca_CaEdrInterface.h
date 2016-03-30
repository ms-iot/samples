/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
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

#include <jni.h>
/* Header for class org_iotivity_ca_CaEdrInterface */

#ifndef Included_org_iotivity_ca_CaEdrInterface
#define Included_org_iotivity_ca_CaEdrInterface
#ifdef __cplusplus
extern "C"
{
#endif
/*
 * Class:     org_iotivity_ca_CaEdrInterface
 * Method:    caEdrStateChangedCallback
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaEdrInterface_caEdrStateChangedCallback
(JNIEnv *, jobject, jint);

/*
 * Class:     org_iotivity_ca_CaEdrInterface
 * Method:    caEdrBondStateChangedCallback
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaEdrInterface_caEdrBondStateChangedCallback
(JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif
