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

namespace AdapterLib
{
    //
    // Mock adapter methods and parameters names
    //

    // Methods return value name
    static Platform::String^ DSB_METHOD_RETURN_VALUE_NAME = ref new Platform::String(L"Return_Value");

    // Reset method
    static Platform::String^ DEVICE_RESET_METHOD = ref new Platform::String(L"Device_Reset");
        static Platform::String^ DEVICE_RESET__PROPERTY_HANDLE = ref new Platform::String(L"Property_Handle");

    //
    // Mock adapter signals and parameters names
    //

    //...
}
