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
#include "AdapterDefinitions.h"

namespace BridgeRT
{
    //Values pulled from : https://git.allseenalliance.org/cgit/lighting/service_framework.git/tree/common/inc/LampValues.h 
    //
    // Enum describing the make of a Lamp
    //
    public enum class LSFLampMake
    {
        MAKE_INVALID,
        MAKE_LIFX,
        MAKE_OEM1,
        MAKE_LASTVALUE
    };


    //
    // Enum describing the model of a Lamp
    //
    public enum class LSFLampModel
    {
        MODEL_INVALID,
        MODEL_LED,
        MODEL_LASTVALUE
    };


    //
    //  Enum describing the type of a LSF Device
    //
    public enum class LSFDeviceType
    {
        TYPE_INVALID,
        TYPE_LAMP,
        TYPE_OUTLET,
        TYPE_LUMINAIRE,
        TYPE_SWITCH,
        TYPE_LASTVALUE
    };

    //
    // Enum describing the type of a Lamp
    //
    public enum class LSFLampType
    {
        LAMPTYPE_INVALID,
        LAMPTYPE_A15,
        LAMPTYPE_A17,
        LAMPTYPE_A19,
        LAMPTYPE_A20,
        LAMPTYPE_A21,
        LAMPTYPE_A23,
        LAMPTYPE_AR70,
        LAMPTYPE_AR111,
        LAMPTYPE_B8,
        LAMPTYPE_B10,
        LAMPTYPE_B11,
        LAMPTYPE_B13,
        LAMPTYPE_BR25,
        LAMPTYPE_BR30,
        LAMPTYPE_BR38,
        LAMPTYPE_BR40,
        LAMPTYPE_BT15,
        LAMPTYPE_BT28,
        LAMPTYPE_BT37,
        LAMPTYPE_BT56,
        LAMPTYPE_C6,
        LAMPTYPE_C7,
        LAMPTYPE_C9,
        LAMPTYPE_C11,
        LAMPTYPE_C15,
        LAMPTYPE_CA5,
        LAMPTYPE_CA7,
        LAMPTYPE_CA8,
        LAMPTYPE_CA10,
        LAMPTYPE_CA11,
        LAMPTYPE_E17,
        LAMPTYPE_E18,
        LAMPTYPE_E23,
        LAMPTYPE_E25,
        LAMPTYPE_E37,
        LAMPTYPE_ED17,
        LAMPTYPE_ED18,
        LAMPTYPE_ED23,
        LAMPTYPE_ED28,
        LAMPTYPE_ED37,
        LAMPTYPE_F10,
        LAMPTYPE_F15,
        LAMPTYPE_F20,
        LAMPTYPE_G9,
        LAMPTYPE_G11,
        LAMPTYPE_G12,
        LAMPTYPE_G16,
        LAMPTYPE_G19,
        LAMPTYPE_G25,
        LAMPTYPE_G30,
        LAMPTYPE_G40,
        LAMPTYPE_T2,
        LAMPTYPE_T3,
        LAMPTYPE_T4,
        LAMPTYPE_T5,
        LAMPTYPE_T6,
        LAMPTYPE_T7,
        LAMPTYPE_T8,
        LAMPTYPE_T9,
        LAMPTYPE_T10,
        LAMPTYPE_T12,
        LAMPTYPE_T14,
        LAMPTYPE_T20,
        LAMPTYPE_MR8,
        LAMPTYPE_MR11,
        LAMPTYPE_MR16,
        LAMPTYPE_MR20,
        LAMPTYPE_PAR14,
        LAMPTYPE_PAR16,
        LAMPTYPE_PAR20,
        LAMPTYPE_PAR30,
        LAMPTYPE_PAR36,
        LAMPTYPE_PAR38,
        LAMPTYPE_PAR46,
        LAMPTYPE_PAR56,
        LAMPTYPE_PAR64,
        LAMPTYPE_PS25,
        LAMPTYPE_PS35,
        LAMPTYPE_R12,
        LAMPTYPE_R14,
        LAMPTYPE_R16,
        LAMPTYPE_R20,
        LAMPTYPE_R25,
        LAMPTYPE_R30,
        LAMPTYPE_R40,
        LAMPTYPE_RP11,
        LAMPTYPE_S6,
        LAMPTYPE_S8,
        LAMPTYPE_S11,
        LAMPTYPE_S14,
        LAMPTYPE_ST18,
        LAMPTYPE_LASTVALUE
    };

    //
    // Enum describing the type of the Lamp Base
    //          
    public enum class LSFLampBaseType
    {
        BASETYPE_INVALID,
        BASETYPE_E5,
        BASETYPE_E10,
        BASETYPE_E11,
        BASETYPE_E12,
        BASETYPE_E14,
        BASETYPE_E17,
        BASETYPE_E26,
        BASETYPE_E27,
        BASETYPE_E29,
        BASETYPE_E39,
        BASETYPE_B22,
        BASETYPE_GU10,
        BASETYPE_LASTVALUE
    };
}
