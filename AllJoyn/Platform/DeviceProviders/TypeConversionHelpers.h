//
// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#pragma once

#include "pch.h"
#include "AllJoynHelpers.h"
#include "AllJoynMessageArgStructure.h"
#include "AllJoynMessageArgVariant.h"
#include "collection.h"

ref class TypeConversionHelpers
{
internal:

    // Check whether the value passed in is an AllJoyn type signature for an array of primitive values,
    // such as an array of int32s or an array of booleans.
    static _Check_return_ bool IsArrayOfPrimitives(_In_ PCSTR signature)
    {
        return (strlen(signature) == 2)
            && (static_cast<alljoyn_typeid>(signature[1]) != ALLJOYN_STRING)
            && (static_cast<alljoyn_typeid>(signature[1]) != ALLJOYN_VARIANT);
    }

    // Find the next complete type in the given AllJoyn type signature and assign it to typeSignature.
    //
    // Examples:
    //   If signature is "ii", it will return "i", because that string describes the integer type
    //   If signature is "a(is)si" it will return "a(is)", because that string fully describes an array of structures.
    static _Check_return_ int32 GetNextCompleteType(_In_ PCSTR signature, _Out_ std::string& typeSignature);

    // Get the key and value types from the type AllJoyn type signature for a dictionary.
    static _Check_return_ int32 GetDictionaryTypeSignatures(_In_ PCSTR signature, _Out_ std::string& keySignature, _Out_ std::string& valueSignature);

    // Given a type signature, return an array of signatures each representing a complete type
    //
    // Examples:
    //    If signature is "ii", it will return a vector with two elements: "i" and "i"
    //    If signature is "a(is)si" it will return a vector with three elements: "a(is)", "s", and "i"
    static _Check_return_ int32 CreateCompleteTypeSignatureArrayFromSignature(_In_ PCSTR signature, _Out_ std::vector<std::string>& individualTypeSignatures);

    // Given the type signature for a structure type, return a vector containing the signatures for the individual
    // fields of that structure.
    //
    // Example:
    //    If signature is (si), it will return an array with two elements: "s" and "i"
    static _Check_return_ int32 GetFieldTypesFromStructSignature(_In_ PCSTR signature, _Out_ std::vector<std::string>& fieldTypeSignatures);

    // Get the value of an alljoyn_msgarg whose value matches WinRT type T.
    //
    // The default implementation of this function passes the value directly to alljoyn_msgarg_get, which
    // will work for any primitive types (int32, byte, etc.)  More complex types that require additional
    // work to convert between the WinRT type and the AllJoyn type must have a specialization of this template
    // function to perform the conversion.
    template<class T>
    static _Check_return_ int32 GetAllJoynMessageArg(alljoyn_msgarg argument, PCSTR signature, _Out_ T* value)
    {
        return alljoyn_msgarg_get(argument, signature, value);
    }

    // Set the value of an alljoyn_msgarg to the value of WinRT type T.
    //
    // The default implementation of this function passes the value directly to alljoyn_msgarg_set, which
    // will work for any primitive types (int32, byte, etc.)  More complex types that require additional
    // work to convert between the WinRT type and the AllJoyn type must have a specialization of this template
    // function to perform the conversion.
    template<class T>
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ T value)
    {
        return alljoyn_msgarg_set(argument, signature, value);
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Platform::String^* value)
    {
        PSTR allJoynValue = NULL;
        QStatus status = alljoyn_msgarg_get(argument, signature, &allJoynValue);
        *value = AllJoynHelpers::MultibyteToPlatformString(allJoynValue);
        return static_cast<int32>(status);
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Platform::String^ value)
    {
        std::vector<char> inputArg = AllJoynHelpers::PlatformToMultibyteString(value);
        return alljoyn_msgarg_set_and_stabilize(argument, signature, inputArg.data());
    }

    // Get the array of primitive values stored in the given alljoyn_msgarg.  The values in the array will be returned as
    // a Windows::Foundation::Collections::IVector.
    template<class T>
    static _Check_return_ int32 GetPrimitiveArrayMessageArgHelper(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<Object^>^* value)
    {
        size_t elementCount = 0;
        T* arrayContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &arrayContents));

        *value = ref new Platform::Collections::Vector<Object^>();

        for (size_t i = 0; i < elementCount; ++i)
        {
            (*value)->Append(arrayContents[i]);
        }

        return ER_OK;
    }

    static _Check_return_ int32 GetPrimitiveArrayMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<Object^>^* value)
    {
        switch (static_cast<alljoyn_typeid>(signature[1]))
        {
        case ALLJOYN_BYTE:
        {
            return GetPrimitiveArrayMessageArgHelper<byte>(argument, signature, value);
        }
        case ALLJOYN_BOOLEAN:
        {
            return GetPrimitiveArrayMessageArgHelper<bool>(argument, signature, value);
        }
        case ALLJOYN_INT16:
        {
            return GetPrimitiveArrayMessageArgHelper<int16>(argument, signature, value);
        }
        case ALLJOYN_UINT16:
        {
            return GetPrimitiveArrayMessageArgHelper<uint16>(argument, signature, value);
        }
        case ALLJOYN_INT32:
        {
            return GetPrimitiveArrayMessageArgHelper<int32>(argument, signature, value);
        }
        case ALLJOYN_UINT32:
        {
            return GetPrimitiveArrayMessageArgHelper<uint32>(argument, signature, value);
        }
        case ALLJOYN_INT64:
        {
            return GetPrimitiveArrayMessageArgHelper<int64>(argument, signature, value);
        }
        case ALLJOYN_UINT64:
        {
            return GetPrimitiveArrayMessageArgHelper<uint64>(argument, signature, value);
        }
        case ALLJOYN_DOUBLE:
        {
            return GetPrimitiveArrayMessageArgHelper<double>(argument, signature, value);
        }
        }
        return ER_BUS_BAD_SIGNATURE;
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<Object^>^* value)
    {
        if (signature[0] != 'a')
        {
            return ER_BUS_BAD_SIGNATURE;
        }

        if (IsArrayOfPrimitives(signature))
        {
            return GetPrimitiveArrayMessageArg(argument, signature, value);
        }


        *value = ref new Platform::Collections::Vector<Object^>();

        // Remove the 'a' to get the signature of an array element.
        PCSTR elementSignature = signature + 1;
        size_t elementCount = 0;
        alljoyn_msgarg arrayContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &arrayContents));

        if (arrayContents != nullptr)
        {
            for (size_t i = 0; i < elementCount; i++)
            {
                Object^ elementValue;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(alljoyn_msgarg_array_element(arrayContents, i), elementSignature, &elementValue));
                (*value)->Append(elementValue);
            }
        }

        return ER_OK;
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IVector<Object^>^ value)
    {
        if (static_cast<alljoyn_typeid>(signature[0]) != ALLJOYN_ARRAY)
        {
            return ER_BUS_BAD_SIGNATURE;
        }

        // Remove the 'a' to get the signature of an array element.
        PCSTR elementSignature = signature + 1;
        alljoyn_msgarg arrayArgument = alljoyn_msgarg_array_create(value->Size);

        for (size_t i = 0; i < value->Size; i++)
        {
            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(alljoyn_msgarg_array_element(arrayArgument, i), elementSignature, value->GetAt(static_cast<unsigned int>(i))));
        }

        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, "a*", (size_t)value->Size, arrayArgument);
        alljoyn_msgarg_destroy(arrayArgument);
        return static_cast<int32>(status);
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IKeyValuePair<Platform::Object^, Platform::Object^>^>^* value)
    {
        std::string keyType, valueType;
        RETURN_IF_QSTATUS_ERROR(GetDictionaryTypeSignatures(signature, keyType, valueType));

        *value = nullptr;
        auto result = ref new Platform::Collections::Vector<Windows::Foundation::Collections::IKeyValuePair<Platform::Object^, Platform::Object^>^>();

        size_t elementCount = 0;
        alljoyn_msgarg dictionaryContents = nullptr;
        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, signature, &elementCount, &dictionaryContents));

        if (dictionaryContents != nullptr)
        {
            for (size_t i = 0; i < elementCount; i++)
            {
                alljoyn_msgarg keyArg, valueArg;
                RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(alljoyn_msgarg_array_element(dictionaryContents, i), "{**}", &keyArg, &valueArg));
                Platform::Object^ dictionaryKey;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(keyArg, keyType.data(), &dictionaryKey));
                Platform::Object^ dictionaryValue;
                RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(valueArg, valueType.data(), &dictionaryValue));

                result->Append(ref new Platform::Collections::Details::KeyValuePair<Platform::Object^, Platform::Object^>(dictionaryKey, dictionaryValue));
            }
        }

        (*value) = result;
        return ER_OK;
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IKeyValuePair<Platform::Object^, Platform::Object^>^>^ value)
    {
        std::string keyType, valueType;
        RETURN_IF_QSTATUS_ERROR(GetDictionaryTypeSignatures(signature, keyType, valueType));

        alljoyn_msgarg dictionaryArg = alljoyn_msgarg_array_create(value->Size);

        int i = 0;
        for (auto dictionaryElement : value)
        {
            alljoyn_msgarg keyArg = alljoyn_msgarg_create();
            alljoyn_msgarg valueArg = alljoyn_msgarg_create();

            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(keyArg, keyType.data(), dictionaryElement->Key));
            if (static_cast<alljoyn_typeid>(keyType[0]) == ALLJOYN_WILDCARD)
            {
                char actualKeySignature[c_MaximumSignatureLength];
                alljoyn_msgarg_signature(keyArg, actualKeySignature, c_MaximumSignatureLength);
                keyType = actualKeySignature;
            }

            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(valueArg, valueType.data(), dictionaryElement->Value));

            RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_set_and_stabilize(alljoyn_msgarg_array_element(dictionaryArg, i++), "{**}", keyArg, valueArg));
            alljoyn_msgarg_destroy(keyArg);
            alljoyn_msgarg_destroy(valueArg);
        }

        std::string actualSignature = "a{" + keyType + valueType + "}";

        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, actualSignature.data(), (size_t)value->Size, dictionaryArg);
        alljoyn_msgarg_destroy(dictionaryArg);
        return static_cast<int32>(status);
    }

    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR s, _Out_ DeviceProviders::AllJoynMessageArgStructure^* value)
    {
        char signature[c_MaximumSignatureLength];
        alljoyn_msgarg_signature(argument, signature, c_MaximumSignatureLength);

        if (static_cast<alljoyn_typeid>(signature[0]) == ALLJOYN_VARIANT)
        {
            RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, "v", &argument));
            alljoyn_msgarg_signature(argument, signature, c_MaximumSignatureLength);
        }

        size_t memberCount = alljoyn_msgarg_getnummembers(argument);
        std::vector<std::string> fieldSignatures;
        RETURN_IF_QSTATUS_ERROR(GetFieldTypesFromStructSignature(signature, fieldSignatures));

        if (memberCount != fieldSignatures.size())
        {
            return ER_BUS_BAD_SIGNATURE;
        }

        auto result = ref new DeviceProviders::AllJoynMessageArgStructure(signature);
        for (size_t i = 0; i < memberCount; ++i)
        {
            Platform::Object^ fieldValue;
            RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(alljoyn_msgarg_getmember(argument, i), fieldSignatures[i].data(), &fieldValue));
            result->Append(fieldValue);
        }
        *value = result;
        return ER_OK;
    }

    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ DeviceProviders::AllJoynMessageArgStructure^ value)
    {
        std::vector<std::string> fieldSignatures;
        RETURN_IF_QSTATUS_ERROR(GetFieldTypesFromStructSignature(signature, fieldSignatures));

        if (value->Size != fieldSignatures.size())
        {
            return ER_BUS_BAD_SIGNATURE;
        }

        alljoyn_msgarg structArg = alljoyn_msgarg_create();
        alljoyn_msgarg fields = alljoyn_msgarg_array_create(value->Size);

        for (unsigned int i = 0; i < value->Size; i++)
        {
            RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(alljoyn_msgarg_array_element(fields, i), fieldSignatures[i].data(), value->GetAt(i)));
        }

        RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_setstruct(structArg, fields, value->Size));
        alljoyn_msgarg_destroy(fields);

        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, "*", structArg);
        alljoyn_msgarg_destroy(structArg);

        return static_cast<int32>(status);
    }

    //
    // Invoke the correct templated function based on signature. This function does not assume the alljoyn_msgarg is a variant,
    // but rather it uses the provided signature
    //
    static _Check_return_ int32 GetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Platform::Object^* value)
    {
        switch (static_cast<alljoyn_typeid>(signature[0]))
        {
        case ALLJOYN_BYTE:
        {
            return GetAllJoynMessageArgHelper<byte>(argument, signature, value);
        }
        case ALLJOYN_BOOLEAN:
        {
            return GetAllJoynMessageArgHelper<bool>(argument, signature, value);
        }
        case ALLJOYN_INT16:
        {
            return GetAllJoynMessageArgHelper<int16>(argument, signature, value);
        }
        case ALLJOYN_UINT16:
        {
            return GetAllJoynMessageArgHelper<uint16>(argument, signature, value);
        }
        case ALLJOYN_INT32:
        {
            return GetAllJoynMessageArgHelper<int32>(argument, signature, value);
        }
        case ALLJOYN_UINT32:
        {
            return GetAllJoynMessageArgHelper<uint32>(argument, signature, value);
        }
        case ALLJOYN_INT64:
        {
            return GetAllJoynMessageArgHelper<int64>(argument, signature, value);
        }
        case ALLJOYN_UINT64:
        {
            return GetAllJoynMessageArgHelper<uint64>(argument, signature, value);
        }
        case ALLJOYN_DOUBLE:
        {
            return GetAllJoynMessageArgHelper<double>(argument, signature, value);
        }
        case ALLJOYN_STRING: __fallthrough;
        case ALLJOYN_OBJECT_PATH: __fallthrough;
        case ALLJOYN_SIGNATURE:
        {
            return GetAllJoynMessageArgHelper<Platform::String^>(argument, signature, value);
        }
        case ALLJOYN_VARIANT:
        {
            return GetVariantArg(argument, value);
        }
        case ALLJOYN_STRUCT_OPEN:
        {
            return GetAllJoynMessageArgHelper<DeviceProviders::AllJoynMessageArgStructure^>(argument, signature, value);
        }
        case ALLJOYN_ARRAY:
        {
            if (strlen(signature) < 2)
            {
                return ER_BUS_BAD_SIGNATURE;
            }
            if (static_cast<alljoyn_typeid>(signature[1]) == ALLJOYN_DICT_ENTRY_OPEN)
            {
                return GetAllJoynMessageArgHelper<Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IKeyValuePair<Platform::Object^, Platform::Object^>^>^>(argument, signature, value);
            }
            else
            {
                return GetAllJoynMessageArgHelper<Windows::Foundation::Collections::IVector<Platform::Object^>^>(argument, signature, value);
            }
        }
        default:
            return ER_BUS_BAD_SIGNATURE;
        }
        return ER_OK;
    }

    //
    // Invoke the correct templated function based on signature. This function does not assume the alljoyn_msgarg is a variant,
    // but rather it uses the provided signature
    //
    static _Check_return_ int32 SetAllJoynMessageArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ Platform::Object^ value)
    {
        auto byteValue = dynamic_cast<Platform::IBox<byte>^>(value);
        if (byteValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "y", signature, byteValue->Value);
        }
        auto boolValue = dynamic_cast<Platform::IBox<bool>^>(value);
        if (boolValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "b", signature, boolValue->Value);
        }
        auto int16Value = dynamic_cast<Platform::IBox<int16>^>(value);
        if (int16Value != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "n", signature, int16Value->Value);
        }
        auto uint16Value = dynamic_cast<Platform::IBox<uint16>^>(value);
        if (uint16Value != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "q", signature, uint16Value->Value);
        }
        auto int32Value = dynamic_cast<Platform::IBox<int32>^>(value);
        if (int32Value != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "i", signature, int32Value->Value);
        }
        auto uint32Value = dynamic_cast<Platform::IBox<uint32>^>(value);
        if (uint32Value != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "u", signature, uint32Value->Value);
        }
        auto int64Value = dynamic_cast<Platform::IBox<int64>^>(value);
        if (int64Value != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "x", signature, int64Value->Value);
        }
        auto uint64Value = dynamic_cast<Platform::IBox<uint64>^>(value);
        if (uint64Value != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "t", signature, uint64Value->Value);
        }
        auto doubleValue = dynamic_cast<Platform::IBox<double>^>(value);
        if (doubleValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "d", signature, doubleValue->Value);
        }
        auto stringValue = dynamic_cast<Platform::String^>(value);
        if (stringValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "s", signature, stringValue);
        }
        auto structValue = dynamic_cast<DeviceProviders::AllJoynMessageArgStructure^>(value);
        if (structValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, structValue->AllJoynSignature.data(), signature, structValue);
        }
        auto vectorValue = dynamic_cast<Windows::Foundation::Collections::IVector<Platform::Object^>^>(value);
        if (vectorValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "av", signature, vectorValue);
        }
        auto mapValue = dynamic_cast<Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IKeyValuePair<Platform::Object^, Platform::Object^>^>^>(value);
        if (mapValue != nullptr)
        {
            return SetAllJoynMessageArgHelper(argument, "a{*v}", signature, mapValue);
        }
        return ER_BUS_BAD_VALUE_TYPE;
    }

    template<typename T>
    static _Check_return_ Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ GetAnnotationsView(
        const T& member,
        std::function<size_t(T)> fnGetAnnotationCount,
        std::function<void(T, size_t, char*, size_t*, char*, size_t*)> fnGetAnnotationAtIndex)
    {
        {
            //annotations
            auto annotations = ref new Map<String^, String^>();
            auto nCount = fnGetAnnotationCount(member);
            for (size_t i = 0; i < nCount; ++i)
            {
                size_t nSizeName = 0;
                size_t nSizeValue = 0;
                
                //get the buffer size for name and value pair
                fnGetAnnotationAtIndex(member, i, nullptr, &nSizeName, nullptr, &nSizeValue);
                auto name = vector<char>(nSizeName);
                auto value = vector<char>(nSizeValue);
                
                //get the annotation at index i
                fnGetAnnotationAtIndex(member, i, name.data(), &nSizeName, value.data(), &nSizeValue);
                annotations->Insert(AllJoynHelpers::MultibyteToPlatformString(name.data()), AllJoynHelpers::MultibyteToPlatformString(value.data()));
            }

            return annotations->GetView();
        }
    }

private:

    template<class T>
    static _Check_return_ int32 GetAllJoynMessageArgHelper(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _Out_ Platform::Object^* value)
    {
        T innerValue;
        RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(argument, signature, &innerValue));
        *value = innerValue;
        return ER_OK;
    }

    template<class T>
    static _Check_return_ int32 SetAllJoynMessageArgHelper(_In_ alljoyn_msgarg argument, _In_ PCSTR expectedSignature, _In_ PCSTR providedSignature, _In_ T value)
    {
        // Unless the provided signature is variant or wildcard, use it.

        auto typeId = static_cast<alljoyn_typeid>(providedSignature[0]);
        if (typeId == ALLJOYN_VARIANT)
        {
            return SetVariantArg(argument, expectedSignature, value);
        }
        else if (typeId == ALLJOYN_WILDCARD)
        {
            return SetAllJoynMessageArg(argument, expectedSignature, value);
        }
        else
        {
            return SetAllJoynMessageArg(argument, providedSignature, value);
        }
    }

    //
    // Extract inner alljoyn_msgarg from a variant, figure out its signature and then get its value
    //
    static _Check_return_ int32 GetVariantArg(_In_ alljoyn_msgarg argument, _Out_ Platform::Object^* value)
    {
        char signature[c_MaximumSignatureLength];
        alljoyn_msgarg_signature(argument, signature, c_MaximumSignatureLength);

        if (static_cast<alljoyn_typeid>(signature[0]) == ALLJOYN_VARIANT)
        {
            RETURN_IF_QSTATUS_ERROR(alljoyn_msgarg_get(argument, "v", &argument));
            alljoyn_msgarg_signature(argument, signature, c_MaximumSignatureLength);
        }

        Platform::Object^ variantValue = nullptr;
        RETURN_IF_QSTATUS_ERROR(GetAllJoynMessageArg(argument, signature, &variantValue));

        *value = ref new DeviceProviders::AllJoynMessageArgVariant(signature, variantValue);
        return QStatus::ER_OK;
    }

    template<class T>
    static _Check_return_ int32 SetVariantArg(_In_ alljoyn_msgarg argument, _In_ PCSTR signature, _In_ T value)
    {
        alljoyn_msgarg variantArg = alljoyn_msgarg_create();
        RETURN_IF_QSTATUS_ERROR(SetAllJoynMessageArg(variantArg, signature, value));
        QStatus status = alljoyn_msgarg_set_and_stabilize(argument, "v", variantArg);
        alljoyn_msgarg_destroy(variantArg);
        return status;
    }
};