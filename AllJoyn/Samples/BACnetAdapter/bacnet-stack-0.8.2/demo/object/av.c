/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

/* Analog Value Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "bactext.h"
#include "config.h"     /* the custom stuff */
#include "device.h"
#include "handlers.h"
#include "av.h"


#ifndef MAX_ANALOG_VALUES
#define MAX_ANALOG_VALUES 4
#endif

ANALOG_VALUE_DESCR AV_Descr[MAX_ANALOG_VALUES];

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Analog_Value_Properties_Required[] = {
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

static const int Analog_Value_Properties_Optional[] = {
    PROP_DESCRIPTION,
#if defined(INTRINSIC_REPORTING)
    PROP_TIME_DELAY,
    PROP_NOTIFICATION_CLASS,
    PROP_HIGH_LIMIT,
    PROP_LOW_LIMIT,
    PROP_DEADBAND,
    PROP_LIMIT_ENABLE,
    PROP_EVENT_ENABLE,
    PROP_ACKED_TRANSITIONS,
    PROP_NOTIFY_TYPE,
    PROP_EVENT_TIME_STAMPS,
#endif
    -1
};

static const int Analog_Value_Properties_Proprietary[] = {
    -1
};

void Analog_Value_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Analog_Value_Properties_Required;
    if (pOptional)
        *pOptional = Analog_Value_Properties_Optional;
    if (pProprietary)
        *pProprietary = Analog_Value_Properties_Proprietary;

    return;
}

void Analog_Value_Init(
    void)
{
#if defined(INTRINSIC_REPORTING)
	unsigned i, j;
#else
	unsigned i;
#endif

    for (i = 0; i < MAX_ANALOG_VALUES; i++) {
        memset(&AV_Descr[i], 0x00, sizeof(ANALOG_VALUE_DESCR));
        AV_Descr[i].Present_Value = 0.0;
        AV_Descr[i].Units = UNITS_NO_UNITS;
#if defined(INTRINSIC_REPORTING)
        AV_Descr[i].Event_State = EVENT_STATE_NORMAL;
        /* notification class not connected */
        AV_Descr[i].Notification_Class = BACNET_MAX_INSTANCE;
        /* initialize Event time stamps using wildcards
           and set Acked_transitions */
        for (j = 0; j < MAX_BACNET_EVENT_TRANSITION; j++) {
            datetime_wildcard_set(&AV_Descr[i].Event_Time_Stamps[j]);
            AV_Descr[i].Acked_Transitions[j].bIsAcked = true;
        }

        /* Set handler for GetEventInformation function */
        handler_get_event_information_set(OBJECT_ANALOG_VALUE,
            Analog_Value_Event_Information);
        /* Set handler for AcknowledgeAlarm function */
        handler_alarm_ack_set(OBJECT_ANALOG_VALUE, Analog_Value_Alarm_Ack);
        /* Set handler for GetAlarmSummary Service */
        handler_get_alarm_summary_set(OBJECT_ANALOG_VALUE,
            Analog_Value_Alarm_Summary);
#endif
    }
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Analog_Value_Valid_Instance(
    uint32_t object_instance)
{
    if (object_instance < MAX_ANALOG_VALUES)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Analog_Value_Count(
    void)
{
    return MAX_ANALOG_VALUES;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Analog_Value_Index_To_Instance(
    unsigned index)
{
    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Analog_Value_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_ANALOG_VALUES;

    if (object_instance < MAX_ANALOG_VALUES)
        index = object_instance;

    return index;
}

/**
 * For a given object instance-number, sets the present-value at a given
 * priority 1..16.
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - floating point analog value
 * @param  priority - priority 1..16
 *
 * @return  true if values are within range and present-value is set.
 */
bool Analog_Value_Present_Value_Set(
    uint32_t object_instance,
    float value,
    uint8_t priority)
{
    unsigned index = 0;
    bool status = false;

    index = Analog_Value_Instance_To_Index(object_instance);
    if (index < MAX_ANALOG_VALUES) {
        AV_Descr[index].Present_Value = value;
        status = true;
    }
    return status;
}

float Analog_Value_Present_Value(
    uint32_t object_instance)
{
    float value = 0;
    unsigned index = 0;

    index = Analog_Value_Instance_To_Index(object_instance);
    if (index < MAX_ANALOG_VALUES) {
        value = AV_Descr[index].Present_Value;
    }

    return value;
}

/* note: the object name must be unique within this device */
bool Analog_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;

    if (object_instance < MAX_ANALOG_VALUES) {
        sprintf(text_string, "ANALOG VALUE %lu",
            (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Analog_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    float real_value = (float) 1.414;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;
    ANALOG_VALUE_DESCR *CurrentAV;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    apdu = rpdata->application_data;

    object_index = Analog_Value_Instance_To_Index(rpdata->object_instance);
    if (object_index < MAX_ANALOG_VALUES)
        CurrentAV = &AV_Descr[object_index];
    else
        return BACNET_STATUS_ERROR;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_ANALOG_VALUE,
                rpdata->object_instance);
            break;

        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Analog_Value_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_ANALOG_VALUE);
            break;

        case PROP_PRESENT_VALUE:
            real_value = Analog_Value_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
#if defined(INTRINSIC_REPORTING)
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM,
                CurrentAV->Event_State ? true : false);
#else
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
#endif
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                CurrentAV->Out_Of_Service);

            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_EVENT_STATE:
#if defined(INTRINSIC_REPORTING)
            apdu_len =
                encode_application_enumerated(&apdu[0],
                CurrentAV->Event_State);
#else
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
#endif
            break;

        case PROP_OUT_OF_SERVICE:
            state = CurrentAV->Out_Of_Service;
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;

        case PROP_UNITS:
            apdu_len =
                encode_application_enumerated(&apdu[0], CurrentAV->Units);
            break;

#if defined(INTRINSIC_REPORTING)
        case PROP_TIME_DELAY:
            apdu_len =
                encode_application_unsigned(&apdu[0], CurrentAV->Time_Delay);
            break;

        case PROP_NOTIFICATION_CLASS:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                CurrentAV->Notification_Class);
            break;

        case PROP_HIGH_LIMIT:
            apdu_len =
                encode_application_real(&apdu[0], CurrentAV->High_Limit);
            break;

        case PROP_LOW_LIMIT:
            apdu_len = encode_application_real(&apdu[0], CurrentAV->Low_Limit);
            break;

        case PROP_DEADBAND:
            apdu_len = encode_application_real(&apdu[0], CurrentAV->Deadband);
            break;

        case PROP_LIMIT_ENABLE:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, 0,
                (CurrentAV->
                    Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ? true : false);
            bitstring_set_bit(&bit_string, 1,
                (CurrentAV->
                    Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ? true : false);

            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_EVENT_ENABLE:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, TRANSITION_TO_OFFNORMAL,
                (CurrentAV->
                    Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false);
            bitstring_set_bit(&bit_string, TRANSITION_TO_FAULT,
                (CurrentAV->
                    Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false);
            bitstring_set_bit(&bit_string, TRANSITION_TO_NORMAL,
                (CurrentAV->
                    Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);

            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_ACKED_TRANSITIONS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, TRANSITION_TO_OFFNORMAL,
                CurrentAV->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                bIsAcked);
            bitstring_set_bit(&bit_string, TRANSITION_TO_FAULT,
                CurrentAV->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked);
            bitstring_set_bit(&bit_string, TRANSITION_TO_NORMAL,
                CurrentAV->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);

            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_NOTIFY_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                CurrentAV->Notify_Type ? NOTIFY_EVENT : NOTIFY_ALARM);
            break;

        case PROP_EVENT_TIME_STAMPS:
            /* Array element zero is the number of elements in the array */
            if (rpdata->array_index == 0)
                apdu_len =
                    encode_application_unsigned(&apdu[0],
                    MAX_BACNET_EVENT_TRANSITION);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                for (i = 0; i < MAX_BACNET_EVENT_TRANSITION; i++) {;
                    len =
                        encode_opening_tag(&apdu[apdu_len],
                        TIME_STAMP_DATETIME);
                    len +=
                        encode_application_date(&apdu[apdu_len + len],
                        &CurrentAV->Event_Time_Stamps[i].date);
                    len +=
                        encode_application_time(&apdu[apdu_len + len],
                        &CurrentAV->Event_Time_Stamps[i].time);
                    len +=
                        encode_closing_tag(&apdu[apdu_len + len],
                        TIME_STAMP_DATETIME);

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
            } else if (rpdata->array_index <= MAX_BACNET_EVENT_TRANSITION) {
                apdu_len =
                    encode_opening_tag(&apdu[apdu_len], TIME_STAMP_DATETIME);
                apdu_len +=
                    encode_application_date(&apdu[apdu_len],
                    &CurrentAV->Event_Time_Stamps[rpdata->array_index].date);
                apdu_len +=
                    encode_application_time(&apdu[apdu_len],
                    &CurrentAV->Event_Time_Stamps[rpdata->array_index].time);
                apdu_len +=
                    encode_closing_tag(&apdu[apdu_len], TIME_STAMP_DATETIME);
            } else {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = BACNET_STATUS_ERROR;
            }
            break;
#endif

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->object_property != PROP_EVENT_TIME_STAMPS) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Analog_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int object_index = 0;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;
    ANALOG_VALUE_DESCR *CurrentAV;

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
        (wp_data->object_property != PROP_EVENT_TIME_STAMPS) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    object_index = Analog_Value_Instance_To_Index(wp_data->object_instance);
    if (object_index < MAX_ANALOG_VALUES)
        CurrentAV = &AV_Descr[object_index];
    else
        return false;

    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                if (Analog_Value_Present_Value_Set(wp_data->object_instance,
                        value.type.Real, wp_data->priority)) {
                    status = true;
                } else if (wp_data->priority == 6) {
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
                status = false;
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
            }
            break;

        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                CurrentAV->Out_Of_Service = value.type.Boolean;
            }
            break;

        case PROP_UNITS:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                CurrentAV->Units = value.type.Enumerated;
            }
            break;

#if defined(INTRINSIC_REPORTING)
        case PROP_TIME_DELAY:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                CurrentAV->Time_Delay = value.type.Unsigned_Int;
                CurrentAV->Remaining_Time_Delay = CurrentAV->Time_Delay;
            }
            break;

        case PROP_NOTIFICATION_CLASS:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                CurrentAV->Notification_Class = value.type.Unsigned_Int;
            }
            break;

        case PROP_HIGH_LIMIT:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                CurrentAV->High_Limit = value.type.Real;
            }
            break;

        case PROP_LOW_LIMIT:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                CurrentAV->Low_Limit = value.type.Real;
            }
            break;

        case PROP_DEADBAND:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                CurrentAV->Deadband = value.type.Real;
            }
            break;

        case PROP_LIMIT_ENABLE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                if (value.type.Bit_String.bits_used == 2) {
                    CurrentAV->Limit_Enable = value.type.Bit_String.value[0];
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    status = false;
                }
            }
            break;

        case PROP_EVENT_ENABLE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BIT_STRING,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                if (value.type.Bit_String.bits_used == 3) {
                    CurrentAV->Event_Enable = value.type.Bit_String.value[0];
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    status = false;
                }
            }
            break;

        case PROP_NOTIFY_TYPE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);

            if (status) {
                switch ((BACNET_NOTIFY_TYPE) value.type.Enumerated) {
                    case NOTIFY_EVENT:
                        CurrentAV->Notify_Type = 1;
                        break;
                    case NOTIFY_ALARM:
                        CurrentAV->Notify_Type = 0;
                        break;
                    default:
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                        status = false;
                        break;
                }
            }
            break;
#endif
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_DESCRIPTION:
#if defined(INTRINSIC_REPORTING)
        case PROP_ACKED_TRANSITIONS:
        case PROP_EVENT_TIME_STAMPS:
#endif
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        case PROP_RELINQUISH_DEFAULT:
        case PROP_PRIORITY_ARRAY:
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}


void Analog_Value_Intrinsic_Reporting(
    uint32_t object_instance)
{
#if defined(INTRINSIC_REPORTING)
    BACNET_EVENT_NOTIFICATION_DATA event_data;
    BACNET_CHARACTER_STRING msgText;
    ANALOG_VALUE_DESCR *CurrentAV;
    unsigned int object_index;
    uint8_t FromState = 0;
    uint8_t ToState;
    float ExceededLimit = 0.0f;
    float PresentVal = 0.0f;
    bool SendNotify = false;


    object_index = Analog_Value_Instance_To_Index(object_instance);
    if (object_index < MAX_ANALOG_VALUES)
        CurrentAV = &AV_Descr[object_index];
    else
        return;

    /* check limits */
    if (!CurrentAV->Limit_Enable)
        return; /* limits are not configured */


    if (CurrentAV->Ack_notify_data.bSendAckNotify) {
        /* clean bSendAckNotify flag */
        CurrentAV->Ack_notify_data.bSendAckNotify = false;
        /* copy toState */
        ToState = CurrentAV->Ack_notify_data.EventState;

#if PRINT_ENABLED
        fprintf(stderr, "Send Acknotification for (%s,%d).\n",
            bactext_object_type_name(OBJECT_ANALOG_VALUE), object_instance);
#endif /* PRINT_ENABLED */

        characterstring_init_ansi(&msgText, "AckNotification");

        /* Notify Type */
        event_data.notifyType = NOTIFY_ACK_NOTIFICATION;

        /* Send EventNotification. */
        SendNotify = true;
    } else {
        /* actual Present_Value */
        PresentVal = Analog_Value_Present_Value(object_instance);
        FromState = CurrentAV->Event_State;
        switch (CurrentAV->Event_State) {
            case EVENT_STATE_NORMAL:
                /* A TO-OFFNORMAL event is generated under these conditions:
                   (a) the Present_Value must exceed the High_Limit for a minimum
                   period of time, specified in the Time_Delay property, and
                   (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
                   (c) the TO-OFFNORMAL flag must be set in the Event_Enable property. */
                if ((PresentVal > CurrentAV->High_Limit) &&
                    ((CurrentAV->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                        EVENT_HIGH_LIMIT_ENABLE) &&
                    ((CurrentAV->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
                    if (!CurrentAV->Remaining_Time_Delay)
                        CurrentAV->Event_State = EVENT_STATE_HIGH_LIMIT;
                    else
                        CurrentAV->Remaining_Time_Delay--;
                    break;
                }

                /* A TO-OFFNORMAL event is generated under these conditions:
                   (a) the Present_Value must exceed the Low_Limit plus the Deadband
                   for a minimum period of time, specified in the Time_Delay property, and
                   (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
                   (c) the TO-NORMAL flag must be set in the Event_Enable property. */
                if ((PresentVal < CurrentAV->Low_Limit) &&
                    ((CurrentAV->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                        EVENT_LOW_LIMIT_ENABLE) &&
                    ((CurrentAV->Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ==
                        EVENT_ENABLE_TO_OFFNORMAL)) {
                    if (!CurrentAV->Remaining_Time_Delay)
                        CurrentAV->Event_State = EVENT_STATE_LOW_LIMIT;
                    else
                        CurrentAV->Remaining_Time_Delay--;
                    break;
                }
                /* value of the object is still in the same event state */
                CurrentAV->Remaining_Time_Delay = CurrentAV->Time_Delay;
                break;

            case EVENT_STATE_HIGH_LIMIT:
                /* Once exceeded, the Present_Value must fall below the High_Limit minus
                   the Deadband before a TO-NORMAL event is generated under these conditions:
                   (a) the Present_Value must fall below the High_Limit minus the Deadband
                   for a minimum period of time, specified in the Time_Delay property, and
                   (b) the HighLimitEnable flag must be set in the Limit_Enable property, and
                   (c) the TO-NORMAL flag must be set in the Event_Enable property. */
                if ((PresentVal < CurrentAV->High_Limit - CurrentAV->Deadband)
                    && ((CurrentAV->Limit_Enable & EVENT_HIGH_LIMIT_ENABLE) ==
                        EVENT_HIGH_LIMIT_ENABLE) &&
                    ((CurrentAV->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                        EVENT_ENABLE_TO_NORMAL)) {
                    if (!CurrentAV->Remaining_Time_Delay)
                        CurrentAV->Event_State = EVENT_STATE_NORMAL;
                    else
                        CurrentAV->Remaining_Time_Delay--;
                    break;
                }
                /* value of the object is still in the same event state */
                CurrentAV->Remaining_Time_Delay = CurrentAV->Time_Delay;
                break;

            case EVENT_STATE_LOW_LIMIT:
                /* Once the Present_Value has fallen below the Low_Limit,
                   the Present_Value must exceed the Low_Limit plus the Deadband
                   before a TO-NORMAL event is generated under these conditions:
                   (a) the Present_Value must exceed the Low_Limit plus the Deadband
                   for a minimum period of time, specified in the Time_Delay property, and
                   (b) the LowLimitEnable flag must be set in the Limit_Enable property, and
                   (c) the TO-NORMAL flag must be set in the Event_Enable property. */
                if ((PresentVal > CurrentAV->Low_Limit + CurrentAV->Deadband)
                    && ((CurrentAV->Limit_Enable & EVENT_LOW_LIMIT_ENABLE) ==
                        EVENT_LOW_LIMIT_ENABLE) &&
                    ((CurrentAV->Event_Enable & EVENT_ENABLE_TO_NORMAL) ==
                        EVENT_ENABLE_TO_NORMAL)) {
                    if (!CurrentAV->Remaining_Time_Delay)
                        CurrentAV->Event_State = EVENT_STATE_NORMAL;
                    else
                        CurrentAV->Remaining_Time_Delay--;
                    break;
                }
                /* value of the object is still in the same event state */
                CurrentAV->Remaining_Time_Delay = CurrentAV->Time_Delay;
                break;

            default:
                return; /* shouldn't happen */
        }       /* switch (FromState) */

        ToState = CurrentAV->Event_State;

        if (FromState != ToState) {
            /* Event_State has changed.
               Need to fill only the basic parameters of this type of event.
               Other parameters will be filled in common function. */

            switch (ToState) {
                case EVENT_STATE_HIGH_LIMIT:
                    ExceededLimit = CurrentAV->High_Limit;
                    characterstring_init_ansi(&msgText, "Goes to high limit");
                    break;

                case EVENT_STATE_LOW_LIMIT:
                    ExceededLimit = CurrentAV->Low_Limit;
                    characterstring_init_ansi(&msgText, "Goes to low limit");
                    break;

                case EVENT_STATE_NORMAL:
                    if (FromState == EVENT_STATE_HIGH_LIMIT) {
                        ExceededLimit = CurrentAV->High_Limit;
                        characterstring_init_ansi(&msgText,
                            "Back to normal state from high limit");
                    } else {
                        ExceededLimit = CurrentAV->Low_Limit;
                        characterstring_init_ansi(&msgText,
                            "Back to normal state from low limit");
                    }
                    break;

                default:
                    ExceededLimit = 0;
                    break;
            }   /* switch (ToState) */

#if PRINT_ENABLED
            fprintf(stderr, "Event_State for (%s,%d) goes from %s to %s.\n",
                bactext_object_type_name(OBJECT_ANALOG_VALUE), object_instance,
                bactext_event_state_name(FromState),
                bactext_event_state_name(ToState));
#endif /* PRINT_ENABLED */

            /* Notify Type */
            event_data.notifyType = CurrentAV->Notify_Type;

            /* Send EventNotification. */
            SendNotify = true;
        }
    }


    if (SendNotify) {
        /* Event Object Identifier */
        event_data.eventObjectIdentifier.type = OBJECT_ANALOG_VALUE;
        event_data.eventObjectIdentifier.instance = object_instance;

        /* Time Stamp */
        event_data.timeStamp.tag = TIME_STAMP_DATETIME;
        Device_getCurrentDateTime(&event_data.timeStamp.value.dateTime);

        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            /* fill Event_Time_Stamps */
            switch (ToState) {
                case EVENT_STATE_HIGH_LIMIT:
                case EVENT_STATE_LOW_LIMIT:
                    CurrentAV->Event_Time_Stamps[TRANSITION_TO_OFFNORMAL] =
                        event_data.timeStamp.value.dateTime;
                    break;

                case EVENT_STATE_FAULT:
                    CurrentAV->Event_Time_Stamps[TRANSITION_TO_FAULT] =
                        event_data.timeStamp.value.dateTime;
                    break;

                case EVENT_STATE_NORMAL:
                    CurrentAV->Event_Time_Stamps[TRANSITION_TO_NORMAL] =
                        event_data.timeStamp.value.dateTime;
                    break;
            }
        }

        /* Notification Class */
        event_data.notificationClass = CurrentAV->Notification_Class;

        /* Event Type */
        event_data.eventType = EVENT_OUT_OF_RANGE;

        /* Message Text */
        event_data.messageText = &msgText;

        /* Notify Type */
        /* filled before */

        /* From State */
        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION)
            event_data.fromState = FromState;

        /* To State */
        event_data.toState = CurrentAV->Event_State;

        /* Event Values */
        if (event_data.notifyType != NOTIFY_ACK_NOTIFICATION) {
            /* Value that exceeded a limit. */
            event_data.notificationParams.outOfRange.exceedingValue =
                PresentVal;
            /* Status_Flags of the referenced object. */
            bitstring_init(&event_data.notificationParams.outOfRange.
                statusFlags);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_IN_ALARM,
                CurrentAV->Event_State ? true : false);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&event_data.notificationParams.outOfRange.
                statusFlags, STATUS_FLAG_OUT_OF_SERVICE,
                CurrentAV->Out_Of_Service);
            /* Deadband used for limit checking. */
            event_data.notificationParams.outOfRange.deadband =
                CurrentAV->Deadband;
            /* Limit that was exceeded. */
            event_data.notificationParams.outOfRange.exceededLimit =
                ExceededLimit;
        }

        /* add data from notification class */
        Notification_Class_common_reporting_function(&event_data);

        /* Ack required */
        if ((event_data.notifyType != NOTIFY_ACK_NOTIFICATION) &&
            (event_data.ackRequired == true)) {
            switch (event_data.toState) {
                case EVENT_STATE_OFFNORMAL:
                case EVENT_STATE_HIGH_LIMIT:
                case EVENT_STATE_LOW_LIMIT:
                    CurrentAV->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                        bIsAcked = false;
                    CurrentAV->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                        Time_Stamp = event_data.timeStamp.value.dateTime;
                    break;

                case EVENT_STATE_FAULT:
                    CurrentAV->Acked_Transitions[TRANSITION_TO_FAULT].
                        bIsAcked = false;
                    CurrentAV->Acked_Transitions[TRANSITION_TO_FAULT].
                        Time_Stamp = event_data.timeStamp.value.dateTime;
                    break;

                case EVENT_STATE_NORMAL:
                    CurrentAV->Acked_Transitions[TRANSITION_TO_NORMAL].
                        bIsAcked = false;
                    CurrentAV->Acked_Transitions[TRANSITION_TO_NORMAL].
                        Time_Stamp = event_data.timeStamp.value.dateTime;
                    break;
            }
        }
    }
#endif /* defined(INTRINSIC_REPORTING) */
}


#if defined(INTRINSIC_REPORTING)
int Analog_Value_Event_Information(
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data)
{
    bool IsNotAckedTransitions;
    bool IsActiveEvent;
    int i;


    /* check index */
    if (index < MAX_ANALOG_VALUES) {
        /* Event_State not equal to NORMAL */
        IsActiveEvent = (AV_Descr[index].Event_State != EVENT_STATE_NORMAL);

        /* Acked_Transitions property, which has at least one of the bits
           (TO-OFFNORMAL, TO-FAULT, TONORMAL) set to FALSE. */
        IsNotAckedTransitions =
            (AV_Descr[index].Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked ==
            false) | (AV_Descr[index].Acked_Transitions[TRANSITION_TO_FAULT].
            bIsAcked ==
            false) | (AV_Descr[index].Acked_Transitions[TRANSITION_TO_NORMAL].
            bIsAcked == false);
    } else
        return -1;      /* end of list  */

    if ((IsActiveEvent) || (IsNotAckedTransitions)) {
        /* Object Identifier */
        getevent_data->objectIdentifier.type = OBJECT_ANALOG_VALUE;
        getevent_data->objectIdentifier.instance =
            Analog_Value_Index_To_Instance(index);
        /* Event State */
        getevent_data->eventState = AV_Descr[index].Event_State;
        /* Acknowledged Transitions */
        bitstring_init(&getevent_data->acknowledgedTransitions);
        bitstring_set_bit(&getevent_data->acknowledgedTransitions,
            TRANSITION_TO_OFFNORMAL,
            AV_Descr[index].Acked_Transitions[TRANSITION_TO_OFFNORMAL].
            bIsAcked);
        bitstring_set_bit(&getevent_data->acknowledgedTransitions,
            TRANSITION_TO_FAULT,
            AV_Descr[index].Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked);
        bitstring_set_bit(&getevent_data->acknowledgedTransitions,
            TRANSITION_TO_NORMAL,
            AV_Descr[index].Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);
        /* Event Time Stamps */
        for (i = 0; i < 3; i++) {
            getevent_data->eventTimeStamps[i].tag = TIME_STAMP_DATETIME;
            getevent_data->eventTimeStamps[i].value.dateTime =
                AV_Descr[index].Event_Time_Stamps[i];
        }
        /* Notify Type */
        getevent_data->notifyType = AV_Descr[index].Notify_Type;
        /* Event Enable */
        bitstring_init(&getevent_data->eventEnable);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_OFFNORMAL,
            (AV_Descr[index].
                Event_Enable & EVENT_ENABLE_TO_OFFNORMAL) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_FAULT,
            (AV_Descr[index].
                Event_Enable & EVENT_ENABLE_TO_FAULT) ? true : false);
        bitstring_set_bit(&getevent_data->eventEnable, TRANSITION_TO_NORMAL,
            (AV_Descr[index].
                Event_Enable & EVENT_ENABLE_TO_NORMAL) ? true : false);
        /* Event Priorities */
        Notification_Class_Get_Priorities(AV_Descr[index].Notification_Class,
            getevent_data->eventPriorities);

        return 1;       /* active event */
    } else
        return 0;       /* no active event at this index */
}

int Analog_Value_Alarm_Ack(
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CODE * error_code)
{
    ANALOG_VALUE_DESCR *CurrentAV;
    unsigned int object_index;


    object_index =
        Analog_Value_Instance_To_Index(alarmack_data->eventObjectIdentifier.
        instance);

    if (object_index < MAX_ANALOG_VALUES)
        CurrentAV = &AV_Descr[object_index];
    else {
        *error_code = ERROR_CODE_UNKNOWN_OBJECT;
        return -1;
    }

    switch (alarmack_data->eventStateAcked) {
        case EVENT_STATE_OFFNORMAL:
        case EVENT_STATE_HIGH_LIMIT:
        case EVENT_STATE_LOW_LIMIT:
            if (CurrentAV->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                bIsAcked == false) {
                if (alarmack_data->eventTimeStamp.tag != TIME_STAMP_DATETIME) {
                    *error_code = ERROR_CODE_INVALID_TIME_STAMP;
                    return -1;
                }
                if (datetime_compare(&CurrentAV->
                        Acked_Transitions[TRANSITION_TO_OFFNORMAL].Time_Stamp,
                        &alarmack_data->eventTimeStamp.value.dateTime) > 0) {
                    *error_code = ERROR_CODE_INVALID_TIME_STAMP;
                    return -1;
                }

                /* Clean transitions flag. */
                CurrentAV->Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                    bIsAcked = true;
            } else {
                *error_code = ERROR_CODE_INVALID_EVENT_STATE;
                return -1;
            }
            break;

        case EVENT_STATE_FAULT:
            if (CurrentAV->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked ==
                false) {
                if (alarmack_data->eventTimeStamp.tag != TIME_STAMP_DATETIME) {
                    *error_code = ERROR_CODE_INVALID_TIME_STAMP;
                    return -1;
                }
                if (datetime_compare(&CurrentAV->
                        Acked_Transitions[TRANSITION_TO_NORMAL].Time_Stamp,
                        &alarmack_data->eventTimeStamp.value.dateTime) > 0) {
                    *error_code = ERROR_CODE_INVALID_TIME_STAMP;
                    return -1;
                }

                /* Clean transitions flag. */
                CurrentAV->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked =
                    true;
            } else {
                *error_code = ERROR_CODE_INVALID_EVENT_STATE;
                return -1;
            }
            break;

        case EVENT_STATE_NORMAL:
            if (CurrentAV->Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked ==
                false) {
                if (alarmack_data->eventTimeStamp.tag != TIME_STAMP_DATETIME) {
                    *error_code = ERROR_CODE_INVALID_TIME_STAMP;
                    return -1;
                }
                if (datetime_compare(&CurrentAV->
                        Acked_Transitions[TRANSITION_TO_FAULT].Time_Stamp,
                        &alarmack_data->eventTimeStamp.value.dateTime) > 0) {
                    *error_code = ERROR_CODE_INVALID_TIME_STAMP;
                    return -1;
                }

                /* Clean transitions flag. */
                CurrentAV->Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked =
                    true;
            } else {
                *error_code = ERROR_CODE_INVALID_EVENT_STATE;
                return -1;
            }
            break;

        default:
            return -2;
    }

    /* Need to send AckNotification. */
    CurrentAV->Ack_notify_data.bSendAckNotify = true;
    CurrentAV->Ack_notify_data.EventState = alarmack_data->eventStateAcked;

    /* Return OK */
    return 1;
}

int Analog_Value_Alarm_Summary(
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data)
{

    /* check index */
    if (index < MAX_ANALOG_VALUES) {
        /* Event_State is not equal to NORMAL  and
           Notify_Type property value is ALARM */
        if ((AV_Descr[index].Event_State != EVENT_STATE_NORMAL) &&
            (AV_Descr[index].Notify_Type == NOTIFY_ALARM)) {
            /* Object Identifier */
            getalarm_data->objectIdentifier.type = OBJECT_ANALOG_VALUE;
            getalarm_data->objectIdentifier.instance =
                Analog_Value_Index_To_Instance(index);
            /* Alarm State */
            getalarm_data->alarmState = AV_Descr[index].Event_State;
            /* Acknowledged Transitions */
            bitstring_init(&getalarm_data->acknowledgedTransitions);
            bitstring_set_bit(&getalarm_data->acknowledgedTransitions,
                TRANSITION_TO_OFFNORMAL,
                AV_Descr[index].Acked_Transitions[TRANSITION_TO_OFFNORMAL].
                bIsAcked);
            bitstring_set_bit(&getalarm_data->acknowledgedTransitions,
                TRANSITION_TO_FAULT,
                AV_Descr[index].
                Acked_Transitions[TRANSITION_TO_FAULT].bIsAcked);
            bitstring_set_bit(&getalarm_data->acknowledgedTransitions,
                TRANSITION_TO_NORMAL,
                AV_Descr[index].
                Acked_Transitions[TRANSITION_TO_NORMAL].bIsAcked);

            return 1;   /* active alarm */
        } else
            return 0;   /* no active alarm at this index */
    } else
        return -1;      /* end of list  */
}
#endif /* defined(INTRINSIC_REPORTING) */


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

void testAnalog_Value(
    Test * pTest)
{
    BACNET_READ_PROPERTY_DATA rpdata;
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;

    Analog_Value_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_ANALOG_VALUE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Analog_Value_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_ANALOG_VALUE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Analog Value", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAnalog_Value);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ANALOG_VALUE */
#endif /* TEST */
