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

#include "AdapterDefinitions.h"

namespace AdapterLib
{
    //
    // MockAdapterIcon
    // Description:
    //  This class demonstrates how to provide an ICON for the Mock device
    //
    ref class AdapterIcon : BridgeRT::IAdapterIcon
    {
    public:

        // Gets the image data (Optional - but must set the URL if this returns null)
        virtual Platform::Array<BYTE>^ GetImage();

        // Gets the Icon's URL.  (Optional - but must set the Image Data if this returns null)
        virtual property Platform::String^ Url
        {
            Platform::String^ get()
            {
                return m_url;
            }
        }

        // Gets the Icon's type. (Returns the Mime Type of Image Data or URL. Must always be the same)
        virtual property Platform::String^ MimeType
        {
            Platform::String^ get()
            {
                return m_mimeType;
            }
        }


    internal:
        AdapterIcon(Windows::Storage::StorageFile^ srcFile, Platform::String^ srcUrl = nullptr);
        AdapterIcon(Platform::String^ srcUrl, Platform::String^ srcMimeType);

    private:
        Windows::Storage::StorageFile^ m_srcImageFile;
        Platform::String^ m_url;
        Platform::String^ m_mimeType;
    };
}   //namespace AdapterLib