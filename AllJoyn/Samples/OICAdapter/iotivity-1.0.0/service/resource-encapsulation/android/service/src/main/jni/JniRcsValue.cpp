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

#include "JniRcsValue.h"

#include "JniRcsObject.h"
#include "JniRcsResourceAttributes.h"
#include "JavaClasses.h"
#include "JavaExceptions.h"
#include "JavaLocalRef.h"
#include "Log.h"

using namespace OIC::Service;

#define LOG_TAG "JNI-RCSValue"

#define CLS_NAME_VALUE_TYPE CLS_NAME_VALUE "$Type"
#define CLS_NAME_VALUE_TYPEID CLS_NAME_VALUE "$TypeId"
#define CLS_NAME_VALUE_NULL_TYPE CLS_NAME_VALUE "$NullType"

namespace
{
    jclass g_cls_RCSValue;
    jclass g_cls_Type;
    jclass g_cls_TypeId;

    jmethodID g_ctor_RCSValue;

    jmethodID g_smethod_Type_getDepth;
    jmethodID g_smethod_Type_getBaseTypeId;

    jmethodID g_method_RCSValue_getType;

    jfieldID g_field_RCSValue_mObject;
    jfieldID g_field_RCSValue_sNullValue;

    jobject g_obj_TypeId_Null;
    jobject g_obj_TypeId_Boolean;
    jobject g_obj_TypeId_Integer;
    jobject g_obj_TypeId_Double;
    jobject g_obj_TypeId_String;
    jobject g_obj_TypeId_Attributes;
    jobject g_obj_TypeId_Array;

    template< int >
    struct Int2Type{ };

    template< int DEPTH, typename BASE_TYPE >
    struct SeqType
    {
        typedef std::vector< typename SeqType< DEPTH - 1, BASE_TYPE >::type > type;
    };

    template< typename BASE_TYPE >
    struct SeqType< 0, BASE_TYPE >
    {
        typedef BASE_TYPE type;
    };

    template< typename T >
    struct BaseType
    {
      typedef T type;
    };

    template< typename T >
    struct BaseType< std::vector< T > >
    {
      typedef typename BaseType< T >::type type;
    };


    template< int D >
    struct ArrayPrefix
    {
        static constexpr char str[] = "";
    };
    template< int D >
    constexpr char ArrayPrefix< D >::str[];

    template<>
    struct ArrayPrefix< 1 >
    {
        static constexpr char str[] = "[";
    };
    constexpr char ArrayPrefix< 1 >::str[];

    template<>
    struct ArrayPrefix< 2 >
    {
        static constexpr char str[] = "[[";
    };
    constexpr char ArrayPrefix< 2 >::str[];

    struct PrimitiveType {
        static constexpr bool isPrimitive = true;
    };

    struct ObjectType {
        static constexpr bool isPrimitive = false;
    };

    template< typename T >
    struct JniTypeTrait;

    template<>
    struct JniTypeTrait< int >: public PrimitiveType
    {
        static_assert(sizeof(int) == sizeof(jint), "int and jint have different size!");

        static constexpr decltype(&JNIEnvWrapper::NewIntArray) newArrayFunc =
                &JNIEnvWrapper::NewIntArray;
        static constexpr decltype(&JNIEnvWrapper::SetIntArrayRegion) setArrayRegionFunc =
                      &JNIEnvWrapper::SetIntArrayRegion;

        static constexpr decltype(&invoke_Integer_intValue<JNIEnvWrapper>) converter =
                      &invoke_Integer_intValue<JNIEnvWrapper>;

        static constexpr decltype(&newIntegerObject<JNIEnvWrapper>) newObjectFunc =
                &newIntegerObject<JNIEnvWrapper>;

        static constexpr char className[] = "I";
    };
    constexpr char JniTypeTrait< int >::className[];

    template<>
    struct JniTypeTrait< bool >: public PrimitiveType
    {
        static_assert(sizeof(bool) == sizeof(jboolean), "bool and jboolean have different size!");

        static constexpr decltype(&JNIEnvWrapper::NewBooleanArray) newArrayFunc =
                &JNIEnvWrapper::NewBooleanArray;
        static constexpr decltype(&JNIEnvWrapper::SetBooleanArrayRegion) setArrayRegionFunc =
                      &JNIEnvWrapper::SetBooleanArrayRegion;

        static constexpr decltype(&invoke_Boolean_booleanValue<JNIEnvWrapper>) converter =
                      &invoke_Boolean_booleanValue<JNIEnvWrapper>;

        static constexpr decltype(&newBooleanObject<JNIEnvWrapper>) newObjectFunc =
                &newBooleanObject<JNIEnvWrapper>;

        static constexpr char className[] = "Z";
    };
    constexpr char JniTypeTrait< bool >::className[];

    template<>
    struct JniTypeTrait< double > : public PrimitiveType
    {
        static_assert(sizeof(double) == sizeof(jdouble), "double and jdouble have different size!");

        static constexpr decltype(&JNIEnvWrapper::NewDoubleArray) newArrayFunc =
                &JNIEnvWrapper::NewDoubleArray;
        static constexpr decltype(&JNIEnvWrapper::SetDoubleArrayRegion) setArrayRegionFunc =
                      &JNIEnvWrapper::SetDoubleArrayRegion;

        static constexpr decltype(&invoke_Double_doubleValue<JNIEnvWrapper>) converter =
                      &invoke_Double_doubleValue<JNIEnvWrapper>;

        static constexpr decltype(&newDoubleObject<JNIEnvWrapper>) newObjectFunc =
                &newDoubleObject<JNIEnvWrapper>;

        static constexpr char className[] = "D";
    };
    constexpr char JniTypeTrait< double >::className[];

    template<>
    struct JniTypeTrait< std::string >: public ObjectType
    {
        static constexpr decltype(&newStringObject<JNIEnvWrapper>) newObjectFunc =
                &newStringObject<JNIEnvWrapper>;

        static constexpr char className[] = "L" CLS_NAME_STRING ";";
    };
    constexpr char JniTypeTrait< std::string >::className[];

    template<>
    struct JniTypeTrait< RCSResourceAttributes >: public ObjectType
    {
        inline static jobject newObjectFunc(JNIEnvWrapper* env, const RCSResourceAttributes& value)
        {
            return newAttributesObject(env, value);
        }

        inline static RCSResourceAttributes converter(JNIEnvWrapper* env, jobject obj)
        {
            return toNativeAttributes(env, obj);
        }

        static constexpr char className[] = "L" CLS_NAME_RESOURCEATTRIBUTES ";";
    };
    constexpr char JniTypeTrait< RCSResourceAttributes >::className[];

    inline void toNativeValue(JNIEnvWrapper* env, jobject obj, std::string& result, Int2Type< 0 >)
    {
        result = toStdString(env, static_cast< jstring >(obj));
    }

    template< typename T >
    inline void toNativeValue(JNIEnvWrapper* env, jobject obj, T& result, Int2Type< 0 >)
    {
        result = JniTypeTrait< T >::converter(env, obj);
    }

    template< int DEPTH, typename RET >
    inline void toNativeValue(JNIEnvWrapper* env, jobject obj, RET& result, Int2Type< DEPTH >)
    {
        const auto arrayObj = static_cast< jobjectArray >(obj);
        result.resize(env->GetArrayLength(arrayObj));

        for (typename RET::size_type i = 0; i < result.size(); ++i)
        {
            JavaLocalObject elementObj{ env, env->GetObjectArrayElement(arrayObj, i) };

            toNativeValue(env, elementObj, result[i], Int2Type< DEPTH - 1 >{ });
        }
    }

    template< typename T >
    inline typename std::enable_if< JniTypeTrait< T >::isPrimitive >::type
    toNativeValue(JNIEnvWrapper* env, jobject obj, std::vector< T >& result, Int2Type< 1 >)
    {
        const auto arrayObj = static_cast< jobjectArray >(obj);
        const jsize arraySize = env->GetArrayLength(arrayObj);

        T* raw = static_cast< T* >(env->GetPrimitiveArrayCritical(arrayObj, nullptr));

        try
        {
            result = std::vector< T >(raw, raw + arraySize);
        } catch (...)
        {
        }

        env->ReleasePrimitiveArrayCritical(arrayObj, raw, JNI_ABORT);
    }

    template< typename T >
    inline typename std::enable_if< !JniTypeTrait< T >::isPrimitive >::type
    toNativeValue(JNIEnvWrapper* env, jobject obj, std::vector< T >& result, Int2Type< 1 >)
    {
        const auto arrayObj = static_cast< jobjectArray >(obj);
        const jsize arraySize = env->GetArrayLength(arrayObj);

        result.resize(arraySize);

        for (typename std::vector< T >::size_type i = 0; i < result.size(); ++i)
        {
            JavaLocalObject elementObj{ env,  env->GetObjectArrayElement(arrayObj, i) };
            toNativeValue(env, elementObj, result[i], Int2Type< 0 >{ });
        }
    }


    template< typename T, int DEPTH, typename RET = typename SeqType< DEPTH, T >::type >
    inline RET toNativeValue(JNIEnvWrapper* env, jobject obj)
    {
        static_assert(DEPTH >= 0, "DEPTH must be positive!");

        typename SeqType< DEPTH, T >::type result;

        toNativeValue(env, obj, result, Int2Type< DEPTH >{ });

        return result;
    }

    template< typename T >
    inline RCSResourceAttributes::Value toNativeValue(JNIEnvWrapper* env, jobject val, int depth)
    {
        switch (depth)
        {
            case 0: return toNativeValue< T, 0 >(env, val);
            case 1: return toNativeValue< T, 1 >(env, val);
            case 2: return toNativeValue< T, 2 >(env, val);
            case 3: return toNativeValue< T, 3 >(env, val);
        }

        return {};
    }

    inline RCSResourceAttributes::Value toNativeValue(JNIEnvWrapper* env, jobject val, int depth,
            RCSResourceAttributes::TypeId typeId)
    {
        LOGD("toNativeValue depth is %d", depth);
        EXPECT_RET(depth >= 0 && depth <= 3, "Unsupported depth!", {});

        switch (typeId)
         {
             case RCSResourceAttributes::TypeId::NULL_T: return { };

             case RCSResourceAttributes::TypeId::BOOL:
                 return toNativeValue< bool >(env, val, depth);

             case RCSResourceAttributes::TypeId::INT:
                 return toNativeValue< int >(env, val, depth);

             case RCSResourceAttributes::TypeId::DOUBLE:
                 return toNativeValue< double >(env, val, depth);

             case RCSResourceAttributes::TypeId::STRING:
                 return toNativeValue< std::string >(env, val, depth);

             case RCSResourceAttributes::TypeId::ATTRIBUTES:
                 return toNativeValue< RCSResourceAttributes >(env, val, depth);
         }

        throwRCSException(env, "Failed to convert RCSValue : unknown type id");
        return {};
    }

    template< typename T, int DEPTH, typename BASE_TYPE = typename BaseType< T >::type >
    inline jobject createJavaObject(JNIEnvWrapper* env, const T& value, Int2Type< DEPTH >)
    {
        const std::string elementClsName{ std::string{ ArrayPrefix< DEPTH - 1 >::str }
            + JniTypeTrait< BASE_TYPE >::className };

        LOGD("create array %dd, %s", DEPTH, elementClsName.c_str());

        auto cls = env->FindClass(elementClsName.c_str());

        const jsize len = value.size();

        auto array = env->NewObjectArray(len, cls, nullptr);

        for(jsize i = 0; i < len; ++i)
        {
            auto element = createJavaObject(env, value[i], Int2Type< DEPTH - 1>{ });
            env->SetObjectArrayElement(array, i, element);
        }

        return array;
    }

    template< typename T, typename TRAIT = JniTypeTrait< T > >
    inline typename std::enable_if< TRAIT::isPrimitive, jobject >::type
    createJavaObject(JNIEnvWrapper* env, const std::vector< T >& value, Int2Type< 1 >)
    {
        LOGD("create array with newArray");
        const jsize len = value.size();

        auto array = (env->*TRAIT::newArrayFunc)(len);

        if (!value.empty()) (env->*TRAIT::setArrayRegionFunc)(array, 0, len, &value[0]);

        return array;
    }

    inline jobject createJavaObject(JNIEnvWrapper* env, const std::vector< bool >& value, Int2Type< 1 >)
    {
        const auto len = value.size();
        LOGD("create bool array with newArray %d", len);

        auto arrayObj = env->NewBooleanArray(len);

        bool* raw = static_cast< bool* >(env->GetPrimitiveArrayCritical(arrayObj, 0));

        std::copy(value.begin(), value.end(), raw);

        env->ReleasePrimitiveArrayCritical(arrayObj, raw, 0);

        return arrayObj;
    }

    template< typename T, typename TRAIT = JniTypeTrait< T > >
    inline jobject createJavaObject(JNIEnvWrapper* env, const T& value, Int2Type< 0 >)
    {
        LOGD("createJavaObject 0-depth");
        return TRAIT::newObjectFunc(env, value);
    }

    template< int DEPTH >
    inline jobject createJavaObject(JNIEnvWrapper* env, const RCSResourceAttributes::Value& value)
    {
        const auto type = value.getType();
        const auto baseType = RCSResourceAttributes::Type::getBaseTypeId(type);

        LOGD("createJavaObject with DEPTH. type is %d", static_cast< int >(baseType));

        switch (baseType)
        {
            case RCSResourceAttributes::TypeId::NULL_T:
                return env->GetStaticObjectField(g_cls_RCSValue, g_field_RCSValue_sNullValue);

            case RCSResourceAttributes::TypeId::BOOL:
                return createJavaObject(env, value.get< typename SeqType< DEPTH, bool >::type >(),
                        Int2Type< DEPTH >{ });

            case RCSResourceAttributes::TypeId::INT:
                return createJavaObject(env, value.get< typename SeqType< DEPTH, int >::type >(),
                        Int2Type< DEPTH >{ });

            case RCSResourceAttributes::TypeId::DOUBLE:
                return createJavaObject(env, value.get< typename SeqType< DEPTH, double >::type >(),
                        Int2Type< DEPTH >{ });

            case RCSResourceAttributes::TypeId::STRING:
                return createJavaObject(env,
                        value.get< typename SeqType< DEPTH, std::string >::type >(),
                        Int2Type< DEPTH >{ });

            case RCSResourceAttributes::TypeId::ATTRIBUTES:
                return createJavaObject(env,
                        value.get< typename SeqType< DEPTH, RCSResourceAttributes >::type >(),
                        Int2Type< DEPTH >{ });
        }

        LOGE("Unknown type!");

        return nullptr;
    }

    inline jobject createJavaObject(JNIEnvWrapper* env, const RCSResourceAttributes::Value& value)
    {
        const auto type = value.getType();
        const auto depth = RCSResourceAttributes::Type::getDepth(type);

        EXPECT_RET(depth >= 0 && depth <= 3, "Unsupported depth!", {});

        switch (depth)
        {
            case 0: return createJavaObject< 0 >(env, value);
            case 1: return createJavaObject< 1 >(env, value);
            case 2: return createJavaObject< 2 >(env, value);
            case 3: return createJavaObject< 3 >(env, value);
        }

        return nullptr;
    }

    inline RCSResourceAttributes::TypeId toNativeTypeId(JNIEnvWrapper* env, jobject typeIdObj)
    {
        typedef RCSResourceAttributes::TypeId TypeId;

        if (env->IsSameObject(g_obj_TypeId_Null, typeIdObj)) return TypeId::NULL_T;
        if (env->IsSameObject(g_obj_TypeId_Boolean, typeIdObj)) return TypeId::BOOL;
        if (env->IsSameObject(g_obj_TypeId_Integer, typeIdObj)) return TypeId::INT;
        if (env->IsSameObject(g_obj_TypeId_Double, typeIdObj)) return TypeId::DOUBLE;
        if (env->IsSameObject(g_obj_TypeId_String, typeIdObj)) return TypeId::STRING;
        if (env->IsSameObject(g_obj_TypeId_Attributes, typeIdObj)) return TypeId::ATTRIBUTES;
        if (env->IsSameObject(g_obj_TypeId_Array, typeIdObj)) return TypeId::VECTOR;

        throwRCSException(env, "Failed to convert RCSValue : unknown type id");
        return TypeId::NULL_T;
    }

    jobject getTypeIdObj(JNIEnvWrapper* env, const char* name)
    {
        return env->NewGlobalRef(
                env->GetStaticObjectField(g_cls_TypeId, name, AS_SIG(CLS_NAME_VALUE_TYPEID)));
    }
}

void initRCSValue(JNIEnvWrapper* env)
{
    g_cls_RCSValue = env->FindClassAsGlobalRef(CLS_NAME_VALUE);
    g_ctor_RCSValue = env->GetConstructorID(g_cls_RCSValue, "(" AS_SIG(CLS_NAME_OBJECT) ")V");

    g_method_RCSValue_getType = env->GetMethodID(g_cls_RCSValue, "getType",
            "()" AS_SIG(CLS_NAME_VALUE_TYPE));

    g_field_RCSValue_mObject = env->GetFieldID(g_cls_RCSValue, "mObject", AS_SIG(CLS_NAME_OBJECT));

    g_field_RCSValue_sNullValue = env->GetStaticFieldID(g_cls_RCSValue, "sNullValue",
            AS_SIG(CLS_NAME_VALUE_NULL_TYPE));

    g_cls_Type = env->FindClassAsGlobalRef(CLS_NAME_VALUE_TYPE);

    g_smethod_Type_getBaseTypeId = env->GetStaticMethodID(g_cls_Type, "getBaseTypeId",
            "(" AS_SIG(CLS_NAME_VALUE_TYPE) ")" AS_SIG(CLS_NAME_VALUE_TYPEID));

    g_smethod_Type_getDepth = env->GetStaticMethodID(g_cls_Type, "getDepth",
                "(" AS_SIG(CLS_NAME_VALUE_TYPE) ")I");

    g_cls_TypeId = env->FindClassAsGlobalRef(CLS_NAME_VALUE_TYPEID);

    g_obj_TypeId_Null = getTypeIdObj(env, "NULL");
    g_obj_TypeId_Boolean = getTypeIdObj(env, "BOOLEAN");
    g_obj_TypeId_Integer = getTypeIdObj(env, "INTEGER");
    g_obj_TypeId_Double = getTypeIdObj(env, "DOUBLE");
    g_obj_TypeId_String = getTypeIdObj(env, "STRING");
    g_obj_TypeId_Attributes = getTypeIdObj(env, "ATTRIBUTES");
    g_obj_TypeId_Array = getTypeIdObj(env, "ARRAY");
}

void clearRCSValue(JNIEnvWrapper* env)
{
    env->DeleteGlobalRef(g_cls_RCSValue);
    env->DeleteGlobalRef(g_cls_Type);
    env->DeleteGlobalRef(g_cls_TypeId);

    env->DeleteGlobalRef(g_obj_TypeId_Null);
    env->DeleteGlobalRef(g_obj_TypeId_Boolean);
    env->DeleteGlobalRef(g_obj_TypeId_Integer);
    env->DeleteGlobalRef(g_obj_TypeId_Double);
    env->DeleteGlobalRef(g_obj_TypeId_String);
    env->DeleteGlobalRef(g_obj_TypeId_Attributes);
    env->DeleteGlobalRef(g_obj_TypeId_Array);
}

RCSResourceAttributes::Value toNativeAttrsValue(JNIEnv* env, jobject valueObj)
{
    JNIEnvWrapper envWrapper(env);

    try
    {
        return toNativeAttrsValue(&envWrapper, valueObj);
    }
    catch (const JavaException&)
    {
        return { };
    }
}

RCSResourceAttributes::Value toNativeAttrsValue(JNIEnvWrapper* env, jobject valueObj)
{
    auto typeObj = env->CallObjectMethod(valueObj, g_method_RCSValue_getType);
    auto typeIdObj = env->CallStaticObjectMethod(g_cls_Type, g_smethod_Type_getBaseTypeId, typeObj);
    auto depth = env->CallStaticIntMethod(g_cls_Type, g_smethod_Type_getDepth, typeObj);
    auto memObj = env->GetObjectField(valueObj, g_field_RCSValue_mObject);

    auto ret = toNativeValue(env, memObj, depth, toNativeTypeId(env, typeIdObj));

    if (env->get()->ExceptionCheck()) throw JavaException();

    return ret;
}

jobject newRCSValueObject(JNIEnv* env, const RCSResourceAttributes::Value& value)
{
    JNIEnvWrapper envWrapper(env);

    try
    {
        return newRCSValueObject(&envWrapper, value);
    }
    catch (const JavaException&)
    {
        return {};
    }
}

jobject newRCSValueObject(JNIEnvWrapper* env, const RCSResourceAttributes::Value& value)
{
    LOGD("newRCSValueObject");

    JavaLocalObject valueObj{ env, createJavaObject(env, value) };

    if (env->get()->ExceptionCheck()) throw JavaException();

    return env->NewObject(g_cls_RCSValue, g_ctor_RCSValue, valueObj.get());
}
