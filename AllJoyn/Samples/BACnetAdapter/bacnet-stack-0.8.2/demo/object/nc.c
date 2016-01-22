/**************************************************************************
*
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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
#include <stdio.h>
#include <string.h>

#include "address.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "client.h"
#include "config.h"
#include "device.h"
#include "event.h"
#include "handlers.h"
#include "txbuf.h"
#include "wp.h"
#include "nc.h"


#ifndef MAX_NOTIFICATION_CLASSES
#define MAX_NOTIFICATION_CLASSES 2
#endif


#if defined(INTRINSIC_REPORTING)
static NOTIFICATION_CLASS_INFO NC_Info[MAX_NOTIFICATION_CLASSES];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Notification_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_NOTIFICATION_CLASS,
    PROP_PRIORITY,
    PROP_ACK_REQUIRED,
    PROP_RECIPIENT_LIST,
    -1
};

static const int Notification_Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Notification_Properties_Proprietary[] = {
    -1
};

void Notification_Class_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Notification_Properties_Required;
    if (pOptional)
        *pOptional = Notification_Properties_Optional;
    if (pProprietary)
        *pProprietary = Notification_Properties_Proprietary;
    return;
}

void Notification_Class_Init(
    void)
{
    uint8_t NotifyIdx = 0;

    for (NotifyIdx = 0; NotifyIdx < MAX_NOTIFICATION_CLASSES; NotifyIdx++) {
        /* init with zeros */
        memset(&NC_Info[NotifyIdx], 0x00, sizeof(NOTIFICATION_CLASS_INFO));
        /* set the basic parameters */
        NC_Info[NotifyIdx].Ack_Required = 0;
        NC_Info[NotifyIdx].Priority[TRANSITION_TO_OFFNORMAL] = 255;     /* The lowest priority for Normal message. */
        NC_Info[NotifyIdx].Priority[TRANSITION_TO_FAULT] = 255; /* The lowest priority for Normal message. */
        NC_Info[NotifyIdx].Priority[TRANSITION_TO_NORMAL] = 255;        /* The lowest priority for Normal message. */
    }

    return;
}


/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Notification_Class_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Notification_Class_Instance_To_Index(object_instance);
    if (index < MAX_NOTIFICATION_CLASSES)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Notification_Class_Count(
    void)
{
    return MAX_NOTIFICATION_CLASSES;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Notification_Class_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Notification_Class_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_NOTIFICATION_CLASSES;

    if (object_instance < MAX_NOTIFICATION_CLASSES)
        index = object_instance;

    return index;
}

bool Notification_Class_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Notification_Class_Instance_To_Index(object_instance);
    if (index < MAX_NOTIFICATION_CLASSES) {
        sprintf(text_string, "NOTIFICATION CLASS %lu", (unsigned long) index);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}



int Notification_Class_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    NOTIFICATION_CLASS_INFO *CurrentNotify;
    BACNET_CHARACTER_STRING char_string;
    BACNET_OCTET_STRING octet_string;
    BACNET_BIT_STRING bit_string;
    uint8_t *apdu = NULL;
    uint8_t u8Val;
    int idx;
    int apdu_len = 0;   /* return value */


    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    apdu = rpdata->application_data;
    CurrentNotify =
        &NC_Info[Notification_Class_Instance_To_Index(rpdata->
            object_instance)];

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0],
                OBJECT_NOTIFICATION_CLASS, rpdata->object_instance);
            break;

        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Notification_Class_Object_Name(rpdata->object_instance,
                &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                OBJECT_NOTIFICATION_CLASS);
            break;

        case PROP_NOTIFICATION_CLASS:
            apdu_len +=
                encode_application_unsigned(&apdu[0], rpdata->object_instance);
            break;

        case PROP_PRIORITY:
            if (rpdata->array_index == 0)
                apdu_len += encode_application_unsigned(&apdu[0], 3);
            else {
                if (rpdata->array_index == BACNET_ARRAY_ALL) {
                    apdu_len +=
                        encode_application_unsigned(&apdu[apdu_len],
                        CurrentNotify->Priority[TRANSITION_TO_OFFNORMAL]);
                    apdu_len +=
                        encode_application_unsigned(&apdu[apdu_len],
                        CurrentNotify->Priority[TRANSITION_TO_FAULT]);
                    apdu_len +=
                        encode_application_unsigned(&apdu[apdu_len],
                        CurrentNotify->Priority[TRANSITION_TO_NORMAL]);
                } else if (rpdata->array_index <= MAX_BACNET_EVENT_TRANSITION) {
                    apdu_len +=
                        encode_application_unsigned(&apdu[apdu_len],
                        CurrentNotify->Priority[rpdata->array_index - 1]);
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = -1;
                }
            }
            break;

        case PROP_ACK_REQUIRED:
            u8Val = CurrentNotify->Ack_Required;

            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, TRANSITION_TO_OFFNORMAL,
                (u8Val & TRANSITION_TO_OFFNORMAL_MASKED) ? true : false);
            bitstring_set_bit(&bit_string, TRANSITION_TO_FAULT,
                (u8Val & TRANSITION_TO_FAULT_MASKED) ? true : false);
            bitstring_set_bit(&bit_string, TRANSITION_TO_NORMAL,
                (u8Val & TRANSITION_TO_NORMAL_MASKED) ? true : false);
            /* encode bitstring */
            apdu_len +=
                encode_application_bitstring(&apdu[apdu_len], &bit_string);
            break;

        case PROP_RECIPIENT_LIST:
            /* encode all entry of Recipient_List */
            for (idx = 0; idx < NC_MAX_RECIPIENTS; idx++) {
                BACNET_DESTINATION *RecipientEntry;
                int i = 0;

                /* get pointer of current element for Recipient_List  - easier for use */
                RecipientEntry = &CurrentNotify->Recipient_List[idx];
                if (RecipientEntry->Recipient.RecipientType !=
                    RECIPIENT_TYPE_NOTINITIALIZED) {
                    /* Valid Days - BACnetDaysOfWeek - [bitstring] monday-sunday */
                    u8Val = 0x01;
                    bitstring_init(&bit_string);

                    for (i = 0; i < MAX_BACNET_DAYS_OF_WEEK; i++) {
                        if (RecipientEntry->ValidDays & u8Val)
                            bitstring_set_bit(&bit_string, i, true);
                        else
                            bitstring_set_bit(&bit_string, i, false);
                        u8Val <<= 1;    /* next day */
                    }
                    apdu_len +=
                        encode_application_bitstring(&apdu[apdu_len],
                        &bit_string);

                    /* From Time */
                    apdu_len +=
                        encode_application_time(&apdu[apdu_len],
                        &RecipientEntry->FromTime);

                    /* To Time */
                    apdu_len +=
                        encode_application_time(&apdu[apdu_len],
                        &RecipientEntry->ToTime);

                    /*
                       BACnetRecipient ::= CHOICE {
                       device [0] BACnetObjectIdentifier,
                       address [1] BACnetAddress
                       } */

                    /* CHOICE - device [0] BACnetObjectIdentifier */
                    if (RecipientEntry->Recipient.RecipientType ==
                        RECIPIENT_TYPE_DEVICE) {
                        apdu_len +=
                            encode_context_object_id(&apdu[apdu_len], 0,
                            OBJECT_DEVICE,
                            RecipientEntry->Recipient._.DeviceIdentifier);
                    }
                    /* CHOICE - address [1] BACnetAddress */
                    else if (RecipientEntry->Recipient.RecipientType ==
                        RECIPIENT_TYPE_ADDRESS) {
                        /* opening tag 1 */
                        apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
                        /* network-number Unsigned16, */
                        apdu_len +=
                            encode_application_unsigned(&apdu[apdu_len],
                            RecipientEntry->Recipient._.Address.net);

                        /* mac-address OCTET STRING */
                        if (RecipientEntry->Recipient._.Address.net) {
                            octetstring_init(&octet_string,
                                RecipientEntry->Recipient._.Address.adr,
                                RecipientEntry->Recipient._.Address.len);
                        } else {
                            octetstring_init(&octet_string,
                                RecipientEntry->Recipient._.Address.mac,
                                RecipientEntry->Recipient._.Address.mac_len);
                        }
                        apdu_len +=
                            encode_application_octet_string(&apdu[apdu_len],
                            &octet_string);

                        /* closing tag 1 */
                        apdu_len += encode_closing_tag(&apdu[apdu_len], 1);

                    } else {;
                    }   /* shouldn't happen */

                    /* Process Identifier - Unsigned32 */
                    apdu_len +=
                        encode_application_unsigned(&apdu[apdu_len],
                        RecipientEntry->ProcessIdentifier);

                    /* Issue Confirmed Notifications - boolean */
                    apdu_len +=
                        encode_application_boolean(&apdu[apdu_len],
                        RecipientEntry->ConfirmedNotify);

                    /* Transitions - BACnet Event Transition Bits [bitstring] */
                    u8Val = RecipientEntry->Transitions;

                    bitstring_init(&bit_string);
                    bitstring_set_bit(&bit_string, TRANSITION_TO_OFFNORMAL,
                        (u8Val & TRANSITION_TO_OFFNORMAL_MASKED) ? true :
                        false);
                    bitstring_set_bit(&bit_string, TRANSITION_TO_FAULT,
                        (u8Val & TRANSITION_TO_FAULT_MASKED) ? true : false);
                    bitstring_set_bit(&bit_string, TRANSITION_TO_NORMAL,
                        (u8Val & TRANSITION_TO_NORMAL_MASKED) ? true : false);

                    apdu_len +=
                        encode_application_bitstring(&apdu[apdu_len],
                        &bit_string);
                }
            }
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = -1;
            break;
    }

    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}


bool Notification_Class_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    NOTIFICATION_CLASS_INFO *CurrentNotify;
    NOTIFICATION_CLASS_INFO TmpNotify;
    BACNET_APPLICATION_DATA_VALUE value;
    bool status = false;
    int iOffset = 0;
    uint8_t idx = 0;
    int len = 0;



    CurrentNotify =
        &NC_Info[Notification_Class_Instance_To_Index(wp_data->
            object_instance)];

    /* decode the some of the request
     */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRIORITY:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                if (wp_data->array_index == 0) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                } else if (wp_data->array_index == BACNET_ARRAY_ALL) {
                    /* FIXME: wite all array */
                } else if (wp_data->array_index <= 3) {
                    CurrentNotify->Priority[wp_data->array_index - 1] =
                        value.type.Unsigned_Int;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                }
            }
            break;

        case PROP_ACK_REQUIRED:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                if (value.type.Bit_String.bits_used == 3) {
                    CurrentNotify->Ack_Required =
                        value.type.Bit_String.value[0];
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;

        case PROP_RECIPIENT_LIST:

            memset(&TmpNotify, 0x00, sizeof(NOTIFICATION_CLASS_INFO));

            /* decode all packed */
            while (iOffset < wp_data->application_data_len) {
                /* Decode Valid Days */
                len =
                    bacapp_decode_application_data(&wp_data->
                    application_data[iOffset], wp_data->application_data_len,
                    &value);

                if ((len == 0) ||
                    (value.tag != BACNET_APPLICATION_TAG_BIT_STRING)) {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }

                if (value.type.Bit_String.bits_used == MAX_BACNET_DAYS_OF_WEEK)
                    /* store value */
                    TmpNotify.Recipient_List[idx].ValidDays =
                        value.type.Bit_String.value[0];
                else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_OTHER;
                    return false;
                }

                iOffset += len;
                /* Decode From Time */
                len =
                    bacapp_decode_application_data(&wp_data->
                    application_data[iOffset], wp_data->application_data_len,
                    &value);

                if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_TIME)) {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }
                /* store value */
                TmpNotify.Recipient_List[idx].FromTime = value.type.Time;

                iOffset += len;
                /* Decode To Time */
                len =
                    bacapp_decode_application_data(&wp_data->
                    application_data[iOffset], wp_data->application_data_len,
                    &value);

                if ((len == 0) || (value.tag != BACNET_APPLICATION_TAG_TIME)) {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }
                /* store value */
                TmpNotify.Recipient_List[idx].ToTime = value.type.Time;

                iOffset += len;
                /* context tag [0] - Device */
                if (decode_is_context_tag(&wp_data->application_data[iOffset],
                        0)) {
                    TmpNotify.Recipient_List[idx].Recipient.RecipientType =
                        RECIPIENT_TYPE_DEVICE;
                    /* Decode Network Number */
                    len =
                        bacapp_decode_context_data(&wp_data->
                        application_data[iOffset],
                        wp_data->application_data_len, &value,
                        PROP_RECIPIENT_LIST);

                    if ((len == 0) ||
                        (value.tag != BACNET_APPLICATION_TAG_OBJECT_ID)) {
                        /* Bad decode, wrong tag or following required parameter missing */
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                        return false;
                    }
                    /* store value */
                    TmpNotify.Recipient_List[idx].Recipient._.
                        DeviceIdentifier = value.type.Object_Id.instance;

                    iOffset += len;
                }
                /* opening tag [1] - Recipient */
                else if (decode_is_opening_tag_number(&wp_data->
                        application_data[iOffset], 1)) {
                    iOffset++;
                    TmpNotify.Recipient_List[idx].Recipient.RecipientType =
                        RECIPIENT_TYPE_ADDRESS;
                    /* Decode Network Number */
                    len =
                        bacapp_decode_application_data(&wp_data->
                        application_data[iOffset],
                        wp_data->application_data_len, &value);

                    if ((len == 0) ||
                        (value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT)) {
                        /* Bad decode, wrong tag or following required parameter missing */
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                        return false;
                    }
                    /* store value */
                    TmpNotify.Recipient_List[idx].Recipient._.Address.net =
                        value.type.Unsigned_Int;

                    iOffset += len;
                    /* Decode Address */
                    len =
                        bacapp_decode_application_data(&wp_data->
                        application_data[iOffset],
                        wp_data->application_data_len, &value);

                    if ((len == 0) ||
                        (value.tag != BACNET_APPLICATION_TAG_OCTET_STRING)) {
                        /* Bad decode, wrong tag or following required parameter missing */
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                        return false;
                    }
                    /* store value */
                    if (TmpNotify.Recipient_List[idx].Recipient._.Address.
                        net == 0) {
                        memcpy(TmpNotify.Recipient_List[idx].Recipient._.
                            Address.mac, value.type.Octet_String.value,
                            value.type.Octet_String.length);
                        TmpNotify.Recipient_List[idx].Recipient._.Address.
                            mac_len = value.type.Octet_String.length;
                    } else {
                        memcpy(TmpNotify.Recipient_List[idx].Recipient._.
                            Address.adr, value.type.Octet_String.value,
                            value.type.Octet_String.length);
                        TmpNotify.Recipient_List[idx].Recipient._.Address.len =
                            value.type.Octet_String.length;
                    }

                    iOffset += len;
                    /* closing tag [1] - Recipient */
                    if (decode_is_closing_tag_number(&wp_data->
                            application_data[iOffset], 1))
                        iOffset++;
                    else {
                        /* Bad decode, wrong tag or following required parameter missing */
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                        return false;
                    }
                } else {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }

                /* Process Identifier */
                len =
                    bacapp_decode_application_data(&wp_data->
                    application_data[iOffset], wp_data->application_data_len,
                    &value);

                if ((len == 0) ||
                    (value.tag != BACNET_APPLICATION_TAG_UNSIGNED_INT)) {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }
                /* store value */
                TmpNotify.Recipient_List[idx].ProcessIdentifier =
                    value.type.Unsigned_Int;

                iOffset += len;
                /* Issue Confirmed Notifications */
                len =
                    bacapp_decode_application_data(&wp_data->
                    application_data[iOffset], wp_data->application_data_len,
                    &value);

                if ((len == 0) ||
                    (value.tag != BACNET_APPLICATION_TAG_BOOLEAN)) {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }
                /* store value */
                TmpNotify.Recipient_List[idx].ConfirmedNotify =
                    value.type.Boolean;

                iOffset += len;
                /* Transitions */
                len =
                    bacapp_decode_application_data(&wp_data->
                    application_data[iOffset], wp_data->application_data_len,
                    &value);

                if ((len == 0) ||
                    (value.tag != BACNET_APPLICATION_TAG_BIT_STRING)) {
                    /* Bad decode, wrong tag or following required parameter missing */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
                    return false;
                }

                if (value.type.Bit_String.bits_used ==
                    MAX_BACNET_EVENT_TRANSITION)
                    /* store value */
                    TmpNotify.Recipient_List[idx].Transitions =
                        value.type.Bit_String.value[0];
                else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_OTHER;
                    return false;
                }
                iOffset += len;

                /* Increasing element of list */
                if (++idx >= NC_MAX_RECIPIENTS) {
                    wp_data->error_class = ERROR_CLASS_RESOURCES;
                    wp_data->error_code =
                        ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
                    return false;
                }
            }

            /* Decoded all recipient list */
            /* copy elements from temporary object */
            for (idx = 0; idx < NC_MAX_RECIPIENTS; idx++) {
                BACNET_ADDRESS src = { 0 };
                unsigned max_apdu = 0;
                int32_t DeviceID;

                CurrentNotify->Recipient_List[idx] =
                    TmpNotify.Recipient_List[idx];

                if (CurrentNotify->Recipient_List[idx].Recipient.
                    RecipientType == RECIPIENT_TYPE_DEVICE) {
                    /* copy Device_ID */
                    DeviceID =
                        CurrentNotify->Recipient_List[idx].Recipient._.
                        DeviceIdentifier;
                    address_bind_request(DeviceID, &max_apdu, &src);

                } else if (CurrentNotify->Recipient_List[idx].Recipient.
                    RecipientType == RECIPIENT_TYPE_ADDRESS) {
                    /* copy Address */
                    /* src = CurrentNotify->Recipient_List[idx].Recipient._.Address; */
                    /* address_bind_request(BACNET_MAX_INSTANCE, &max_apdu, &src); */
                }
            }

            status = true;

            break;

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}


void Notification_Class_Get_Priorities(
    uint32_t Object_Instance,
    uint32_t * pPriorityArray)
{
    NOTIFICATION_CLASS_INFO *CurrentNotify;
    uint32_t object_index;
    int i;

    object_index = Notification_Class_Instance_To_Index(Object_Instance);

    if (object_index < MAX_NOTIFICATION_CLASSES)
        CurrentNotify = &NC_Info[object_index];
    else {
        for (i = 0; i < 3; i++)
            pPriorityArray[i] = 255;
        return; /* unknown object */
    }

    for (i = 0; i < 3; i++)
        pPriorityArray[i] = CurrentNotify->Priority[i];
}


static bool IsRecipientActive(
    BACNET_DESTINATION * pBacDest,
    uint8_t EventToState)
{
    BACNET_DATE_TIME DateTime;

    /* valid Transitions */
    switch (EventToState) {
        case EVENT_STATE_OFFNORMAL:
        case EVENT_STATE_HIGH_LIMIT:
        case EVENT_STATE_LOW_LIMIT:
            if (!(pBacDest->Transitions & TRANSITION_TO_OFFNORMAL_MASKED))
                return false;
            break;

        case EVENT_STATE_FAULT:
            if (!(pBacDest->Transitions & TRANSITION_TO_FAULT_MASKED))
                return false;
            break;

        case EVENT_STATE_NORMAL:
            if (!(pBacDest->Transitions & TRANSITION_TO_NORMAL_MASKED))
                return false;
            break;

        default:
            return false;       /* shouldn't happen */
    }

    /* get actual date and time */
    Device_getCurrentDateTime(&DateTime);

    /* valid Days */
    if (!((0x01 << (DateTime.date.wday - 1)) & pBacDest->ValidDays))
        return false;

    /* valid FromTime */
    if (datetime_compare_time(&DateTime.time, &pBacDest->FromTime) < 0)
        return false;

    /* valid ToTime */
    if (datetime_compare_time(&pBacDest->ToTime, &DateTime.time) < 0)
        return false;

    return true;
}


void Notification_Class_common_reporting_function(
    BACNET_EVENT_NOTIFICATION_DATA * event_data)
{
    /* Fill the parameters common for all types of events. */

    NOTIFICATION_CLASS_INFO *CurrentNotify;
    BACNET_DESTINATION *pBacDest;
    uint32_t notify_index;
    uint8_t index;


    notify_index =
        Notification_Class_Instance_To_Index(event_data->notificationClass);

    if (notify_index < MAX_NOTIFICATION_CLASSES)
        CurrentNotify = &NC_Info[notify_index];
    else
        return;


    /* Initiating Device Identifier */
    event_data->initiatingObjectIdentifier.type = OBJECT_DEVICE;
    event_data->initiatingObjectIdentifier.instance =
        Device_Object_Instance_Number();

    /* Priority and AckRequired */
    switch (event_data->toState) {
        case EVENT_STATE_NORMAL:
            event_data->priority =
                CurrentNotify->Priority[TRANSITION_TO_NORMAL];
            event_data->ackRequired =
                (CurrentNotify->
                Ack_Required & TRANSITION_TO_NORMAL_MASKED) ? true : false;
            break;

        case EVENT_STATE_FAULT:
            event_data->priority =
                CurrentNotify->Priority[TRANSITION_TO_FAULT];
            event_data->ackRequired =
                (CurrentNotify->
                Ack_Required & TRANSITION_TO_FAULT_MASKED) ? true : false;
            break;

        case EVENT_STATE_OFFNORMAL:
        case EVENT_STATE_HIGH_LIMIT:
        case EVENT_STATE_LOW_LIMIT:
            event_data->priority =
                CurrentNotify->Priority[TRANSITION_TO_OFFNORMAL];
            event_data->ackRequired =
                (CurrentNotify->Ack_Required & TRANSITION_TO_OFFNORMAL_MASKED)
                ? true : false;
            break;

        default:       /* shouldn't happen */
            break;
    }

    /* send notifications for active recipients */
    /* pointer to first recipient */
    pBacDest = &CurrentNotify->Recipient_List[0];
    for (index = 0; index < NC_MAX_RECIPIENTS; index++, pBacDest++) {
        /* check if recipient is defined */
        if (pBacDest->Recipient.RecipientType == RECIPIENT_TYPE_NOTINITIALIZED)
            break;      /* recipient doesn't defined - end of list */

        if (IsRecipientActive(pBacDest, event_data->toState) == true) {
            BACNET_ADDRESS dest;
            uint32_t device_id;
            unsigned max_apdu;

            /* Process Identifier */
            event_data->processIdentifier = pBacDest->ProcessIdentifier;

            /* send notification */
            if (pBacDest->Recipient.RecipientType == RECIPIENT_TYPE_DEVICE) {
                /* send notification to the specified device */
                device_id = pBacDest->Recipient._.DeviceIdentifier;

                if (pBacDest->ConfirmedNotify == true)
                    Send_CEvent_Notify(device_id, event_data);
                else if (address_get_by_device(device_id, &max_apdu, &dest))
                    Send_UEvent_Notify(Handler_Transmit_Buffer, event_data,
                        &dest);
            } else if (pBacDest->Recipient.RecipientType ==
                RECIPIENT_TYPE_ADDRESS) {
                /* send notification to the address indicated */
                if (pBacDest->ConfirmedNotify == true) {
                    if (address_get_device_id(&dest, &device_id))
                        Send_CEvent_Notify(device_id, event_data);
                } else {
                    dest = pBacDest->Recipient._.Address;
                    Send_UEvent_Notify(Handler_Transmit_Buffer, event_data,
                        &dest);
                }
            }
        }
    }
}

/* This function tries to find the addresses of the defined devices. */
/* It should be called periodically (example once per minute). */
void Notification_Class_find_recipient(
    void)
{
    NOTIFICATION_CLASS_INFO *CurrentNotify;
    BACNET_DESTINATION *pBacDest;
    BACNET_ADDRESS src = { 0 };
    unsigned max_apdu = 0;
    uint32_t notify_index;
    uint32_t DeviceID;
    uint8_t idx;


    for (notify_index = 0; notify_index < MAX_NOTIFICATION_CLASSES;
        notify_index++) {
        /* pointer to current notification */
        CurrentNotify =
            &NC_Info[Notification_Class_Instance_To_Index(notify_index)];
        /* pointer to first recipient */
        pBacDest = &CurrentNotify->Recipient_List[0];
        for (idx = 0; idx < NC_MAX_RECIPIENTS; idx++, pBacDest++) {
            if (CurrentNotify->Recipient_List[idx].Recipient.RecipientType ==
                RECIPIENT_TYPE_DEVICE) {
                /* Device ID */
                DeviceID =
                    CurrentNotify->Recipient_List[idx].Recipient._.
                    DeviceIdentifier;
                /* Send who_ is request only when address of device is unknown. */
                if (!address_bind_request(DeviceID, &max_apdu, &src))
                    Send_WhoIs(DeviceID, DeviceID);
            } else if (CurrentNotify->Recipient_List[idx].Recipient.
                RecipientType == RECIPIENT_TYPE_ADDRESS) {

            }
        }
    }
}
#endif /* defined(INTRINSIC_REPORTING) */
