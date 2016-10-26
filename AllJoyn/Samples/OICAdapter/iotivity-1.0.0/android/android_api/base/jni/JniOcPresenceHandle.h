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
#include "JniOnPresenceListener.h"
#include "OCPlatform.h"

#ifndef _Included_org_iotivity_base_OcPresenceHandle
#define _Included_org_iotivity_base_OcPresenceHandle

using namespace OC::OCPlatform;

class JniOcPresenceHandle
{
public:

    JniOcPresenceHandle(JniOnPresenceListener* jniListener, OCPresenceHandle presenceHandle);
    ~JniOcPresenceHandle();
    JniOcPresenceHandle(const JniOcPresenceHandle &) = delete;
    void operator=(const JniOcPresenceHandle &) = delete;

    static JniOcPresenceHandle* getJniOcPresenceHandlePtr(JNIEnv *env, jobject thiz);

    OCPresenceHandle getOCPresenceHandle();
    JniOnPresenceListener* getJniOnPresenceListener();

private:
    JniOnPresenceListener* m_jniListener;
    OCPresenceHandle m_presenceHandle;
};

#ifdef __cplusplus
extern "C" {
#endif

    /*
    * Class:     org_iotivity_base_OcPresenceHandle
    * Method:    dispose
    * Signature: ()V
    */
    JNIEXPORT void JNICALL Java_org_iotivity_base_OcPresenceHandle_dispose
        (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
