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

namespace BridgeRT
{
    const uint32_t EN_WIDGET = 0x00000001;
    const uint32_t EN_WRITABLE = 0x00000002;

    const char PROPERTY_VERSION_STR[] = "Version";
    const char PROPERTY_STATES_STR[] = "States";
    const char PROPERTY_OPTPARAMS_STR[] = "OptParams";
    const char SIGNAL_METACHANGED_STR[] = "MetadataChanged";
    const char SIGNAL_VALUCHANGED_STR[] = "ValueChanged";
    const char ARG_NONE_STR[] = "";
    const char ARG_VARIANT_STR[] = "v";
    const char ARG_BOOLEAN_STR[] = "b";
    const char ARG_STRING_STR[] = "s";
    const char ARG_UINT16_STR[] = "q";
    const char ARG_UINT16_ARRY_STR[] = "aq";
    const char ARG_UINT32_STR[] = "u";
    const char ARG_DICT_ITEM_STR[] = "{qv}";
    const char ARG_DICT_STR[] = "a{qv}";

    const char ARG_CONSTRAINT_STR[] = "(vs)";
    const char ARG_CONSTRAINTS_STR[] = "a(vs)";


    enum OPT_PARAM_KEYS
    {
        LABEL_KEY = 0,
        BGCOLOR_KEY = 1,
        HINT_KEY = 2,
        UNIT_OF_MEASURE = 3,
        CONSTRAINT_LIST = 4,
        CONSTRAINT_RANGE = 5,
        LABEL_ACTION1 = 6,
        LABEL_ACTION2 = 7,
        LABEL_ACTION3 = 8,
        NUM_OPT_PARAMS = 9
    };
}