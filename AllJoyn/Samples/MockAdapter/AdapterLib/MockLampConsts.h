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
    extern Platform::String^ DEVICE_NAME;
    extern Platform::String^ VERSION_VALUE_NAME;

    extern Platform::String^ LAMP_STATE_CHANGED_SIGNAL_NAME;
    extern Platform::String^ SIGNAL_PARAMETER__LAMP_ID__NAME;

    extern Platform::String^ LAMP_ID;
    extern Platform::String^ LAMP_MODEL;
    extern Platform::String^ LAMP_MANUFACTURER;

    extern bool LAMP_STATE_ON_OFF;
    extern uint32 LAMP_STATE_BRIGHTNESS;
    extern uint32 LAMP_STATE_HUE;
    extern uint32 LAMP_STATE_SATURATION;
    extern uint32 LAMP_STATE_COLORTEMP;
}