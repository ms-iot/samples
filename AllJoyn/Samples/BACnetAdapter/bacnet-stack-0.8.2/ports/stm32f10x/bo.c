/**************************************************************************
*
* Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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

/* Binary Output Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "config.h"
#include "wp.h"
#include "hardware.h"
#include "handlers.h"
#include "bo.h"

#ifndef MAX_BINARY_OUTPUTS
#error Please define MAX_BINARY_OUTPUTS
#endif

/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define RELINQUISH_DEFAULT BINARY_INACTIVE
/* Here is our Priority Array.*/
static uint8_t Binary_Output_Level[MAX_BINARY_OUTPUTS][BACNET_MAX_PRIORITY];
/* Writable out-of-service allows others to play with our Present Value */
/* without changing the physical output */
static uint8_t Out_Of_Service[MAX_BINARY_OUTPUTS];
/* polarity - normal or inverse */
static uint8_t Polarity[MAX_BINARY_OUTPUTS];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Binary_Output_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_POLARITY,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    -1
};

static const int Binary_Output_Properties_Optional[] = {
    PROP_ACTIVE_TEXT,
    PROP_INACTIVE_TEXT,
    -1
};

static const int Binary_Output_Properties_Proprietary[] = {
    -1
};

void Binary_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Binary_Output_Properties_Required;
    if (pOptional)
        *pOptional = Binary_Output_Properties_Optional;
    if (pProprietary)
        *pProprietary = Binary_Output_Properties_Proprietary;

    return;
}

/* we simply have 0-n object instances. */
bool Binary_Output_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_BINARY_OUTPUTS)
        return true;

    return false;
}

/* we simply have 0-n object instances. */
unsigned Binary_Output_Count(
    void)
{
    return MAX_BINARY_OUTPUTS;
}

/* we simply have 0-n object instances. */
uint32_t Binary_Output_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  */
unsigned Binary_Output_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_BINARY_OUTPUTS;

    if (object_instance < MAX_BINARY_OUTPUTS)
        index = object_instance;

    return index;
}

static BACNET_BINARY_PV Present_Value(
    unsigned int index)
{
    BACNET_BINARY_PV value = RELINQUISH_DEFAULT;
    BACNET_BINARY_PV current_value = RELINQUISH_DEFAULT;
    unsigned i = 0;

    if (index < MAX_BINARY_OUTPUTS) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            current_value = (BACNET_BINARY_PV) Binary_Output_Level[index][i];
            if (current_value != BINARY_NULL) {
                value = (BACNET_BINARY_PV) Binary_Output_Level[index][i];
                break;
            }
        }
    }

    return value;
}

BACNET_BINARY_PV Binary_Output_Present_Value(
    uint32_t object_instance)
{
    unsigned index = 0;

    index = Binary_Output_Instance_To_Index(object_instance);

    return Present_Value(index);
}

bool Binary_Output_Present_Value_Set(
    uint32_t instance,
    BACNET_BINARY_PV binary_value,
    unsigned priority)
{       /* 0..15 */
    bool status = false;

    if (instance < MAX_BINARY_OUTPUTS) {
        if (priority < BACNET_MAX_PRIORITY) {
            Binary_Output_Level[instance][priority] = (uint8_t) binary_value;
            status = true;
        }
    }

    return status;
}

bool Binary_Output_Polarity_Set(
    uint32_t instance,
    BACNET_POLARITY polarity)
{
    bool status = false;

    if (instance < MAX_BINARY_OUTPUTS) {
        if (polarity < MAX_POLARITY) {
            Polarity[instance] = polarity;
            status = true;
        }
    }

    return status;
}

BACNET_POLARITY Binary_Output_Polarity(
    uint32_t instance)
{
    BACNET_POLARITY polarity = POLARITY_NORMAL;

    if (instance < MAX_BINARY_OUTPUTS) {
        polarity = (BACNET_POLARITY) Polarity[instance];
    }

    return polarity;
}

void Binary_Output_Out_Of_Service_Set(
    uint32_t instance,
    bool flag)
{
    if (instance < MAX_BINARY_OUTPUTS) {
        Out_Of_Service[instance] = flag;
    }
}

bool Binary_Output_Out_Of_Service(
    uint32_t instance)
{
    bool flag = false;

    if (instance < MAX_BINARY_OUTPUTS) {
        flag = Out_Of_Service[instance];
    }

    return flag;
}

/* note: the object name must be unique within this device */
bool Binary_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[16] = "BO-0";       /* okay for single thread */
    bool status = false;

    if (object_instance < MAX_BINARY_OUTPUTS) {
        text_string[3] = '0' + (uint8_t) object_instance;
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/* return apdu len, or -1 on error */
int Binary_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BINARY_PV present_value = BINARY_INACTIVE;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
            /* object id, object name, object type are handled in Device object */
        case PROP_PRESENT_VALUE:
            present_value =
                Binary_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
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
            object_index =
                Binary_Output_Instance_To_Index(rpdata->object_instance);
            state = Out_Of_Service[object_index];
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_POLARITY:
            object_index =
                Binary_Output_Instance_To_Index(rpdata->object_instance);
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Polarity[object_index]);
            break;
        case PROP_PRIORITY_ARRAY:
            /* Array element zero is the number of elements in the array */
            if (rpdata->array_index == 0)
                apdu_len =
                    encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                object_index =
                    Binary_Output_Instance_To_Index(rpdata->object_instance);
                for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                    /* FIXME: check if we have room before adding it to APDU */
                    present_value = (BACNET_BINARY_PV)
                        Binary_Output_Level[object_index][i];
                    if (present_value == BINARY_NULL) {
                        len = encode_application_null(&apdu[apdu_len]);
                    } else {
                        len =
                            encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                    }
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU)
                        apdu_len += len;
                    else {
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                object_index =
                    Binary_Output_Instance_To_Index(rpdata->object_instance);
                if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                    present_value = (BACNET_BINARY_PV)
                        Binary_Output_Level[object_index][rpdata->array_index -
                        1];
                    if (present_value == BINARY_NULL) {
                        apdu_len = encode_application_null(&apdu[apdu_len]);
                    } else {
                        apdu_len =
                            encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                    }
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_RELINQUISH_DEFAULT:
            present_value = RELINQUISH_DEFAULT;
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_ACTIVE_TEXT:
            characterstring_init_ansi(&char_string, "on");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_INACTIVE_TEXT:
            characterstring_init_ansi(&char_string, "off");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Binary_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int priority = 0;
    BACNET_BINARY_PV level = BINARY_NULL;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                priority = wp_data->priority;
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                    (priority != 6 /* reserved */ ) &&
                    (value.type.Enumerated <= MAX_BINARY_PV)) {
                    level = (BACNET_BINARY_PV) value.type.Enumerated;
                    priority--;
                    Binary_Output_Present_Value_Set(wp_data->object_instance,
                        level, priority);
                } else if (priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    level = BINARY_NULL;
                    priority = wp_data->priority;
                    if (priority && (priority <= BACNET_MAX_PRIORITY)) {
                        priority--;
                        Binary_Output_Present_Value_Set
                            (wp_data->object_instance, level, priority);
                    } else if (priority == 6) {
                        status = false;
                        /* Command priority 6 is reserved for use by Minimum On/Off
                           algorithm and may not be used for other purposes in any
                           object. */
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                    } else {
                        status = false;
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Binary_Output_Out_Of_Service_Set(wp_data->object_instance,
                    value.type.Boolean);
            }
            break;
        case PROP_POLARITY:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if (value.type.Enumerated < MAX_POLARITY) {
                    Binary_Output_Polarity_Set(wp_data->object_instance,
                        (BACNET_POLARITY) value.type.Enumerated);
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_RELIABILITY:
        case PROP_EVENT_STATE:
        case PROP_PRIORITY_ARRAY:
        case PROP_RELINQUISH_DEFAULT:
        case PROP_ACTIVE_TEXT:
        case PROP_INACTIVE_TEXT:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }
    /* not using len at this time */
    len = len;

    return status;
}

void Binary_Output_Init(
    void)
{
    unsigned i, j;

    /* initialize all the analog output priority arrays to NULL */
    for (i = 0; i < MAX_BINARY_OUTPUTS; i++) {
        Binary_Output_Polarity_Set(i, POLARITY_NORMAL);
        Binary_Output_Out_Of_Service_Set(i, false);
        for (j = 0; j < BACNET_MAX_PRIORITY; j++) {
            Binary_Output_Level[i][j] = BINARY_NULL;
        }
    }

    return;
}
