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

#include "JniRcsLockedAttributes.h"

#include "JniRcsObject.h"
#include "JniRcsResourceAttributes.h"
#include "JniRcsValue.h"
#include "Log.h"
#include "Verify.h"

#include "RCSResourceObject.h"

#define LOG_TAG "JNI-RCSLockedAttributes"

using namespace OIC::Service;

namespace
{
    inline RCSResourceObject::Ptr& getResource(JNIEnv* env, jobject obj)
    {
        return getNativeHandleAs< RCSResourceObject::Ptr >(env, obj);
    }
}

// The prerequisite for below methods is for ResourceObject's attributes being locked.
// This is guaranteed by class named RCSResourceObject.AttributesLock on Java layer.

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeIsEmpty
(JNIEnv* env, jclass, jobject resourceObject)
{
    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->getAttributes().empty();
}

JNIEXPORT jint JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeSize
(JNIEnv* env, jclass, jobject resourceObject)
{
    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->getAttributes().size();
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeRemove
(JNIEnv* env, jclass, jobject resourceObject, jstring keyObj)
{
    EXPECT_RET_DEF(keyObj, "keyObj is null");

    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC_RET_DEF(env);

    auto key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->getAttributes().erase(key);
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeClear
(JNIEnv* env, jclass, jobject resourceObject)
{
    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC(env);

    res->getAttributes().clear();
}

JNIEXPORT jboolean JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeContains
(JNIEnv* env, jclass, jobject resourceObject, jstring keyObj)
{
    EXPECT_RET_DEF(keyObj, "keyObj is null");

    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC_RET_DEF(env);

    auto key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);

    return res->getAttributes().contains(key);
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeAddKeys
(JNIEnv* env, jclass, jobject resourceObject, jstring setObj)
{
    EXPECT(setObj, "set is null.");

    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC(env);

    for (const auto& keyValue : res->getAttributes())
    {
        JavaLocalString localObj{ env, env->NewStringUTF(keyValue.key().c_str()) };
        VERIFY_NO_EXC(env);

        invoke_Collection_add(env, setObj, localObj);
        VERIFY_NO_EXC(env);
    }
}

JNIEXPORT jobject JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeAsJavaObject
(JNIEnv* env, jclass, jobject resourceObject, jstring keyObj)
{
    EXPECT_RET_DEF(keyObj, "Key is null.");

    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC_RET_DEF(env);

    auto key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);

    auto& attrs = res->getAttributes();

    EXPECT_RET_DEF(attrs.contains(key), "no matched value");

    jobject valueObj = newRCSValueObject(env, attrs[key]);
    VERIFY_NO_EXC_RET_DEF(env);

    return valueObj;
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeApply
(JNIEnv* env, jclass, jobject resourceObject, jstring cacheObj)
{
    EXPECT(cacheObj, "cacheObj is null.");

    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC(env);

    writeNativeAttributesFromMap(env, cacheObj, res->getAttributes());
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeLock
(JNIEnv* env, jobject obj, jobject resourceObject)
{
    auto res = getResource(env, resourceObject);
    VERIFY_NO_EXC(env);

    setSafeNativeHandle< RCSResourceObject::LockGuard >(env, obj, res);
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_server_RcsLockedAttributes_nativeUnlock(JNIEnv* env, jobject obj)
{
    releaseNativeHandle(env, obj);
}
