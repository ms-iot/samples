/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
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

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacstr.h"
#include "bacenum.h"
#include "apdu.h"
#include "dcc.h"
#include "dlmstp.h"
#include "rs485.h"
#include "version.h"
/* objects */
#include "device.h"
#include "av.h"

/* note: you really only need to define variables for
   properties that are writable or that may change.
   The properties that are constant can be hard coded
   into the read-property encoding. */
static uint32_t Object_Instance_Number = 260001;
static char *Object_Name = "My Device";
static BACNET_DEVICE_STATUS System_Status = STATUS_OPERATIONAL;

void Device_Init(
    object_functions_t * object_table)
{
    (void) object_table;
    /* Reinitialize_State = BACNET_REINIT_IDLE; */
    /* dcc_set_status_duration(COMMUNICATION_ENABLE, 0); */
    /* FIXME: Get the data from the eeprom */
    /* I2C_Read_Block(EEPROM_DEVICE_ADDRESS,
       (char *)&Object_Instance_Number,
       sizeof(Object_Instance_Number),
       EEPROM_BACNET_ID_ADDR); */
}

/* methods to manipulate the data */
uint32_t Device_Object_Instance_Number(
    void)
{
    return Object_Instance_Number;
}

bool Device_Set_Object_Instance_Number(
    uint32_t object_id)
{
    bool status = true; /* return value */

    if (object_id <= BACNET_MAX_INSTANCE) {
        Object_Instance_Number = object_id;
        /* FIXME: Write the data to the eeprom */
        /* I2C_Write_Block(
           EEPROM_DEVICE_ADDRESS,
           (char *)&Object_Instance_Number,
           sizeof(Object_Instance_Number),
           EEPROM_BACNET_ID_ADDR); */
    } else
        status = false;

    return status;
}

bool Device_Valid_Object_Instance_Number(
    uint32_t object_id)
{
    /* BACnet allows for a wildcard instance number */
    return ((Object_Instance_Number == object_id) ||
        (object_id == BACNET_MAX_INSTANCE));
}

uint16_t Device_Vendor_Identifier(
    void)
{
    return BACNET_VENDOR_ID;
}

unsigned Device_Object_List_Count(
    void)
{
    unsigned count = 1; /* at least 1 for device object */

#if MAX_ANALOG_VALUES
    /* FIXME: add objects as needed */
    count += Analog_Value_Count();
#endif
#if MAX_BINARY_VALUES
    /* FIXME: add objects as needed */
    count += Binary_Value_Count();
#endif

    return count;
}

bool Device_Object_List_Identifier(
    unsigned array_index,
    int *object_type,
    uint32_t * instance)
{
    bool status = false;
    unsigned object_index = 0;
    unsigned object_count = 0;

    /* device object */
    if (array_index == 1) {
        *object_type = OBJECT_DEVICE;
        *instance = Object_Instance_Number;
        status = true;
    }
    /* normalize the index since
       we know it is not the previous objects */
    /* array index starts at 1 */
    object_index = array_index - 1;
    /* 1 for the device object */
    object_count = 1;
    /* FIXME: add objects as needed */
#if MAX_ANALOG_VALUES
    /* analog value objects */
    if (!status) {
        /* array index starts at 1, and 1 for the device object */
        object_index -= object_count;
        object_count = Analog_Value_Count();
        if (object_index < object_count) {
            *object_type = OBJECT_ANALOG_VALUE;
            *instance = Analog_Value_Index_To_Instance(object_index);
            status = true;
        }
    }
#endif
#if MAX_BINARY_VALUES
    /* binary value objects */
    if (!status) {
        object_index -= object_count;
        object_count = Binary_Value_Count();
        /* is it a valid index for this object? */
        if (object_index < object_count) {
            *object_type = OBJECT_BINARY_VALUE;
            *instance = Binary_Value_Index_To_Instance(object_index);
            status = true;
        }
    }
#endif

    return status;
}

/* return the length of the apdu encoded or -1 for error */
int Device_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    int len = 0;        /* apdu len intermediate value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    unsigned i = 0;
    int object_type = 0;
    uint32_t instance = 0;
    unsigned count = 0;
    uint8_t *apdu = NULL;

    if ((rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    /* FIXME: change the hardcoded names to suit your application */
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], rpdata->object_type,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            characterstring_init_ansi(&char_string, Object_Name);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_DEVICE);
            break;
        case PROP_SYSTEM_STATUS:
            apdu_len = encode_application_enumerated(&apdu[0], System_Status);
            break;
        case PROP_VENDOR_NAME:
            characterstring_init_ansi(&char_string, BACNET_VENDOR_NAME);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_VENDOR_IDENTIFIER:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Vendor_Identifier());
            break;
        case PROP_MODEL_NAME:
            characterstring_init_ansi(&char_string, "GNU Demo");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_FIRMWARE_REVISION:
            characterstring_init_ansi(&char_string, BACNET_VERSION_TEXT);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_APPLICATION_SOFTWARE_VERSION:
            characterstring_init_ansi(&char_string, "1.0");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_PROTOCOL_VERSION:
            apdu_len =
                encode_application_unsigned(&apdu[0], BACNET_PROTOCOL_VERSION);
            break;
        case PROP_PROTOCOL_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                BACNET_PROTOCOL_REVISION);
            break;
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
            /* Note: list of services that are executed, not initiated. */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_BACNET_SERVICES_SUPPORTED; i++) {
                /* automatic lookup based on handlers set */
                bitstring_set_bit(&bit_string, (uint8_t) i,
                    apdu_service_supported((BACNET_SERVICES_SUPPORTED) i));
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
            /* Note: this is the list of objects that can be in this device,
               not a list of objects that this device can access */
            bitstring_init(&bit_string);
            /* must have the bit string as big as it can be */
            for (i = 0; i < MAX_ASHRAE_OBJECT_TYPE; i++) {
                /* initialize all the object types to not-supported */
                bitstring_set_bit(&bit_string, (uint8_t) i, false);
            }
            /* FIXME: indicate the objects that YOU support */
            bitstring_set_bit(&bit_string, OBJECT_DEVICE, true);
#if MAX_ANALOG_VALUES
            bitstring_set_bit(&bit_string, OBJECT_ANALOG_VALUE, true);
#endif
#if MAX_BINARY_VALUES
            bitstring_set_bit(&bit_string, OBJECT_BINARY_VALUE, true);
#endif
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_OBJECT_LIST:
            count = Device_Object_List_Count();
            /* Array element zero is the number of objects in the list */
            if (rpdata->array_index == 0)
                apdu_len = encode_application_unsigned(&apdu[0], count);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet.  Note that more than likely you will have */
            /* to return an error if the number of encoded objects exceeds */
            /* your maximum APDU size. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                for (i = 1; i <= count; i++) {
                    Device_Object_List_Identifier(i, &object_type, &instance);
                    len =
                        encode_application_object_id(&apdu[apdu_len],
                        object_type, instance);
                    apdu_len += len;
                    /* assume next one is the same size as this one */
                    /* can we all fit into the APDU? */
                    if ((apdu_len + len) >= MAX_APDU) {
                        rpdata->error_code =
                            ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                        apdu_len = BACNET_STATUS_ABORT;
                        break;
                    }
                }
            } else {
                if (Device_Object_List_Identifier(rpdata->array_index,
                        &object_type, &instance))
                    apdu_len =
                        encode_application_object_id(&apdu[0], object_type,
                        instance);
                else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
            apdu_len = encode_application_unsigned(&apdu[0], MAX_APDU);
            break;
        case PROP_SEGMENTATION_SUPPORTED:
            apdu_len =
                encode_application_enumerated(&apdu[0], SEGMENTATION_NONE);
            break;
        case PROP_APDU_TIMEOUT:
            apdu_len = encode_application_unsigned(&apdu[0], 60000);
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        case PROP_DEVICE_ADDRESS_BINDING:
            /* FIXME: encode the list here, if it exists */
            break;
        case PROP_DATABASE_REVISION:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = -1;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_OBJECT_LIST) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}
