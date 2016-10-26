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
#include "AllJoynHelpers.h"
#include "TypeConversionHelpers.h"

_Check_return_ int32 GetNextCompleteTypeHelper(_In_ PCSTR signature, _Out_ std::string& typeSignature)
{
    if (signature[0] == '\0')
    {
        return ER_BUS_BAD_SIGNATURE;
    }

    typeSignature += signature[0];
    if ((signature[0] == '(') || (signature[0] == '{'))
    {
        char openingParenthesis = signature[0];
        char closingParenthesis = signature[0] == '(' ? ')' : '}';
        // Find the matching closing parenthesis.
        int openParenthesisCount = 1;
        int index = 1;
        while (openParenthesisCount > 0)
        {
            if ('\0' == signature[index])
            {
                return ER_BUS_BAD_SIGNATURE;
            }

            typeSignature += signature[index];
            if (openingParenthesis == signature[index])
            {
                openParenthesisCount++;
            }
            else if (closingParenthesis == signature[index])
            {
                openParenthesisCount--;
            }
            index++;
        }
        return S_OK;
    }
    else if (signature[0] == 'a')
    {
        return GetNextCompleteTypeHelper(signature + 1, typeSignature);
    }
    else
    {
        return S_OK;
    }
}

_Check_return_ int32 TypeConversionHelpers::GetNextCompleteType(_In_ PCSTR signature, _Out_ std::string& typeSignature)
{
    typeSignature.clear();
    return GetNextCompleteTypeHelper(signature, typeSignature);
}

_Check_return_ int32 TypeConversionHelpers::GetDictionaryTypeSignatures(_In_ PCSTR signature, _Out_ std::string& keySignature, _Out_ std::string& valueSignature)
{
    if ((strlen(signature) < 3) || (signature[0] != 'a') || (signature[1] != '{'))
    {
        return ER_BUS_BAD_SIGNATURE;
    }

    // Skip past the opening "a{".
    signature += 2;
    RETURN_IF_QSTATUS_ERROR(GetNextCompleteType(signature, keySignature));
    signature += keySignature.length();
    RETURN_IF_QSTATUS_ERROR(GetNextCompleteType(signature, valueSignature));
    return ER_OK;
}


_Check_return_ int32 TypeConversionHelpers::CreateCompleteTypeSignatureArrayFromSignature(_In_ PCSTR signature, _Out_ std::vector<std::string>& individualTypeSignatures)
{
    individualTypeSignatures.clear();
    std::string singleTypeSignature;

    while (*signature != '\0')
    {
        RETURN_IF_QSTATUS_ERROR(TypeConversionHelpers::GetNextCompleteType(signature, singleTypeSignature));
        individualTypeSignatures.push_back(singleTypeSignature);
        signature += singleTypeSignature.length();
    }
    return ER_OK;
}

_Check_return_ int32 TypeConversionHelpers::GetFieldTypesFromStructSignature(_In_ PCSTR signature, _Out_ std::vector<std::string>& fieldTypeSignatures)
{
    fieldTypeSignatures.clear();
    auto signatureLength = strlen(signature);
    if (signatureLength < 3 || signature[0] != '(' || signature[signatureLength - 1] != ')')
    {
        return ER_BUS_BAD_SIGNATURE;
    }

    // skip past opening '('
    signature++;
    std::string fieldType;
    while (*signature != '\0' && *signature != ')')
    {
        RETURN_IF_QSTATUS_ERROR(TypeConversionHelpers::GetNextCompleteType(signature, fieldType));
        fieldTypeSignatures.push_back(fieldType);
        signature += fieldType.length();
    }
    return ER_OK;
}