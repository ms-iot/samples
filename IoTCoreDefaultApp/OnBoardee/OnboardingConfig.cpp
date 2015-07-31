// Copyright (c) Microsoft. All rights reserved.


#include "pch.h"
#include "OnboardingConfig.h"
#include <ppltasks.h>

using namespace Concurrency;
using namespace Platform;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::ApplicationModel;
using namespace Windows::Storage;

const WCHAR DEFAULT_ONBOARD_CFG_XML[] =
L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
L"<OnBoardConfig>"
L"    <SSID>IoTOnBoardingAP</SSID>"
L"    <Password>password</Password>"
L"    <AboutData>"
L"        <DefaultDescription>IoT on boarding</DefaultDescription>"
L"        <DefaultManufacturer>Microsoft</DefaultManufacturer>"
L"    </AboutData>"
L"</OnBoardConfig>";

OnboardingConfig::OnboardingConfig()
{
    m_pXmlDocument = ref new XmlDocument();
}

OnboardingConfig::~OnboardingConfig()
{

}

String^ OnboardingConfig::SSID()
{
    return FindInXml("/OnBoardConfig/SSID");
}

String^ OnboardingConfig::Password()
{
    return FindInXml("/OnBoardConfig/Password");
}

String^ OnboardingConfig::DefaultDescription()
{
    return FindInXml("/OnBoardConfig/AboutData/DefaultDescription");
}

String^ OnboardingConfig::DefaultManufacturer()
{
    return FindInXml("/OnBoardConfig/AboutData/DefaultManufacturer");
}

String^ OnboardingConfig::FindInXml(Platform::String^ queryString)
{
    String^ result(L"");
    // Create XPATH Query.
    // Try to find specified node relative to the current node
    auto ourNode = m_pXmlDocument->SelectSingleNode(queryString);

    // If node found, then get the value out of the node and return it
    if (ourNode != nullptr)
    {
        result = dynamic_cast<String^>(ourNode->InnerText);
    }

    return result;
}

HRESULT OnboardingConfig::Init(String^ pFileName)
{
    HRESULT hr = S_OK;

    // If a filename was specified, try to initialize the configuration document from the specified name
    if (pFileName != nullptr)
    {
        //  Does the specified file exist?
        if (!isFilePresent(ApplicationData::Current->LocalFolder, pFileName))
        {
            //Not present. see if it is present in appxpackage
            if (isFilePresent(Package::Current->InstalledLocation, pFileName))
            {
                //copy the file
                auto srcFolder = Package::Current->InstalledLocation;
                auto destFolder = ApplicationData::Current->LocalFolder;

                try
                {
                    create_task(srcFolder->GetFileAsync(pFileName)).then([destFolder](StorageFile^ file)
                    {
                        return file->CopyAsync(destFolder);
                    }).wait();
                }
                catch (Exception^)
                {
                    //ignore
                }
            }
        }

        if (!isFilePresent(ApplicationData::Current->LocalFolder, pFileName))
        {
            hr = initXml();
            if (FAILED(hr))
            {
                goto CleanUp;
            }
            hr = ToFile(pFileName);
            if (FAILED(hr))
            {
                goto CleanUp;
            }
        }
        // Otherwise the file is loaded
        else
        {
            // Try to read file
            hr = FromFile(pFileName);
            if (FAILED(hr))
            {
                goto CleanUp;
            }
        }
    }

CleanUp:
    return hr;
}

HRESULT OnboardingConfig::initXml()
{
    HRESULT hr = S_OK;
    StringReference target(DEFAULT_ONBOARD_CFG_XML);
    m_pXmlDocument->LoadXml(target.GetString());

    return hr;
}

HRESULT OnboardingConfig::FromFile(String^ pFileName)
{
    HRESULT hr = S_OK;
    task<StorageFile^> getConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // Make sure that a filename is (or was) specified for this configuration file
    if ((pFileName == nullptr))
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    // otherwise, the input is valid so load the XML
    // Update the file name and convert to a variant
    m_fileName = pFileName;

    // Load the XML file 
    try
    {
        getConfigFileTask = create_task(appFolder->GetFileAsync(m_fileName));
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

HRESULT OnboardingConfig::ToFile(String^ pFileName)
{
    HRESULT hr = S_OK;
    task<StorageFile^> createConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // Make sure that a filename is (or was) specified for this configuration file
    if (((pFileName == nullptr) && (m_fileName->Length() == 0)))
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    // If filename is specified then change it before saving
    if (pFileName != nullptr)
    {
        m_fileName = pFileName;
    }

    // Save/Update the configuration file
    try
    {
        createConfigFileTask = create_task(appFolder->CreateFileAsync(
            m_fileName,
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

bool OnboardingConfig::isFilePresent(StorageFolder^ appFolder, String^ pFileName)
{
    IStorageItem^ sItem = nullptr;

    task<IStorageItem^> tryGetFileTask = create_task(appFolder->TryGetItemAsync(pFileName));
    tryGetFileTask.then([&](IStorageItem^ item)
    {
        sItem = item;
    }).wait();

    return sItem != nullptr;
}