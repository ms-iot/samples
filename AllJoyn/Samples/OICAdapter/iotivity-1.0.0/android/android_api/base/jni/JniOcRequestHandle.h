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
#include "ocstack.h"

#ifndef _Included_org_iotivity_base_OcRequestHandle
#define _Included_org_iotivity_base_OcRequestHandle

class JniOcRequestHandle
{
public:

    JniOcRequestHandle(OCRequestHandle request);
    ~JniOcRequestHandle();

    static JniOcRequestHandle* getJniOcRequestHandlePtr(JNIEnv *env, jobject thiz);

    OCRequestHandle getOCRequestHandle();

private:
    OCRequestHandle m_requestHandle;
};

#ifdef __cplusplus
extern "C" {
#endif

    /*
    * Class:     org_iotivity_base_OcRequestHandle
    * Method:    dispose
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcRequestHandle_dispose
        (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
