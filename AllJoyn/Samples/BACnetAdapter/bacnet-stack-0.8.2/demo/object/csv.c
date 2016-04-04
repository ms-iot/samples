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
*
*********************************************************************/

/* CharacterString Value Objects */

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
#include "csv.h"
#include "handlers.h"

/* number of demo objects */
#ifndef MAX_CHARACTERSTRING_VALUES
#define MAX_CHARACTERSTRING_VALUES 1
#endif

/* Here is our Present Value */
static BACNET_CHARACTER_STRING Present_Value[MAX_CHARACTERSTRING_VALUES];
/* Writable out-of-service allows others to manipulate our Present Value */
static bool Out_Of_Service[MAX_CHARACTERSTRING_VALUES];
static char Object_Name[MAX_CHARACTERSTRING_VALUES][64];
static char Object_Description[MAX_CHARACTERSTRING_VALUES][64];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    -1
};

static const int Properties_Optional[] = {
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};

void CharacterString_Value_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;

    return;
}

void CharacterString_Value_Init(
    void)
{
    unsigned i;

    /* initialize all Present Values */
    for (i = 0; i < MAX_CHARACTERSTRING_VALUES; i++) {
        characterstring_init_ansi(&Present_Value[i], "");
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned CharacterString_Value_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_CHARACTERSTRING_VALUES;

    if (object_instance < MAX_CHARACTERSTRING_VALUES) {
        index = object_instance;
    }

    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t CharacterString_Value_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned CharacterString_Value_Count(
    void)
{
    return MAX_CHARACTERSTRING_VALUES;
}

bool CharacterString_Value_Valid_Instance(
    uint32_t object_instance)
{
    unsigned index = 0; /* offset from instance lookup */

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        return true;
    }

    return false;
}

bool CharacterString_Value_Present_Value(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;
    unsigned index = 0; /* offset from instance lookup */

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (object_name && (index < MAX_CHARACTERSTRING_VALUES)) {
        status = characterstring_copy(object_name, &Present_Value[index]);
    }

    return status;
}

bool CharacterString_Value_Present_Value_Set(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;
    unsigned index = 0; /* offset from instance lookup */

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        status = characterstring_copy(&Present_Value[index], object_name);
    }

    return status;
}

bool CharacterString_Value_Out_Of_Service(
    uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        value = Out_Of_Service[index];
    }

    return value;
}

static void CharacterString_Value_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value)
{
    unsigned index = 0;

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        Out_Of_Service[index] = value;
    }

    return;
}

static char *CharacterString_Value_Description(
    uint32_t object_instance)
{
    unsigned index = 0; /* offset from instance lookup */
    char *pName = NULL; /* return value */

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        pName = Object_Description[index];
    }

    return pName;
}

bool CharacterString_Value_Description_Set(
    uint32_t object_instance,
    char *new_name)
{
    unsigned index = 0; /* offset from instance lookup */
    size_t i = 0;       /* loop counter */
    bool status = false;        /* return value */

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        status = true;
        if (new_name) {
            for (i = 0; i < sizeof(Object_Description[index]); i++) {
                Object_Description[index][i] = new_name[i];
                if (new_name[i] == 0) {
                    break;
                }
            }
        } else {
            for (i = 0; i < sizeof(Object_Description[index]); i++) {
                Object_Description[index][i] = 0;
            }
        }
    }

    return status;
}

bool CharacterString_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        status = characterstring_init_ansi(object_name, Object_Name[index]);
    }

    return status;
}

/* note: the object name must be unique within this device */
bool CharacterString_Value_Name_Set(
    uint32_t object_instance,
    char *new_name)
{
    unsigned index = 0; /* offset from instance lookup */
    size_t i = 0;       /* loop counter */
    bool status = false;        /* return value */

    index = CharacterString_Value_Instance_To_Index(object_instance);
    if (index < MAX_CHARACTERSTRING_VALUES) {
        status = true;
        /* FIXME: check to see if there is a matching name */
        if (new_name) {
            for (i = 0; i < sizeof(Object_Name[index]); i++) {
                Object_Name[index][i] = new_name[i];
                if (new_name[i] == 0) {
                    break;
                }
            }
        } else {
            for (i = 0; i < sizeof(Object_Name[index]); i++) {
                Object_Name[index][i] = 0;
            }
        }
    }

    return status;
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int CharacterString_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
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
                OBJECT_CHARACTERSTRING_VALUE, rpdata->object_instance);
            break;
            /* note: Name and Description don't have to be the same.
               You could make Description writable and different */
        case PROP_OBJECT_NAME:
            CharacterString_Value_Object_Name(rpdata->object_instance,
                &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_DESCRIPTION:
            characterstring_init_ansi(&char_string,
                CharacterString_Value_Description(rpdata->object_instance));
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                OBJECT_CHARACTERSTRING_VALUE);
            break;
        case PROP_PRESENT_VALUE:
            CharacterString_Value_Present_Value(rpdata->object_instance,
                &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            if (CharacterString_Value_Out_Of_Service(rpdata->object_instance)) {
                bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                    true);
            } else {
                bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                    false);
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            object_index =
                CharacterString_Value_Instance_To_Index
                (rpdata->object_instance);
            state = Out_Of_Service[object_index];
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_STATE_TEXT) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool CharacterString_Value_Write_Property(
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
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            status =
                WPValidateArgType(&value,
                BACNET_APPLICATION_TAG_CHARACTER_STRING, &wp_data->error_class,
                &wp_data->error_code);
            if (status) {
                status =
                    CharacterString_Value_Present_Value_Set
                    (wp_data->object_instance, &value.type.Character_String);
                if (!status) {
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
                CharacterString_Value_Out_Of_Service_Set
                    (wp_data->object_instance, value.type.Boolean);
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
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

void testCharacterStringValue(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    CharacterString_Value_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_CHARACTERSTRING_VALUE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = CharacterString_Value_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_CHARACTERSTRING_VALUE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet CharacterString Value", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testCharacterStringValue);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif
#endif /* TEST */
