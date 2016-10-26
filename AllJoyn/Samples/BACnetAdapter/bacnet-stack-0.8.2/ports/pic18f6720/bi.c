/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
*
*********************************************************************/

/* Binary Input Objects customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "config.h"
#include "wp.h"
#include "rp.h"
#include "bi.h"

#define MAX_BINARY_INPUTS 8

static BACNET_BINARY_PV Present_Value[MAX_BINARY_INPUTS];

static void Binary_Input_Initialize(
    void)
{
    static bool initialized = false;
    unsigned i;

    if (!initialized) {
        initialized = true;
        for (i = 0; i < MAX_BINARY_INPUTS; i++) {
            Present_Value[i] = BINARY_INACTIVE;
        }
    }
}

/* we simply have 0-n object instances. */
bool Binary_Input_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_BINARY_INPUTS)
        return true;

    return false;
}

/* we simply have 0-n object instances. */
unsigned Binary_Input_Count(
    void)
{
    return MAX_BINARY_INPUTS;
}

/* we simply have 0-n object instances.*/
uint32_t Binary_Input_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Input_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_BINARY_INPUTS;

    if (object_instance < MAX_BINARY_INPUTS)
        index = object_instance;

    return index;
}

BACNET_BINARY_PV Binary_Input_Present_Value(
    uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = 0;

    Binary_Input_Initialize();
    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        value = Present_Value[index];
    }

    return value;
}

char *Binary_Input_Name(
    uint32_t object_instance)
{
    static char text_string[16] = "";   /* okay for single thread */

    if (object_instance < MAX_BINARY_INPUTS) {
        sprintf(text_string, "BI-%lu", object_instance);
        return text_string;
    }

    return NULL;
}

/* return apdu length, or -1 on error */
/* assumption - object already exists, and has been bounds checked */
int Binary_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_POLARITY polarity = POLARITY_NORMAL;
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    uint8_t *apdu = NULL;

    Binary_Input_Initialize();
    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_BINARY_INPUT,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            /* note: object name must be unique in our device */
            characterstring_init_ansi(&char_string,
                Binary_Input_Name(rpdata->object_instance));
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_INPUT);
            break;
        case PROP_PRESENT_VALUE:
            value = Binary_Input_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], value);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        case PROP_POLARITY:
            apdu_len = encode_application_enumerated(&apdu[0], polarity);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = -1;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = -1;
    }

    return apdu_len;
}
