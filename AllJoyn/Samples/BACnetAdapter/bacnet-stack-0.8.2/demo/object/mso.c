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

/* Multi-state Output Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "rp.h"
#include "wp.h"
#include "mso.h"
#include "handlers.h"

#ifndef MAX_MULTISTATE_OUTPUTS
#define MAX_MULTISTATE_OUTPUTS 4
#endif

/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define MULTISTATE_RELINQUISH_DEFAULT 1		/* multistates cannot be 0 !	*/

/* NULL part of the array */
#define MULTISTATE_NULL (255)
/* how many states? 1 to 254 states, 0 is not allowed */
#define MULTISTATE_NUMBER_OF_STATES (254)
/* Here is our Priority Array.*/
static uint8_t
    Multistate_Output_Level[MAX_MULTISTATE_OUTPUTS][BACNET_MAX_PRIORITY];
/* Writable out-of-service allows others to play with our Present Value */
/* without changing the physical output */
static bool Out_Of_Service[MAX_MULTISTATE_OUTPUTS];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Multistate_Output_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_NUMBER_OF_STATES,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    -1
};

static const int Multistate_Output_Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Multistate_Output_Properties_Proprietary[] = {
    -1
};

void Multistate_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Multistate_Output_Properties_Required;
    if (pOptional)
        *pOptional = Multistate_Output_Properties_Optional;
    if (pProprietary)
        *pProprietary = Multistate_Output_Properties_Proprietary;

    return;
}

void Multistate_Output_Init(
    void)
{
    unsigned i, j;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        /* initialize all the analog output priority arrays to NULL */
        for (i = 0; i < MAX_MULTISTATE_OUTPUTS; i++) {
            for (j = 0; j < BACNET_MAX_PRIORITY; j++) {
                Multistate_Output_Level[i][j] = MULTISTATE_NULL;
            }
        }
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Multistate_Output_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_MULTISTATE_OUTPUTS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Multistate_Output_Count(
    void)
{
    return MAX_MULTISTATE_OUTPUTS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Multistate_Output_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Multistate_Output_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_MULTISTATE_OUTPUTS;

    if (object_instance < MAX_MULTISTATE_OUTPUTS)
        index = object_instance;

    return index;
}

uint32_t Multistate_Output_Present_Value(
    uint32_t object_instance)
{
    uint32_t value = MULTISTATE_RELINQUISH_DEFAULT;
    unsigned index = 0;
    unsigned i = 0;

    index = Multistate_Output_Instance_To_Index(object_instance);
    if (index < MAX_MULTISTATE_OUTPUTS) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (Multistate_Output_Level[index][i] != MULTISTATE_NULL) {
                value = Multistate_Output_Level[index][i];
                break;
            }
        }
    }

    return value;
}

/* note: the object name must be unique within this device */
bool Multistate_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;

    if (object_instance < MAX_MULTISTATE_OUTPUTS) {
        sprintf(text_string, "MULTISTATE OUTPUT %u", object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

bool Multistate_Output_Out_Of_Service(
    uint32_t instance)
{
    unsigned index = 0;
    bool oos_flag = false;

    index = Multistate_Output_Instance_To_Index(instance);
    if (index < MAX_MULTISTATE_OUTPUTS) {
        oos_flag = Out_Of_Service[index];
    }

    return oos_flag;
}

void Multistate_Output_Out_Of_Service_Set(
    uint32_t instance,
    bool oos_flag)
{
    unsigned index = 0;

    index = Multistate_Output_Instance_To_Index(instance);
    if (index < MAX_MULTISTATE_OUTPUTS) {
        Out_Of_Service[index] = oos_flag;
    }
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Multistate_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint32_t present_value = 0;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0],
                OBJECT_MULTI_STATE_OUTPUT, rpdata->object_instance);
            break;
            /* note: Name and Description don't have to be the same.
               You could make Description writable and different */
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Multistate_Output_Object_Name(rpdata->object_instance,
                &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                OBJECT_MULTI_STATE_OUTPUT);
            break;
        case PROP_PRESENT_VALUE:
            present_value =
                Multistate_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_unsigned(&apdu[0], present_value);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            state = Multistate_Output_Out_Of_Service(rpdata->object_instance);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, state);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            state = Multistate_Output_Out_Of_Service(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
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
                    Multistate_Output_Instance_To_Index
                    (rpdata->object_instance);
                for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                    /* FIXME: check if we have room before adding it to APDU */
                    if (Multistate_Output_Level[object_index][i] ==
                        MULTISTATE_NULL)
                        len = encode_application_null(&apdu[apdu_len]);
                    else {
                        present_value =
                            Multistate_Output_Level[object_index][i];
                        len =
                            encode_application_unsigned(&apdu[apdu_len],
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
                    Multistate_Output_Instance_To_Index
                    (rpdata->object_instance);
                if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                    if (Multistate_Output_Level[object_index]
                        [rpdata->array_index - 1] == MULTISTATE_NULL)
                        apdu_len = encode_application_null(&apdu[0]);
                    else {
                        present_value = Multistate_Output_Level[object_index]
                            [rpdata->array_index - 1];
                        apdu_len =
                            encode_application_unsigned(&apdu[0],
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
            present_value = MULTISTATE_RELINQUISH_DEFAULT;
            apdu_len = encode_application_unsigned(&apdu[0], present_value);
            break;
        case PROP_NUMBER_OF_STATES:
            apdu_len =
                encode_application_unsigned(&apdu[apdu_len],
                MULTISTATE_NUMBER_OF_STATES);
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_STATE_TEXT) &&
        (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Multistate_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int object_index = 0;
    unsigned int priority = 0;
    uint32_t level = 0;
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
    if ((wp_data->object_property != PROP_STATE_TEXT) &&
        (wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                priority = wp_data->priority;
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                    (priority != 6 /* reserved */ ) &&
                    (value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= MULTISTATE_NUMBER_OF_STATES)) {
                    level = value.type.Unsigned_Int;
                    object_index =
                        Multistate_Output_Instance_To_Index
                        (wp_data->object_instance);
                    priority--;
                    Multistate_Output_Level[object_index][priority] =
                        (uint8_t) level;
                    /* Note: you could set the physical output here if we
                       are the highest priority.
                       However, if Out of Service is TRUE, then don't set the
                       physical output.  This comment may apply to the
                       main loop (i.e. check out of service before changing output) */
                    status = true;
                } else if (priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    level = MULTISTATE_NULL;
                    object_index =
                        Multistate_Output_Instance_To_Index
                        (wp_data->object_instance);
                    priority = wp_data->priority;
                    if (priority && (priority <= BACNET_MAX_PRIORITY)) {
                        priority--;
                        Multistate_Output_Level[object_index][priority] =
                            (uint8_t) level;
                        /* Note: you could set the physical output here to the next
                           highest priority, or to the relinquish default if no
                           priorities are set.
                           However, if Out of Service is TRUE, then don't set the
                           physical output.  This comment may apply to the
                           main loop (i.e. check out of service before changing output) */
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
                Multistate_Output_Out_Of_Service_Set(wp_data->object_instance,
                    value.type.Boolean);
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_NUMBER_OF_STATES:
        case PROP_DESCRIPTION:
        case PROP_PRIORITY_ARRAY:
        case PROP_RELINQUISH_DEFAULT:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void testMultistateOutput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Multistate_Output_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_MULTI_STATE_OUTPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Multistate_Output_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_MULTISTATE_OUTPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Multi-state Output", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testMultistateOutput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_INPUT */
#endif /* TEST */
