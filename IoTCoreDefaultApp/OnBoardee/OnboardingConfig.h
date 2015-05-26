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

class OnboardingConfig
{
public:
    OnboardingConfig();
    virtual ~OnboardingConfig();

    HRESULT Init(Platform::String^ filename);

    Platform::String^ SSID();
    Platform::String^ Password();
    Platform::String^ DefaultDescription();
    Platform::String^ DefaultManufacturer();

private:
    HRESULT FromFile(Platform::String^ pFileName);
    HRESULT ToFile(Platform::String^ pFileName = nullptr);
    HRESULT initXml();
    Platform::String^ FindInXml(Platform::String^ queryString);

    bool isFilePresent(Windows::Storage::StorageFolder^ appFolder, Platform::String^ pFileName);

    Windows::Data::Xml::Dom::XmlDocument^ m_pXmlDocument;
    Platform::String^ m_fileName;
};