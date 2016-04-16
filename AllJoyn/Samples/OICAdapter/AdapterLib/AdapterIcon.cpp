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
#include "AdapterIcon.h"
#include <ppltasks.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::FileProperties;

using namespace BridgeRT;
using namespace std;
using namespace concurrency;

using Platform::Array;

namespace AdapterLib
{
    AdapterIcon::AdapterIcon(Windows::Storage::StorageFile^ srcFile, Platform::String^ srcUrl)
        : m_url(srcUrl)
        , m_srcImageFile(srcFile)

    {
       m_mimeType = srcFile->ContentType;
    }

    AdapterIcon::AdapterIcon(Platform::String^ srcUrl, Platform::String^ srcMimeType)
        : m_url(srcUrl)
        , m_mimeType(srcMimeType)
        , m_srcImageFile(nullptr)
    {
    }

    Array<BYTE>^ AdapterIcon::GetImage()
    {
        Array<BYTE>^ imageData = nullptr;

        try
        {
            // Start a sequential read of the source image file
            auto readOp = m_srcImageFile->OpenSequentialReadAsync();
            auto readTask = create_task(readOp);
            auto stream = readTask.get();

            // Get the File Size
            auto getPropsOp = m_srcImageFile->GetBasicPropertiesAsync();
            auto getPropsTask = create_task(getPropsOp);
            auto props = getPropsTask.get();
            unsigned int fileSize = (unsigned int)(props->Size);

            // Create a holding buffer
            imageData = ref new Platform::Array<BYTE>(fileSize);

            // Load the Image Data
            auto dataReader = ref new DataReader(stream);
            auto loadOp = dataReader->LoadAsync(fileSize);
            auto loadTask = create_task(loadOp);
            loadTask.get();

            // Read image data into a byte array
            dataReader->ReadBytes(imageData);
        }
        catch (...)
        {
            imageData = nullptr;
        }

        return imageData;
    }

}   //namespace AdapterLib