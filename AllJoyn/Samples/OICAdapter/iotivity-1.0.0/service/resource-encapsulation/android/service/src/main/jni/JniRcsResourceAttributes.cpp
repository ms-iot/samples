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

#include "JniRcsResourceAttributes.h"

#include "JniRcsObject.h"
#include "JavaLocalRef.h"
#include "JniRcsValue.h"
#include "Log.h"
#include "Verify.h"

using namespace OIC::Service;

#define LOG_TAG "JNI-RCSResourceAttributes"

namespace
{
    jclass g_cls_RCSResourceAttributes;

    jmethodID g_ctor_RCSResourceAttributes;

    jfieldID g_field_mCache;
}

void initRCSResourceAttributes(JNIEnvWrapper* env)
{
    g_cls_RCSResourceAttributes = env->FindClassAsGlobalRef(CLS_NAME_RESOURCEATTRIBUTES);
    g_ctor_RCSResourceAttributes = env->GetConstructorID(g_cls_RCSResourceAttributes, "()V");

    g_field_mCache = env->GetFieldID(g_cls_RCSResourceAttributes, "mCache", AS_SIG(CLS_NAME_MAP));
}

void clearRCSResourceAttributes(JNIEnvWrapper* env)
{
    env->DeleteGlobalRef(g_cls_RCSResourceAttributes);
}

jobject newAttributesObject(JNIEnv* env, const RCSResourceAttributes& attrs)
{
    jobject obj = env->NewObject(g_cls_RCSResourceAttributes, g_ctor_RCSResourceAttributes);
    VERIFY_NO_EXC_RET_DEF(env);

    setSafeNativeHandle< RCSResourceAttributes >(env, obj, attrs);

    return obj;
}

jobject newAttributesObject(JNIEnvWrapper* env, const RCSResourceAttributes& attrs)
{
    jobject obj = env->NewObject(g_cls_RCSResourceAttributes, g_ctor_RCSResourceAttributes);

    setSafeNativeHandle< RCSResourceAttributes >(env, obj, attrs);

    return obj;
}

RCSResourceAttributes toNativeAttributes(JNIEnv* env, jobject attrsObj)
{
    EXPECT_RET(attrsObj, "attrsObj is null!", { });

    JNIEnvWrapper envWrapper{ env };

    try
    {
        return toNativeAttributes(&envWrapper, attrsObj);
    }
    catch (const JavaException&)
    {
        return {};
    }
}

RCSResourceAttributes toNativeAttributes(JNIEnvWrapper* env, jobject attrsObj)
{
    EXPECT_RET(attrsObj, "attrsObj is null!", { });

    RCSResourceAttributes attrs;

    if (hasNativeHandle(env, attrsObj))
    {
        attrs = getNativeHandleAs< RCSResourceAttributes >(env, attrsObj);
    }

    writeNativeAttributesFromMap(env,
            JavaLocalObject{ env, env->GetObjectField(attrsObj, g_field_mCache) }, attrs);

    return attrs;
}

void writeNativeAttributesFromMap(JNIEnv* env, jobject mapObj, RCSResourceAttributes& targetAttrs)
{
    JNIEnvWrapper envWrapper{ env };

    try
    {
        return writeNativeAttributesFromMap(&envWrapper, mapObj, targetAttrs);
    }
    catch (const JavaException&)
    {
    }
}

void writeNativeAttributesFromMap(JNIEnvWrapper* env, jobject mapObj,
        RCSResourceAttributes& targetAttrs)
{
    JavaLocalObject setObj{ env, invoke_Map_entrySet(env, mapObj) };
    JavaLocalObject iterObj{ env, invoke_Set_iterator(env, setObj) };

    while (invoke_Iterator_hasNext(env, iterObj))
    {
        JavaLocalObject entryObj{ env, invoke_Iterator_next(env, iterObj) };
        JavaLocalObject keyObj{ env, invoke_MapEntry_getKey(env, entryObj) };
        JavaLocalObject valueObj{ env, invoke_MapEntry_getValue(env, entryObj) };

        auto key = toStdString(env, static_cast< jstring >(keyObj.get()));

        targetAttrs[std::move(key)] = toNativeAttrsValue(env, valueObj);
    }
}

JNIEXPORT jboolean JNICALL Java_org_iotivity_service_RcsResourceAttributes_nativeIsEmpty
(JNIEnv* env, jobject obj)
{
    LOGD("isEmpty");
    EXPECT_RET(hasNativeHandle(env, obj), "no native handle.", true);

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return attrs.empty();
}

JNIEXPORT jint JNICALL Java_org_iotivity_service_RcsResourceAttributes_nativeSize
(JNIEnv* env, jobject obj)
{
    LOGD("size");
    EXPECT_RET(hasNativeHandle(env, obj), "no native handle.", 0);

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    return attrs.size();
}

JNIEXPORT jboolean JNICALL Java_org_iotivity_service_RcsResourceAttributes_nativeRemove
(JNIEnv* env, jobject obj, jstring keyObj)
{
    LOGD("remove");
    EXPECT_RET_DEF(keyObj, "Key is null.");
    EXPECT_RET_DEF(hasNativeHandle(env, obj), "no native handle.");

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);
    const auto& key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);

    auto ret = attrs.erase(key);

    if (attrs.empty()) releaseNativeHandle(env, obj);

    return ret;
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeClear
(JNIEnv* env, jobject obj)
{
    LOGD("clear");

    releaseNativeHandle(env, obj);
}

JNIEXPORT jboolean JNICALL Java_org_iotivity_service_RcsResourceAttributes_nativeContains
(JNIEnv *env, jobject obj, jstring keyObj)
{
    LOGD("contains");
    EXPECT_RET(keyObj, "Key is null.", false);
    EXPECT_RET(hasNativeHandle(env, obj), "no native handle.", false);

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);
    const auto& key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);
    return attrs.contains(key);
}

JNIEXPORT void JNICALL Java_org_iotivity_service_RcsResourceAttributes_nativeAddKeys
(JNIEnv *env, jobject obj, jstring setObj)
{
    LOGD("addKeys");
    EXPECT(setObj, "set is null.");
    EXPECT(hasNativeHandle(env, obj), "no native handle.");

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC(env);

    for (const auto& keyValue : attrs)
    {
        JavaLocalString localObj{ env, env->NewStringUTF(keyValue.key().c_str()) };
        VERIFY_NO_EXC(env);

        invoke_Collection_add(env, setObj, localObj);
        VERIFY_NO_EXC(env);
    }
}

JNIEXPORT jobject JNICALL Java_org_iotivity_service_RcsResourceAttributes_nativeExtract
(JNIEnv* env, jobject obj, jstring keyObj)
{
    LOGD("extract");
    EXPECT_RET_DEF(keyObj, "Key is null.");
    EXPECT_RET_DEF(hasNativeHandle(env, obj), "no native handle.");

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC_RET_DEF(env);

    const auto& key = toStdString(env, keyObj);
    VERIFY_NO_EXC_RET_DEF(env);

    EXPECT_RET_DEF(attrs.contains(key), "no matched value");

    jobject valueObj = newRCSValueObject(env, attrs[key]);
    VERIFY_NO_EXC_RET_DEF(env);

    attrs.erase(key);
    if (attrs.empty()) releaseNativeHandle(env, obj);
    return valueObj;
}

JNIEXPORT void JNICALL
Java_org_iotivity_service_RcsResourceAttributes_nativeExtractAll
(JNIEnv* env, jobject obj, jobject mapObj)
{
    LOGD("extractAll");
    EXPECT(mapObj, "Map is null.");
    EXPECT(hasNativeHandle(env, obj), "no native handle.");

    auto& attrs = getNativeHandleAs< RCSResourceAttributes >(env, obj);
    VERIFY_NO_EXC(env);

    for (const auto& p : attrs) {
        JavaLocalObject keyObj{ env, newStringObject(env, p.key()) };
        VERIFY_NO_EXC(env);

        JavaLocalObject valueObj{ env, newRCSValueObject(env, p.value()) };
        VERIFY_NO_EXC(env);

        invoke_Map_put(env, mapObj, keyObj, valueObj);
        VERIFY_NO_EXC(env);
    }

    attrs.clear();
    releaseNativeHandle(env, obj);
}

