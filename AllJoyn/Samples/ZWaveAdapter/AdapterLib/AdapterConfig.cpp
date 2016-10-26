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
#include "AdapterUtils.h"

#include <string>
#include <ppltasks.h>


using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::Data::Xml::Dom;
using namespace concurrency;
using namespace Windows::ApplicationModel;

using namespace std;


namespace AdapterLib
{
    const wstring AppxConfigFolderName = L"OpenZWave\\config";
    const wstring ConfigFolderName = L"config";
    const wstring ConfigFileName = L"options.xml";
    const wstring ConfigFilePath = L"\\" + ConfigFileName;


    AdapterConfig::AdapterConfig()
    {
    }

    AdapterConfig::~AdapterConfig()
    {
    }

    HRESULT AdapterConfig::GetConfig(String^* XmlStringPtr)
    {
        HRESULT hr = S_OK;

        if (XmlStringPtr == nullptr)
        {
            return E_INVALIDARG;
        }

        try
        {
            //First check if the config file is present in LocalState folder
            wstring wPath = GetUserPath() + ConfigFilePath;
            if (!AdapterLib::FileExist(StringReference(wPath.c_str())))
            {
                //if not get it from AppX folder
                wPath = GetConfigPath() + ConfigFilePath;
            }
            StringReference configPath = wPath.c_str();

            create_task(StorageFile::GetFileFromPathAsync(configPath)).then([&](StorageFile^ sFile)
            {
                return XmlDocument::LoadFromFileAsync(sFile);
            }).then([&](XmlDocument^ doc)
            {
                *XmlStringPtr = doc->GetXml();
            }).then([&](task<void> taskresult)
            {
                try
                {
                    taskresult.get();
                }
                catch (Platform::Exception^ ex)
                {
                    hr = ex->HResult;
                }
            }).wait();
        }
        catch (Platform::Exception^ ex)
        {
            hr = ex->HResult;
        }

        return hr;
    }

    HRESULT AdapterConfig::SetConfig(String^ XmlString)
    {
        HRESULT hr = S_OK;

        if (XmlString == nullptr)
        {
            return E_INVALIDARG;
        }

        try
        {
            XmlDocument^ xmlDoc = ref new XmlDocument();
            xmlDoc->LoadXml(XmlString);

            // Then save the XmlDocument to disk
            create_task(StorageFolder::GetFolderFromPathAsync(ref new String(GetUserPath().c_str()))
                ).then([&](StorageFolder^ configFolder)
            {
                return configFolder->CreateFileAsync(ref new String(ConfigFileName.c_str()),
                    CreationCollisionOption::ReplaceExisting);
            }).then([&](StorageFile^ sFile)
            {
                return xmlDoc->SaveToFileAsync(sFile);
            }).then([&](task<void> taskresult)
            {
                try
                {
                    taskresult.get();
                }
                catch (Platform::Exception^ ex)
                {
                    hr = ex->HResult;
                }
            }).wait();
        }
        catch (Exception^ e)
        {
            return e->HResult;
        }

        return hr;
    }

    HRESULT AdapterConfig::Init()
    {
        HRESULT hr = S_OK;

        return hr;
    }

    wstring AdapterConfig::GetConfigPath()
    {
        return wstring(Package::Current->InstalledLocation->Path->Data()) + L"\\" + AppxConfigFolderName;
    }

    wstring AdapterConfig::GetUserPath()
    {
        return ApplicationData::Current->LocalFolder->Path->Data();

    }
}
