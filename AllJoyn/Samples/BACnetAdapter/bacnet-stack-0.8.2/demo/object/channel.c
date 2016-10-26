/**
 * @file
 * @author Steve Karg
 * @date 2013
 * @brief Channel objects, customize for your use
 *
 * @section DESCRIPTION
 *
 * The Channel object is a command object without a priority array, and the
 * present-value property uses a priority array and a single precision floating point
 * data type.
 *
 * @section LICENSE
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
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "wp.h"
#include "handlers.h"
#include "proplist.h"
#include "lighting.h"
#include "device.h"
#if defined (CHANNEL_LIGHTING_COMMAND) || defined (BACAPP_LIGHTING_COMMAND)
#include "lo.h"
#endif
/* me! */
#include "channel.h"

#ifndef BACNET_CHANNELS_MAX
#define BACNET_CHANNELS_MAX 1
#endif

#ifndef CONTROL_GROUPS_MAX
#define CONTROL_GROUPS_MAX 8
#endif

#ifndef CHANNEL_MEMBERS_MAX
#define CHANNEL_MEMBERS_MAX 8
#endif

struct bacnet_channel_object {
    bool Out_Of_Service:1;
    BACNET_CHANNEL_VALUE Present_Value;
    unsigned Last_Priority;
    BACNET_WRITE_STATUS Write_Status;
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE Members[CHANNEL_MEMBERS_MAX];
    uint16_t Number;
    uint32_t Control_Groups[CONTROL_GROUPS_MAX];
};

struct bacnet_channel_object Channel[BACNET_CHANNELS_MAX];

/* These arrays are used by the ReadPropertyMultiple handler
   property-list property (as of protocol-revision 14) */
static const int Channel_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_LAST_PRIORITY,
    PROP_WRITE_STATUS,
    PROP_STATUS_FLAGS,
    PROP_OUT_OF_SERVICE,
    PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES,
    PROP_CHANNEL_NUMBER,
    PROP_CONTROL_GROUPS,
    -1
};

static const int Channel_Properties_Optional[] = {
    -1
};

static const int Channel_Properties_Proprietary[] = {
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
void Channel_Property_Lists(const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Channel_Properties_Required;
    if (pOptional)
        *pOptional = Channel_Properties_Optional;
    if (pProprietary)
        *pProprietary = Channel_Properties_Proprietary;

    return;
}

/**
 * Determines if a given Channel instance is valid
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  true if the instance is valid, and false if not
 */
bool Channel_Valid_Instance(uint32_t object_instance)
{
    unsigned int index;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        return true;
    }

    return false;
}

/**
 * Determines the number of Channel objects
 *
 * @return  Number of Channel objects
 */
unsigned Channel_Count(void)
{
    return BACNET_CHANNELS_MAX;
}

/**
 * Determines the object instance-number for a given 0..N index
 * of Channel objects where N is Channel_Count().
 *
 * @param  index - 0..BACNET_CHANNELS_MAX value
 *
 * @return  object instance-number for the given index
 */
uint32_t Channel_Index_To_Instance(unsigned index)
{
    uint32_t instance = 1;

    instance += index;

    return instance;
}

/**
 * For a given object instance-number, determines a 0..N index
 * of Channel objects where N is Channel_Count().
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  index for the given instance-number, or BACNET_CHANNELS_MAX
 * if not valid.
 */
unsigned Channel_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = BACNET_CHANNELS_MAX;

    if (object_instance) {
        index = object_instance - 1;
        if (index > BACNET_CHANNELS_MAX) {
            index = BACNET_CHANNELS_MAX;
        }
    }

    return index;
}

/**
 * For a given object instance-number, determines the present-value
 *
 * @param  object_instance - object-instance number of the object
 * @return pointer to the BACNET_CHANNEL_VALUE present-value
 */
BACNET_CHANNEL_VALUE * Channel_Present_Value(uint32_t object_instance)
{
    unsigned index = 0;
    BACNET_CHANNEL_VALUE *cvalue = NULL;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        cvalue = &Channel[index].Present_Value;
    }

    return cvalue;
}

/**
 * For a given object instance-number, determines the last priority.
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return priority - priority 1..16
 */
unsigned Channel_Last_Priority(uint32_t object_instance)
{
    unsigned index = 0;
    unsigned priority = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        priority = Channel[index].Last_Priority;
    }

    return priority;
}

/**
 * For a given object instance-number, determines the write status.
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return BACNET_WRITE_STATUS value
 */
BACNET_WRITE_STATUS Channel_Write_Status(uint32_t object_instance)
{
    unsigned index = 0;
    unsigned priority = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        priority = Channel[index].Write_Status;
    }

    return priority;
}

/**
 * For a given object instance-number, determines the Number
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return Channel Number value
 */
uint16_t Channel_Number(uint32_t object_instance)
{
    unsigned index = 0;
    uint16_t value = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        value = Channel[index].Number;
    }

    return value;
}

/**
 * For a given object instance-number, sets the channel-number
 * property value
 *
 * @param object_instance - object-instance number of the object
 * @param value - channel-number value to set
 *
 * @return true if set
 */
bool Channel_Number_Set(uint32_t object_instance, uint16_t value)
{
    bool status = false;
    unsigned index = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        Channel[index].Number = value;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, determines the member count
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return member count
 */
static bool Channel_Reference_List_Member_Valid(
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember)
{
    bool status = false;

    if ((pMember) &&
        (pMember->objectIdentifier.instance != BACNET_MAX_INSTANCE) &&
        (pMember->deviceIndentifier.instance != BACNET_MAX_INSTANCE)) {
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, determines the member count
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return member count
 */
unsigned Channel_Reference_List_Member_Count(uint32_t object_instance)
{
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember = NULL;
    unsigned count = 0;
    unsigned m = 0;
    unsigned index = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        for (m = 0; m < CHANNEL_MEMBERS_MAX; m++) {
            pMember = &Channel[index].Members[m];
            if (Channel_Reference_List_Member_Valid(pMember)) {
                count++;
            }
        }
    }

    return count;
}

/**
 * For a given object instance-number, returns the member element
 *
 * @param object_instance - object-instance number of the object
 * @param  array_index - 1-based array index
 *
 * @return pointer to member element or NULL if not found
 */
BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *
Channel_Reference_List_Member_Element(uint32_t object_instance,
    unsigned array_index)
{
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember = NULL;
    unsigned count = 0;
    unsigned m = 0;
    unsigned index = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        for (m = 0; m < CHANNEL_MEMBERS_MAX; m++) {
            pMember = &Channel[index].Members[m];
            if (Channel_Reference_List_Member_Valid(pMember)) {
                count++;
                if (count == array_index) {
                    return pMember;
                }
            }
        }
    }

    return NULL;
}

/**
 * For a given object instance-number, returns the member element
 *
 * @param object_instance - object-instance number of the object
 * @param  array_index - 1-based array index
 *
 * @return pointer to member element or NULL if not found
 */
bool Channel_Reference_List_Member_Element_Set(uint32_t object_instance,
    unsigned array_index,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMemberSrc)
{
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember = NULL;
    unsigned count = 0;
    unsigned m = 0;
    unsigned index = 0;
    bool status = false;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        for (m = 0; m < CHANNEL_MEMBERS_MAX; m++) {
            pMember = &Channel[index].Members[m];
            if (Channel_Reference_List_Member_Valid(pMember)) {
                count++;
                if (count == array_index) {
                    memcpy(pMember, pMemberSrc,
                        sizeof(BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE));
                    status = true;
                    break;
                }
            }
        }
    }

    return status;
}

/**
 * For a given object instance-number, adds a member element
 *
 * @param object_instance - object-instance number of the object
 * @param pMemberSrc - pointer to a object property reference element
 *
 * @return array_index - 1-based array index value for added element, or
 * zero if not added
 */
unsigned Channel_Reference_List_Member_Element_Add(uint32_t object_instance,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMemberSrc)
{
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember = NULL;
    unsigned count = 0;
    unsigned m = 0;
    unsigned index = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        for (m = 0; m < CHANNEL_MEMBERS_MAX; m++) {
            pMember = &Channel[index].Members[m];
            if (Channel_Reference_List_Member_Valid(pMember)) {
                count++;
            } else {
                /* first empty slot */
                count++;
                memcpy(pMember, pMemberSrc,
                    sizeof(BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE));
                break;
            }
        }
    }

    return count;
}

/**
 * For a given object instance-number, adds a member element
 *
 * @param object_instance - object-instance number of the object
 * @param type - object type
 * @param instance - object instance number
 * @param propertyIdentifier - property identifier BACNET_PROPERTY_ID
 * @param  array_index - 1-based array index of object property
 *
 * @return array_index - 1-based array index value for added element, or
 * zero if not added
 */
unsigned Channel_Reference_List_Member_Local_Add(
    uint32_t object_instance,
    uint16_t type,
    uint32_t instance,
    BACNET_PROPERTY_ID propertyIdentifier,
    uint32_t arrayIndex)
{
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE member = {{0}};

    member.objectIdentifier.type = type;
    member.objectIdentifier.instance = instance;
    member.propertyIdentifier = propertyIdentifier;
    member.arrayIndex = arrayIndex;
    member.deviceIndentifier.type = OBJECT_DEVICE;
    member.deviceIndentifier.instance = Device_Object_Instance_Number();

    return Channel_Reference_List_Member_Element_Add(
        object_instance,
        &member);
}

/**
 * For a given object instance-number, determines the Number
 *
 * @param  object_instance - object-instance number of the object
 * @param  array_index - 1-based array index
 *
 * @return group number in the array
 */
uint16_t Channel_Control_Groups_Element(
    uint32_t object_instance,
    int32_t array_index)
{
    unsigned index = 0;
    uint16_t value = 0;

    index = Channel_Instance_To_Index(object_instance);
    if ((index < BACNET_CHANNELS_MAX) &&
        (array_index > 0) &&
        (array_index <= CONTROL_GROUPS_MAX)) {
        array_index--;
        value = Channel[index].Control_Groups[array_index];
    }

    return value;
}

/**
 * For a given object instance-number, determines the Number
 *
 * @param object_instance - object-instance number of the object
 * @param array_index - 1-based array index
 * @param value - control group value 0..65535
 *
 * @return true if parameters are value and control group is set
 */
bool Channel_Control_Groups_Element_Set(
    uint32_t object_instance,
    int32_t array_index,
    uint16_t value)
{
    bool status = false;
    unsigned index = 0;

    index = Channel_Instance_To_Index(object_instance);
    if ((index < BACNET_CHANNELS_MAX) &&
        (array_index > 0) &&
        (array_index <= CONTROL_GROUPS_MAX)) {
        array_index--;
        Channel[index].Control_Groups[array_index] = value;
        status = true;
    }

    return status;
}

/**
 * For a given application value, copy to the channel value
 *
 * @param  cvalue - BACNET_CHANNEL_VALUE value
 * @param  value - BACNET_APPLICATION_DATA_VALUE value
 *
 * @return  true if values are able to be copied
 */
bool Channel_Value_Copy(BACNET_CHANNEL_VALUE * cvalue,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    bool status = false;

    if (!value || !cvalue) {
        return false;
    }
    switch (value->tag) {
#if defined (BACAPP_NULL)
        case BACNET_APPLICATION_TAG_NULL:
            cvalue->tag = value->tag;
            status = true;
            break;
#endif
#if defined (BACAPP_BOOLEAN) && defined (CHANNEL_BOOLEAN)
        case BACNET_APPLICATION_TAG_BOOLEAN:
            cvalue->tag = value->tag;
            cvalue->type.Boolean = value->type.Boolean;
            status = true;
            break;
#endif
#if defined (BACAPP_UNSIGNED) && defined (CHANNEL_UNSIGNED)
        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            cvalue->tag = value->tag;
            cvalue->type.Unsigned_Int = value->type.Unsigned_Int;
            status = true;
            break;
#endif
#if defined (BACAPP_SIGNED) && defined (CHANNEL_SIGNED)
        case BACNET_APPLICATION_TAG_SIGNED_INT:
            cvalue->tag = value->tag;
            cvalue->type.Signed_Int = value->type.Signed_Int;
            status = true;
            break;
#endif
#if defined (BACAPP_REAL) && defined (CHANNEL_REAL)
        case BACNET_APPLICATION_TAG_REAL:
            cvalue->tag = value->tag;
            cvalue->type.Real = value->type.Real;
            status = true;
            break;
#endif
#if defined (BACAPP_DOUBLE) && defined (CHANNEL_DOUBLE)
        case BACNET_APPLICATION_TAG_DOUBLE:
            cvalue->tag = value->tag;
            cvalue->type.Double = value->type.Double;
            status = true;
            break;
#endif
#if defined (BACAPP_OCTET_STRING) && defined (CHANNEL_OCTET_STRING)
        case BACNET_APPLICATION_TAG_OCTET_STRING:
            cvalue->tag = value->tag;
            octetstring_copy(
                &cvalue->type.Octet_String,
                &value->type.Octet_String);
            status = true;
            break;
#endif
#if defined (BACAPP_CHARACTER_STRING) && defined (CHANNEL_CHARACTER_STRING)
        case BACNET_APPLICATION_TAG_CHARACTER_STRING:
            cvalue->tag = value->tag;
            characterstring_copy(
                &cvalue->type.Character_String,
                &value->type.Character_String);
            status = true;
            break;
#endif
#if defined (BACAPP_BIT_STRING) && defined (CHANNEL_BIT_STRING)
        case BACNET_APPLICATION_TAG_BIT_STRING:
            cvalue->tag = value->tag;
            bitstring_copy(
                &cvalue->type.Bit_String,
                &value->type.Bit_String);
            status = true;
            break;
#endif
#if defined (BACAPP_ENUMERATED) && defined (CHANNEL_ENUMERATED)
        case BACNET_APPLICATION_TAG_ENUMERATED:
            cvalue->tag = value->tag;
            cvalue->type.Enumerated = value->type.Enumerated;
            status = true;
            break;
#endif
#if defined (BACAPP_DATE) && defined (CHANNEL_DATE)
        case BACNET_APPLICATION_TAG_DATE:
            cvalue->tag = value->tag;
            datetime_date_copy(
                &cvalue->type.Date,
                &value->type.Date);
            apdu_len = encode_application_date(&apdu[0], &value->type.Date);
            status = true;
            break;
#endif
#if defined (BACAPP_TIME) && defined (CHANNEL_TIME)
        case BACNET_APPLICATION_TAG_TIME:
            cvalue->tag = value->tag;
            datetime_time_copy(
                &cvalue->type.Time,
                &value->type.Time);
            break;
#endif
#if defined (BACAPP_OBJECT_ID) && defined (CHANNEL_OBJECT_ID)
        case BACNET_APPLICATION_TAG_OBJECT_ID:
            cvalue->tag = value->tag;
            cvalue->type.Object_Id.type = value->type.Object_Id.type;
            cvalue->type.Object_Id.instance = value->type.Object_Id.instance;
            status = true;
            break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND) && defined (CHANNEL_LIGHTING_COMMAND)
        case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
            cvalue->tag = value->tag;
            lighting_command_copy(
                &cvalue->type.Lighting_Command,
                &value->type.Lighting_Command);
            status = true;
            break;
#endif
        default:
            break;
    }

    return status;
}

/**
 * For a given application value, copy to the channel value
 *
 * @param  apdu - APDU buffer for storing the encoded data
 * @param  apdu_max - size of APDU buffer available for storing data
 * @param  value - BACNET_CHANNEL_VALUE value
 *
 * @return  number of bytes in the APDU, or BACNET_STATUS_ERROR
 */
int Channel_Value_Encode(uint8_t *apdu, int apdu_max,
    BACNET_CHANNEL_VALUE * value)
{
    int apdu_len = BACNET_STATUS_ERROR;

    if (!apdu || !value) {
        return BACNET_STATUS_ERROR;
    }
    switch (value->tag) {
        case BACNET_APPLICATION_TAG_NULL:
            apdu_len = encode_application_null(&apdu[0]);
            break;
#if defined (CHANNEL_BOOLEAN)
        case BACNET_APPLICATION_TAG_BOOLEAN:
            apdu_len = encode_application_boolean(&apdu[0],
                value->type.Boolean);
            break;
#endif
#if defined (CHANNEL_UNSIGNED)
        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                value->type.Unsigned_Int);
            break;
#endif
#if defined (CHANNEL_SIGNED)
        case BACNET_APPLICATION_TAG_SIGNED_INT:
            apdu_len =
                encode_application_signed(&apdu[0],
                value->type.Signed_Int);
            break;
#endif
#if defined (CHANNEL_REAL)
        case BACNET_APPLICATION_TAG_REAL:
            apdu_len = encode_application_real(&apdu[0], value->type.Real);
            break;
#endif
#if defined (CHANNEL_DOUBLE)
        case BACNET_APPLICATION_TAG_DOUBLE:
            apdu_len =
                encode_application_double(&apdu[0], value->type.Double);
            break;
#endif
#if defined (CHANNEL_OCTET_STRING)
        case BACNET_APPLICATION_TAG_OCTET_STRING:
            apdu_len =
                encode_application_octet_string(&apdu[0],
                &value->type.Octet_String);
            break;
#endif
#if defined (CHANNEL_CHARACTER_STRING)
        case BACNET_APPLICATION_TAG_CHARACTER_STRING:
            apdu_len =
                encode_application_character_string(&apdu[0],
                &value->type.Character_String);
            break;
#endif
#if defined (CHANNEL_BIT_STRING)
        case BACNET_APPLICATION_TAG_BIT_STRING:
            apdu_len =
                encode_application_bitstring(&apdu[0],
                &value->type.Bit_String);
            break;
#endif
#if defined (CHANNEL_ENUMERATED)
        case BACNET_APPLICATION_TAG_ENUMERATED:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                value->type.Enumerated);
            break;
#endif
#if defined (CHANNEL_DATE)
        case BACNET_APPLICATION_TAG_DATE:
            apdu_len =
                encode_application_date(&apdu[0], &value->type.Date);
            break;
#endif
#if defined (CHANNEL_TIME)
        case BACNET_APPLICATION_TAG_TIME:
            apdu_len =
                encode_application_time(&apdu[0], &value->type.Time);
            break;
#endif
#if defined (CHANNEL_OBJECT_ID)
        case BACNET_APPLICATION_TAG_OBJECT_ID:
            apdu_len =
                encode_application_object_id(&apdu[0],
                (int) value->type.Object_Id.type,
                value->type.Object_Id.instance);
            break;
#endif
#if defined (CHANNEL_LIGHTING_COMMAND)
        case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
            apdu_len =
                lighting_command_encode(&apdu[0],
                &value->type.Lighting_Command);
            break;
#endif
        default:
            break;
    }

    return apdu_len;
}

/**
 * For a given application value, coerce the encoding, if necessary
 *
 * @param  apdu - buffer to hold the encoding
 * @param  apdu_max - max size of the buffer to hold the encoding
 * @param  value - BACNET_APPLICATION_DATA_VALUE value
 * @param  tag - application tag to be coerced, if possible
 *
 * @return  number of bytes in the APDU, or BACNET_STATUS_ERROR if error.
 */
int Channel_Coerce_Data_Encode(
    uint8_t * apdu,
    unsigned max_apdu,
    BACNET_APPLICATION_DATA_VALUE * value,
    BACNET_APPLICATION_TAG tag)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    float float_value = 0.0;
    double double_value = 0.0;
    uint32_t unsigned_value = 0;
    int32_t signed_value = 0;
    bool boolean_value = false;

    (void)max_apdu;
    if (apdu && value) {
        switch (value->tag) {
#if defined (BACAPP_NULL)
            case BACNET_APPLICATION_TAG_NULL:
                if (tag == BACNET_APPLICATION_TAG_LIGHTING_COMMAND) {
                    apdu_len = BACNET_STATUS_ERROR;
                } else {
                    /* no coercion */
                    apdu[0] = value->tag;
                    apdu_len++;
                }
                break;
#endif
#if defined (BACAPP_BOOLEAN)
            case BACNET_APPLICATION_TAG_BOOLEAN:
                if (tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                    apdu_len =
                        encode_application_boolean(&apdu[0], value->type.Boolean);
                } else if (tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                    if (value->type.Boolean) {
                        unsigned_value = 1;
                    }
                    apdu_len = encode_application_unsigned(&apdu[0], unsigned_value);
                } else if (tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
                    if (value->type.Boolean) {
                        signed_value = 1;
                    }
                    apdu_len = encode_application_signed(&apdu[0], signed_value);
                } else if (tag == BACNET_APPLICATION_TAG_REAL) {
                    if (value->type.Boolean) {
                        float_value = 1;
                    }
                    apdu_len = encode_application_real(&apdu[0], float_value);
                } else if (tag == BACNET_APPLICATION_TAG_DOUBLE) {
                    if (value->type.Boolean) {
                        double_value = 1;
                    }
                    apdu_len = encode_application_double(&apdu[0], double_value);
                } else if (tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                    if (value->type.Boolean) {
                        unsigned_value = 1;
                    }
                    apdu_len =
                        encode_application_enumerated(&apdu[0], unsigned_value);
                } else {
                    apdu_len = BACNET_STATUS_ERROR;
                }
                break;
#endif
#if defined (BACAPP_UNSIGNED)
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                if (tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                    if (value->type.Unsigned_Int) {
                        boolean_value = true;
                    }
                    apdu_len =
                        encode_application_boolean(&apdu[0], boolean_value);
                } else if (tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                    unsigned_value = value->type.Unsigned_Int;
                    apdu_len = encode_application_unsigned(&apdu[0],
                        unsigned_value);
                } else if (tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
                    if (value->type.Unsigned_Int <= 2147483647) {
                        signed_value = value->type.Unsigned_Int;
                        apdu_len = encode_application_signed(&apdu[0],
                            signed_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_REAL) {
                    if (value->type.Unsigned_Int <= 9999999) {
                        float_value = (float)value->type.Unsigned_Int;
                        apdu_len = encode_application_real(&apdu[0],
                            float_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_DOUBLE) {
                    double_value = value->type.Unsigned_Int;
                    apdu_len = encode_application_double(&apdu[0],
                        double_value);
                } else if (tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                    unsigned_value = value->type.Unsigned_Int;
                    apdu_len = encode_application_enumerated(&apdu[0],
                        unsigned_value);
                } else {
                    apdu_len = BACNET_STATUS_ERROR;
                }
                break;
#endif
#if defined (BACAPP_SIGNED)
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                if (tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                    if (value->type.Signed_Int) {
                        boolean_value = true;
                    }
                    apdu_len =
                        encode_application_boolean(&apdu[0], boolean_value);
                } else if (tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                    if ((value->type.Signed_Int >= 0) &&
                        (value->type.Signed_Int <= 2147483647)) {
                        unsigned_value = value->type.Signed_Int;
                        apdu_len = encode_application_unsigned(&apdu[0],
                            unsigned_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
                    signed_value = value->type.Signed_Int;
                    apdu_len = encode_application_signed(&apdu[0],
                        signed_value);
                } else if (tag == BACNET_APPLICATION_TAG_REAL) {
                    if (value->type.Signed_Int <= 9999999) {
                        float_value = (float)value->type.Signed_Int;
                        apdu_len = encode_application_real(&apdu[0],
                            float_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_DOUBLE) {
                    double_value = value->type.Signed_Int;
                    apdu_len = encode_application_double(&apdu[0],
                        double_value);
                } else if (tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                    unsigned_value = value->type.Signed_Int;
                    apdu_len = encode_application_enumerated(&apdu[0],
                        unsigned_value);
                } else {
                    apdu_len = BACNET_STATUS_ERROR;
                }
                break;
#endif
#if defined (BACAPP_REAL)
            case BACNET_APPLICATION_TAG_REAL:
                if (tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                    if (value->type.Real) {
                        boolean_value = true;
                    }
                    apdu_len =
                        encode_application_boolean(&apdu[0], boolean_value);
                } else if (tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                    if ((value->type.Real >= 0.0) &&
                        (value->type.Real <= 2147483000.0)) {
                        unsigned_value = (uint32_t)value->type.Real;
                        apdu_len = encode_application_unsigned(&apdu[0],
                            unsigned_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
                    if ((value->type.Real >= -2147483000.0) &&
                        (value->type.Real <= 214783000.0)) {
                        signed_value = (int32_t)value->type.Real;
                        apdu_len = encode_application_signed(&apdu[0],
                            signed_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_REAL) {
                    float_value = value->type.Real;
                    apdu_len = encode_application_real(&apdu[0],
                        float_value);
                } else if (tag == BACNET_APPLICATION_TAG_DOUBLE) {
                    double_value = value->type.Real;
                    apdu_len = encode_application_double(&apdu[0],
                        double_value);
                } else if (tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                    if ((value->type.Real >= 0.0) &&
                        (value->type.Real <= 2147483000.0)) {
                        unsigned_value = (uint32_t)value->type.Real;
                        apdu_len = encode_application_enumerated(&apdu[0],
                            unsigned_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else {
                    apdu_len = BACNET_STATUS_ERROR;
                }
                break;
#endif
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                if (tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                    if (value->type.Double) {
                        boolean_value = true;
                    }
                    apdu_len =
                        encode_application_boolean(&apdu[0], boolean_value);
                } else if (tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                    if ((value->type.Double >= 0.0) &&
                        (value->type.Double <= 2147483000.0)) {
                        unsigned_value = (uint32_t)value->type.Double;
                        apdu_len = encode_application_unsigned(&apdu[0],
                            unsigned_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
                    if ((value->type.Double >= -2147483000.0) &&
                        (value->type.Double <= 214783000.0)) {
                        signed_value = (int32_t)value->type.Double;
                        apdu_len = encode_application_signed(&apdu[0],
                            signed_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_REAL) {
                    if ((value->type.Double >= 3.4E-38) &&
                        (value->type.Double <= 3.4E+38)) {
                        float_value = (float)value->type.Double;
                        apdu_len = encode_application_real(&apdu[0],
                            float_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_DOUBLE) {
                    double_value = value->type.Double;
                    apdu_len = encode_application_double(&apdu[0],
                        double_value);
                } else if (tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                    if ((value->type.Double >= 0.0) &&
                        (value->type.Double <= 2147483000.0)) {
                        unsigned_value = (uint32_t)value->type.Double;
                        apdu_len = encode_application_enumerated(&apdu[0],
                            unsigned_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else {
                    apdu_len = BACNET_STATUS_ERROR;
                }
                break;
#endif
#if defined (BACAPP_ENUMERATED)
            case BACNET_APPLICATION_TAG_ENUMERATED:
                if (tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                    if (value->type.Enumerated) {
                        boolean_value = true;
                    }
                    apdu_len =
                        encode_application_boolean(&apdu[0], boolean_value);
                } else if (tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                    unsigned_value = value->type.Enumerated;
                    apdu_len = encode_application_unsigned(&apdu[0],
                        unsigned_value);
                } else if (tag == BACNET_APPLICATION_TAG_SIGNED_INT) {
                    if (value->type.Enumerated <= 2147483647) {
                        signed_value = value->type.Enumerated;
                        apdu_len = encode_application_signed(&apdu[0],
                            signed_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_REAL) {
                    if (value->type.Enumerated <= 9999999) {
                        float_value = (float)value->type.Enumerated;
                        apdu_len = encode_application_real(&apdu[0],
                            float_value);
                    } else {
                        apdu_len = BACNET_STATUS_ERROR;
                    }
                } else if (tag == BACNET_APPLICATION_TAG_DOUBLE) {
                    double_value = value->type.Enumerated;
                    apdu_len = encode_application_double(&apdu[0],
                        double_value);
                } else if (tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                    unsigned_value = value->type.Enumerated;
                    apdu_len = encode_application_enumerated(&apdu[0],
                        unsigned_value);
                } else {
                    apdu_len = BACNET_STATUS_ERROR;
                }
                break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
        case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
            if (tag == BACNET_APPLICATION_TAG_LIGHTING_COMMAND) {
                apdu_len =
                    lighting_command_encode(&apdu[0],
                    &value->type.Lighting_Command);
            } else {
                apdu_len = BACNET_STATUS_ERROR;
            }
            break;
#endif
            default:
                apdu_len = BACNET_STATUS_ERROR;
                break;
        }
    }

    return apdu_len;
}

/**
 * For a given object instance-number, sets the present-value at a given
 * priority 1..16.
 *
 * @param  wp_data - all of the WriteProperty data structure
 *
 * @return  true if values are within range and present-value is sent.
 */
bool Channel_Write_Member_Value(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    bool status = false;
    int apdu_len = 0;

    if (wp_data && value) {
        if (((wp_data->object_type == OBJECT_ANALOG_INPUT) ||
            (wp_data->object_type == OBJECT_ANALOG_OUTPUT) ||
            (wp_data->object_type == OBJECT_ANALOG_VALUE)) &&
            (wp_data->object_property == PROP_PRESENT_VALUE) &&
            (wp_data->array_index == BACNET_ARRAY_ALL)) {
            apdu_len = Channel_Coerce_Data_Encode(
                wp_data->application_data,
                wp_data->application_data_len,
                value,
                BACNET_APPLICATION_TAG_REAL);
            if (apdu_len != BACNET_STATUS_ERROR) {
                wp_data->application_data_len = apdu_len;
                status = true;
            }
        } else if (((wp_data->object_type == OBJECT_BINARY_INPUT) ||
            (wp_data->object_type == OBJECT_BINARY_OUTPUT) ||
            (wp_data->object_type == OBJECT_BINARY_VALUE)) &&
            (wp_data->object_property == PROP_PRESENT_VALUE) &&
            (wp_data->array_index == BACNET_ARRAY_ALL)) {
            apdu_len = Channel_Coerce_Data_Encode(
                wp_data->application_data,
                wp_data->application_data_len,
                value,
                BACNET_APPLICATION_TAG_ENUMERATED);
            if (apdu_len != BACNET_STATUS_ERROR) {
                wp_data->application_data_len = apdu_len;
                status = true;
            }
        } else if (((wp_data->object_type == OBJECT_MULTI_STATE_INPUT) ||
            (wp_data->object_type == OBJECT_MULTI_STATE_OUTPUT) ||
            (wp_data->object_type == OBJECT_MULTI_STATE_VALUE)) &&
            (wp_data->object_property == PROP_PRESENT_VALUE) &&
            (wp_data->array_index == BACNET_ARRAY_ALL)) {
            apdu_len = Channel_Coerce_Data_Encode(
                wp_data->application_data,
                wp_data->application_data_len,
                value,
                BACNET_APPLICATION_TAG_UNSIGNED_INT);
            if (apdu_len != BACNET_STATUS_ERROR) {
                wp_data->application_data_len = apdu_len;
                status = true;
            }
        } else if (wp_data->object_type == OBJECT_LIGHTING_OUTPUT) {
            if ((wp_data->object_property == PROP_PRESENT_VALUE) &&
                (wp_data->array_index == BACNET_ARRAY_ALL)) {
                apdu_len = Channel_Coerce_Data_Encode(
                    wp_data->application_data,
                    wp_data->application_data_len,
                    value,
                    BACNET_APPLICATION_TAG_REAL);
                if (apdu_len != BACNET_STATUS_ERROR) {
                    wp_data->application_data_len = apdu_len;
                    status = true;
                }
            } else if ((wp_data->object_property == PROP_LIGHTING_COMMAND) &&
                (wp_data->array_index == BACNET_ARRAY_ALL)) {
                apdu_len = Channel_Coerce_Data_Encode(
                    wp_data->application_data,
                    wp_data->application_data_len,
                    value,
                    BACNET_APPLICATION_TAG_LIGHTING_COMMAND);
                if (apdu_len != BACNET_STATUS_ERROR) {
                    wp_data->application_data_len = apdu_len;
                    status = true;
                }
            }
        }
    }

    return status;
}

/**
 * For a given object instance-number, sets the present-value at a given
 * priority 1..16.
 *
 * @param  wp_data - all of the WriteProperty data structure
 *
 * @return  true if values are within range and present-value is sent.
 */
static bool Channel_Write_Members(
    struct bacnet_channel_object * pChannel,
    BACNET_APPLICATION_DATA_VALUE * value,
    uint8_t priority)
{
    BACNET_WRITE_PROPERTY_DATA wp_data = {0};
    bool status = false;
    unsigned m = 0;
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember = NULL;

    if (pChannel && value) {
        pChannel->Write_Status = BACNET_WRITE_STATUS_IN_PROGRESS;
        for (m = 0; m < CHANNEL_MEMBERS_MAX; m++) {
            pMember = &pChannel->Members[m];
            /* NOTE: our implementation is for internal objects only */
            /* NOTE: we could check to match our Device ID, but then
               we would need to update all channels when our device ID
               changed.  Instead, we'll just screen when members are
               set. */
            if ((pMember->deviceIndentifier.type == OBJECT_DEVICE) &&
                (pMember->deviceIndentifier.instance != BACNET_MAX_INSTANCE) &&
                (pMember->objectIdentifier.instance != BACNET_MAX_INSTANCE)) {
                wp_data.object_type = pMember->objectIdentifier.type;
                wp_data.object_instance = pMember->objectIdentifier.instance;
                wp_data.object_property = pMember->propertyIdentifier;
                wp_data.array_index = pMember->arrayIndex;
                wp_data.priority = priority;
                wp_data.application_data_len =
                    sizeof(wp_data.application_data);
                status = Channel_Write_Member_Value(&wp_data, value);
                if (status) {
                    status = Device_Write_Property(&wp_data);
                } else {
                    pChannel->Write_Status = BACNET_WRITE_STATUS_FAILED;
                }
            }
        }
        if (pChannel->Write_Status == BACNET_WRITE_STATUS_IN_PROGRESS) {
            pChannel->Write_Status = BACNET_WRITE_STATUS_SUCCESSFUL;
        }
    }

    return status;
}

/**
 * For a given object instance-number, sets the present-value at a given
 * priority 1..16.
 *
 * @param  wp_data - all of the WriteProperty data structure
 *
 * @return  true if values are within range and present-value is sent.
 */
bool Channel_Present_Value_Set(
    BACNET_WRITE_PROPERTY_DATA * wp_data,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    unsigned index = 0;
    bool status = false;

    index = Channel_Instance_To_Index(wp_data->object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        if ((wp_data->priority > 0) &&
            (wp_data->priority <= BACNET_MAX_PRIORITY)) {
            if (wp_data->priority != 6 /* reserved */ ) {
                status = Channel_Value_Copy(&Channel[index].Present_Value,
                    value);
                status = Channel_Write_Members(&Channel[index], value,
                    wp_data->priority);
                status = true;
            } else {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
        } else {
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        }
    }

    return status;
}

/**
 * For a given object instance-number, loads the object-name into
 * a characterstring. Note that the object name must be unique
 * within this device.
 *
 * @param  object_instance - object-instance number of the object
 * @param  object_name - holds the object-name retrieved
 *
 * @return  true if object-name was retrieved
 */
bool Channel_Object_Name(uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    char text_string[32] = "";
    bool status = false;
    unsigned index = 0;

    index = Channel_Instance_To_Index(object_instance);
    if (index < BACNET_CHANNELS_MAX) {
        sprintf(text_string, "CHANNEL %lu",
            (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/**
 * For a given object instance-number, returns the out-of-service
 * property value
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  out-of-service property value
 */
bool Channel_Out_Of_Service(uint32_t instance)
{
    unsigned int index = 0;
    bool value = false;

    index = Channel_Instance_To_Index(instance);
    if (index < BACNET_CHANNELS_MAX) {
        value = Channel[index].Out_Of_Service;
    }

    return value;
}

/**
 * For a given object instance-number, sets the out-of-service property value
 *
 * @param object_instance - object-instance number of the object
 * @param value - boolean out-of-service value
 *
 * @return true if the out-of-service property value was set
 */
void Channel_Out_Of_Service_Set(uint32_t instance,
    bool value)
{
    unsigned int index = 0;

    index = Channel_Instance_To_Index(instance);
    if (index < BACNET_CHANNELS_MAX) {
        Channel[index].Out_Of_Service = value;
    }
}

/**
 * ReadProperty handler for this object.  For the given ReadProperty
 * data, the application_data is loaded or the error flags are set.
 *
 * @param  rpdata - ReadProperty data, including requested data and
 * data for the reply, or error response.
 *
 * @return number of APDU bytes in the response, or
 * BACNET_STATUS_ERROR on error.
 */
int Channel_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_CHANNEL_VALUE * cvalue = NULL;
    uint32_t unsigned_value = 0;
    unsigned i = 0;
    unsigned count = 0;
    bool state = false;
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMember = NULL;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_CHANNEL,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            Channel_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_CHANNEL);
            break;
        case PROP_PRESENT_VALUE:
            cvalue = Channel_Present_Value(rpdata->object_instance);
            apdu_len = Channel_Value_Encode(&apdu[0], MAX_APDU, cvalue);
            if (apdu_len == BACNET_STATUS_ERROR) {
                apdu_len = encode_application_null(&apdu[0]);
            }
            break;
        case PROP_LAST_PRIORITY:
            unsigned_value = Channel_Last_Priority(rpdata->object_instance);
            apdu_len =
                encode_application_unsigned(&apdu[0], unsigned_value);
            break;
        case PROP_WRITE_STATUS:
            unsigned_value =
                (BACNET_WRITE_STATUS)Channel_Write_Status(
                rpdata->object_instance);
            apdu_len =
                encode_application_enumerated(&apdu[0], unsigned_value);
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            state = Channel_Out_Of_Service(rpdata->object_instance);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, state);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_OUT_OF_SERVICE:
            state = Channel_Out_Of_Service(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
            if (rpdata->array_index == 0) {
                /* Array element zero is the number of elements in the array */
                count = Channel_Reference_List_Member_Count(rpdata->object_instance);
                apdu_len = encode_application_unsigned(&apdu[0], count);
            } else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                /* if no index was specified, then try to encode the entire list */
                /* into one packet. */
                count = Channel_Reference_List_Member_Count(rpdata->object_instance);
                for (i = 1; i <= count; i++) {
                    pMember = Channel_Reference_List_Member_Element(
                        rpdata->object_instance, i);
                    len =
                        bacapp_encode_device_obj_property_ref(&apdu[apdu_len],
                        pMember);
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU) {
                        apdu_len += len;
                    } else {
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                /* a specific element was requested */
                count = Channel_Reference_List_Member_Count(rpdata->object_instance);
                if (rpdata->array_index <= count) {
                    pMember = Channel_Reference_List_Member_Element(
                        rpdata->object_instance, rpdata->array_index);
                    apdu_len +=
                        bacapp_encode_device_obj_property_ref(&apdu[0],
                        pMember);
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_CHANNEL_NUMBER:
            unsigned_value = Channel_Number(rpdata->object_instance);
            apdu_len =
                encode_application_unsigned(&apdu[0], unsigned_value);
            break;
        case PROP_CONTROL_GROUPS:
            if (rpdata->array_index == 0) {
                /* Array element zero is the number of elements in the array */
                apdu_len = encode_application_unsigned(&apdu[0],
                    CONTROL_GROUPS_MAX);
            } else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                /* if no index was specified, then try to encode the entire list */
                /* into one packet. */
                for (i = 1; i <= CONTROL_GROUPS_MAX; i++) {
                    unsigned_value = Channel_Control_Groups_Element(
                        rpdata->object_instance, i);
                    len =
                        encode_application_unsigned(&apdu[apdu_len],
                        unsigned_value);
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU) {
                        apdu_len += len;
                    } else {
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                /* a specific element was requested */
                if (rpdata->array_index <= CONTROL_GROUPS_MAX) {
                    unsigned_value = Channel_Control_Groups_Element(
                        rpdata->object_instance, rpdata->array_index);
                    apdu_len =
                        encode_application_unsigned(&apdu[apdu_len],
                        unsigned_value);
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0)
        && (rpdata->object_property != PROP_PRIORITY_ARRAY)
        && (rpdata->object_property != PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES)
        && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/**
 * WriteProperty handler for this object.  For the given WriteProperty
 * data, the application_data is loaded or the error flags are set.
 *
 * @param  wp_data - BACNET_WRITE_PROPERTY_DATA data, including
 * requested data and space for the reply, or error response.
 *
 * @return false if an error is loaded, true if no errors
 */
bool Channel_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;
    int element_len = 0;
    uint32_t count = 0;
    uint32_t array_index = 0;

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
            status = Channel_Present_Value_Set(wp_data, &value);
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Channel_Out_Of_Service_Set(wp_data->object_instance,
                    value.type.Boolean);
            }
            break;
        case PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES:
// FIXME: add property handling
//            status = Channel_List_Of_Object_Property_References_Set(
//                wp_data,
//                &value);
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED;
            break;
        case PROP_CHANNEL_NUMBER:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Channel_Number_Set(wp_data->object_instance,
                    value.type.Unsigned_Int);
            }
            break;
        case PROP_CONTROL_GROUPS:
            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                if (wp_data->array_index == 0) {
                    /* Array element zero is the number of elements in the array */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else if (wp_data->array_index == BACNET_ARRAY_ALL) {
                    count = CONTROL_GROUPS_MAX;
                    array_index = 1;
                    /* extra elements still encoded in application data */
                    element_len = len;
                    do {
                        if ((element_len > 0) &&
                            (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT)) {
                            if ((wp_data->array_index <= CONTROL_GROUPS_MAX) &&
                                (value.type.Unsigned_Int <= 65535)) {
                                status = Channel_Control_Groups_Element_Set(
                                    wp_data->object_instance,
                                    wp_data->array_index,
                                    value.type.Unsigned_Int);
                            }
                            if (!status) {
                                wp_data->error_class = ERROR_CLASS_PROPERTY;
                                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                                break;
                            }
                        }
                        count--;
                        array_index++;
                        if (count) {
                            element_len = bacapp_decode_application_data(
                                &wp_data->application_data[len],
                                wp_data->application_data_len-len,
                                &value);
                            if (element_len < 0) {
                                wp_data->error_class = ERROR_CLASS_PROPERTY;
                                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                                break;
                            }
                            len += element_len;
                        }
                    } while (count);
                } else {
                    if ((wp_data->array_index <= CONTROL_GROUPS_MAX) &&
                        (value.type.Unsigned_Int <= 65535)) {
                        status = Channel_Control_Groups_Element_Set(
                            wp_data->object_instance,
                            wp_data->array_index,
                            value.type.Unsigned_Int);
                    }
                    if (!status) {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_LAST_PRIORITY:
        case PROP_WRITE_STATUS:
        case PROP_STATUS_FLAGS:
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

/**
 * Initializes the Channel object data
 */
void Channel_Init(void)
{
    unsigned i, m, g;

    for (i = 0; i < BACNET_CHANNELS_MAX; i++) {
        Channel[i].Present_Value.tag = BACNET_APPLICATION_TAG_EMPTYLIST;
        Channel[i].Out_Of_Service = false;
        Channel[i].Last_Priority = BACNET_NO_PRIORITY;
        Channel[i].Write_Status = BACNET_WRITE_STATUS_IDLE;
        for (m = 0; m < CHANNEL_MEMBERS_MAX; m++) {
            Channel[i].Members[m].objectIdentifier.type =
                OBJECT_LIGHTING_OUTPUT;
            Channel[i].Members[m].objectIdentifier.instance = i+1;
            Channel[i].Members[m].propertyIdentifier = PROP_LIGHTING_COMMAND;
            Channel[i].Members[m].arrayIndex = BACNET_ARRAY_ALL;
            Channel[i].Members[m].deviceIndentifier.type =
                OBJECT_DEVICE;
            Channel[i].Members[m].deviceIndentifier.instance = 0;
        }
        Channel[i].Number = 0;
        for (g = 0; g < CONTROL_GROUPS_MAX; g++) {
            Channel[i].Control_Groups[g] = 0;
        }
    }

    return;
}
