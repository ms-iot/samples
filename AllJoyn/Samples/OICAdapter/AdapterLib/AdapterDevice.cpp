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
#include "AdapterDevice.h"
#include "Adapter.h"
#include "AdapterValue.h"
#include "AdapterSignal.h"
#include "AdapterProperty.h"
#include "ocrandom.h"

#include <ppltasks.h>
#include <sstream>
#include <iomanip>
#include <string>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;

using namespace std;
using namespace concurrency;

using namespace BridgeRT;

namespace AdapterLib
{
    static const wstring IconUri = L"ms-appx:///AdapterLib/OIC_Logo.png";

    AdapterDevice::AdapterDevice(const PlatformInfo& pInfo, const OCDevicePayload& deviceInfo, Adapter^ parent) : m_parent(parent)
    {
        char uuidString[UUID_STRING_SIZE];

        if (deviceInfo.sid)
        {
            if (RAND_UUID_OK == OCConvertUuidToString(deviceInfo.sid, uuidString))
            {
                m_serialNumber = ref new String(ConvertTo<wstring, string>(uuidString).c_str());
            }
            else
            {
                wostringstream sid;
                for (int i = 0; i < UUID_SIZE; ++i)
                {
                    sid << hex << setfill(L'0') << setw(2) << deviceInfo.sid[i];
                }
                m_serialNumber = ref new String(sid.str().c_str());
            }
        }
        else
        {
            throw ref new NullReferenceException();
        }
        if (deviceInfo.deviceName)
        {
            m_name = ref new String(ConvertTo<wstring,string>(deviceInfo.deviceName).c_str());
        }
        else
        {
            m_name = m_serialNumber;
        }
        
        if(deviceInfo.specVersion)
        {
            m_version = ref new String(ConvertTo<wstring, string>(deviceInfo.specVersion).c_str());
        }
        else
        {
            m_version = ref new String(ConvertTo<wstring>(pInfo.platformVersion).c_str());
        }

        if (deviceInfo.dataModelVersion)
        {
            m_description = ref new String(ConvertTo<wstring, string>(deviceInfo.dataModelVersion).c_str());
        }
        else
        {
            m_description = ref new String(L"");
        }
        
        m_vendor = ref new String(ConvertTo<wstring>(pInfo.manufacturerName).c_str());
        m_model = ref new String(ConvertTo<wstring>(pInfo.modelNumber).c_str());
        m_firmwareVersion = ref new String(ConvertTo<wstring>(pInfo.firmwareVersion).c_str());

        memset(&m_addr, 0, sizeof(m_addr));
        memcpy(&m_addr, &pInfo.addr, sizeof(m_addr));

        m_LastDiscoveredTime = chrono::steady_clock::now();

        //Add aplaceholder COV signal to the device
        AddChangeOfValueSignal(ref new AdapterProperty("temp", "temp", this), ref new AdapterValue("temp"));

        //icon
        auto loadImg = StorageFile::GetFileFromApplicationUriAsync(ref new Uri(StringReference(IconUri.c_str())));
        auto loadTask = create_task(loadImg);
        loadTask.then([this](StorageFile^ file)
        {
            if (file == nullptr)
            {
                return;
            }
            m_icon = ref new AdapterIcon(file);
        }).wait();
    }

    AdapterSignal^ AdapterDevice::GetSignal(String^ name)
    {
        for (auto signal : m_signals)
        {
            if (signal->Name == ref new String(name->Data()))
            {
                return dynamic_cast<AdapterSignal^>(signal);
            }
        }
        return nullptr;
    }

    void
    AdapterDevice::AddChangeOfValueSignal(
        IAdapterProperty^ Property,
        IAdapterValue^ Attribute)
    {
        AdapterSignal^ covSignal = ref new AdapterSignal(Constants::CHANGE_OF_VALUE_SIGNAL);
        covSignal += ref new AdapterValue(Constants::COV__PROPERTY_HANDLE, Property);
        covSignal += ref new AdapterValue(Constants::COV__ATTRIBUTE_HANDLE, Attribute);

        m_signals.push_back(covSignal);
    }
} // namespace AdapterLib