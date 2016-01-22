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
#include "AllJoynStatus.h"

namespace DeviceProviders
{
    public ref class AllJoynStatus sealed
    {
    public:
        static property AllJoynStatus^ Succeeded
        {
            AllJoynStatus^ get()
            {
                static AllJoynStatus^ _succeeded = ref new AllJoynStatus(QStatus::ER_OK);
                return _succeeded;
            }
        }

    internal:
        AllJoynStatus(QStatus statusCode);
        AllJoynStatus(QStatus statusCode, const std::string& statusText);

    public:
        property uint32 StatusCode
        {
            uint32 get();
        }

        property Platform::String ^ StatusText
        {
            Platform::String ^ get();
        }

        property bool IsSuccess
        {
            bool get();
        }

        property bool IsFailure
        {
            bool get();
        }

    private:
        QStatus m_statusCode;
        std::string m_statusText;
    };
}
