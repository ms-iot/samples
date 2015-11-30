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
#include <ppltasks.h>
#include <string>

#include "AdapterConfig.h"
#include "Misc.h"

using namespace Platform;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Storage;
using namespace Windows::ApplicationModel;
using namespace concurrency;
using namespace std;

using namespace DsbCommon;


//
// Adapter configurable file name
//
#define ADAPTER_CFG_XML_FILE L"AdapterConfig.xml"

//
// Adapter configurable parameters XML tags
//

// The root BACnet configuration element tag
#define BACNET_CONFIG_XML_ELEMENT L"BACnetConfig"

// BACnet stack parameters tags
#define BACNET_STACK_XML_ELEMENT L"BACnetStack"
    #define BACNET_STACK_XML_BBMD_IP_ADDR L"BBMD_IPAddress"
    #define BACNET_STACK_XML_BBMD_IP_PORT L"BBMD_Port"
    #define BACNET_STACK_XML_REQUEST_PRIORITY L"RequestPriority"
    #define BACNET_STACK_XML_NETWORK_INTERFACE L"NetworkInterface"
    #define BACNET_STACK_XML_DEV_INSTANCE_MIN L"DeviceInstanceMin"
    #define BACNET_STACK_XML_DEV_INSTANCE_MAX L"DeviceInstanceMax"

// General configuration parameters tags
#define GENERAL_CFG_XML_ALLOWED_DEVICE_LIST_ELEMENT L"AllowedDeviceList"
    #define GENERAL_CFG_XML_ALLOWED_DEVICE_LIST_ALLOWED L"Allowed"
    // Sample device filter token
    #define BACNET_DEVICE_FILTER_TOKEN_SAMPLE   L"Device_Model_Filter_Token"

//
// AdapterConfig.
// Description:
//  The class that includes:
//  - BACnet adapter configuration parameters.
//  - BACnet interface (stack) parameters.
//
AdapterConfig::AdapterConfig()
    : isValid(false)
    , NetworkNumber(BACNET_DEF_NETWORK_ADDRESS)
    , DeviceInstanceMin(BACNET_DEF_MIN_DEVICE_INSTANCE)
    , DeviceInstanceMax(BACNET_DEF_MAX_DEVICE_INSTANCE)
    , BbmdIpPort(BACNET_DEF_BBMD_IP_PORT)
    , BbmdIpAddress(BACNET_DEF_BBMD_IP_ADDR)
    , BbmdTimetoliveSeconds(BACNET_DEF_BBMD_TTL)
    , RequestPriority(BACNET_DEF_REQUEST_PRIORITY)
    , RxPacketTimeoutMsec(BACNET_PACKET_TIMEOUT_MSEC)
    , DeviceDiscoveryIntervalMin(DEVICE_DISCOVERY_DEF_INTERVAL_MIN)
    , DeviceDiscoveryIdleTimeMsec(DEVICE_DISCOVERY_DEF_IDLE_MSEC)
{
    this->isValid = true;

    this->AllowedDeviceList.push_back(ref new String(BACNET_DEVICE_FILTER_TOKEN_SAMPLE));
}

AdapterConfig::~AdapterConfig()
{
}

HRESULT
AdapterConfig::Init()
{
    HRESULT hr = S_OK;

    // If the configuration file exist, get the configurations
    if (this->isConfigFilePresent())
    {
        // Load xml document from adapter configuration file
        hr = this->fromFile();
    }
    else
    {
        // Save xml document from adapter configuration file
        hr = this->toFile();
    }

    this->isValid = hr == S_OK;

    return hr;
}

_Use_decl_annotations_
HRESULT
AdapterConfig::SetConfig(String^ XmlString)
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

        // Load and validate the parameters
        hr = this->fromXml(xmlDoc);
        if (FAILED(hr))
        {
            return hr;
        }

        DSB_ASSERT(this->isValid);

        hr = this->toFile();
    }
    catch (Exception^ e)
    {
        return e->HResult;
    }

    return hr;
}

_Use_decl_annotations_
HRESULT
AdapterConfig::GetConfig(String^* XmlStringPtr)
{
    HRESULT hr = S_OK;
    XmlDocument^ xmlDoc;

    if (XmlStringPtr == nullptr)
    {
        return E_INVALIDARG;
    }

    // Save/Update the configuration file
    try
    {
        xmlDoc = ref new XmlDocument();

        hr = this->toXml(xmlDoc);
        if (FAILED(hr))
        {
            return hr;
        }

        *XmlStringPtr = xmlDoc->GetXml();
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
    }

    return hr;
}

HRESULT
AdapterConfig::fromFile()
{
    HRESULT hr = S_OK;
    XmlDocument^ xmlDoc;
    task<StorageFile^> getConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // First load the XML document from file
    try
    {
        xmlDoc = ref new XmlDocument();

        getConfigFileTask = create_task(appFolder->GetFileAsync(ADAPTER_CFG_XML_FILE));
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
        goto CleanUp;
    }

    getConfigFileTask
    .then([&](StorageFile^ sFile)
    {
        return XmlDocument::LoadFromFileAsync(sFile);
    })
    .then([&](XmlDocument^ doc)
    {
        xmlDoc = doc;
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

    // Then load the parameters from the XML document
    hr = this->fromXml(xmlDoc);

CleanUp:
    return hr;
}

HRESULT
AdapterConfig::toFile()
{
    HRESULT hr = S_OK;
    XmlDocument^ xmlDoc;
    task<StorageFile^> createConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // Save/Update the configuration file
    try
    {
        // First dump the parameter to a XmlDocument
        xmlDoc = ref new XmlDocument();

        hr = this->toXml(xmlDoc);
        if (FAILED(hr))
        {
            goto CleanUp;
        }

        // Then save the XmlDocument to disk
        createConfigFileTask = create_task(appFolder->CreateFileAsync(
            ADAPTER_CFG_XML_FILE,
            CreationCollisionOption::ReplaceExisting)
            );
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
        goto CleanUp;
    }

    createConfigFileTask
    .then([&](StorageFile^ sFile)
    {
        return xmlDoc->SaveToFileAsync(sFile);
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
AdapterConfig::fromXml(XmlDocument^ XmlDoc)
{
    HRESULT hr = S_OK;
    AdapterConfig tempConfig;

    XmlElement^ rootElement = XmlDoc->DocumentElement;
    if (rootElement == nullptr)
    {
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    try
    {
        //
        // BACnet stack parameters
        //
        {
            XmlElement^ stackParamsElement = dynamic_cast<XmlElement^>(rootElement->SelectSingleNode(BACNET_STACK_XML_ELEMENT));
            if (stackParamsElement == nullptr)
            {
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }

            XmlNamedNodeMap^ stackAttributes = stackParamsElement->Attributes;
            IXmlNode^ stackAttr;

            // IP address
            stackAttr = stackAttributes->GetNamedItem(BACNET_STACK_XML_BBMD_IP_ADDR);
            if (stackAttr == nullptr)
            {
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            tempConfig.BbmdIpAddress = stackAttr->NodeValue->ToString();

            // IP port
            stackAttr = stackAttributes->GetNamedItem(BACNET_STACK_XML_BBMD_IP_PORT);
            if (stackAttr == nullptr)
            {
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            else
            {
                UINT32 ipPort = stoul(stackAttr->NodeValue->ToString()->Data());
                if (ipPort > MAXWORD)
                {
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
                tempConfig.BbmdIpPort = ipPort;
            }

            // Network interface (optional)
            stackAttr = stackAttributes->GetNamedItem(BACNET_STACK_XML_NETWORK_INTERFACE);
            if (stackAttr != nullptr)
            {
                tempConfig.NetworkInterface = stackAttr->NodeValue->ToString();
            }

            // Request priority (optional)
            stackAttr = stackAttributes->GetNamedItem(BACNET_STACK_XML_REQUEST_PRIORITY);
            if (stackAttr != nullptr)
            {
                UINT32 requestPriority = stoul(stackAttr->NodeValue->ToString()->Data());
                if ((requestPriority < BACNET_REQUEST_PRIORITY_HIGHEST) ||
                    (requestPriority > BACNET_REQUEST_PRIORITY_LOWEST))
                {
                    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
                tempConfig.RequestPriority = UINT8(requestPriority);
            }

            // Min device instance
            stackAttr = stackAttributes->GetNamedItem(BACNET_STACK_XML_DEV_INSTANCE_MIN);
            if (stackAttr == nullptr)
            {
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            tempConfig.DeviceInstanceMin = stoul(stackAttr->NodeValue->ToString()->Data());

            // Max device instance
            stackAttr = stackAttributes->GetNamedItem(BACNET_STACK_XML_DEV_INSTANCE_MAX);
            if (stackAttr == nullptr)
            {
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            tempConfig.DeviceInstanceMax = stoul(stackAttr->NodeValue->ToString()->Data());

        } // BACnet stack parameters

        //
        // General configuration parameters
        //

        // Allowed device list
        {
            XmlElement^ allowedDevsElement = dynamic_cast<XmlElement^>(rootElement->SelectSingleNode(GENERAL_CFG_XML_ALLOWED_DEVICE_LIST_ELEMENT));
            if (allowedDevsElement == nullptr)
            {
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            tempConfig.AllowedDeviceList.clear();

            // Device filters (optional)
            XmlElement^ allowedDev = dynamic_cast<XmlElement^>(allowedDevsElement->FirstChild);
            while (allowedDev != nullptr)
            {
                if (allowedDev->NodeName != GENERAL_CFG_XML_ALLOWED_DEVICE_LIST_ALLOWED)
                {
                    return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                }

                // Skip the sample filter...
                if (_wcsicmp(allowedDev->InnerText->Data(), BACNET_DEVICE_FILTER_TOKEN_SAMPLE) != 0)
                {
                    tempConfig.AllowedDeviceList.push_back(ToLower(allowedDev->InnerText->Data()));
                }

                // Next filter...
                allowedDev = dynamic_cast<XmlElement^>(allowedDev->NextSibling);

            } // More filters

        } // Allowed device list

        *this = tempConfig;
        this->isValid = true;
    }
    catch (Exception^ e)
    {
        hr = e->HResult;
    }

    return hr;
}

_Use_decl_annotations_
HRESULT
AdapterConfig::toXml(XmlDocument^& XmlDoc)
{
    try
    {
        XmlDoc->AppendChild(XmlDoc->CreateProcessingInstruction(L"xml", L" version='1.0' encoding='UTF-8'"));
        XmlElement^ adapterConfigElement = XmlDoc->CreateElement(BACNET_CONFIG_XML_ELEMENT);

        //
        // BACnet stack parameters
        //
        {
            XmlElement^ stackElement = XmlDoc->CreateElement(BACNET_STACK_XML_ELEMENT);

            // BBMD IP address
            stackElement->SetAttribute(BACNET_STACK_XML_BBMD_IP_ADDR, this->BbmdIpAddress);

            // BBMD IP Port
            {
                String^ portStr;
                if (FormatString(portStr, L"%d", this->BbmdIpPort) == -1)
                {
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                }
                stackElement->SetAttribute(BACNET_STACK_XML_BBMD_IP_PORT, portStr);
            }

            // Network interface
            stackElement->SetAttribute(BACNET_STACK_XML_NETWORK_INTERFACE, this->NetworkInterface);

            // Request priority
            {
                String^ prioritytStr;
                if (FormatString(prioritytStr, L"%d", this->RequestPriority) == -1)
                {
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                }
                stackElement->SetAttribute(BACNET_STACK_XML_REQUEST_PRIORITY, prioritytStr);
            }

            // Device instance min/max
            {
                String^ devInstaceStr;
                if (FormatString(devInstaceStr, L"%d", this->DeviceInstanceMin) == -1)
                {
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                }
                stackElement->SetAttribute(BACNET_STACK_XML_DEV_INSTANCE_MIN, devInstaceStr);

                if (FormatString(devInstaceStr, L"%d", this->DeviceInstanceMax) == -1)
                {
                    return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                }
                stackElement->SetAttribute(BACNET_STACK_XML_DEV_INSTANCE_MAX, devInstaceStr);
            }

            adapterConfigElement->AppendChild(stackElement);

        } // BACnet stack parameters

        //
        // General configuration parameters
        //

        // Allowed device list
        {
            XmlElement^ allowedDevicesElement = XmlDoc->CreateElement(GENERAL_CFG_XML_ALLOWED_DEVICE_LIST_ELEMENT);

            for (String^ devModelToken : this->AllowedDeviceList)
            {
                XmlElement^ allowedFilterElement = XmlDoc->CreateElement(GENERAL_CFG_XML_ALLOWED_DEVICE_LIST_ALLOWED);

                allowedFilterElement->InnerText = ToLower(devModelToken->Data());
                allowedDevicesElement->AppendChild(allowedFilterElement);
            }
            adapterConfigElement->AppendChild(allowedDevicesElement);
        }

        XmlDoc->AppendChild(adapterConfigElement);
    }
    catch (Exception^ e)
    {
        return e->HResult;
    }

    return S_OK;
}

bool
AdapterConfig::isConfigFilePresent()
{
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;
    IStorageItem^ sItem;

    task<IStorageItem^> tryGetFileTask = create_task(appFolder->TryGetItemAsync(ADAPTER_CFG_XML_FILE));
    tryGetFileTask.then([&](IStorageItem^ item)
    {
        sItem = item;
    }).wait();

    return sItem != nullptr;
}

