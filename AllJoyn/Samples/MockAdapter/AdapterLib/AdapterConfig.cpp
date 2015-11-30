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
#include "AdapterConfig.h"

#define ADAPTER_CONFIG_XML_FILE L"AdapterConfig.xml"

const WCHAR DEFAULT_ADAPTER_CFG_XML[] =
L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
L"<AdapterCfg></AdapterCfg>\n";

AdapterConfig::AdapterConfig()
{
    m_pXmlDocument = ref new XmlDocument();
}

AdapterConfig::~AdapterConfig()
{
}

_Use_decl_annotations_
HRESULT
AdapterConfig::SetConfig(String^ xmlString)
{
    HRESULT hr = S_OK;

    //If the xml string is empty, then set default configurations
    if (xmlString == nullptr)
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    else
    {
        CHK_HR(fromString(xmlString));
        CHK_HR(toFile());
    }

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
AdapterConfig::GetConfig(String^* xmlString)
{
    HRESULT hr = S_OK;
    CHK_HR(fromFile());
    CHK_HR(toString(xmlString));

CleanUp:
    return hr;
}

HRESULT
AdapterConfig::Init()
{
    HRESULT hr = S_OK;
    if (!isConfigFilePresent())
    {
        CHK_HR(initXml());
        CHK_HR(toFile());
    }

CleanUp:
    return hr;
}

HRESULT
AdapterConfig::fromFile()
{
    HRESULT hr = S_OK;
    task<StorageFile^> getConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    // Load the XML file
    try
    {
        getConfigFileTask = create_task(appFolder->GetFileAsync(ADAPTER_CONFIG_XML_FILE));
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
        goto CleanUp;
    }

    getConfigFileTask
    .then([this](StorageFile^ sFile)
    {
        return  XmlDocument::LoadFromFileAsync(sFile);
    })
    .then([this](XmlDocument^ doc)
    {
        m_pXmlDocument = doc;
    })
    .then([&](task<void> checkExceptionTask)
    {
        try
        {
            checkExceptionTask.get();
        }
        catch (Platform::Exception^ ex)
        {
            hr = ex->HResult;
        }
    })
    .wait();

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
AdapterConfig::fromString(String^ xmlString)
{
    HRESULT hr = S_OK;

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    m_pXmlDocument->LoadXml(xmlString);

CleanUp:
    return hr;
}

HRESULT
AdapterConfig::toFile()
{
    HRESULT hr = S_OK;
    task<StorageFile^> createConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // Save/Update the configuration file
    try
    {
        createConfigFileTask = create_task(appFolder->CreateFileAsync(
            ADAPTER_CONFIG_XML_FILE,
            CreationCollisionOption::ReplaceExisting));
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
        goto CleanUp;
    }

    createConfigFileTask
    .then([this](StorageFile^ sFile)
    {
        return m_pXmlDocument->SaveToFileAsync(sFile);
    })
    .then([&](task<void> checkExceptionTask)
    {
        try
        {
            checkExceptionTask.get();
        }
        catch (Platform::Exception^ ex)
        {
            hr = ex->HResult;
        }
    })
    .wait();

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
AdapterConfig::toString(String^* xmlString)
{
    HRESULT hr = S_OK;

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    *(xmlString) = m_pXmlDocument->GetXml();

CleanUp:
    return hr;
}

HRESULT
AdapterConfig::initXml()
{
    HRESULT hr = S_OK;
    StringReference target(DEFAULT_ADAPTER_CFG_XML);
    m_pXmlDocument->LoadXml(target.GetString());

    return hr;
}

bool
AdapterConfig::isConfigFilePresent()
{
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;
    IStorageItem^ sItem;

    task<IStorageItem^> tryGetFileTask = create_task(appFolder->TryGetItemAsync(ADAPTER_CONFIG_XML_FILE));
    tryGetFileTask.then([&](IStorageItem^ item)
    {
        sItem = item;
    }).wait();

    return sItem != nullptr;
}