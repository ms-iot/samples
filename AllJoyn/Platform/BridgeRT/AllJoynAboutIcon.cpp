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
#include "AllJoynAboutIcon.h"
#include <IAdapter.h>
#include "BridgeLog.h"
#include "BridgeUtils.h"


const size_t MAX_ALLJOYN_ARRAY_SIZE = 128 * 1024;

using namespace BridgeRT;

AllJoynAboutIcon::AllJoynAboutIcon()
{
}

AllJoynAboutIcon::~AllJoynAboutIcon()
{
    ShutDown();
}

QStatus AllJoynAboutIcon::Initialize(_In_ alljoyn_busattachment bus, _In_ IAdapterIcon^ icon)
{
    QStatus status = ER_OK;
    Platform::Array<BYTE>^ image = nullptr;

    if (bus == nullptr)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    // Verify that an icon was passed
    if (icon == nullptr)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    // Verify that Icon contains at least image data or an URL (could have both)
    image = icon->GetImage();
    if ((image == nullptr) && (icon->Url == nullptr))
    {
        status = ER_ABOUT_INVALID_ABOUTDATA_FIELD_VALUE;
        goto leave;
    }

    // If image data present, then make further checks on its content
    if (image != nullptr)
    {
        if (image->Length > MAX_ALLJOYN_ARRAY_SIZE)
        {
            status = ER_PACKET_TOO_LARGE;
            goto leave;
        }
        else if (image->Length == 0)
        {
            status = ER_INVALID_DATA;
            goto leave;
        }
    }

    // If a re-init, clear out the old icon
    if (m_iconObj != nullptr)
    {
        alljoyn_abouticonobj_destroy(m_iconObj);
        m_iconObj = nullptr;
    }

    m_imageData.clear();

    // If the icon is not already configured, then create the object
    if (m_icon == nullptr)
    {
        // Create a new about icon
        m_icon = alljoyn_abouticon_create();
        CHK_POINTER(m_icon);
    }
    else
    {
        // Clear the existing icon content
        alljoyn_abouticon_clear(m_icon);
    }

    try
    {
        // Get the MIME type from the ICON
        std::string mimeType = ConvertTo<std::string>(icon->MimeType);

        // copy the source icon to the about data
        if (image != nullptr)
        {
            try
            {
                // Prepare the vector for copying into
                m_imageData.resize(image->Length);

                // Copy the image data into the vector
                std::copy(
                    image->begin(),
                    image->end(),
                    m_imageData.begin());
            }
            catch (...)
            {
                status = ER_OUT_OF_MEMORY;
                goto leave;
            }
            CHK_AJSTATUS(alljoyn_abouticon_setcontent(m_icon, mimeType.c_str(), m_imageData.data(), m_imageData.size(), false));
        }

        // copy the url to the about data
        if (icon->Url != nullptr)
        {
            std::string url = ConvertTo<std::string>(icon->Url);
            CHK_AJSTATUS(alljoyn_abouticon_seturl(m_icon, mimeType.c_str(), url.c_str()));
        }

        // create the about icon bus object
        m_iconObj = alljoyn_abouticonobj_create(bus, m_icon);
        CHK_POINTER(m_iconObj);

    }
    catch (...)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

leave:
    if (ER_OK != status)
    {
        ShutDown();
    }

    return status;
}

void AllJoynAboutIcon::ShutDown()
{
    if (m_iconObj != nullptr)
    {
        alljoyn_abouticonobj_destroy(m_iconObj);
        m_iconObj = nullptr;
    }

    if (m_icon != nullptr)
    {
        alljoyn_abouticon_destroy(m_icon);
        m_icon = nullptr;
    }

    m_imageData.clear();
}

