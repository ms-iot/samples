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

#include "Bridge.h"
#include "DsbServiceNames.h"
#include "CspBridge.h"
#include "ConfigManager.h"
#include "BridgeUtils.h"

using namespace BridgeRT;


CspBridge::CspBridge()
    : AllJoynFileTransfer(),
	m_configManager(nullptr)
{
    m_tempConfigFilePath.clear();
}

CspBridge::~CspBridge()
{
}


//************************************************************************************************
//
// Initialize
//
// Creates an AllJoyn Object for this Bridge that handles Bridge Specific Configuration data.
// The bridge implements the Microsoft standard alljoyn management configuration interface.
//
//************************************************************************************************
QStatus CspBridge::Initialize(_In_ alljoyn_busattachment* bus, _In_ ConfigManager *configManager, _In_ BridgeConfig& bridgeConfig)
{
    QStatus status = ER_OK;

    try
    {
        m_busObjectPath = "/BridgeConfig";
    }
    catch (...)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

	m_configManager = configManager;

    try
    {
        StorageFolder^ tempFolder = ApplicationData::Current->LocalFolder;
        String^ tempFilePath = tempFolder->Path + L"\\";
        m_srcConfigFilePath = tempFilePath->Data();
        m_srcConfigFilePath += bridgeConfig.FileName()->Data();
    }
    catch (...)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    status = AllJoynFileTransfer::Initialize(bus, m_busObjectPath.c_str(), configManager);

leave:
    return status;
}

/************************************************************************************************
*
* Post File Write
*
* We transfer the bridge configuration file directly.  Not temporary file is necessary
*
************************************************************************************************/
HRESULT CspBridge::PostFileWriteAction(_In_ std::wstring &appRelativeFileName, _Out_ HANDLE *finalEvent)
{
	return m_configManager->SetDeviceConfig(appRelativeFileName, finalEvent);
}

//************************************************************************************************
//
// Pre File Read Handler for Bridge
//
// For the bridge, transfer the bridge configuration file directly.
// No temporary file is necessary
//
//************************************************************************************************
HRESULT CspBridge::PreFileReadAction(_Out_ std::wstring &appRelativeFileName)
{
    HRESULT hr = S_OK;
    try
    {
        WCHAR tempString[MAX_GUID_STRING_LEN];
        int guidLength = 0;
        GUID guid = { 0 };
        StorageFolder^ tempFolder = ApplicationData::Current->LocalFolder;
        String^ tempFilePath = tempFolder->Path + L"\\";

        // create 'new' temp file name
        hr = CoCreateGuid(&guid);
        if (FAILED(hr))
        {
            goto leave;
        }

        //Convert GUID into String
        guidLength = StringFromGUID2(guid, tempString, MAX_GUID_STRING_LEN);
        if (0 == guidLength)
        {
            hr = E_FAIL;
            goto leave;
        }

        m_tempConfigFilePath = tempFilePath->Data();
        m_tempConfigFilePath += tempString;
        appRelativeFileName = tempString;

        AutoLock bridgeLocker(DsbBridge::SingleInstance()->GetLock());
        hr = CopyFile2(m_srcConfigFilePath.c_str(), m_tempConfigFilePath.c_str(), nullptr);
        if (FAILED(hr))
        {
            DeleteFile(m_tempConfigFilePath.c_str());
        }
    }
    catch (...)
    {
        hr = E_OUTOFMEMORY;
    }

leave:

    return hr;
}

//************************************************************************************************
//
// Post File Read Handler.
//
//************************************************************************************************
HRESULT CspBridge::PostFileReadAction(void)
{
    DeleteFile(m_tempConfigFilePath.c_str());
    return S_OK;
}