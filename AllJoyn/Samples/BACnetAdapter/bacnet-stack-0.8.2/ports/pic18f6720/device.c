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
#include <string.h>     /* for memmove */
#include "bacdef.h"
#include "bacdcode.h"
#include "bacstr.h"
#include "bacenum.h"
#include "config.h"     /* the custom stuff */
#include "apdu.h"
#include "dlmstp.h"
#include "rs485.h"
#include "ai.h"
#include "av.h"
#include "bi.h"
#include "bv.h"
#include "rp.h"
#include "wp.h"
#include "dcc.h"
#include "version.h"
#include "device.h"     /* me */

/* note: you really only need to define variables for
   properties that are writable or that may change.
   The properties that are constant can be hard coded
   into the read-property encoding. */
static uint32_t Object_Instance_Number = 12345;
static BACNET_DEVICE_STATUS System_Status = STATUS_OPERATIONAL;
static uint8_t Database_Revision;
BACNET_REINITIALIZED_STATE Reinitialize_State = BACNET_REINIT_IDLE;

bool Device_Reinitialize(
    BACNET_REINITIALIZE_DEVICE_DATA * rd_data)
{
    bool status = false;
    char password[16] = "filister";

    if (characterstring_ansi_same(&rd_data->password, password)) {
        Reinitialize_State = rd_data->state;
        dcc_set_status_duration(COMMUNICATION_ENABLE, 0);
        /* Note: you could use a mix of state
           and password to multiple things */
        /* note: you probably want to restart *after* the
           simple ack has been sent from the return handler
           so just set a flag from here */
        status = true;
    } else {
        rd_data->error_class = ERROR_CLASS_SECURITY;
        rd_data->error_code = ERROR_CODE_PASSWORD_FAILURE;
    }

    return status;
}

BACNET_REINITIALIZED_STATE Device_Reinitialized_State(
    void)
{
    return Reinitialize_State;
}

void Device_Init(
    object_functions_t * object_table)
{
    (void) object_table;
    Reinitialize_State = BACNET_REINIT_IDLE;
    dcc_set_status_duration(COMMUNICATION_ENABLE, 0);
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
        Database_Revision++;
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
    return (Object_Instance_Number == object_id);
}

BACNET_DEVICE_STATUS Device_System_Status(
    void)
{
    return System_Status;
}

int Device_Set_System_Status(
    BACNET_DEVICE_STATUS status,
    bool local)
{
    if (status < MAX_DEVICE_STATUS) {
        System_Status = status;
    }
}

uint16_t Device_Vendor_Identifier(
    void)
{
    return BACNET_VENDOR_ID;
}

uint8_t Device_Protocol_Version(
    void)
{
    return BACNET_PROTOCOL_VERSION;
}

uint8_t Device_Protocol_Revision(
    void)
{
    return BACNET_PROTOCOL_REVISION;
}

BACNET_SEGMENTATION Device_Segmentation_Supported(
    void)
{
    return SEGMENTATION_NONE;
}

uint32_t Device_Database_Revision(
    void)
{
    return Database_Revision;
}

/* Since many network clients depend on the object list */
/* for discovery, it must be consistent! */
unsigned Device_Object_List_Count(
    void)
{
    unsigned count = 1; /* at least 1 for device object */

/* FIXME: add objects as needed */
    count += Binary_Value_Count();
    count += Analog_Input_Count();
    count += Binary_Input_Count();
    count += Analog_Value_Count();

    return count;
}

/* Since many network clients depend on the object list */
/* for discovery, it must be consistent! */
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
    /* analog input objects */
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
    /* analog input objects */
    if (!status) {
        /* array index starts at 1, and 1 for the device object */
        object_index -= object_count;
        object_count = Analog_Input_Count();
        if (object_index < object_count) {
            *object_type = OBJECT_ANALOG_INPUT;
            *instance = Analog_Input_Index_To_Instance(object_index);
            status = true;
        }
    }
    /* binary input objects */
    if (!status) {
        /* normalize the index since
           we know it is not the previous objects */
        object_index -= object_count;
        object_count = Binary_Input_Count();
        /* is it a valid index for this object? */
        if (object_index < object_count) {
            *object_type = OBJECT_BINARY_INPUT;
            *instance = Binary_Input_Index_To_Instance(object_index);
            status = true;
        }
    }

    return status;
}

/* returns true if successful */
int Device_Read_Property_Local(
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
    BACNET_TIME local_time;
    BACNET_DATE local_date;
    uint8_t year = 0;
    char string_buffer[28];
    int16_t TimeZone = 0;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    /* FIXME: change the hardcoded names to suit your application */
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_DEVICE,
                Object_Instance_Number);
            break;
        case PROP_OBJECT_NAME:
            (void) strcpypgm2ram(&string_buffer[0], "PIC18F6720 Device");
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_DEVICE);
            break;
        case PROP_DESCRIPTION:
            (void) strcpypgm2ram(&string_buffer[0], "BACnet Demo");
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_SYSTEM_STATUS:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Device_System_Status());
            break;
        case PROP_VENDOR_NAME:
            (void) strcpypgm2ram(&string_buffer[0], BACNET_VENDOR_NAME);
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_VENDOR_IDENTIFIER:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Vendor_Identifier());
            break;
        case PROP_MODEL_NAME:
            (void) strcpypgm2ram(&string_buffer[0], "GNU Demo");
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_FIRMWARE_REVISION:
            (void) strcpypgm2ram(&string_buffer[0], BACNET_VERSION_TEXT);
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_APPLICATION_SOFTWARE_VERSION:
            (void) strcpypgm2ram(&string_buffer[0], "1.0");
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_LOCATION:
            (void) strcpypgm2ram(&string_buffer[0], "USA");
            characterstring_init_ansi(&char_string, string_buffer);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_PROTOCOL_VERSION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Protocol_Version());
            break;
        case PROP_PROTOCOL_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Protocol_Revision());
            break;
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
            /* Note: list of services that are executed, not initiated. */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_BACNET_SERVICES_SUPPORTED; i++) {
                /* automatic lookup based on handlers set */
                bitstring_set_bit(&bit_string, (uint8_t) i,
                    apdu_service_supported(i));
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
            /* Note: this is the list of objects that can be in this device,
               not a list of objects that this device can access */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_ASHRAE_OBJECT_TYPE; i++) {
                /* initialize all the object types to not-supported */
                bitstring_set_bit(&bit_string, (uint8_t) i, false);
            }
            /* FIXME: indicate the objects that YOU support */
            bitstring_set_bit(&bit_string, OBJECT_DEVICE, true);
            bitstring_set_bit(&bit_string, OBJECT_ANALOG_VALUE, true);
            bitstring_set_bit(&bit_string, OBJECT_BINARY_VALUE, true);
            bitstring_set_bit(&bit_string, OBJECT_ANALOG_INPUT, true);
            bitstring_set_bit(&bit_string, OBJECT_BINARY_INPUT, true);
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
                    if (Device_Object_List_Identifier(i, &object_type,
                            &instance)) {
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
                    } else {
                        /* error: internal error? */
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_OTHER;
                        apdu_len = BACNET_STATUS_ERROR;
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
                encode_application_enumerated(&apdu[0],
                Device_Segmentation_Supported());
            break;
        case PROP_APDU_TIMEOUT:
            apdu_len = encode_application_unsigned(&apdu[0], apdu_timeout());
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
            apdu_len = encode_application_unsigned(&apdu[0], apdu_retries());
            break;
        case PROP_DEVICE_ADDRESS_BINDING:
            /* FIXME: encode the list here, if it exists */
            break;
        case PROP_DATABASE_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Database_Revision());
            break;
        case PROP_MAX_INFO_FRAMES:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                dlmstp_max_info_frames());
            break;
        case PROP_MAX_MASTER:
            apdu_len =
                encode_application_unsigned(&apdu[0], dlmstp_max_master());
            break;
        case PROP_LOCAL_TIME:
            /* FIXME: if you support time */
            local_time.hour = 0;
            local_time.min = 0;
            local_time.sec = 0;
            local_time.hundredths = 0;
            apdu_len = encode_application_time(&apdu[0], &local_time);
            break;
        case PROP_UTC_OFFSET:
            /* Note: BACnet Time Zone is offset of local time and UTC,
               rather than offset of GMT.  It is expressed in minutes */
            apdu_len = encode_application_signed(&apdu[0], 5 * 60 /* EST */ );
            break;
        case PROP_LOCAL_DATE:
            /* FIXME: if you support date */
            local_date.year = 2006;     /* AD */
            local_date.month = 4;       /* Jan=1..Dec=12 */
            local_date.day = 11;        /* 1..31 */
            local_date.wday = 0;        /* 1=Mon..7=Sun */
            apdu_len = encode_application_date(&apdu[0], &local_date);
            break;
        case PROP_DAYLIGHT_SAVINGS_STATUS:
            /* FIXME: if you support time/date */
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        case 9600:
            apdu_len =
                encode_application_unsigned(&apdu[0], RS485_Get_Baud_Rate());
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

int Device_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = BACNET_STATUS_ERROR;

    /* initialize the default return values */
    rpdata->error_class = ERROR_CLASS_OBJECT;
    rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    switch (rpdata->object_type) {
        case OBJECT_ANALOG_INPUT:
            if (Analog_Input_Valid_Instance(rpdata->object_instance)) {
                apdu_len = Analog_Input_Read_Property(rpdata);
            }
            break;
        case OBJECT_ANALOG_VALUE:
            if (Analog_Value_Valid_Instance(rpdata->object_instance)) {
                apdu_len = Analog_Value_Read_Property(rpdata);
            }
            break;
        case OBJECT_BINARY_INPUT:
            if (Binary_Input_Valid_Instance(rpdata->object_instance)) {
                apdu_len = Binary_Input_Read_Property(rpdata);
            }
            break;
        case OBJECT_BINARY_VALUE:
            if (Binary_Value_Valid_Instance(rpdata->object_instance)) {
                apdu_len = Binary_Value_Read_Property(rpdata);
            }
            break;
        case OBJECT_DEVICE:
            if (Device_Valid_Object_Instance_Number(rpdata->object_instance)) {
                apdu_len = Device_Read_Property_Local(rpdata);
            }
            break;
        default:
            break;
    }

    return apdu_len;
}

bool Device_Write_Property_Local(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!Device_Valid_Object_Instance_Number(wp_data->object_instance)) {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return false;
    }
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
    if ((wp_data->object_property != PROP_OBJECT_LIST) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            if (value.tag == BACNET_APPLICATION_TAG_OBJECT_ID) {
                if ((value.type.Object_Id.type == OBJECT_DEVICE) &&
                    (Device_Set_Object_Instance_Number(value.type.
                            Object_Id.instance))) {
                    /* we could send an I-Am broadcast to let the world know */
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_MAX_INFO_FRAMES:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if (value.type.Unsigned_Int <= 255) {
                    dlmstp_set_max_info_frames(value.type.Unsigned_Int);
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_MAX_MASTER:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if ((value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= 127)) {
                    dlmstp_set_max_master(value.type.Unsigned_Int);
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_OBJECT_NAME:
            if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
                uint8_t encoding;
                size_t len;

                encoding =
                    characterstring_encoding(&value.type.Character_String);
                len = characterstring_length(&value.type.Character_String);
                if (encoding == CHARACTER_ANSI_X34) {
                    if (len <= 20) {
                        /* FIXME: set the name */
                        /* Display_Set_Name(
                           characterstring_value(&value.type.Character_String)); */
                        /* FIXME:  All the object names in a device must be unique.
                           Disallow setting the Device Object Name to any objects in
                           the device. */
                    } else {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code =
                            ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
                    }
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code =
                        ERROR_CODE_CHARACTER_SET_NOT_SUPPORTED;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case 9600:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if (value.type.Unsigned_Int > 115200) {
                    RS485_Set_Baud_Rate(value.type.Unsigned_Int);
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
        case PROP_APDU_TIMEOUT:
        case PROP_VENDOR_IDENTIFIER:
        case PROP_SYSTEM_STATUS:
        case PROP_LOCATION:
        case PROP_DESCRIPTION:
        case PROP_MODEL_NAME:
        case PROP_VENDOR_NAME:
        case PROP_FIRMWARE_REVISION:
        case PROP_APPLICATION_SOFTWARE_VERSION:
        case PROP_LOCAL_TIME:
        case PROP_UTC_OFFSET:
        case PROP_LOCAL_DATE:
        case PROP_DAYLIGHT_SAVINGS_STATUS:
        case PROP_PROTOCOL_VERSION:
        case PROP_PROTOCOL_REVISION:
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
        case PROP_OBJECT_LIST:
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
        case PROP_SEGMENTATION_SUPPORTED:
        case PROP_DEVICE_ADDRESS_BINDING:
        case PROP_DATABASE_REVISION:
        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
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

bool Device_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* Ever the pessamist! */
    struct object_functions *pObject = NULL;

    /* initialize the default return values */
    wp_data->error_class = ERROR_CLASS_OBJECT;
    wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    switch (wp_data->object_type) {
        case OBJECT_ANALOG_INPUT:
            if (Analog_Input_Valid_Instance(wp_data->object_instance)) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
            break;
        case OBJECT_ANALOG_VALUE:
            if (Analog_Value_Valid_Instance(wp_data->object_instance)) {
                status = Analog_Value_Write_Property(wp_data);
            }
            break;
        case OBJECT_BINARY_INPUT:
            if (Binary_Input_Valid_Instance(wp_data->object_instance)) {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
            break;
        case OBJECT_BINARY_VALUE:
            if (Binary_Value_Valid_Instance(wp_data->object_instance)) {
                status = Binary_Value_Write_Property(wp_data);
            }
            break;
        case OBJECT_DEVICE:
            if (Device_Valid_Object_Instance_Number(wp_data->object_instance)) {
                status = Device_Write_Property_Local(wp_data);
            }
            break;
        default:
            break;
    }

    return (status);
}
