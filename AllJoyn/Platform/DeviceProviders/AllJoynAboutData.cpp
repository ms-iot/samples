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
#include "AllJoynAboutData.h"
#include "AllJoynAboutIcon.h"
#include "AllJoynHelpers.h"
#include "AllJoynMessageArgVariant.h"
#include "AllJoynSession.h"
#include "AllJoynStatus.h"
#include "TypeConversionHelpers.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Platform::Collections::Details;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace concurrency;
using namespace std;

namespace DeviceProviders
{
    static inline String ^ GetAboutFieldValue(QStatus status, const char* value)
    {
        return ER_OK == status
            ? AllJoynHelpers::MultibyteToPlatformString(value)
            : nullptr;
    }

    AllJoynAboutData::AllJoynAboutData(AllJoynService^ service, alljoyn_msgarg aboutDataMsgArg)
        : m_service(service)
    {
        DEBUG_LIFETIME_IMPL(AllJoynAboutData);

        auto aboutData = alljoyn_aboutdata_create_empty();
        if (nullptr == aboutData)
        {
            throw ref new Platform::OutOfMemoryException();
        }

        auto status = alljoyn_aboutdata_createfrommsgarg(aboutData, aboutDataMsgArg, nullptr);
        if (ER_OK != status)
        {
            throw ref new Platform::FailureException(ref new String(L"alljoyn_aboutdata_createfrommsgarg failed"));
        }

        m_aboutData = aboutData;

        char* buffer = nullptr;
        status = alljoyn_aboutdata_getdefaultlanguage(m_aboutData, &buffer);
        if (ER_OK != status)
        {
            throw ref new Platform::FailureException(ref new String(L"alljoyn_aboutdata_getdefaultlanguage failed"));
        }
        m_defaultLanguage = AllJoynHelpers::MultibyteToPlatformString(buffer);
        m_currentLanguage = buffer;
    }

    AllJoynAboutData::~AllJoynAboutData()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr != m_aboutData)
        {
            alljoyn_aboutdata_destroy(m_aboutData);
            m_aboutData = nullptr;
        }
    }

    String ^ AllJoynAboutData::CurrentLanguage::get()
    {
        AutoLock lock(&m_lock, true);

        if (m_currentLanguage.empty())
            return this->DefaultLanguage;

        return AllJoynHelpers::MultibyteToPlatformString(m_currentLanguage.c_str());
    }

    void AllJoynAboutData::CurrentLanguage::set(String ^ value)
    {
        if (nullptr == value || value->IsEmpty())
            value = this->DefaultLanguage;

        unsigned int index = -1;
        if (!value->Equals(this->DefaultLanguage) && !this->SupportedLanguages->IndexOf(value, &index))
            throw ref new InvalidArgumentException(ref new String(L"Unsupported language specified."));

        auto newLanguage = AllJoynHelpers::PlatformToMultibyteStandardString(value);

        AutoLock lock(&m_lock, true);

        if (m_currentLanguage != newLanguage)
        {
            m_currentLanguage = newLanguage;

            // Clear the language-dependent values
            m_appName = nullptr;
            m_deviceName = nullptr;
            m_description = nullptr;
            m_manufacturer = nullptr;
            m_announcedFields = nullptr;
        }
    }

    String ^ AllJoynAboutData::AppId::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_appId)
        {
            // Get and format the AppId
            uint8_t* appId = nullptr;
            size_t appIdSize = 0;
            auto status = alljoyn_aboutdata_getappid(m_aboutData, &appId, &appIdSize);
            if (ER_OK == status)
            {
                wostringstream wstrAppId;
                wstrAppId << setfill(L'0') << setw(2) << hex << uppercase;
                for (size_t index = 0; index < appIdSize; index++)
                {
                    if (4 == index || 8 == index || 10 == index || 12 == index)
                    {
                        wstrAppId << "-";
                    }
                    wstrAppId << appId[index];
                }

                m_appId = ref new String(wstrAppId.str().c_str());
            }
        }

        return m_appId;
    }

    String ^ AllJoynAboutData::DefaultLanguage::get()
    {
        AutoLock lock(&m_lock, true);

        return m_defaultLanguage;
    }

    String ^ AllJoynAboutData::DeviceName::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_deviceName)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getdevicename(m_aboutData, &buffer, this->GetCurrentLanguage());
            m_deviceName = GetAboutFieldValue(status, buffer);
        }

        return m_deviceName;
    }

    String ^ AllJoynAboutData::DeviceId::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_deviceId)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getdeviceid(m_aboutData, &buffer);
            m_deviceId = GetAboutFieldValue(status, buffer);
        }

        return m_deviceId;
    }

    String ^ AllJoynAboutData::AppName::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_appName)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getappname(m_aboutData, &buffer, this->GetCurrentLanguage());
            m_appName = GetAboutFieldValue(status, buffer);
        }

        return m_appName;
    }

    String ^ AllJoynAboutData::Manufacturer::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_manufacturer)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getmanufacturer(m_aboutData, &buffer, this->GetCurrentLanguage());
            m_manufacturer = GetAboutFieldValue(status, buffer);
        }

        return m_manufacturer;
    }

    String ^ AllJoynAboutData::ModelNumber::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_modelNumber)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getmodelnumber(m_aboutData, &buffer);
            m_modelNumber = GetAboutFieldValue(status, buffer);
        }

        return m_modelNumber;
    }

    String ^ AllJoynAboutData::Description::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_description)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getdescription(m_aboutData, &buffer, this->GetCurrentLanguage());
            m_description = GetAboutFieldValue(status, buffer);
        }

        return m_description;
    }

    String ^ AllJoynAboutData::DateOfManufacture::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_dateOfManufacture)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getdateofmanufacture(m_aboutData, &buffer);
            m_dateOfManufacture = GetAboutFieldValue(status, buffer);
        }

        return m_dateOfManufacture;
    }

    String ^ AllJoynAboutData::SoftwareVersion::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_softwareVersion)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getsoftwareversion(m_aboutData, &buffer);
            m_softwareVersion = GetAboutFieldValue(status, buffer);
        }

        return m_softwareVersion;
    }

    String ^ AllJoynAboutData::AllJoynSoftwareVersion::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_allJoynSoftwareVersion)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getajsoftwareversion(m_aboutData, &buffer);
            m_allJoynSoftwareVersion = GetAboutFieldValue(status, buffer);
        }

        return m_allJoynSoftwareVersion;
    }

    String ^ AllJoynAboutData::HardwareVersion::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_hardwareVersion)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_gethardwareversion(m_aboutData, &buffer);
            m_hardwareVersion = GetAboutFieldValue(status, buffer);
        }

        return m_hardwareVersion;
    }

    String ^ AllJoynAboutData::SupportUrl::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_supportUrl)
        {
            char* buffer = nullptr;
            auto status = alljoyn_aboutdata_getsupporturl(m_aboutData, &buffer);
            m_supportUrl = GetAboutFieldValue(status, buffer);
        }

        return m_supportUrl;
    }

    IVectorView<String^> ^ AllJoynAboutData::SupportedLanguages::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_supportedLanguages)
        {
            auto supportedLanguages = ref new Vector<String^>();

            auto languageCount = alljoyn_aboutdata_getsupportedlanguages(m_aboutData, nullptr, 0);
            if (languageCount)
            {
                auto languages = vector<const char *>(languageCount);
                alljoyn_aboutdata_getsupportedlanguages(m_aboutData, languages.data(), languageCount);
                for (auto& language : languages)
                {
                    if (language)
                    {
                        supportedLanguages->Append(AllJoynHelpers::MultibyteToPlatformString(language));
                    }
                }
            }

            m_supportedLanguages = supportedLanguages->GetView();
        }

        return m_supportedLanguages;
    }

    IVectorView<IStringVariantPair ^> ^ AllJoynAboutData::AnnouncedFields::get()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_announcedFields)
        {
            Vector<IStringVariantPair ^> ^ allFields = ref new Vector<IStringVariantPair ^>();
            QStatus status = GetVectorOfFields(m_aboutData, allFields);
            m_announcedFields = allFields->GetView();
        }

        return m_announcedFields;
    }

    IVectorView<IStringVariantPair ^> ^ AllJoynAboutData::GetAllFields()
    {
        AutoLock lock(&m_lock, true);

        if (nullptr == m_allFields)
        {
            auto allFields = ref new Vector<IStringVariantPair ^>();

            auto session = m_service->JoinSession();
            if (session)
            {
                auto aboutProxy = alljoyn_aboutproxy_create(m_service->GetBusAttachment(), m_service->GetName().c_str(), session->SessionId);

                alljoyn_msgarg extendedAboutMsgArg = alljoyn_msgarg_create();
                QStatus status = alljoyn_aboutproxy_getaboutdata(aboutProxy, nullptr, extendedAboutMsgArg);
                if (ER_OK == status)
                {
                    alljoyn_aboutdata extendedAboutData = alljoyn_aboutdata_create_empty();
                    status = alljoyn_aboutdata_createfrommsgarg(extendedAboutData, extendedAboutMsgArg, nullptr);
                    if (ER_OK == status)
                    {
                        status = GetVectorOfFields(extendedAboutData, allFields);
                    }
                }

                m_allFields = allFields->GetView();
            }
        }

        return m_allFields;
    }

    IAsyncOperation<IAboutIcon^>^ AllJoynAboutData::GetIconAsync()
    {
        return create_async([this]() -> IAboutIcon^
        {
            AllJoynAboutIcon^ icon;
            AllJoynBusObject^ iconBusObject;
            AllJoynSession^ session;

            try
            {
                session = m_service->JoinSession();
                if (session)
                {
                    iconBusObject = session->GetImplementation()->GetOrCreateBusObject(AllJoynAboutIcon::AboutIconObjectPath);
                    if (iconBusObject)
                    {
                        icon = ref new AllJoynAboutIcon();
                        create_task(icon->InitializeAsync(iconBusObject)).wait();
                    }
                }
            }
            catch (...)
            {
            }

            return icon;
        });
    }

    QStatus AllJoynAboutData::GetVectorOfFields(alljoyn_aboutdata aboutData, Vector<IStringVariantPair^>^ allFields)
    {
        QStatus status = QStatus::ER_OK;
        allFields->Clear();

        auto fieldCount = alljoyn_aboutdata_getfields(aboutData, nullptr, 0);
        if (fieldCount)
        {
            auto fieldNames = vector<const char*>(fieldCount);
            alljoyn_aboutdata_getfields(aboutData, fieldNames.data(), fieldCount);

            for (auto& fieldName : fieldNames)
            {
                auto fieldSignature = alljoyn_aboutdata_getfieldsignature(aboutData, fieldName);

                alljoyn_msgarg fieldValue = nullptr;
                status = alljoyn_aboutdata_getfield(aboutData, fieldName, &fieldValue, this->GetCurrentLanguage());
                if (ER_OK != status)
                    break;

                Object ^ value;
                status = (QStatus)TypeConversionHelpers::GetAllJoynMessageArg(fieldValue, fieldSignature, &value);
                if (ER_OK != status)
                    break;

                auto variantValue = ref new AllJoynMessageArgVariant(fieldSignature, value);
                allFields->Append(ref new StringVariantPair(AllJoynHelpers::MultibyteToPlatformString(fieldName), variantValue));
            }
        }

        if (ER_OK != status)
        {
            allFields->Clear();
        }

        return status;
    }
}
