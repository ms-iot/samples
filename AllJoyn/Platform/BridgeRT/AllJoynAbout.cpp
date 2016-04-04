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
#include <vector>
#include <strsafe.h>

#include "DsbServiceNames.h"
#include "AllJoynAbout.h"

#include "Shlwapi.h"
#include "alljoyn_c/AjAPI.h"
#include "BridgeUtils.h"

using namespace BridgeRT;


using namespace Windows::Storage;
using namespace Platform;

#define APPDATA_CONTAINER_DSB_SETTINGS  L"DSBSettings"
#define DSB_SETTING_DEVICE_ID           L"ID"

static const size_t DEVICE_GUID_STRING_LEN = 39;   // 38 chars + terminator null

static const char DEFAULT_LANGUAGE_FOR_ABOUT[] = "en";
static const WCHAR UNKNOWN_ADAPTER[] = L"Unknown device";
static const WCHAR UNKNOWN_MANUFACTURER[] = L"Unknown";
static const WCHAR UNKNOWN_VERSION[] = L"0.0.0.0";
static const WCHAR DSB_DEFAULT_APP_NAME[] = L"Device System Bridge";
static const WCHAR DSB_DEFAULT_MODEL[] = L"DSB";
static const WCHAR DSB_DEFAULT_DESCRIPTION[] = L"Device System Bridge";
// {EF116A26-9888-47C2-AE85-B77142F24EFA}
static const GUID DSB_DEFAULT_APP_GUID =
{ 0xef116a26, 0x9888, 0x47c2,{ 0xae, 0x85, 0xb7, 0x71, 0x42, 0xf2, 0x4e, 0xfa } };

AllJoynAbout::AllJoynAbout()
    : m_aboutData(NULL),
    m_aboutObject(NULL),
    m_isAnnounced(false)
{
}

AllJoynAbout::~AllJoynAbout()
{
    ShutDown();
}

QStatus AllJoynAbout::Initialize(_In_ alljoyn_busattachment bus)
{
    QStatus status = ER_OK;

    // sanity check
    if (NULL == bus)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    // create the about object that is used to communicate about data
    // note that the about interface won't be part of the announce
    m_aboutObject = alljoyn_aboutobj_create(bus, UNANNOUNCED);
    if (NULL == m_aboutObject)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // create about data with default language
    m_aboutData = alljoyn_aboutdata_create(DEFAULT_LANGUAGE_FOR_ABOUT);
    if (NULL == m_aboutData)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // fill about data with default value
    status = SetDefaultAboutData();

leave:
    if (ER_OK != status)
    {
        ShutDown();
    }

    return status;
}

void AllJoynAbout::ShutDown()
{
    if (NULL != m_aboutObject)
    {
        if (m_isAnnounced)
        {
            alljoyn_aboutobj_unannounce(m_aboutObject);
        }
        alljoyn_aboutobj_destroy(m_aboutObject);
        m_aboutObject = NULL;
        m_isAnnounced = false;
    }
    if (NULL != m_aboutData)
    {
        alljoyn_aboutdata_destroy(m_aboutData);
        m_aboutData = NULL;
    }
}

QStatus AllJoynAbout::Announce(alljoyn_sessionport sp)
{
    QStatus status = ER_OK;

    // sanity check
    if (NULL == m_aboutObject)
    {
        status = ER_INIT_FAILED;
        goto leave;
    }

    if (!alljoyn_aboutdata_isvalid(m_aboutData, DEFAULT_LANGUAGE_FOR_ABOUT))
    {
        status = ER_ABOUT_ABOUTDATA_MISSING_REQUIRED_FIELD;
        goto leave;
    }

    status = alljoyn_aboutobj_announce(m_aboutObject, sp, m_aboutData);
    if (ER_OK != status)
    {
        goto leave;
    }

    m_isAnnounced = true;

leave:
    return status;
}

QStatus AllJoynAbout::AddObject(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription interfaceDescription)
{
    return alljoyn_busobject_setannounceflag(busObject, interfaceDescription, ANNOUNCED);
}

QStatus AllJoynAbout::RemoveObject(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription interfaceDescription)
{
    return alljoyn_busobject_setannounceflag(busObject, interfaceDescription, UNANNOUNCED);
}

QStatus AllJoynAbout::SetManufacturer(_In_z_ const wchar_t* value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setmanufacturer(m_aboutData, stringValue.c_str(), nullptr);
}

QStatus AllJoynAbout::SetDeviceName(_In_z_ const wchar_t *value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setdevicename(m_aboutData, stringValue.c_str(), nullptr);
}

QStatus AllJoynAbout::SetSWVersion(_In_z_ const wchar_t *value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setsoftwareversion(m_aboutData, stringValue.c_str());
}

QStatus AllJoynAbout::SetHWVersion(_In_z_ const wchar_t *value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_sethardwareversion(m_aboutData, stringValue.c_str());
}

QStatus AllJoynAbout::SetDeviceId(_In_z_ const wchar_t * value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setdeviceid(m_aboutData, stringValue.c_str());
}

QStatus AllJoynAbout::SetModel(_In_z_ const wchar_t * value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setmodelnumber(m_aboutData, stringValue.c_str());
}

QStatus AllJoynAbout::SetDescription(_In_z_ const wchar_t * value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setdescription(m_aboutData, stringValue.c_str(), nullptr);
}

QStatus AllJoynAbout::SetApplicationName(_In_z_ const wchar_t *value)
{
    std::string stringValue = ConvertTo<std::string>(value);
    return alljoyn_aboutdata_setappname(m_aboutData, stringValue.c_str(), nullptr);
}

QStatus AllJoynAbout::SetApplicationGuid(_In_ const GUID &value)
{
    uint8_t buffer[sizeof(value)];
    int offset = 0;

    // convert GUID into array of unsigned int 8 using the right endian order
    *((unsigned long *)&buffer[offset]) = _byteswap_ulong(value.Data1);
    offset += sizeof(value.Data1);
    *((unsigned short *)&buffer[offset]) = _byteswap_ushort(value.Data2);
    offset += sizeof(value.Data2);
    *((unsigned short *)&buffer[offset]) = _byteswap_ushort(value.Data3);
    offset += sizeof(value.Data3);
    for (int index = 0; index < sizeof(value.Data4); index++)
    {
        buffer[offset + index] = value.Data4[index];
    }

    return alljoyn_aboutdata_setappid(m_aboutData, buffer, sizeof(buffer));
}

QStatus AllJoynAbout::SetDefaultAboutData()
{
    QStatus status = ER_OK;
    std::string deviceId;
    std::string deviceName;

    // only set the required fields:
    // - DefaultLanguage (already set upon m_aboutData creation)
    // - DeviceId
    // - DeviceName
    // - AppId
    // - AppName
    // - Manufacturer
    // - ModelNumber
    // - Description
    // - SoftwareVersion

    // default device ID to bridge device Id
    CHK_AJSTATUS( GetDeviceID(deviceId) );
    CHK_AJSTATUS( alljoyn_aboutdata_setdeviceid(m_aboutData, deviceId.c_str()));

    // default about data to bridge about data
    CHK_AJSTATUS(SetDeviceName(UNKNOWN_ADAPTER));
    CHK_AJSTATUS(SetSWVersion(UNKNOWN_VERSION));
    CHK_AJSTATUS(SetDescription(DSB_DEFAULT_DESCRIPTION));
    CHK_AJSTATUS(SetApplicationGuid(DSB_DEFAULT_APP_GUID));
    CHK_AJSTATUS(SetApplicationName(DSB_DEFAULT_APP_NAME));
    CHK_AJSTATUS(SetManufacturer(UNKNOWN_MANUFACTURER));
    CHK_AJSTATUS(SetModel(DSB_DEFAULT_MODEL));


    if (!alljoyn_aboutdata_isvalid(m_aboutData, DEFAULT_LANGUAGE_FOR_ABOUT))
    {
        status = ER_ABOUT_ABOUTDATA_MISSING_REQUIRED_FIELD;
    }

leave:
    return status;
}

QStatus AllJoynAbout::ReadDeviceID(_Out_ std::wstring &deviceId)
{
    QStatus status = ER_OK;

    auto localSettings = ApplicationData::Current->LocalSettings;
    auto container = localSettings->CreateContainer(APPDATA_CONTAINER_DSB_SETTINGS, ApplicationDataCreateDisposition::Always);

    // empty output string
    deviceId.clear();

    if (localSettings->Containers->HasKey(APPDATA_CONTAINER_DSB_SETTINGS))
    {
        auto values = localSettings->Containers->Lookup(APPDATA_CONTAINER_DSB_SETTINGS)->Values;

        //check if the id exists
        bool idPresent = values->HasKey(DSB_SETTING_DEVICE_ID);
        if (idPresent)
        {
            //get the value
            String^ value = dynamic_cast<String^>(values->Lookup(DSB_SETTING_DEVICE_ID));
            if (!value || value->IsEmpty())
            {
                status = ER_FAIL;
            }
            else
            {
                deviceId = value->Data();
            }
        }
        else
        {
            status = ER_FAIL;
        }
    }
    else
    {
        status = ER_FAIL;
    }

    return status;
}

QStatus AllJoynAbout::CreateAndSaveDeviceID(_Out_ std::wstring &deviceId)
{
    QStatus status = ER_OK;
    HRESULT hr = S_OK;

    GUID guid = { 0 };
    int length = 0;
    WCHAR *tempGuid = nullptr;
    WCHAR tempString[DEVICE_GUID_STRING_LEN];

    // empty output string
    deviceId.clear();

    //Create GUID
    hr = ::CoCreateGuid(&guid);
    if (FAILED(hr))
    {
        status = ER_FAIL;
        goto leave;
    }

    //Convert GUID into String
    length = ::StringFromGUID2(guid, tempString, DEVICE_GUID_STRING_LEN);
    if (0 == length)
    {
        status = ER_FAIL;
        goto leave;
    }

    // remove '{' and '}' from GUID string
    tempGuid = tempString;
    if (tempString[length - 2] == L'}')
    {
        tempString[length - 2] = L'\0';
    }
    if (tempString[0] == L'{')
    {
        tempGuid = &tempString[1];
    }

    deviceId = tempGuid;

    {
        //create the setting
        auto localSettings = ApplicationData::Current->LocalSettings;
        auto container = localSettings->CreateContainer(APPDATA_CONTAINER_DSB_SETTINGS, ApplicationDataCreateDisposition::Always);

        if (localSettings->Containers->HasKey(APPDATA_CONTAINER_DSB_SETTINGS))
        {
            auto values = localSettings->Containers->Lookup(APPDATA_CONTAINER_DSB_SETTINGS)->Values;
            values->Insert(DSB_SETTING_DEVICE_ID, ref new String(deviceId.c_str()));
        }
        else
        {
            status = ER_FAIL;
            goto leave;
        }
    }

leave:
    if (ER_OK != status)
    {
        // reset device Id in case of error
        deviceId.clear();
    }
    return status;
}

QStatus AllJoynAbout::GetDeviceID(_Out_ std::string &deviceId)
{
    QStatus status = ER_OK;
    std::wstring tempId;

    // reset out param
    deviceId.clear();

    // read device Id (create it if necessary)
    status = ReadDeviceID(tempId);
    if (status != ER_OK)
    {
        status = CreateAndSaveDeviceID(tempId);
        if (status != ER_OK)
        {
            goto leave;
        }
    }

    //convert types
    deviceId = ConvertTo<std::string>(tempId);

leave:
    return status;
}

