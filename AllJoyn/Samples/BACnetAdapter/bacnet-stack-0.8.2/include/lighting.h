/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef LIGHTING_H
#define LIGHTING_H

#include <stdint.h>
#include <stdbool.h>
#include "bacenum.h"

/* BACnetLightingCommand ::= SEQUENCE {
    operation [0] BACnetLightingOperation,
    target-level [1] REAL (0.0..100.0) OPTIONAL,
    ramp-rate [2] REAL (0.1..100.0) OPTIONAL,
    step-increment [3] REAL (0.1..100.0) OPTIONAL,
    fade-time [4] Unsigned (100.. 86400000) OPTIONAL,
    priority [5] Unsigned (1..16) OPTIONAL
}
-- Note that the combination of level, ramp-rate, step-increment, and fade-time fields is
-- dependent on the specific lighting operation. See Table 12-67.
*/
typedef struct BACnetLightingCommand {
    BACNET_LIGHTING_OPERATION operation;
    /* fields are optional */
    bool use_target_level:1;
    bool use_ramp_rate:1;
    bool use_step_increment:1;
    bool use_fade_time:1;
    bool use_priority:1;
    float target_level;
    float ramp_rate;
    float step_increment;
    uint32_t fade_time;
    uint8_t priority;
} BACNET_LIGHTING_COMMAND;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int lighting_command_encode(
        uint8_t * apdu,
        BACNET_LIGHTING_COMMAND * data);
    int lighting_command_encode_context(
        uint8_t * apdu,
        uint8_t tag_number,
        BACNET_LIGHTING_COMMAND * value);
    int lighting_command_decode(
        uint8_t * apdu,
        unsigned apdu_max_len,
        BACNET_LIGHTING_COMMAND * data);
    bool lighting_command_copy(
        BACNET_LIGHTING_COMMAND * dst,
        BACNET_LIGHTING_COMMAND * src);
    bool lighting_command_same(
        BACNET_LIGHTING_COMMAND * dst,
        BACNET_LIGHTING_COMMAND * src);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
