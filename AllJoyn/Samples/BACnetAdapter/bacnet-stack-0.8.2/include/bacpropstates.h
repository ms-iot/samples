/**************************************************************************
*
* Copyright (C) 2008 John Minack
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/
#ifndef _BAC_PROP_STATES_H_
#define _BAC_PROP_STATES_H_

#include <stdint.h>
#include <stdbool.h>
#include "bacenum.h"
#include "bacapp.h"
#include "timestamp.h"

typedef enum {
    BOOLEAN_VALUE,
    BINARY_VALUE,
    EVENT_TYPE,
    POLARITY,
    PROGRAM_CHANGE,
    PROGRAM_STATE,
    REASON_FOR_HALT,
    RELIABILITY,
    STATE,
    SYSTEM_STATUS,
    UNITS,
    UNSIGNED_VALUE,
    LIFE_SAFETY_MODE,
    LIFE_SAFETY_STATE
} BACNET_PROPERTY_STATE_TYPE;

typedef struct {
    BACNET_PROPERTY_STATE_TYPE tag;
    union {
        bool booleanValue;
        BACNET_BINARY_PV binaryValue;
        BACNET_EVENT_TYPE eventType;
        BACNET_POLARITY polarity;
        BACNET_PROGRAM_REQUEST programChange;
        BACNET_PROGRAM_STATE programState;
        BACNET_PROGRAM_ERROR programError;
        BACNET_RELIABILITY reliability;
        BACNET_EVENT_STATE state;
        BACNET_DEVICE_STATUS systemStatus;
        BACNET_ENGINEERING_UNITS units;
        uint32_t unsignedValue;
        BACNET_LIFE_SAFETY_MODE lifeSafetyMode;
        BACNET_LIFE_SAFETY_STATE lifeSafetyState;
    } state;
} BACNET_PROPERTY_STATE;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int bacapp_decode_property_state(
        uint8_t * apdu,
        BACNET_PROPERTY_STATE * value);

    int bacapp_decode_context_property_state(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_PROPERTY_STATE * value);

    int bacapp_encode_property_state(
        uint8_t * apdu,
        BACNET_PROPERTY_STATE * value);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
