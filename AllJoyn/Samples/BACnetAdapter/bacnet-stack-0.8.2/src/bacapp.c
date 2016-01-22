/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacint.h"
#include "bacreal.h"
#include "bacdef.h"
#include "bacapp.h"
#include "bactext.h"
#include "datetime.h"
#include "bacstr.h"
#include "lighting.h"

/** @file bacapp.c  Utilities for the BACnet_Application_Data_Value */

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

int bacapp_encode_application_data(
    uint8_t * apdu,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (value && apdu) {
        switch (value->tag) {
#if defined (BACAPP_NULL)
            case BACNET_APPLICATION_TAG_NULL:
                apdu[0] = value->tag;
                apdu_len++;
                break;
#endif
#if defined (BACAPP_BOOLEAN)
            case BACNET_APPLICATION_TAG_BOOLEAN:
                apdu_len =
                    encode_application_boolean(&apdu[0], value->type.Boolean);
                break;
#endif
#if defined (BACAPP_UNSIGNED)
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                apdu_len =
                    encode_application_unsigned(&apdu[0],
                    value->type.Unsigned_Int);
                break;
#endif
#if defined (BACAPP_SIGNED)
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                apdu_len =
                    encode_application_signed(&apdu[0],
                    value->type.Signed_Int);
                break;
#endif
#if defined (BACAPP_REAL)
            case BACNET_APPLICATION_TAG_REAL:
                apdu_len = encode_application_real(&apdu[0], value->type.Real);
                break;
#endif
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                apdu_len =
                    encode_application_double(&apdu[0], value->type.Double);
                break;
#endif
#if defined (BACAPP_OCTET_STRING)
            case BACNET_APPLICATION_TAG_OCTET_STRING:
                apdu_len =
                    encode_application_octet_string(&apdu[0],
                    &value->type.Octet_String);
                break;
#endif
#if defined (BACAPP_CHARACTER_STRING)
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                apdu_len =
                    encode_application_character_string(&apdu[0],
                    &value->type.Character_String);
                break;
#endif
#if defined (BACAPP_BIT_STRING)
            case BACNET_APPLICATION_TAG_BIT_STRING:
                apdu_len =
                    encode_application_bitstring(&apdu[0],
                    &value->type.Bit_String);
                break;
#endif
#if defined (BACAPP_ENUMERATED)
            case BACNET_APPLICATION_TAG_ENUMERATED:
                apdu_len =
                    encode_application_enumerated(&apdu[0],
                    value->type.Enumerated);
                break;
#endif
#if defined (BACAPP_DATE)
            case BACNET_APPLICATION_TAG_DATE:
                apdu_len =
                    encode_application_date(&apdu[0], &value->type.Date);
                break;
#endif
#if defined (BACAPP_TIME)
            case BACNET_APPLICATION_TAG_TIME:
                apdu_len =
                    encode_application_time(&apdu[0], &value->type.Time);
                break;
#endif
#if defined (BACAPP_OBJECT_ID)
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                apdu_len =
                    encode_application_object_id(&apdu[0],
                    (int) value->type.Object_Id.type,
                    value->type.Object_Id.instance);
                break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                apdu_len =
                    lighting_command_encode(&apdu[0],
                    &value->type.Lighting_Command);
                break;
#endif
#if defined (BACAPP_DEVICE_OBJECT_PROP_REF)
            case BACNET_APPLICATION_TAG_DEVICE_OBJECT_PROPERTY_REFERENCE:
                /* BACnetDeviceObjectPropertyReference */
                apdu_len =
                    bacapp_encode_device_obj_property_ref(&apdu[0],
                    &value->type.Device_Object_Property_Reference);
                break;
#endif
            default:
                break;
        }
    }

    return apdu_len;
}

/* decode the data and store it into value.
   Return the number of octets consumed. */
int bacapp_decode_data(
    uint8_t * apdu,
    uint8_t tag_data_type,
    uint32_t len_value_type,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    int len = 0;

    if (apdu && value) {
        switch (tag_data_type) {
#if defined (BACAPP_NULL)
            case BACNET_APPLICATION_TAG_NULL:
                /* nothing else to do */
                break;
#endif
#if defined (BACAPP_BOOLEAN)
            case BACNET_APPLICATION_TAG_BOOLEAN:
                value->type.Boolean = decode_boolean(len_value_type);
                break;
#endif
#if defined (BACAPP_UNSIGNED)
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                len =
                    decode_unsigned(&apdu[0], len_value_type,
                    &value->type.Unsigned_Int);
                break;
#endif
#if defined (BACAPP_SIGNED)
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                len =
                    decode_signed(&apdu[0], len_value_type,
                    &value->type.Signed_Int);
                break;
#endif
#if defined (BACAPP_REAL)
            case BACNET_APPLICATION_TAG_REAL:
                len =
                    decode_real_safe(&apdu[0], len_value_type,
                    &(value->type.Real));
                break;
#endif
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                len =
                    decode_double_safe(&apdu[0], len_value_type,
                    &(value->type.Double));
                break;
#endif
#if defined (BACAPP_OCTET_STRING)
            case BACNET_APPLICATION_TAG_OCTET_STRING:
                len =
                    decode_octet_string(&apdu[0], len_value_type,
                    &value->type.Octet_String);
                break;
#endif
#if defined (BACAPP_CHARACTER_STRING)
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                len =
                    decode_character_string(&apdu[0], len_value_type,
                    &value->type.Character_String);
                break;
#endif
#if defined (BACAPP_BIT_STRING)
            case BACNET_APPLICATION_TAG_BIT_STRING:
                len =
                    decode_bitstring(&apdu[0], len_value_type,
                    &value->type.Bit_String);
                break;
#endif
#if defined (BACAPP_ENUMERATED)
            case BACNET_APPLICATION_TAG_ENUMERATED:
                len =
                    decode_enumerated(&apdu[0], len_value_type,
                    &value->type.Enumerated);
                break;
#endif
#if defined (BACAPP_DATE)
            case BACNET_APPLICATION_TAG_DATE:
                len =
                    decode_date_safe(&apdu[0], len_value_type,
                    &value->type.Date);
                break;
#endif
#if defined (BACAPP_TIME)
            case BACNET_APPLICATION_TAG_TIME:
                len =
                    decode_bacnet_time_safe(&apdu[0], len_value_type,
                    &value->type.Time);
                break;
#endif
#if defined (BACAPP_OBJECT_ID)
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                {
                    uint16_t object_type = 0;
                    uint32_t instance = 0;
                    len =
                        decode_object_id_safe(&apdu[0], len_value_type,
                        &object_type, &instance);
                    value->type.Object_Id.type = object_type;
                    value->type.Object_Id.instance = instance;
                }
                break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                len =
                    lighting_command_decode(
                        &apdu[0], len_value_type,
                        &value->type.Lighting_Command);
                break;
#endif
            default:
                break;
        }
    }

    if ((len == 0) && (tag_data_type != BACNET_APPLICATION_TAG_NULL) &&
        (tag_data_type != BACNET_APPLICATION_TAG_BOOLEAN) &&
        (tag_data_type != BACNET_APPLICATION_TAG_OCTET_STRING)) {
        /* indicate that we were not able to decode the value */
        value->tag = MAX_BACNET_APPLICATION_TAG;
    }
    return len;
}

int bacapp_decode_application_data(
    uint8_t * apdu,
    unsigned max_apdu_len,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    int len = 0;
    int tag_len = 0;
    int decode_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;

    /* FIXME: use max_apdu_len! */
    max_apdu_len = max_apdu_len;
    if (apdu && value && !IS_CONTEXT_SPECIFIC(*apdu)) {
        value->context_specific = false;
        tag_len =
            decode_tag_number_and_value(&apdu[0], &tag_number,
            &len_value_type);
        if (tag_len) {
            len += tag_len;
            value->tag = tag_number;
            decode_len =
                bacapp_decode_data(&apdu[len], tag_number, len_value_type,
                value);
            if (value->tag != MAX_BACNET_APPLICATION_TAG) {
                len += decode_len;
            } else {
                len = BACNET_STATUS_ERROR;
            }
        }
        value->next = NULL;
    }

    return len;
}

/*
** Usage: Similar to strtok. Call function the first time with new_apdu and new_adu_len set to apdu buffer
** to be processed. Subsequent calls should pass in NULL.
**
** Returns true if a application message is correctly parsed.
** Returns false if no more application messages are available.
**
** This function is NOT thread safe.
**
** Notes: The _safe suffix is there because the function should be relatively safe against buffer overruns.
**
*/

bool bacapp_decode_application_data_safe(
    uint8_t * new_apdu,
    uint32_t new_apdu_len,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    /* The static variables that store the apdu buffer between function calls */
    static uint8_t *apdu = NULL;
    static uint32_t apdu_len_remaining = 0;
    static uint32_t apdu_len = 0;
    int len = 0;
    int tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;

    bool ret = false;

    if (new_apdu != NULL) {
        apdu = new_apdu;
        apdu_len_remaining = new_apdu_len;
        apdu_len = 0;
    }

    if (value && apdu_len_remaining > 0 &&
        !IS_CONTEXT_SPECIFIC(apdu[apdu_len])) {
        value->context_specific = false;
        tag_len =
            decode_tag_number_and_value_safe(&apdu[apdu_len],
            apdu_len_remaining, &tag_number, &len_value_type);
        /* If tag_len is zero, then the tag information is truncated */
        if (tag_len) {
            apdu_len += tag_len;
            apdu_len_remaining -= tag_len;
            /* The tag is boolean then len_value_type is interpreted as value, not length, so dont bother
             ** checking with apdu_len_remaining */
            if (tag_number == BACNET_APPLICATION_TAG_BOOLEAN ||
                len_value_type <= apdu_len_remaining) {
                value->tag = tag_number;
                len =
                    bacapp_decode_data(&apdu[apdu_len], tag_number,
                    len_value_type, value);
                apdu_len += len;
                apdu_len_remaining -= len;

                ret = true;
            }
        }
        value->next = NULL;
    }


    return ret;
}

/* Decode the data and
   return the number of octets consumed. */
int bacapp_decode_data_len(
    uint8_t * apdu,
    uint8_t tag_data_type,
    uint32_t len_value_type)
{
    int len = 0;

    if (apdu) {
        switch (tag_data_type) {
            case BACNET_APPLICATION_TAG_NULL:
                break;
            case BACNET_APPLICATION_TAG_BOOLEAN:
                break;
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            case BACNET_APPLICATION_TAG_SIGNED_INT:
            case BACNET_APPLICATION_TAG_REAL:
            case BACNET_APPLICATION_TAG_DOUBLE:
            case BACNET_APPLICATION_TAG_OCTET_STRING:
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
            case BACNET_APPLICATION_TAG_BIT_STRING:
            case BACNET_APPLICATION_TAG_ENUMERATED:
            case BACNET_APPLICATION_TAG_DATE:
            case BACNET_APPLICATION_TAG_TIME:
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                len = (int) len_value_type;
                break;
            default:
                break;
        }
    }

    return len;
}

int bacapp_decode_application_data_len(
    uint8_t * apdu,
    unsigned max_apdu_len)
{
    int len = 0;
    int tag_len = 0;
    int decode_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;

    /* FIXME: use max_apdu_len! */
    max_apdu_len = max_apdu_len;
    if (apdu && !IS_CONTEXT_SPECIFIC(*apdu)) {
        tag_len =
            decode_tag_number_and_value(&apdu[0], &tag_number,
            &len_value_type);
        if (tag_len) {
            len += tag_len;
            decode_len =
                bacapp_decode_data_len(&apdu[len], tag_number, len_value_type);
            len += decode_len;
        }
    }

    return len;
}

int bacapp_encode_context_data_value(
    uint8_t * apdu,
    uint8_t context_tag_number,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (value && apdu) {
        switch (value->tag) {
#if defined (BACAPP_NULL)
            case BACNET_APPLICATION_TAG_NULL:
                apdu_len = encode_context_null(&apdu[0], context_tag_number);
                break;
#endif
#if defined (BACAPP_BOOLEAN)
            case BACNET_APPLICATION_TAG_BOOLEAN:
                apdu_len =
                    encode_context_boolean(&apdu[0], context_tag_number,
                    value->type.Boolean);
                break;
#endif
#if defined (BACAPP_UNSIGNED)
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                apdu_len =
                    encode_context_unsigned(&apdu[0], context_tag_number,
                    value->type.Unsigned_Int);
                break;
#endif
#if defined (BACAPP_SIGNED)
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                apdu_len =
                    encode_context_signed(&apdu[0], context_tag_number,
                    value->type.Signed_Int);
                break;
#endif
#if defined (BACAPP_REAL)
            case BACNET_APPLICATION_TAG_REAL:
                apdu_len =
                    encode_context_real(&apdu[0], context_tag_number,
                    value->type.Real);
                break;
#endif
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                apdu_len =
                    encode_context_double(&apdu[0], context_tag_number,
                    value->type.Double);
                break;
#endif
#if defined (BACAPP_OCTET_STRING)
            case BACNET_APPLICATION_TAG_OCTET_STRING:
                apdu_len =
                    encode_context_octet_string(&apdu[0], context_tag_number,
                    &value->type.Octet_String);
                break;
#endif
#if defined (BACAPP_CHARACTER_STRING)
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                apdu_len =
                    encode_context_character_string(&apdu[0],
                    context_tag_number, &value->type.Character_String);
                break;
#endif
#if defined (BACAPP_BIT_STRING)
            case BACNET_APPLICATION_TAG_BIT_STRING:
                apdu_len =
                    encode_context_bitstring(&apdu[0], context_tag_number,
                    &value->type.Bit_String);
                break;
#endif
#if defined (BACAPP_ENUMERATED)
            case BACNET_APPLICATION_TAG_ENUMERATED:
                apdu_len =
                    encode_context_enumerated(&apdu[0], context_tag_number,
                    value->type.Enumerated);
                break;
#endif
#if defined (BACAPP_DATE)
            case BACNET_APPLICATION_TAG_DATE:
                apdu_len =
                    encode_context_date(&apdu[0], context_tag_number,
                    &value->type.Date);
                break;
#endif
#if defined (BACAPP_TIME)
            case BACNET_APPLICATION_TAG_TIME:
                apdu_len =
                    encode_context_time(&apdu[0], context_tag_number,
                    &value->type.Time);
                break;
#endif
#if defined (BACAPP_OBJECT_ID)
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                apdu_len =
                    encode_context_object_id(&apdu[0], context_tag_number,
                    (int) value->type.Object_Id.type,
                    value->type.Object_Id.instance);
                break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                apdu_len =
                    lighting_command_encode_context(
                    &apdu[0], context_tag_number,
                    &value->type.Lighting_Command);
                break;
#endif
            default:
                break;
        }
    }

    return apdu_len;
}

/* returns the fixed tag type for certain context tagged properties */
BACNET_APPLICATION_TAG bacapp_context_tag_type(
    BACNET_PROPERTY_ID property,
    uint8_t tag_number)
{
    BACNET_APPLICATION_TAG tag = MAX_BACNET_APPLICATION_TAG;

    switch (property) {
        case PROP_ACTUAL_SHED_LEVEL:
        case PROP_REQUESTED_SHED_LEVEL:
        case PROP_EXPECTED_SHED_LEVEL:
            switch (tag_number) {
                case 0:
                case 1:
                    tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
                    break;
                case 2:
                    tag = BACNET_APPLICATION_TAG_REAL;
                    break;
                default:
                    break;
            }
            break;
        case PROP_ACTION:
            switch (tag_number) {
                case 0:
                case 1:
                    tag = BACNET_APPLICATION_TAG_OBJECT_ID;
                    break;
                case 2:
                    tag = BACNET_APPLICATION_TAG_ENUMERATED;
                    break;
                case 3:
                case 5:
                case 6:
                    tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
                    break;
                case 7:
                case 8:
                    tag = BACNET_APPLICATION_TAG_BOOLEAN;
                    break;
                case 4:        /* propertyValue: abstract syntax */
                default:
                    break;
            }
            break;
        case PROP_LIST_OF_GROUP_MEMBERS:
            /* Sequence of ReadAccessSpecification */
            switch (tag_number) {
                case 0:
                    tag = BACNET_APPLICATION_TAG_OBJECT_ID;
                    break;
                default:
                    break;
            }
            break;
        case PROP_EXCEPTION_SCHEDULE:
            switch (tag_number) {
                case 1:
                    tag = BACNET_APPLICATION_TAG_OBJECT_ID;
                    break;
                case 3:
                    tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
                    break;
                case 0:        /* calendarEntry: abstract syntax + context */
                case 2:        /* list of BACnetTimeValue: abstract syntax */
                default:
                    break;
            }
            break;
        case PROP_LOG_DEVICE_OBJECT_PROPERTY:
            switch (tag_number) {
                case 0:        /* Object ID */
                case 3:        /* Device ID */
                    tag = BACNET_APPLICATION_TAG_OBJECT_ID;
                    break;
                case 1:        /* Property ID */
                    tag = BACNET_APPLICATION_TAG_ENUMERATED;
                    break;
                case 2:        /* Array index */
                    tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
                    break;
                default:
                    break;
            }
            break;
        case PROP_SUBORDINATE_LIST:
            /* BACnetARRAY[N] of BACnetDeviceObjectReference */
            switch (tag_number) {
                case 0:        /* Optional Device ID */
                case 1:        /* Object ID */
                    tag = BACNET_APPLICATION_TAG_OBJECT_ID;
                    break;
                default:
                    break;
            }
            break;

        case PROP_RECIPIENT_LIST:
            /* List of BACnetDestination */
            switch (tag_number) {
                case 0:        /* Device Object ID */
                    tag = BACNET_APPLICATION_TAG_OBJECT_ID;
                    break;
                case 1:
                    /* 2015.08.22 EKH 135-2012 pg 708
                    todo - Context 1 in Recipient list would be a BACnetAddress, not coded yet...
                    BACnetRecipient::= CHOICE {
                         device  [0] BACnetObjectIdentifier,
                         address  [1] BACnetAddress
                          }
                          */
                    break;
                default:
                    break;
            }
            break;
        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
            /* BACnetCOVSubscription */
            switch (tag_number) {
                case 0:        /* BACnetRecipientProcess */
                case 1:        /* BACnetObjectPropertyReference */
                    break;
                case 2:        /* issueConfirmedNotifications */
                    tag = BACNET_APPLICATION_TAG_BOOLEAN;
                    break;
                case 3:        /* timeRemaining */
                    tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
                    break;
                case 4:        /* covIncrement */
                    tag = BACNET_APPLICATION_TAG_REAL;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return tag;
}

int bacapp_encode_context_data(
    uint8_t * apdu,
    BACNET_APPLICATION_DATA_VALUE * value,
    BACNET_PROPERTY_ID property)
{
    int apdu_len = 0;
    BACNET_APPLICATION_TAG tag_data_type;

    if (value && apdu) {
        tag_data_type = bacapp_context_tag_type(property, value->context_tag);
        if (tag_data_type < MAX_BACNET_APPLICATION_TAG) {
            apdu_len =
                bacapp_encode_context_data_value(&apdu[0], value->context_tag,
                value);
        } else {
            /* FIXME: what now? */
            apdu_len = 0;
        }
        value->next = NULL;
    }

    return apdu_len;
}

int bacapp_decode_context_data(
    uint8_t * apdu,
    unsigned max_apdu_len,
    BACNET_APPLICATION_DATA_VALUE * value,
    BACNET_PROPERTY_ID property)
{
    int apdu_len = 0, len = 0;
    int tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;

    if (apdu && value && IS_CONTEXT_SPECIFIC(*apdu)) {
        value->context_specific = true;
        value->next = NULL;
        tag_len =
            decode_tag_number_and_value(&apdu[0], &tag_number,
            &len_value_type);
        apdu_len = tag_len;
        /* Empty construct : (closing tag) => returns NULL value */
        if (tag_len && ((unsigned) tag_len <= max_apdu_len) &&
            !decode_is_closing_tag_number(&apdu[0], tag_number)) {
            value->context_tag = tag_number;
            value->tag = bacapp_context_tag_type(property, tag_number);
            if (value->tag < MAX_BACNET_APPLICATION_TAG) {
                len =
                    bacapp_decode_data(&apdu[apdu_len], value->tag,
                    len_value_type, value);
                apdu_len += len;
            } else if (len_value_type) {
                /* Unknown value : non null size (elementary type) */
                apdu_len += len_value_type;
                /* SHOULD NOT HAPPEN, EXCEPTED WHEN READING UNKNOWN CONTEXTUAL PROPERTY */
            } else {
                apdu_len = BACNET_STATUS_ERROR;
            }
        } else if (tag_len == 1)        /* and is a Closing tag */
            apdu_len = 0;       /* Don't advance over that closing tag. */
    }

    return apdu_len;
}

int bacapp_decode_context_data_len(
    uint8_t * apdu,
    unsigned max_apdu_len,
    BACNET_PROPERTY_ID property)
{
    int apdu_len = 0, len = 0;
    int tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint8_t tag = 0;

    /* FIXME: use max_apdu_len! */
    max_apdu_len = max_apdu_len;
    if (apdu && IS_CONTEXT_SPECIFIC(*apdu)) {
        tag_len =
            decode_tag_number_and_value(&apdu[0], &tag_number,
            &len_value_type);
        if (tag_len) {
            apdu_len = tag_len;
            tag = bacapp_context_tag_type(property, tag_number);
            if (tag < MAX_BACNET_APPLICATION_TAG) {
                len =
                    bacapp_decode_data_len(&apdu[apdu_len], tag,
                    len_value_type);
                apdu_len += len;
            } else {
                apdu_len += len_value_type;
            }
        }
    }

    return apdu_len;
}

int bacapp_encode_data(
    uint8_t * apdu,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (value && apdu) {
        if (value->context_specific) {
            apdu_len =
                bacapp_encode_context_data_value(&apdu[0], value->context_tag,
                value);
        } else {
            apdu_len = bacapp_encode_application_data(&apdu[0], value);
        }
    }

    return apdu_len;
}


bool bacapp_copy(
    BACNET_APPLICATION_DATA_VALUE * dest_value,
    BACNET_APPLICATION_DATA_VALUE * src_value)
{
    bool status = true; /*return value */

    if (dest_value && src_value) {
        dest_value->tag = src_value->tag;
        switch (src_value->tag) {
#if defined (BACAPP_NULL)
            case BACNET_APPLICATION_TAG_NULL:
                break;
#endif
#if defined (BACAPP_BOOLEAN)
            case BACNET_APPLICATION_TAG_BOOLEAN:
                dest_value->type.Boolean = src_value->type.Boolean;
                break;
#endif
#if defined (BACAPP_UNSIGNED)
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                dest_value->type.Unsigned_Int = src_value->type.Unsigned_Int;
                break;
#endif
#if defined (BACAPP_SIGNED)
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                dest_value->type.Signed_Int = src_value->type.Signed_Int;
                break;
#endif
#if defined (BACAPP_REAL)
            case BACNET_APPLICATION_TAG_REAL:
                dest_value->type.Real = src_value->type.Real;
                break;
#endif
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                dest_value->type.Double = src_value->type.Double;
                break;
#endif
#if defined (BACAPP_OCTET_STRING)
            case BACNET_APPLICATION_TAG_OCTET_STRING:
                octetstring_copy(&dest_value->type.Octet_String,
                    &src_value->type.Octet_String);
                break;
#endif
#if defined (BACAPP_CHARACTER_STRING)
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                characterstring_copy(&dest_value->type.Character_String,
                    &src_value->type.Character_String);
                break;
#endif
#if defined (BACAPP_BIT_STRING)
            case BACNET_APPLICATION_TAG_BIT_STRING:
                bitstring_copy(&dest_value->type.Bit_String,
                    &src_value->type.Bit_String);
                break;
#endif
#if defined (BACAPP_ENUMERATED)
            case BACNET_APPLICATION_TAG_ENUMERATED:
                dest_value->type.Enumerated = src_value->type.Enumerated;
                break;
#endif
#if defined (BACAPP_DATE)
            case BACNET_APPLICATION_TAG_DATE:
                datetime_copy_date(&dest_value->type.Date,
                    &src_value->type.Date);
                break;
#endif
#if defined (BACAPP_TIME)
            case BACNET_APPLICATION_TAG_TIME:
                datetime_copy_time(&dest_value->type.Time,
                    &src_value->type.Time);
                break;
#endif
#if defined (BACAPP_OBJECT_ID)
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                dest_value->type.Object_Id.type =
                    src_value->type.Object_Id.type;
                dest_value->type.Object_Id.instance =
                    src_value->type.Object_Id.instance;
                break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                status = lighting_command_copy(
                    &dest_value->type.Lighting_Command,
                    &src_value->type.Lighting_Command);
                break;
#endif
            default:
                status = false;
                break;
        }
        dest_value->next = src_value->next;
    }

    return status;
}

/* returns the length of data between an opening tag and a closing tag.
   Expects that the first octet contain the opening tag.
   Include a value property identifier for context specific data
   such as the value received in a WriteProperty request */
int bacapp_data_len(
    uint8_t * apdu,
    unsigned max_apdu_len,
    BACNET_PROPERTY_ID property)
{
    int len = 0;
    int total_len = 0;
    int apdu_len = 0;
    uint8_t tag_number = 0;
    uint8_t opening_tag_number = 0;
    uint8_t opening_tag_number_counter = 0;
    uint32_t value = 0;

    if (IS_OPENING_TAG(apdu[0])) {
        len =
            decode_tag_number_and_value(&apdu[apdu_len], &tag_number, &value);
        apdu_len += len;
        opening_tag_number = tag_number;
        opening_tag_number_counter = 1;
        while (opening_tag_number_counter) {
            if (IS_OPENING_TAG(apdu[apdu_len])) {
                len =
                    decode_tag_number_and_value(&apdu[apdu_len], &tag_number,
                    &value);
                if (tag_number == opening_tag_number)
                    opening_tag_number_counter++;
            } else if (IS_CLOSING_TAG(apdu[apdu_len])) {
                len =
                    decode_tag_number_and_value(&apdu[apdu_len], &tag_number,
                    &value);
                if (tag_number == opening_tag_number)
                    opening_tag_number_counter--;
            } else if (IS_CONTEXT_SPECIFIC(apdu[apdu_len])) {
                /* context-specific tagged data */
                len =
                    bacapp_decode_context_data_len(&apdu[apdu_len],
                    max_apdu_len - apdu_len, property);
            } else {
                /* application tagged data */
                len =
                    bacapp_decode_application_data_len(&apdu[apdu_len],
                    max_apdu_len - apdu_len);
            }
            apdu_len += len;
            if (opening_tag_number_counter) {
                if (len > 0) {
                    total_len += len;
                } else {
                    /* error: len is not incrementing */
                    total_len = BACNET_STATUS_ERROR;
                    break;
                }
            }
            if ((unsigned) apdu_len > max_apdu_len) {
                /* error: exceeding our buffer limit */
                total_len = BACNET_STATUS_ERROR;
                break;
            }
        }
    }

    return total_len;
}

#ifdef BACAPP_SNPRINTF_ENABLED
static bool append_str(
    char **str,
    size_t * rem_str_len,
    const char *add_str)
{
    bool retval;
    uint16_t bytes_written;

    bytes_written = snprintf(*str, *rem_str_len, "%s", add_str);
    if ((bytes_written < 0) || (bytes_written >= *rem_str_len)) {
        /* If there was an error or output was truncated, return error */
        retval = false;
    } else {
        /* Successfully wrote the contents to the string. Let's advance the
         * string pointer to the end, and account for the used space */
        *str += bytes_written;
        *rem_str_len -= bytes_written;
        retval = true;
    }

    return retval;
}

/* Extract the value into a string
 *  Inputs:  str - the buffer to store the extracted value.
 *           str_len - the size of the buffer
 *           object_value - ptr to BACnet object value from which to extract str
 *  Return:  number of bytes (excluding terminating NULL byte) that were stored
 *           to the output string. If output was truncated due to string size,
 *           then the returned value is greater than str_len (a la snprintf() ).
 */
int bacapp_snprintf_value(
    char *str,
    size_t str_len,
    BACNET_OBJECT_PROPERTY_VALUE * object_value)
{
    size_t len = 0, i = 0;
    char *char_str;
    uint8_t *octet_str;
    BACNET_APPLICATION_DATA_VALUE *value;
    BACNET_PROPERTY_ID property = PROP_ALL;
    BACNET_OBJECT_TYPE object_type = MAX_BACNET_OBJECT_TYPE;
    int ret_val = -1;
    char *p_str = str;
    size_t rem_str_len = str_len;
    char temp_str[32];

    if (object_value && object_value->value) {
        value = object_value->value;
        property = object_value->object_property;
        object_type = object_value->object_type;
        switch (value->tag) {
            case BACNET_APPLICATION_TAG_NULL:
                ret_val = snprintf(str, str_len, "Null");
                break;
            case BACNET_APPLICATION_TAG_BOOLEAN:
                ret_val =
                    (value->type.Boolean) ? snprintf(str, str_len,
                    "TRUE") : snprintf(str, str_len, "FALSE");
                break;
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                ret_val =
                    snprintf(str, str_len, "%lu",
                    (unsigned long) value->type.Unsigned_Int);
                break;
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                ret_val =
                    snprintf(str, str_len, "%ld",
                    (long) value->type.Signed_Int);
                break;
            case BACNET_APPLICATION_TAG_REAL:
                ret_val =
                    snprintf(str, str_len, "%f", (double) value->type.Real);
                break;
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                ret_val = snprintf(str, str_len, "%f", value->type.Double);
                break;
#endif
            case BACNET_APPLICATION_TAG_OCTET_STRING:
                len = octetstring_length(&value->type.Octet_String);
                octet_str = octetstring_value(&value->type.Octet_String);
                for (i = 0; i < len; i++) {
                    snprintf(temp_str, sizeof(temp_str), "%02X", *octet_str);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                    octet_str++;
                }
                if (i == len) {
                    /* Everything went fine */
                    ret_val = str_len - rem_str_len;
                }
                break;
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                len = characterstring_length(&value->type.Character_String);
                char_str =
                    characterstring_value(&value->type.Character_String);
                if (!append_str(&p_str, &rem_str_len, "\""))
                    break;
                for (i = 0; i < len; i++) {
                    if (isprint(*((unsigned char *) char_str))) {
                        snprintf(temp_str, sizeof(temp_str), "%c", *char_str);
                    } else {
                        snprintf(temp_str, sizeof(temp_str), "%c", '.');
                    }
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                    char_str++;
                }
                if ((i == len) && append_str(&p_str, &rem_str_len, "\"")
                    ) {
                    /* Everything is fine. Indicate how many bytes were */
                    /* written */
                    ret_val = str_len - rem_str_len;
                }
                break;
            case BACNET_APPLICATION_TAG_BIT_STRING:
                len = bitstring_bits_used(&value->type.Bit_String);
                if (!append_str(&p_str, &rem_str_len, "{"))
                    break;
                for (i = 0; i < len; i++) {
                    snprintf(temp_str, sizeof(temp_str), "%s",
                        bitstring_bit(&value->type.Bit_String,
                            (uint8_t) i) ? "true" : "false");
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                    if (i < len - 1) {
                        if (!append_str(&p_str, &rem_str_len, ","))
                            break;
                    }
                }
                if ((i == len) && append_str(&p_str, &rem_str_len, "}")
                    ) {
                    /* Everything is fine. Indicate how many bytes were */
                    /* written */
                    ret_val = str_len - rem_str_len;
                }
                break;
            case BACNET_APPLICATION_TAG_ENUMERATED:
                switch (property) {
                    case PROP_PROPERTY_LIST:
                        char_str = (char *) bactext_property_name_default(
                            value->type.Enumerated, NULL);
                        if (char_str) {
                            ret_val = snprintf(str, str_len, "%s", char_str);
                        } else {
                            ret_val =
                                snprintf(str, str_len, "%lu",
                                (unsigned long) value->type.Enumerated);
                        }
                        break;
                    case PROP_OBJECT_TYPE:
                        if (value->type.Enumerated < MAX_ASHRAE_OBJECT_TYPE) {
                            ret_val =
                                snprintf(str, str_len, "%s",
                                bactext_object_type_name(value->type.
                                    Enumerated));
                        } else if (value->type.Enumerated < 128) {
                            ret_val =
                                snprintf(str, str_len, "reserved %lu",
                                (unsigned long) value->type.Enumerated);
                        } else {
                            ret_val =
                                snprintf(str, str_len, "proprietary %lu",
                                (unsigned long) value->type.Enumerated);
                        }
                        break;
                    case PROP_EVENT_STATE:
                        ret_val =
                            snprintf(str, str_len, "%s",
                            bactext_event_state_name(value->type.Enumerated));
                        break;
                    case PROP_UNITS:
                        if (value->type.Enumerated < 256) {
                            ret_val =
                                snprintf(str, str_len, "%s",
                                bactext_engineering_unit_name(value->
                                    type.Enumerated));
                        } else {
                            ret_val =
                                snprintf(str, str_len, "proprietary %lu",
                                (unsigned long) value->type.Enumerated);
                        }
                        break;
                    case PROP_POLARITY:
                        ret_val =
                            snprintf(str, str_len, "%s",
                            bactext_binary_polarity_name(value->
                                type.Enumerated));
                        break;
                    case PROP_PRESENT_VALUE:
                    case PROP_RELINQUISH_DEFAULT:
                        if (object_type < OBJECT_PROPRIETARY_MIN) {
                            ret_val =
                                snprintf(str, str_len, "%s",
                                bactext_binary_present_value_name(value->type.
                                    Enumerated));
                        } else {
                            ret_val =
                                snprintf(str, str_len, "%lu",
                                (unsigned long) value->type.Enumerated);
                        }
                        break;
                    case PROP_RELIABILITY:
                        ret_val =
                            snprintf(str, str_len, "%s",
                            bactext_reliability_name(value->type.Enumerated));
                        break;
                    case PROP_SYSTEM_STATUS:
                        ret_val =
                            snprintf(str, str_len, "%s",
                            bactext_device_status_name(value->
                                type.Enumerated));
                        break;
                    case PROP_SEGMENTATION_SUPPORTED:
                        ret_val =
                            snprintf(str, str_len, "%s",
                            bactext_segmentation_name(value->type.Enumerated));
                        break;
                    case PROP_NODE_TYPE:
                        ret_val =
                            snprintf(str, str_len, "%s",
                            bactext_node_type_name(value->type.Enumerated));
                        break;
                    default:
                        ret_val =
                            snprintf(str, str_len, "%lu",
                            (unsigned long) value->type.Enumerated);
                        break;
                }
                break;
            case BACNET_APPLICATION_TAG_DATE:
                if (!append_str(&p_str, &rem_str_len,
                        bactext_day_of_week_name(value->type.Date.wday)
                    )
                    )
                    break;
                if (!append_str(&p_str, &rem_str_len, ", "))
                    break;

                if (!append_str(&p_str, &rem_str_len,
                        bactext_month_name(value->type.Date.month)
                    )
                    )
                    break;
                if (value->type.Date.day == 255) {
                    if (!append_str(&p_str, &rem_str_len, " (unspecified), "))
                        break;
                } else {
                    snprintf(temp_str, sizeof(temp_str), " %u, ",
                        (unsigned) value->type.Date.day);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                if (value->type.Date.year == 2155) {
                    if (!append_str(&p_str, &rem_str_len, "(unspecified)"))
                        break;
                } else {
                    snprintf(temp_str, sizeof(temp_str), "%u",
                        (unsigned) value->type.Date.year);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                /* If we get here, then everything is OK. Indicate how many */
                /* bytes were written. */
                ret_val = str_len - rem_str_len;
                break;
            case BACNET_APPLICATION_TAG_TIME:
                if (value->type.Time.hour == 255) {
                    if (!append_str(&p_str, &rem_str_len, "**:"))
                        break;
                } else {
                    snprintf(temp_str, sizeof(temp_str), "%02u:",
                        (unsigned) value->type.Time.hour);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                if (value->type.Time.min == 255) {
                    if (!append_str(&p_str, &rem_str_len, "**:"))
                        break;
                } else {
                    snprintf(temp_str, sizeof(temp_str), "%02u:",
                        (unsigned) value->type.Time.min);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                if (value->type.Time.sec == 255) {
                    if (!append_str(&p_str, &rem_str_len, "**."))
                        break;
                } else {
                    snprintf(temp_str, sizeof(temp_str), "%02u.",
                        (unsigned) value->type.Time.sec);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                if (value->type.Time.hundredths == 255) {
                    if (!append_str(&p_str, &rem_str_len, "**"))
                        break;
                } else {
                    snprintf(temp_str, sizeof(temp_str), "%02u",
                        (unsigned) value->type.Time.hundredths);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                /* If we get here, then everything is OK. Indicate how many */
                /* bytes were written. */
                ret_val = str_len - rem_str_len;
                break;
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                if (!append_str(&p_str, &rem_str_len, "("))
                    break;
                if (value->type.Object_Id.type < MAX_ASHRAE_OBJECT_TYPE) {
                    if (!append_str(&p_str, &rem_str_len,
                            bactext_object_type_name(value->type.
                                Object_Id.type)
                        )
                        )
                        break;
                    snprintf(temp_str, sizeof(temp_str), ", %lu",
                        (unsigned long) value->type.Object_Id.instance);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                } else if (value->type.Object_Id.type < 128) {
                    if (!append_str(&p_str, &rem_str_len, "reserved "))
                        break;
                    snprintf(temp_str, sizeof(temp_str), "%u, ",
                        (unsigned) value->type.Object_Id.type);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                    snprintf(temp_str, sizeof(temp_str), "%lu",
                        (unsigned long) value->type.Object_Id.instance);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                } else {
                    if (!append_str(&p_str, &rem_str_len, "proprietary "))
                        break;
                    snprintf(temp_str, sizeof(temp_str), "%u, ",
                        (unsigned) value->type.Object_Id.type);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                    snprintf(temp_str, sizeof(temp_str), "%lu",
                        (unsigned long) value->type.Object_Id.instance);
                    if (!append_str(&p_str, &rem_str_len, temp_str))
                        break;
                }
                if (!append_str(&p_str, &rem_str_len, ")"))
                    break;
                /* If we get here, then everything is OK. Indicate how many */
                /* bytes were written. */
                ret_val = str_len - rem_str_len;
                break;
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                if (!append_str(&p_str, &rem_str_len, "("))
                    break;
                if (!append_str(&p_str, &rem_str_len,
                        bactext_lighting_operation_name(value->type.
                            Lighting_Command.operation))) {
                    break;
                }
                /* FIXME: add the Lighting Command optional values */
                if (!append_str(&p_str, &rem_str_len, ")"))
                    break;
                /* If we get here, then everything is OK. Indicate how many */
                /* bytes were written. */
                ret_val = str_len - rem_str_len;
                break;
            default:
                ret_val = 0;
                break;
        }
    }

    return ret_val;
}
#endif /* BACAPP_SNPRINTF_ENABLED */

#ifdef BACAPP_PRINT_ENABLED
/* Print the extracted value from the requested BACnet object property to the
 * specified stream. If stream is NULL, do not print anything. If extraction
 * failed, do not print anything. Return the status of the extraction.
 */
bool bacapp_print_value(
    FILE * stream,
    BACNET_OBJECT_PROPERTY_VALUE * object_value)
{
    char *str;
    bool retval = false;
    size_t str_len = 32;
    int status;

    while (true) {
        /* Try to allocate memory for the output string. Give up if unable. */
        str = (char *) calloc(sizeof(char), str_len);
        if (!str)
            break;

        /* Try to extract the value into allocated memory. If unable, try again */
        /* another time with a string that is twice as large. */
        status = bacapp_snprintf_value(str, str_len, object_value);
        if ((status < 0) || ((size_t)status >= str_len)) {
            free(str);
            str_len *= 2;
        } else if (status == 0) {
            free(str);
            break;
        } else {
            if (stream)
                fprintf(stream, "%s", str);
            free(str);
            retval = true;
            break;
        }
    }
    return retval;
}
#endif

#ifdef BACAPP_PRINT_ENABLED
/* used to load the app data struct with the proper data
   converted from a command line argument */
bool bacapp_parse_application_data(
    BACNET_APPLICATION_TAG tag_number,
    const char *argv,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    int hour, min, sec, hundredths;
    int year, month, day, wday;
    int object_type = 0;
    uint32_t instance = 0;
    bool status = false;
    long long_value = 0;
    unsigned long unsigned_long_value = 0;
    double double_value = 0.0;
    int count = 0;

    if (value && (tag_number < MAX_BACNET_APPLICATION_TAG)) {
        status = true;
        value->tag = tag_number;
        switch (tag_number) {
            case BACNET_APPLICATION_TAG_BOOLEAN:
                long_value = strtol(argv, NULL, 0);
                if (long_value)
                    value->type.Boolean = true;
                else
                    value->type.Boolean = false;
                break;
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                unsigned_long_value = strtoul(argv, NULL, 0);
                value->type.Unsigned_Int = unsigned_long_value;
                break;
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                long_value = strtol(argv, NULL, 0);
                value->type.Signed_Int = long_value;
                break;
            case BACNET_APPLICATION_TAG_REAL:
                double_value = strtod(argv, NULL);
                value->type.Real = (float) double_value;
                break;
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                double_value = strtod(argv, NULL);
                value->type.Double = double_value;
                break;
#endif
            case BACNET_APPLICATION_TAG_OCTET_STRING:
#if PRINT_ENABLED       /* Apparently ain't necessarily so. */
                status =
                    octetstring_init_ascii_hex(&value->type.Octet_String,
                    argv);
#endif
                break;
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                status =
                    characterstring_init_ansi(&value->type.Character_String,
                    (char *) argv);
                break;
            case BACNET_APPLICATION_TAG_BIT_STRING:
#if PRINT_ENABLED
                status = bitstring_init_ascii(&value->type.Bit_String, argv);
#endif
                break;
            case BACNET_APPLICATION_TAG_ENUMERATED:
                unsigned_long_value = strtoul(argv, NULL, 0);
                value->type.Enumerated = unsigned_long_value;
                break;
            case BACNET_APPLICATION_TAG_DATE:
                count =
                    sscanf(argv, "%4d/%3d/%3d:%3d", &year, &month, &day,
                    &wday);
                if (count == 3) {
                    datetime_set_date(&value->type.Date, (uint16_t) year,
                        (uint8_t) month, (uint8_t) day);
                } else if (count == 4) {
                    value->type.Date.year = (uint16_t) year;
                    value->type.Date.month = (uint8_t) month;
                    value->type.Date.day = (uint8_t) day;
                    value->type.Date.wday = (uint8_t) wday;
                } else {
                    status = false;
                }
                break;
            case BACNET_APPLICATION_TAG_TIME:
                count =
                    sscanf(argv, "%3d:%3d:%3d.%3d", &hour, &min, &sec,
                    &hundredths);
                if (count == 4) {
                    value->type.Time.hour = (uint8_t) hour;
                    value->type.Time.min = (uint8_t) min;
                    value->type.Time.sec = (uint8_t) sec;
                    value->type.Time.hundredths = (uint8_t) hundredths;
                } else if (count == 3) {
                    value->type.Time.hour = (uint8_t) hour;
                    value->type.Time.min = (uint8_t) min;
                    value->type.Time.sec = (uint8_t) sec;
                    value->type.Time.hundredths = 0;
                } else if (count == 2) {
                    value->type.Time.hour = (uint8_t) hour;
                    value->type.Time.min = (uint8_t) min;
                    value->type.Time.sec = 0;
                    value->type.Time.hundredths = 0;
                } else {
                    status = false;
                }
                break;
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                count = sscanf(argv, "%4d:%7d", &object_type, &instance);
                if (count == 2) {
                    value->type.Object_Id.type = (uint16_t) object_type;
                    value->type.Object_Id.instance = instance;
                } else {
                    status = false;
                }
                break;
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                /* FIXME: add parsing for lighting command */
                break;
            default:
                break;
        }
        value->next = NULL;
    }

    return status;
}
#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"


#include <assert.h>

/* generic - can be used by other unit tests
   returns true if matching or same, false if different */
bool bacapp_same_value(
    BACNET_APPLICATION_DATA_VALUE * value,
    BACNET_APPLICATION_DATA_VALUE * test_value)
{
    bool status = false;        /*return value */

    /* does the tag match? */
    if (test_value->tag == value->tag)
        status = true;
    if (status) {
        /* second test for same-ness */
        status = false;
        /* does the value match? */
        switch (test_value->tag) {
#if defined (BACAPP_NULL)
            case BACNET_APPLICATION_TAG_NULL:
                status = true;
                break;
#endif
#if defined (BACAPP_BOOLEAN)
            case BACNET_APPLICATION_TAG_BOOLEAN:
                if (test_value->type.Boolean == value->type.Boolean)
                    status = true;
                break;
#endif
#if defined (BACAPP_UNSIGNED)
            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                if (test_value->type.Unsigned_Int == value->type.Unsigned_Int)
                    status = true;
                break;
#endif
#if defined (BACAPP_SIGNED)
            case BACNET_APPLICATION_TAG_SIGNED_INT:
                if (test_value->type.Signed_Int == value->type.Signed_Int)
                    status = true;
                break;
#endif
#if defined (BACAPP_REAL)
            case BACNET_APPLICATION_TAG_REAL:
                if (test_value->type.Real == value->type.Real)
                    status = true;
                break;
#endif
#if defined (BACAPP_DOUBLE)
            case BACNET_APPLICATION_TAG_DOUBLE:
                if (test_value->type.Double == value->type.Double)
                    status = true;
                break;
#endif
#if defined (BACAPP_ENUMERATED)
            case BACNET_APPLICATION_TAG_ENUMERATED:
                if (test_value->type.Enumerated == value->type.Enumerated)
                    status = true;
                break;
#endif
#if defined (BACAPP_DATE)
            case BACNET_APPLICATION_TAG_DATE:
                if (datetime_compare_date(&test_value->type.Date,
                        &value->type.Date) == 0)
                    status = true;
                break;
#endif
#if defined (BACAPP_TIME)
            case BACNET_APPLICATION_TAG_TIME:
                if (datetime_compare_time(&test_value->type.Time,
                        &value->type.Time) == 0)
                    status = true;
                break;
#endif
#if defined (BACAPP_OBJECT_ID)
            case BACNET_APPLICATION_TAG_OBJECT_ID:
                if ((test_value->type.Object_Id.type ==
                        value->type.Object_Id.type) &&
                    (test_value->type.Object_Id.instance ==
                        value->type.Object_Id.instance)) {
                    status = true;
                }
                break;
#endif
#if defined (BACAPP_CHARACTER_STRING)
            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                status =
                    characterstring_same(&value->type.Character_String,
                    &test_value->type.Character_String);
                break;
#endif
#if defined (BACAPP_OCTET_STRING)
            case BACNET_APPLICATION_TAG_OCTET_STRING:
                status =
                    octetstring_value_same(&value->type.Octet_String,
                    &test_value->type.Octet_String);
                break;
#endif
#if defined (BACAPP_BIT_STRING)
            case BACNET_APPLICATION_TAG_BIT_STRING:
                status =
                    bitstring_same(&value->type.Bit_String,
                    &test_value->type.Bit_String);
                break;
#endif
#if defined (BACAPP_LIGHTING_COMMAND)
            case BACNET_APPLICATION_TAG_LIGHTING_COMMAND:
                status = lighting_command_same(
                    &value->type.Lighting_Command,
                    &test_value->type.Lighting_Command);
                break;
#endif
            default:
                status = false;
                break;
        }
    }
    return status;
}

void testBACnetApplicationData_Safe(
    Test * pTest)
{
    int i;
    uint8_t apdu[MAX_APDU];
    int len = 0;
    int apdu_len;
    BACNET_APPLICATION_DATA_VALUE input_value[13];
    uint32_t len_segment[13];
    uint32_t single_length_segment;
    BACNET_APPLICATION_DATA_VALUE value;


    for (i = 0; i < 13; i++) {
        input_value[i].tag = (BACNET_APPLICATION_TAG) i;
        input_value[i].context_specific = 0;
        input_value[i].context_tag = 0;
        input_value[i].next = NULL;
        switch (input_value[i].tag) {
            case BACNET_APPLICATION_TAG_NULL:
                /* NULL: no data */
                break;

            case BACNET_APPLICATION_TAG_BOOLEAN:
                input_value[i].type.Boolean = true;
                break;

            case BACNET_APPLICATION_TAG_UNSIGNED_INT:
                input_value[i].type.Unsigned_Int = 0xDEADBEEF;
                break;

            case BACNET_APPLICATION_TAG_SIGNED_INT:
                input_value[i].type.Signed_Int = 0x00C0FFEE;
                break;
            case BACNET_APPLICATION_TAG_REAL:
                input_value[i].type.Real = 3.141592654f;
                break;
            case BACNET_APPLICATION_TAG_DOUBLE:
                input_value[i].type.Double = 2.32323232323;
                break;

            case BACNET_APPLICATION_TAG_OCTET_STRING:
                {
                    uint8_t test_octet[5] = { "Karg" };
                    octetstring_init(&input_value[i].type.Octet_String,
                        test_octet, sizeof(test_octet));
                }
                break;

            case BACNET_APPLICATION_TAG_CHARACTER_STRING:
                characterstring_init_ansi(&input_value[i].type.
                    Character_String, "Hello There!");
                break;

            case BACNET_APPLICATION_TAG_BIT_STRING:
                bitstring_init(&input_value[i].type.Bit_String);
                bitstring_set_bit(&input_value[i].type.Bit_String, 0, true);
                bitstring_set_bit(&input_value[i].type.Bit_String, 1, false);
                bitstring_set_bit(&input_value[i].type.Bit_String, 2, false);
                bitstring_set_bit(&input_value[i].type.Bit_String, 3, true);
                bitstring_set_bit(&input_value[i].type.Bit_String, 4, false);
                bitstring_set_bit(&input_value[i].type.Bit_String, 5, true);
                bitstring_set_bit(&input_value[i].type.Bit_String, 6, true);
                break;

            case BACNET_APPLICATION_TAG_ENUMERATED:
                input_value[i].type.Enumerated = 0x0BADF00D;
                break;

            case BACNET_APPLICATION_TAG_DATE:
                input_value[i].type.Date.day = 10;
                input_value[i].type.Date.month = 9;
                input_value[i].type.Date.wday = 3;
                input_value[i].type.Date.year = 1998;
                break;

            case BACNET_APPLICATION_TAG_TIME:
                input_value[i].type.Time.hour = 12;
                input_value[i].type.Time.hundredths = 56;
                input_value[i].type.Time.min = 20;
                input_value[i].type.Time.sec = 41;
                break;

            case BACNET_APPLICATION_TAG_OBJECT_ID:
                input_value[i].type.Object_Id.instance = 1234;
                input_value[i].type.Object_Id.type = 12;
                break;

            default:
                break;
        }
        single_length_segment =
            bacapp_encode_data(&apdu[len], &input_value[i]);;
        assert(single_length_segment > 0);
        /* len_segment is accumulated length */
        if (i == 0) {
            len_segment[i] = single_length_segment;
        } else {
            len_segment[i] = single_length_segment + len_segment[i - 1];
        }
        len = len_segment[i];
    }
    /*
     ** Start processing packets at processivly truncated lengths
     */

    for (apdu_len = len; apdu_len >= 0; apdu_len--) {
        bool status;
        bool expected_status;
        for (i = 0; i < 14; i++) {
            if (i == 13) {
                expected_status = false;
            } else {

                if (apdu_len < len_segment[i]) {
                    expected_status = false;
                } else {
                    expected_status = true;
                }
            }
            status =
                bacapp_decode_application_data_safe(i == 0 ? apdu : NULL,
                apdu_len, &value);

            ct_test(pTest, status == expected_status);
            if (status) {
                ct_test(pTest, value.tag == i);
                ct_test(pTest, bacapp_same_value(&input_value[i], &value));
                ct_test(pTest, !value.context_specific);
                ct_test(pTest, value.next == NULL);
            } else {
                break;
            }
        }
    }
}


void testBACnetApplicationDataLength(
    Test * pTest)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;        /* total length of the apdu, return value */
    int test_len = 0;   /* length of the data */
    uint8_t apdu[480] = { 0 };
    BACNET_TIME local_time;
    BACNET_DATE local_date;

    /* create some constructed data */
    /* 1. zero elements */
    test_len = 0;
    apdu_len = 0;
    len = encode_opening_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    len = encode_closing_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    /* verify the length of the data inside the opening/closing tags */
    len =
        bacapp_data_len(&apdu[0], apdu_len,
        PROP_LIST_OF_OBJECT_PROPERTY_REFERENCES);
    ct_test(pTest, test_len == len);

    /* 2. application tagged data, one element */
    test_len = 0;
    apdu_len = 0;
    len = encode_opening_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    len = encode_application_unsigned(&apdu[apdu_len], 4194303);
    test_len += len;
    apdu_len += len;
    len = encode_closing_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    /* verify the length of the data inside the opening/closing tags */
    len = bacapp_data_len(&apdu[0], apdu_len, PROP_OBJECT_IDENTIFIER);
    ct_test(pTest, test_len == len);

    /* 3. application tagged data, multiple elements */
    test_len = 0;
    apdu_len = 0;
    len = encode_opening_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_unsigned(&apdu[apdu_len], 1);
    test_len += len;
    apdu_len += len;
    len = encode_application_unsigned(&apdu[apdu_len], 42);
    test_len += len;
    apdu_len += len;
    len = encode_application_unsigned(&apdu[apdu_len], 91);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_application_null(&apdu[apdu_len]);
    test_len += len;
    apdu_len += len;
    len = encode_closing_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    /* verify the length of the data inside the opening/closing tags */
    len = bacapp_data_len(&apdu[0], apdu_len, PROP_PRIORITY_ARRAY);
    ct_test(pTest, test_len == len);

    /* 4. complex datatype - one element */
    test_len = 0;
    apdu_len = 0;
    len = encode_opening_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    len = encode_opening_tag(&apdu[apdu_len], 3);
    test_len += len;
    apdu_len += len;
    local_date.year = 2006;     /* AD */
    local_date.month = 4;       /* 1=Jan */
    local_date.day = 1; /* 1..31 */
    local_date.wday = 6;        /* 1=Monday */
    len = encode_application_date(&apdu[apdu_len], &local_date);
    test_len += len;
    apdu_len += len;
    local_time.hour = 7;
    local_time.min = 0;
    local_time.sec = 3;
    local_time.hundredths = 1;
    len = encode_application_time(&apdu[apdu_len], &local_time);
    test_len += len;
    apdu_len += len;
    len = encode_closing_tag(&apdu[apdu_len], 3);
    test_len += len;
    apdu_len += len;
    len = encode_closing_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    /* verify the length of the data inside the opening/closing tags */
    len = bacapp_data_len(&apdu[0], apdu_len, PROP_START_TIME);
    ct_test(pTest, test_len == len);

    /* 5. complex datatype - multiple elements */



    /* 6. context tagged data, one element */
    test_len = 0;
    apdu_len = 0;
    len = encode_opening_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    len = encode_context_unsigned(&apdu[apdu_len], 1, 91);
    test_len += len;
    apdu_len += len;
    len = encode_closing_tag(&apdu[apdu_len], 3);
    apdu_len += len;
    /* verify the length of the data inside the opening/closing tags */
    len = bacapp_data_len(&apdu[0], apdu_len, PROP_REQUESTED_SHED_LEVEL);
    ct_test(pTest, test_len == len);
}

static bool testBACnetApplicationDataValue(
    BACNET_APPLICATION_DATA_VALUE * value)
{
    uint8_t apdu[480] = { 0 };
    int apdu_len = 0;
    BACNET_APPLICATION_DATA_VALUE test_value;

    apdu_len = bacapp_encode_application_data(&apdu[0], value);
    bacapp_decode_application_data(&apdu[0], apdu_len, &test_value);

    return bacapp_same_value(value, &test_value);
}

void testBACnetApplicationData(
    Test * pTest)
{
    BACNET_APPLICATION_DATA_VALUE value;
    bool status = false;


    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_NULL, NULL,
        &value);
    ct_test(pTest, status == true);
    status = testBACnetApplicationDataValue(&value);
    ct_test(pTest, status == true);

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_BOOLEAN, "1",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Boolean == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_BOOLEAN, "0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Boolean == false);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_UNSIGNED_INT, "0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Unsigned_Int == 0);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_UNSIGNED_INT,
        "0xFFFF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Unsigned_Int == 0xFFFF);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_UNSIGNED_INT,
        "0xFFFFFFFF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Unsigned_Int == 0xFFFFFFFF);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_SIGNED_INT, "0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Signed_Int == 0);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_SIGNED_INT, "-1",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Signed_Int == -1);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_SIGNED_INT,
        "32768", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Signed_Int == 32768);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_SIGNED_INT,
        "-32768", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Signed_Int == -32768);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, "0.0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, "-1.0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, "1.0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, "3.14159",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, "-3.14159",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_ENUMERATED, "0",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Enumerated == 0);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_ENUMERATED,
        "0xFFFF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Enumerated == 0xFFFF);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_ENUMERATED,
        "0xFFFFFFFF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Enumerated == 0xFFFFFFFF);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_DATE,
        "2005/5/22:1", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Date.year == 2005);
    ct_test(pTest, value.type.Date.month == 5);
    ct_test(pTest, value.type.Date.day == 22);
    ct_test(pTest, value.type.Date.wday == 1);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    /* Happy Valentines Day! */
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_DATE, "2007/2/14",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Date.year == 2007);
    ct_test(pTest, value.type.Date.month == 2);
    ct_test(pTest, value.type.Date.day == 14);
    ct_test(pTest, value.type.Date.wday == BACNET_WEEKDAY_WEDNESDAY);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    /* Wildcard Values */
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_DATE,
        "2155/255/255:255", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Date.year == 2155);
    ct_test(pTest, value.type.Date.month == 255);
    ct_test(pTest, value.type.Date.day == 255);
    ct_test(pTest, value.type.Date.wday == 255);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_TIME,
        "23:59:59.12", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Time.hour == 23);
    ct_test(pTest, value.type.Time.min == 59);
    ct_test(pTest, value.type.Time.sec == 59);
    ct_test(pTest, value.type.Time.hundredths == 12);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_TIME, "23:59:59",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Time.hour == 23);
    ct_test(pTest, value.type.Time.min == 59);
    ct_test(pTest, value.type.Time.sec == 59);
    ct_test(pTest, value.type.Time.hundredths == 0);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_TIME, "23:59",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Time.hour == 23);
    ct_test(pTest, value.type.Time.min == 59);
    ct_test(pTest, value.type.Time.sec == 0);
    ct_test(pTest, value.type.Time.hundredths == 0);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    /* Wildcard Values */
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_TIME,
        "255:255:255.255", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Time.hour == 255);
    ct_test(pTest, value.type.Time.min == 255);
    ct_test(pTest, value.type.Time.sec == 255);
    ct_test(pTest, value.type.Time.hundredths == 255);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OBJECT_ID,
        "0:100", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, value.type.Object_Id.type == 0);
    ct_test(pTest, value.type.Object_Id.instance == 100);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_CHARACTER_STRING,
        "Karg!", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    /* test empty string */
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_CHARACTER_STRING,
        "", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,
        "1234567890ABCDEF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,
        "12-34-56-78-90-AB-CD-EF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING,
        "12 34 56 78 90 AB CD EF", &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));
    /* test empty string */
    status =
        bacapp_parse_application_data(BACNET_APPLICATION_TAG_OCTET_STRING, "",
        &value);
    ct_test(pTest, status == true);
    ct_test(pTest, testBACnetApplicationDataValue(&value));

    return;
}


#ifdef TEST_BACNET_APPLICATION_DATA
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Application Data", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBACnetApplicationData);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACnetApplicationDataLength);
    assert(rc);
    rc = ct_addTestFunction(pTest, testBACnetApplicationData_Safe);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BACNET_APPLICATION_DATA */
#endif /* TEST */
