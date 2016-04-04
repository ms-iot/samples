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

/* Analog Input Objects customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "config.h"
#include "ai.h"
#include "handlers.h"

#ifndef MAX_ANALOG_INPUTS
#define MAX_ANALOG_INPUTS 2
#endif

static float Present_Value[MAX_ANALOG_INPUTS];
static bool Out_Of_Service[MAX_ANALOG_INPUTS];
static BACNET_ENGINEERING_UNITS Units[MAX_ANALOG_INPUTS];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Analog_Input_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_UNITS,
    -1
};

static const int Analog_Input_Properties_Optional[] = {
    -1
};

static const int Analog_Input_Properties_Proprietary[] = {
    -1
};

void Analog_Input_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Analog_Input_Properties_Required;
    if (pOptional)
        *pOptional = Analog_Input_Properties_Optional;
    if (pProprietary)
        *pProprietary = Analog_Input_Properties_Proprietary;

    return;
}

void Analog_Input_Init(
    void)
{
    return;
}

/* we simply have 0-n object instances. */
uint32_t Analog_Input_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances. */
unsigned Analog_Input_Instance_To_Index(
    uint32_t instance)
{
    return instance;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Analog_Input_Valid_Instance(
    uint32_t object_instance)
{
	unsigned index = 0;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
        return true;
    }

    return false;
}

/* we simply have 0-n object instances. */
unsigned Analog_Input_Count(
    void)
{
    return MAX_ANALOG_INPUTS;
}

bool Analog_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32];        /* okay for single thread */
    bool status = false;
	unsigned index = 0;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
        sprintf(text_string, "AI-%lu", object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

float Analog_Input_Present_Value(
    uint32_t object_instance)
{
    float value = 0.0;
	unsigned index = 0;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
        value = Present_Value[index];
    }

    return value;
}

void Analog_Input_Present_Value_Set(
    uint32_t object_instance,
    float value)
{
	unsigned index = 0;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
        Present_Value[index] = value;
    }
}

bool Analog_Input_Out_Of_Service(
    uint32_t object_instance)
{
	unsigned index = 0;
	bool value = false;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
		value = Out_Of_Service[index];
	}

	return value;
}

void Analog_Input_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value)
{
	unsigned index = 0;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
		Out_Of_Service[index] = value;
	}
}

bool Analog_Input_Units_Set(
    uint32_t object_instance,
    uint16_t value)
{
	unsigned index = 0;
    bool status = false;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
        Units[index] = value;
        status = true;
    }

    return status;
}

uint16_t Analog_Input_Units(
    uint32_t object_instance)
{
	unsigned index = 0;
    uint16_t value = UNITS_NO_UNITS;

	index = Analog_Input_Instance_To_Index(object_instance);
	if (index < MAX_ANALOG_INPUTS) {
        value = Units[index];
    }

    return value;
}

/* return apdu length, or -1 on error */
/* assumption - object already exists */
int Analog_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string = { 0 };
    BACNET_BIT_STRING bit_string = { 0 };
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], rpdata->object_type,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            Analog_Input_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], rpdata->object_type);
            break;
        case PROP_PRESENT_VALUE:
            apdu_len =
                encode_application_real(&apdu[0],
                Analog_Input_Present_Value(rpdata->object_instance));
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                Analog_Input_Out_Of_Service(rpdata->object_instance));
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0],
                Analog_Input_Out_Of_Service(rpdata->object_instance));
            break;
        case PROP_UNITS:
            apdu_len = encode_application_enumerated(&apdu[0],
                Analog_Input_Units(rpdata->object_instance));
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
bool Analog_Input_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
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
    if ((wp_data->object_property != PROP_EVENT_TIME_STAMPS) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch ((int) wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if (Analog_Input_Out_Of_Service(wp_data->object_instance)) {
                    Analog_Input_Present_Value_Set(wp_data->object_instance,
                    value.type.Real);
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                    status = false;
                }
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Analog_Input_Out_Of_Service_Set(
                    wp_data->object_instance,
                    value.type.Boolean);
            }
            break;
        case PROP_UNITS:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Analog_Input_Out_Of_Service_Set(
                    wp_data->object_instance,
                    value.type.Enumerated);
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
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
