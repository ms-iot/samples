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
#include "BridgeUtils.h"
#include "AdapterUtils.h"

using namespace std;
using namespace BridgeRT;
using namespace Platform;
using namespace Windows::Storage;
using namespace concurrency;


uint32
AdapterLib::StringToArray(String^ SourceString, Array<BYTE>^* TargetDataArrayPtr)
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
        (*TargetDataArrayPtr) = ref new Array<BYTE>(stringByteLen);

        CopyMemory((*TargetDataArrayPtr)->Data, SourceString->Data(), stringByteLen);
    }
    catch (OutOfMemoryException^ ex)
    {
        status = WIN32_FROM_HRESULT(ex->HResult);
        goto done;
    }

done:
    return status;
}

bool
AdapterLib::FileExist(String^ filePath)
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


bool
AdapterLib::FolderExist(String^ folderName)
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


HRESULT
AdapterLib::CreateFolder(String^ folderName)
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
        if (!AdapterLib::FolderExist(parent))
        {
            hr = AdapterLib::CreateFolder(parent);
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


HRESULT
AdapterLib::CopyFolder(String^ source, String^ destination)
{
    HRESULT hr = S_OK;
    try
    {
        if (!AdapterLib::FolderExist(destination))
        {
            //create the folder
            hr = AdapterLib::CreateFolder(destination);
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
                hr = AdapterLib::CopyFolder(source + L"\\" + item->Name, destination + L"\\" + item->Name);
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


HRESULT
AdapterLib::DeleteFolder(_In_ String^ dir)
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