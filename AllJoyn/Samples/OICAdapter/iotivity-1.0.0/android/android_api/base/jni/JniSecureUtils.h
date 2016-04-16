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

class JniSecureUtils
{
    private:
        static std::string convertUUIDtoStr(OicUuid_t uuid);
        static void convertStrToUUID(char *str, OicUuid_t &uuid);
    public:
        static jobject convertProvisionresultVectorToJavaList(JNIEnv *,
                const OC::PMResultList_t *);
        static jobjectArray convertDeviceVectorToJavaArray(JNIEnv *env,
                std::vector<std::shared_ptr<OC::OCSecureResource>>& deviceListVector);
        static jobject convertUUIDVectorToJavaStrList(JNIEnv *env, OC::UuidList_t &vector);
        static OCStackResult convertJavaACLToOCAcl(JNIEnv *env, jobject in, OicSecAcl_t *out);
};
