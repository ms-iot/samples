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

#include "pch.h"
#include "AllJoynTypeDefinition.h"
#include "TypeConversionHelpers.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace Platform::Collections;
using namespace Platform;
using namespace std;

namespace DeviceProviders
{
    AllJoynTypeDefinition::AllJoynTypeDefinition(const string& signature)
        : m_type(TypeId::Invalid)
        , m_keyType(nullptr)
        , m_valueType(nullptr)
        , m_fields(nullptr)
        , m_allJoynSignature(signature)
    {
        DEBUG_LIFETIME_IMPL(AllJoynTypeDefinition);

        if (signature.empty())
        {
            return;
        }

        switch (signature[0])
        {
        case 'b': case 'd': case 'g': case 'i': case 'n': case 'o': case 'q':
        case 's': case 't': case 'u': case 'v': case 'x': case 'y':
        {
            if (signature.length() == 1)
            {
                m_type = (TypeId)signature[0];
            }
            break;
        }
        case 'a':
        {
            if (signature.length() < 2)
            {
                return;
            }

            switch (signature[1])
            {
            case 'b': case 'd': case 'g': case 'i': case 'n': case 'o': case 'q':
            case 's': case 't': case 'u': case 'v': case 'x': case 'y':
            {
                if (signature.length() == 2)
                {
                    uint16 temp = signature[1] << 8;
                    m_type = (TypeId)(temp | 'a');
                }
                break;
            }
            case '{': // we are a dictionary (array of key value pairs)
            {
                string keySignature, valueSignature;
                if (signature[signature.length() - 1] == '}' &&
                    ER_OK == TypeConversionHelpers::GetDictionaryTypeSignatures(signature.c_str(), keySignature, valueSignature))
                {
                    m_keyType = ref new AllJoynTypeDefinition(keySignature.c_str());
                    m_valueType = ref new AllJoynTypeDefinition(valueSignature.c_str());

                    if (m_keyType->Type != TypeId::Invalid && m_valueType->Type != TypeId::Invalid)
                    {
                        m_type = TypeId::Dictionary;
                    }
                }
                break;
            }
            case '(': // we are an array of structs
            {
                PopulateFields(signature.substr(1));
                if (m_fields != nullptr)
                {
                    m_type = TypeId::StructArray;
                }
                break;
            }
            default:
                break;
            }

            break;
        }
        case '(': // we are a struct
        {
            PopulateFields(signature);
            if (m_fields != nullptr)
            {
                m_type = TypeId::Struct;
            }
            break;
        }
        default:
            break;
        }
    }

    void AllJoynTypeDefinition::PopulateFields(const string& structSignature)
    {
        auto length = structSignature.length();
        if (length < 3 || structSignature[0] != '(' || structSignature[length - 1] != ')')
        {
            return;
        }
        m_fields = CreateTypeDefintions(structSignature.substr(1, length - 2));
    }

    IVector<ITypeDefinition ^>^ AllJoynTypeDefinition::CreateTypeDefintions(Platform::String^ signature)
    {
        return CreateTypeDefintions(AllJoynHelpers::PlatformToMultibyteStandardString(signature));
    }

    Vector<ITypeDefinition ^>^ AllJoynTypeDefinition::CreateTypeDefintions(const string& signature)
    {
        auto typeDefinitions = ref new Vector<ITypeDefinition ^>();
        vector<string> completeTypeSignatureVector;
        TypeConversionHelpers::CreateCompleteTypeSignatureArrayFromSignature(signature.c_str(), completeTypeSignatureVector);

        for (auto& completeTypeSignature : completeTypeSignatureVector)
        {
            auto typeDef = ref new AllJoynTypeDefinition(completeTypeSignature);

            if (typeDef->Type == TypeId::Invalid)
            {
                return nullptr;
            }
            typeDefinitions->Append(typeDef);
        }
        return typeDefinitions;
    }

    Vector<ParameterInfo ^>^ AllJoynTypeDefinition::CreateParameterInfo(const string& signature, const vector<string>& argNames)
    {
        auto typeInfoVector = AllJoynTypeDefinition::CreateTypeDefintions(signature);
        auto parameterInfoVector = ref new Vector<ParameterInfo ^>();

        size_t index = 0;
        for (auto typeInfo : typeInfoVector)
        {
            auto paramInfo = ref new ParameterInfo();
            paramInfo->TypeDefinition = typeInfo;
            paramInfo->Name = index < argNames.size() ? AllJoynHelpers::MultibyteToPlatformString(argNames[index++].c_str()) : nullptr;
            parameterInfoVector->Append(paramInfo);
        }

        return parameterInfoVector;
    }

    string AllJoynTypeDefinition::GetAllJoynSignatureString(ITypeDefinition^ typeDefinition)
    {
        auto allJoynTypeDefinition = dynamic_cast<AllJoynTypeDefinition^>(typeDefinition);
        if (allJoynTypeDefinition != nullptr)
        {
            return allJoynTypeDefinition->m_allJoynSignature;
        }

        string signature;
        if ((typeDefinition->Type & TypeId::ArrayByteMask) == TypeId::ArrayByte)
        {
            signature += 'a';
            auto arrayType = static_cast<TypeId>(static_cast<uint32>(typeDefinition->Type) >> 8);

            if (arrayType == TypeId::Struct)
            {
                signature += '(';
                for (ITypeDefinition^ fieldTypeDef : typeDefinition->Fields)
                {
                    signature += GetAllJoynSignatureString(fieldTypeDef);
                }
                signature += ")";
            }
            else
            {
                signature += static_cast<char>(arrayType);
            }
        }
        else if (typeDefinition->Type == TypeId::Struct)
        {
            signature += '(';
            for (ITypeDefinition^ fieldTypeDef : typeDefinition->Fields)
            {
                signature += GetAllJoynSignatureString(fieldTypeDef);
            }
            signature += ")";
        }
        else if (typeDefinition->Type == TypeId::Dictionary)
        {
            signature += "a{";
            signature += GetAllJoynSignatureString(typeDefinition->KeyType);
            signature += GetAllJoynSignatureString(typeDefinition->ValueType);
            signature += '}';
        }
        else
        {
            signature += static_cast<char>(typeDefinition->Type);
        }
        return signature;
    }
}