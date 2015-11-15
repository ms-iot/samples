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

#include "DsbServiceNames.h"
#include "Bridge.h"
#include "ConfigManager.h"
#include "CspAdapter.h"

using namespace BridgeRT;
using namespace DsbCommon;

CspAdapter::CspAdapter()
    : AllJoynFileTransfer(),
	m_configManager(nullptr)
{
}

CspAdapter::~CspAdapter()
{
}


//************************************************************************************************
//
// Initialize
//
// Creates an All Joyn Object for this Bridge that handles Adapter Specific Configuration data.
// The adapter implements the microsoft standard alljoyn management configuration interface.
//
//************************************************************************************************
QStatus CspAdapter::Initialize(_In_ alljoyn_busattachment* bus, _In_ ConfigManager *configManager)
{
    QStatus status = ER_OK;

    try
    {
        m_busObjectPath = "/AdapterConfig";
    }
    catch (std::bad_alloc& exception)
    {
        UNREFERENCED_PARAMETER(exception);
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    m_configManager = configManager;

    status = AllJoynFileTransfer::Initialize(bus, m_busObjectPath.c_str(), configManager);

leave:
    return status;
}


//************************************************************************************************
//
// Post-File-Write Handler for the Adapter
//
// Converts the temporary file to an array of bytes and pass it to the adapter
//
//************************************************************************************************
HRESULT CspAdapter::PostFileWriteAction(_In_ std::wstring &appRelativeFileName, _Out_ HANDLE *finalEvent)
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD numBytesRead = 0;
    WIN32_FILE_ATTRIBUTE_DATA fileAttribute = { 0 };
    Array<BYTE>^ adapterConfigData;

    StorageFolder^ tempFolder = ApplicationData::Current->LocalFolder;
    String^ tempFilePath = tempFolder->Path + L"\\";
    std::wstring absoluteFileName(L"");

    // default post file write event to nothing
    if (nullptr != finalEvent)
    {
        *finalEvent = NULL;
    }

    // Create the full file name and path to the source directory
    absoluteFileName = tempFilePath->Data();
    absoluteFileName += appRelativeFileName;


    // Figure out how large the new temporary config file is (also verifies that the file is present)
    if (!GetFileAttributesEx(absoluteFileName.c_str(), GetFileExInfoStandard, &fileAttribute))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

    // Create a temporary array for holding the adapter configuration data
    adapterConfigData = ref new Array<BYTE>(fileAttribute.nFileSizeLow);

    // Open a handle to the temporary data file
    hFile = CreateFile2(absoluteFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

    // Read the entire contents of the temporary data file
    if (!ReadFile(hFile, adapterConfigData->Data, fileAttribute.nFileSizeLow, &numBytesRead, nullptr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

    // If the number of bytes read does not equal the expected number of bytes then this was unexpected.
    if (numBytesRead != fileAttribute.nFileSizeLow)
    {
        hr = HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY);
        goto leave;
    }

    // Pass the configuration data to the adapter.
    hr = m_configManager->SetAdapterConfig(adapterConfigData);

leave:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
    return hr;
}



//************************************************************************************************
//
// Pre-File-Read Handler for the Adapter
//
// Make a temporary file of the current adapter configuration settings and provide filename
// to the caller
//
//************************************************************************************************
HRESULT CspAdapter::PreFileReadAction(_Out_ std::wstring &appRelativeFileName)
{
    HRESULT hr = S_OK;
    GUID guid = { 0 };
    WCHAR tempString[MAX_GUID_STRING_LEN];
    int guidCharLength = 0;
    StorageFolder^ tempFolder = ApplicationData::Current->LocalFolder;
    String^ tempFilePath = tempFolder->Path + L"\\";
    HANDLE hFile = INVALID_HANDLE_VALUE;
    Array<BYTE>^ adapterConfigData;
    DWORD numBytesWritten=0;
    appRelativeFileName.clear();
    m_readFileNameFullPath.clear();

    // Get the adapter configuration file
    hr = m_configManager->GetAdapterConfig(&adapterConfigData);
    if (FAILED(hr))
    {
        goto leave;
    }

    // create 'new' temp file name
    hr = CoCreateGuid(&guid);
    if (FAILED(hr))
    {
        goto leave;
    }

    //Convert GUID into String
    guidCharLength = StringFromGUID2(guid, tempString, MAX_GUID_STRING_LEN);
    if (0 == guidCharLength)
    {
        hr = E_FAIL;
        goto leave;
    }
    appRelativeFileName = tempString;
    // Create the full file name and path to the source directory
    m_readFileNameFullPath = tempFilePath->Data();
    m_readFileNameFullPath += tempString;

    // create 'new' temp file
    hFile = CreateFile2(m_readFileNameFullPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

    // Write all adapter data to the temporary file
    if (!WriteFile(hFile, adapterConfigData->Data, adapterConfigData->Length, &numBytesWritten, nullptr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

leave:

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;

        if (FAILED(hr))
        {
            DeleteFile(m_readFileNameFullPath.c_str());
            appRelativeFileName.clear();
            m_readFileNameFullPath.clear();
        }
    }

    return hr;
}


//************************************************************************************************
//
// Delete the temporary configuration file
//
//************************************************************************************************
HRESULT CspAdapter::PostFileReadAction(void)
{
    DeleteFile(m_readFileNameFullPath.c_str());
    return S_OK;
}
