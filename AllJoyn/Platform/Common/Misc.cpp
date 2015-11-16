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
#include "Misc.h"

//for wstring conversions
#include <cvt\wstring>
#include <codecvt>
#include <Strsafe.h>
#include <ppltasks.h>

using namespace Platform;
using namespace std;
using namespace concurrency;

using namespace Windows::Storage;

namespace DsbCommon
{
    std::string To_Ascii_String(const std::wstring& wstr)
    {
        std::string ascii;
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            ascii = converter.to_bytes(wstr.c_str());
        }
        catch (const std::range_error&)
        {
            ascii.clear();
        }
        return ascii;
    }

    std::string To_Ascii_String(Platform::String^ platformString)
    {
        std::string ascii;
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            ascii = converter.to_bytes(platformString->Data());
        }
        catch (const std::range_error&)
        {
            ascii.clear();
        }
        return ascii;
    }

    std::wstring String_To_Wstring(const std::string& str)
    {
        std::wstring wstr;
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            wstr = converter.from_bytes(str);
        }
        catch (const std::range_error&)
        {
            wstr.clear();
        }
        return wstr;
    }

    _Use_decl_annotations_
    int
    FormatString(String^& OutputString, const wchar_t* FormatWsz, ...)
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
        catch (std::bad_alloc)
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

    _Use_decl_annotations_
    uint32
    StringToArray(String^ SourceString, Platform::Array<BYTE>^* TargetDataArrayPtr)
    {
        uint32 status = ERROR_SUCCESS;
        int stringByteLen = 0;

        if ((SourceString == nullptr) || (TargetDataArrayPtr == nullptr))
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        stringByteLen = (SourceString->Length() + 1) * sizeof(wchar_t);

        try
        {
            (*TargetDataArrayPtr) = ref new Platform::Array<BYTE>(stringByteLen);

            CopyMemory((*TargetDataArrayPtr)->Data, SourceString->Data(), stringByteLen);
        }
        catch (Platform::OutOfMemoryException^ ex)
        {
            status = WIN32_FROM_HRESULT(ex->HResult);
            goto done;
        }

    done:
        return status;
    }

    _Use_decl_annotations_
    String^
    ToLower(const wchar_t* SourceStringWsz)
    {
        // First validate input string
        size_t stringLength;
        if (::StringCbLengthW(
            SourceStringWsz,
            USHRT_MAX,
            &stringLength) != S_OK)
        {
            throw std::length_error("Invalid string");
        }
        stringLength /= sizeof(wchar_t);

        wchar_t* lowerCaseWsz = new wchar_t[stringLength + 1];

        unsigned charInx;
        for (charInx = 0; charInx < stringLength; ++charInx)
        {
            lowerCaseWsz[charInx] = towlower(SourceStringWsz[charInx]);
        }
        lowerCaseWsz[charInx] = L'\0';

        String^ lowerCaseString = ref new String(lowerCaseWsz);

        delete [] lowerCaseWsz;

        return lowerCaseString;
    }

    _Use_decl_annotations_
    bool
    FileExist(String^ filePath)
    {
        try
        {
            create_task(StorageFile::GetFileFromPathAsync(filePath)).wait();
            return true;
        }
        catch (Exception^ ex)
        {
            return false;
        }
    }

    _Use_decl_annotations_
    bool
    FolderExist(String^ folderName)
    {
        try
        {
            create_task(StorageFolder::GetFolderFromPathAsync(folderName)).wait();
            return true;
        }
        catch (Exception^ ex)
        {
            return false;
        }
    }

    _Use_decl_annotations_
    HRESULT
    CreateFolder(String^ folderName)
    {
        HRESULT hr = S_OK;
        wstring wStr{ folderName->Data() };
        auto pos = wStr.find_last_of(L"\\/");
        if (pos == wStr.npos)
        {
            return E_INVALIDARG;
        }
        else
        {
            String^ parent = ref new String(wStr.c_str(), static_cast<unsigned int>(pos));
            String^ newFolder = ref new String(&wStr[pos + 1]);
            if (!FolderExist(parent))
            {
                hr = CreateFolder(parent);
            }
            if (hr == S_OK)
            {
                try
                {
                    create_task(StorageFolder::GetFolderFromPathAsync(parent)).then([=](StorageFolder^ folder)
                    {
                        return folder->CreateFolderAsync(newFolder);
                    }).wait();
                }
                catch (Exception^ ex)
                {
                    hr = ex->HResult;
                }
            }
        }

        return hr;
    }

    _Use_decl_annotations_
    HRESULT
    CopyFolder(String^ source, String^ destination)
    {
        HRESULT hr = S_OK;
        try
        {
            if (!FolderExist(destination))
            {
                //create the folder
                hr = CreateFolder(destination);
                if (hr != S_OK)
                {
                    return hr;
                }
            }

            auto srcFolder = create_task(StorageFolder::GetFolderFromPathAsync(source)).get();
            auto destFolder = create_task(StorageFolder::GetFolderFromPathAsync(destination)).get();

            auto folderItems = create_task(srcFolder->GetItemsAsync()).get();

            for (auto it = folderItems->First(); it->HasCurrent; it->MoveNext())
            {
                IStorageItem^ item = it->Current;
                if (item->IsOfType(StorageItemTypes::File))
                {
                    IStorageFile^ file = create_task(srcFolder->GetFileAsync(item->Name)).get();
                    create_task(file->CopyAsync(destFolder)).wait();
                }
                else
                {
                    //folder
                    hr = CopyFolder(source + L"\\" + item->Name, destination + L"\\" + item->Name);
                    if (hr != S_OK)
                    {
                        return hr;
                    }

                }
            }
        }
        catch (Exception^ ex)
        {
            hr = ex->HResult;
        }

        return hr;
    }

    _Use_decl_annotations_
    HRESULT
    DeleteFolder(_In_ Platform::String^ dir)
    {
        HRESULT hr = S_OK;
        try
        {
            create_task(StorageFolder::GetFolderFromPathAsync(dir)).then([](StorageFolder^ folder)
            {
                folder->DeleteAsync();

            }).wait();
        }
        catch (Exception^ ex)
        {
            hr = ex->HResult;
        }

        return hr;
    }

}   //namespace DsbCommon
