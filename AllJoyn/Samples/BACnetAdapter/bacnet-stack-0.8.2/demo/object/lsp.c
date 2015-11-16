/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

/* Life Safety Point Objects - customize for your use */

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
#include "lsp.h"
#include "handlers.h"
#include "proplist.h"

#ifndef MAX_LIFE_SAFETY_POINTS
#define MAX_LIFE_SAFETY_POINTS 7
#endif

/* Here are our stored levels.*/
static BACNET_LIFE_SAFETY_MODE Life_Safety_Point_Mode[MAX_LIFE_SAFETY_POINTS];
static BACNET_LIFE_SAFETY_STATE
    Life_Safety_Point_State[MAX_LIFE_SAFETY_POINTS];
static BACNET_SILENCED_STATE
    Life_Safety_Point_Silenced_State[MAX_LIFE_SAFETY_POINTS];
static BACNET_LIFE_SAFETY_OPERATION
    Life_Safety_Point_Operation[MAX_LIFE_SAFETY_POINTS];
/* Writable out-of-service allows others to play with our Present Value */
/* without changing the physical output */
static bool Life_Safety_Point_Out_Of_Service[MAX_LIFE_SAFETY_POINTS];
/* These arrays are used by the ReadPropertyMultiple handler and
   property-list property (as of protocol-revision 14) */
static const int Life_Safety_Point_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_TRACKING_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_RELIABILITY,
    PROP_MODE,
    PROP_ACCEPTED_MODES,
    PROP_SILENCED,
    PROP_OPERATION_EXPECTED,
    -1
};

static const int Life_Safety_Point_Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Life_Safety_Point_Properties_Proprietary[] = {
    -1
};

/**
 * Returns the list of required, optional, and proprietary properties.
 * Used by ReadPropertyMultiple service.
 *
 * @param pRequired - pointer to list of int terminated by -1, of
 * BACnet required properties for this object.
 * @param pOptional - pointer to list of int terminated by -1, of
 * BACnet optkional properties for this object.
 * @param pProprietary - pointer to list of int terminated by -1, of
 * BACnet proprietary properties for this object.
 */
void Life_Safety_Point_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired) {
        *pRequired = Life_Safety_Point_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Life_Safety_Point_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Life_Safety_Point_Properties_Proprietary;
    }

    return;
}

void Life_Safety_Point_Init(
    void)
{
    static bool initialized = false;
    unsigned i;

    if (!initialized) {
        initialized = true;

        /* initialize all the analog output priority arrays to NULL */
        for (i = 0; i < MAX_LIFE_SAFETY_POINTS; i++) {
            Life_Safety_Point_Mode[i] = LIFE_SAFETY_MODE_DEFAULT;
            Life_Safety_Point_State[i] = LIFE_SAFETY_STATE_QUIET;
            Life_Safety_Point_Silenced_State[i] = SILENCED_STATE_UNSILENCED;
            Life_Safety_Point_Operation[i] = LIFE_SAFETY_OP_NONE;
        }
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Life_Safety_Point_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_LIFE_SAFETY_POINTS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Life_Safety_Point_Count(
    void)
{
    return MAX_LIFE_SAFETY_POINTS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Life_Safety_Point_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Life_Safety_Point_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_LIFE_SAFETY_POINTS;

    if (object_instance < MAX_LIFE_SAFETY_POINTS)
        index = object_instance;

    return index;
}

static BACNET_LIFE_SAFETY_STATE Life_Safety_Point_Present_Value(
    uint32_t object_instance)
{
    BACNET_LIFE_SAFETY_STATE present_value = LIFE_SAFETY_STATE_QUIET;
    unsigned index = 0;

    index = Life_Safety_Point_Instance_To_Index(object_instance);
    if (index < MAX_LIFE_SAFETY_POINTS)
        present_value = Life_Safety_Point_State[index];

    return present_value;
}

/* note: the object name must be unique within this device */
bool Life_Safety_Point_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;

    if (object_instance < MAX_LIFE_SAFETY_POINTS) {
        sprintf(text_string, "LS POINT %u", object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Life_Safety_Point_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_LIFE_SAFETY_STATE present_value = LIFE_SAFETY_STATE_QUIET;
    BACNET_LIFE_SAFETY_MODE mode = LIFE_SAFETY_MODE_DEFAULT;
    BACNET_SILENCED_STATE silenced_state = SILENCED_STATE_UNSILENCED;
    BACNET_LIFE_SAFETY_OPERATION operation = LIFE_SAFETY_OP_NONE;
    unsigned object_index = 0;
    bool state = false;
    BACNET_RELIABILITY reliability = RELIABILITY_NO_FAULT_DETECTED;
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
                OBJECT_LIFE_SAFETY_POINT, rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Life_Safety_Point_Object_Name(rpdata->object_instance,
                &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                OBJECT_LIFE_SAFETY_POINT);
            break;
        case PROP_PRESENT_VALUE:
            present_value =
                Life_Safety_Point_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_TRACKING_VALUE:
            /* FIXME: tracking value is a local matter how it is derived */
            present_value =
                Life_Safety_Point_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            object_index =
                Life_Safety_Point_Instance_To_Index(rpdata->object_instance);
            state = Life_Safety_Point_Out_Of_Service[object_index];
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_RELIABILITY:
            /* see standard for details about this property */
            reliability = RELIABILITY_NO_FAULT_DETECTED;
            apdu_len = encode_application_enumerated(&apdu[0], reliability);
            break;
        case PROP_MODE:
            object_index =
                Life_Safety_Point_Instance_To_Index(rpdata->object_instance);
            mode = Life_Safety_Point_Mode[object_index];
            apdu_len = encode_application_enumerated(&apdu[0], mode);
            break;
        case PROP_ACCEPTED_MODES:
            for (mode = MIN_LIFE_SAFETY_MODE; mode < MAX_LIFE_SAFETY_MODE;
                mode++) {
                len = encode_application_enumerated(&apdu[apdu_len], mode);
                apdu_len += len;
            }
            break;
        case PROP_SILENCED:
            object_index =
                Life_Safety_Point_Instance_To_Index(rpdata->object_instance);
            silenced_state = Life_Safety_Point_Silenced_State[object_index];
            apdu_len = encode_application_enumerated(&apdu[0], silenced_state);
            break;
        case PROP_OPERATION_EXPECTED:
            object_index =
                Life_Safety_Point_Instance_To_Index(rpdata->object_instance);
            operation = Life_Safety_Point_Operation[object_index];
            apdu_len = encode_application_enumerated(&apdu[0], operation);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Life_Safety_Point_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int object_index = 0;
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
    /*  only array properties can have array options */
    if (wp_data->array_index != BACNET_ARRAY_ALL) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_MODE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if (value.type.Enumerated <= MAX_LIFE_SAFETY_MODE) {
                    object_index =
                        Life_Safety_Point_Instance_To_Index
                        (wp_data->object_instance);
                    Life_Safety_Point_Mode[object_index] =
                        value.type.Enumerated;
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                object_index =
                    Life_Safety_Point_Instance_To_Index
                    (wp_data->object_instance);
                Life_Safety_Point_Out_Of_Service[object_index] =
                    value.type.Boolean;
            }
            break;



        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
        case PROP_OBJECT_TYPE:
        case PROP_PRESENT_VALUE:
        case PROP_TRACKING_VALUE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_RELIABILITY:
        case PROP_ACCEPTED_MODES:
        case PROP_SILENCED:
        case PROP_OPERATION_EXPECTED:
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

void testLifeSafetyPoint(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Life_Safety_Point_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_LIFE_SAFETY_POINT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Life_Safety_Point_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_LIFE_SAFETY_POINT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Life Safety Point", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testLifeSafetyPoint);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_LIFE_SAFETY_POINT */
#endif /* TEST */
