#include "pch.h"

_Check_return_ int32 TypeConversionHelpers::AppendNextCompleteType(_In_ PCSTR signature, _Out_ std::vector<char>* typeSignature)
{
    if (signature[0] == '\0')
    {
        return ER_BUS_BAD_SIGNATURE;
    }

    typeSignature->push_back(signature[0]);
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

            typeSignature->push_back(signature[index]);
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
        return AppendNextCompleteType(signature + 1, typeSignature);
    }
    else
    {
        return S_OK;
    }
}

_Check_return_ int32 TypeConversionHelpers::GetDictionaryTypeSignatures(_In_ PCSTR signature, _Out_ std::vector<char>* keySignature, _Out_ std::vector<char>* valueSignature)
{
    if ((strlen(signature) < 3) || (signature[0] != 'a') || (signature[1] != '{'))
    {
        return ER_BUS_BAD_SIGNATURE;
    }

    // Skip past the opening "a{".
    signature += 2;
    RETURN_IF_QSTATUS_ERROR(AppendNextCompleteType(signature, keySignature));
    signature += keySignature->size();
    RETURN_IF_QSTATUS_ERROR(AppendNextCompleteType(signature, valueSignature));
    keySignature->push_back('\0');
    valueSignature->push_back('\0');
    return ER_OK;
}
