#include "pch.h"
#include "BridgeUtils.h"
#include <cvt\wstring>
#include <codecvt>
#include <strsafe.h>

using namespace BridgeRT;
using namespace std;
using namespace Platform;

_Use_decl_annotations_
int
BridgeRT::FormatString(String^& OutputString, const wchar_t* FormatWsz, ...)
{
    // Default string buffer
    wchar_t stringBufferWsz[1024] = { 0 };
    //
    // Alternate buffer in case the default buffer is not
    // big enough to hold the output string.
    //
    wchar_t* altStringBufferPtr = nullptr;
    // Current buffer parameters
    size_t stringMaxCharCount = ARRAYSIZE(stringBufferWsz);
    wchar_t* stringBufferToUsePtr = &stringBufferWsz[0];
    int charCount = 0;

    va_list argList;
    va_start(argList, FormatWsz);

    try
    {
        do
        {
            charCount = _vsnwprintf_s(
                stringBufferToUsePtr,
                stringMaxCharCount - 1,
                _TRUNCATE,
                FormatWsz,
                argList
                );
            if (charCount == -1)
            {
                //
                // Default buffer is not big enough,
                // reallocate, and use alternate buffer...
                //

                if (stringMaxCharCount >= MAX_STRING_SIZE)
                {
                    goto done;
                }
                stringMaxCharCount *= 2;

                delete[] altStringBufferPtr;
                altStringBufferPtr = new wchar_t[stringMaxCharCount];

                RtlZeroMemory(altStringBufferPtr, stringMaxCharCount*sizeof(wchar_t));

                stringBufferToUsePtr = altStringBufferPtr;

            } // Buffer is too small...

        } while (charCount == -1);

        OutputString = ref new String(stringBufferToUsePtr);
    }
    catch (bad_alloc)
    {
        charCount = -1;
    }
    catch (OutOfMemoryException^)
    {
        charCount = -1;
    }

done:
    va_end(argList);

    delete[] altStringBufferPtr;

    return charCount;
}