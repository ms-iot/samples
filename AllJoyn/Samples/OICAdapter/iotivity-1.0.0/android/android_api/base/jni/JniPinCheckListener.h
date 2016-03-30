/*
* //******************************************************************
* //
* // Copyright 2015 Samsung Electronics All Rights Reserved.
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
#include "OCProvisioningManager.h"

#ifndef _Included_org_iotivity_base_PinCheckListener
#define _Included_org_iotivity_base_PinCheckListener

typedef std::function<void(JNIEnv* env, jobject jListener)> RemoveCallback;
class JniPinCheckListener
{
    public:
        JniPinCheckListener(JNIEnv *, jobject);
        ~JniPinCheckListener();

        void PinCallback(char*, size_t);
    private:
        jobject m_jListener;
};
#endif
