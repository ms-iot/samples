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

#pragma once

#include "pch.h"
#include "IAboutData.h"
#include "IAboutIcon.h"
#include "AllJoynMessageArgVariant.h"

namespace DeviceProviders
{
    ref class AllJoynService;
    typedef Windows::Foundation::Collections::IKeyValuePair<Platform::String^, AllJoynMessageArgVariant ^> IStringVariantPair;
    typedef Platform::Collections::Details::KeyValuePair<Platform::String^, AllJoynMessageArgVariant ^> StringVariantPair;

    private ref class AllJoynAboutData sealed : public IAboutData
    {
        DEBUG_LIFETIME_DECL(AllJoynAboutData);

    internal:
        AllJoynAboutData(AllJoynService^ service, alljoyn_msgarg aboutDataMsgArg);

    public:
        virtual ~AllJoynAboutData();

    public:
        virtual property Platform::String ^ CurrentLanguage { Platform::String ^ get(); void set(Platform::String ^ value); }

        virtual property Platform::String ^ AppId { Platform::String ^ get(); }
        virtual property Platform::String ^ DefaultLanguage { Platform::String ^ get(); }
        virtual property Platform::String ^ DeviceName { Platform::String ^ get(); }
        virtual property Platform::String ^ DeviceId { Platform::String ^ get(); }
        virtual property Platform::String ^ AppName { Platform::String ^ get(); }
        virtual property Platform::String ^ Manufacturer { Platform::String ^ get(); }
        virtual property Platform::String ^ ModelNumber { Platform::String ^ get(); }
        virtual property Platform::String ^ Description { Platform::String ^ get(); }
        virtual property Platform::String ^ DateOfManufacture { Platform::String ^ get(); }
        virtual property Platform::String ^ SoftwareVersion { Platform::String ^ get(); }
        virtual property Platform::String ^ AllJoynSoftwareVersion { Platform::String ^ get(); }
        virtual property Platform::String ^ HardwareVersion { Platform::String ^ get(); }
        virtual property Platform::String ^ SupportUrl { Platform::String ^ get(); }
        virtual property Windows::Foundation::Collections::IVectorView<Platform::String^> ^ SupportedLanguages
        {
            Windows::Foundation::Collections::IVectorView<Platform::String^> ^ get();
        }
        virtual property Windows::Foundation::Collections::IVectorView<IStringVariantPair ^> ^ AnnouncedFields
        {
            Windows::Foundation::Collections::IVectorView<IStringVariantPair ^> ^ get();
        }
        virtual Windows::Foundation::Collections::IVectorView<IStringVariantPair ^> ^ GetAllFields();
        virtual Windows::Foundation::IAsyncOperation<IAboutIcon ^>^ GetIconAsync();

    private:
        const char* GetCurrentLanguage() const { return m_currentLanguage.empty() ? nullptr : m_currentLanguage.c_str(); }
        QStatus GetVectorOfFields(alljoyn_aboutdata aboutData, Platform::Collections::Vector<IStringVariantPair^>^ allFields);

    private:
        CSLock m_lock;
        AllJoynService^ m_service;
        alljoyn_aboutdata m_aboutData;
        std::string m_currentLanguage;

        Platform::String ^ m_appId;
        Platform::String ^ m_defaultLanguage;
        Platform::String ^ m_deviceName;
        Platform::String ^ m_deviceId;
        Platform::String ^ m_appName;
        Platform::String ^ m_manufacturer;
        Platform::String ^ m_modelNumber;
        Platform::String ^ m_description;
        Platform::String ^ m_dateOfManufacture;
        Platform::String ^ m_softwareVersion;
        Platform::String ^ m_allJoynSoftwareVersion;
        Platform::String ^ m_hardwareVersion;
        Platform::String ^ m_supportUrl;
        Windows::Foundation::Collections::IVectorView<Platform::String^> ^ m_supportedLanguages;
        Windows::Foundation::Collections::IVectorView<IStringVariantPair ^> ^ m_announcedFields;
        Windows::Foundation::Collections::IVectorView<IStringVariantPair ^> ^ m_allFields;
    };
}
