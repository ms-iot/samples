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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bits.h"
#include "apdu.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "tsm.h"
#include "dcc.h"
#include "iam.h"

/** @file apdu.c  Handles APDU services */

extern int Routed_Device_Service_Approval(
    BACNET_CONFIRMED_SERVICE service,
    int service_argument,
    uint8_t * apdu_buff,
    uint8_t invoke_id);


/* APDU Timeout in Milliseconds */
static uint16_t Timeout_Milliseconds = 3000;
/* Number of APDU Retries */
static uint8_t Number_Of_Retries = 3;

/* a simple table for crossing the services supported */
static BACNET_SERVICES_SUPPORTED
    confirmed_service_supported[MAX_BACNET_CONFIRMED_SERVICE] = {
    SERVICE_SUPPORTED_ACKNOWLEDGE_ALARM,
    SERVICE_SUPPORTED_CONFIRMED_COV_NOTIFICATION,
    SERVICE_SUPPORTED_CONFIRMED_EVENT_NOTIFICATION,
    SERVICE_SUPPORTED_GET_ALARM_SUMMARY,
    SERVICE_SUPPORTED_GET_ENROLLMENT_SUMMARY,
    SERVICE_SUPPORTED_SUBSCRIBE_COV,
    SERVICE_SUPPORTED_ATOMIC_READ_FILE,
    SERVICE_SUPPORTED_ATOMIC_WRITE_FILE,
    SERVICE_SUPPORTED_ADD_LIST_ELEMENT,
    SERVICE_SUPPORTED_REMOVE_LIST_ELEMENT,
    SERVICE_SUPPORTED_CREATE_OBJECT,
    SERVICE_SUPPORTED_DELETE_OBJECT,
    SERVICE_SUPPORTED_READ_PROPERTY,
    SERVICE_SUPPORTED_READ_PROP_CONDITIONAL,
    SERVICE_SUPPORTED_READ_PROP_MULTIPLE,
    SERVICE_SUPPORTED_WRITE_PROPERTY,
    SERVICE_SUPPORTED_WRITE_PROP_MULTIPLE,
    SERVICE_SUPPORTED_DEVICE_COMMUNICATION_CONTROL,
    SERVICE_SUPPORTED_PRIVATE_TRANSFER,
    SERVICE_SUPPORTED_TEXT_MESSAGE,
    SERVICE_SUPPORTED_REINITIALIZE_DEVICE,
    SERVICE_SUPPORTED_VT_OPEN,
    SERVICE_SUPPORTED_VT_CLOSE,
    SERVICE_SUPPORTED_VT_DATA,
    SERVICE_SUPPORTED_AUTHENTICATE,
    SERVICE_SUPPORTED_REQUEST_KEY,
    SERVICE_SUPPORTED_READ_RANGE,
    SERVICE_SUPPORTED_LIFE_SAFETY_OPERATION,
    SERVICE_SUPPORTED_SUBSCRIBE_COV_PROPERTY,
    SERVICE_SUPPORTED_GET_EVENT_INFORMATION
};

/* a simple table for crossing the services supported */
static BACNET_SERVICES_SUPPORTED
    unconfirmed_service_supported[MAX_BACNET_UNCONFIRMED_SERVICE] = {
    SERVICE_SUPPORTED_I_AM,
    SERVICE_SUPPORTED_I_HAVE,
    SERVICE_SUPPORTED_UNCONFIRMED_COV_NOTIFICATION,
    SERVICE_SUPPORTED_UNCONFIRMED_EVENT_NOTIFICATION,
    SERVICE_SUPPORTED_UNCONFIRMED_PRIVATE_TRANSFER,
    SERVICE_SUPPORTED_UNCONFIRMED_TEXT_MESSAGE,
    SERVICE_SUPPORTED_TIME_SYNCHRONIZATION,
    SERVICE_SUPPORTED_WHO_HAS,
    SERVICE_SUPPORTED_WHO_IS,
    SERVICE_SUPPORTED_UTC_TIME_SYNCHRONIZATION
};

/* Confirmed Function Handlers */
/* If they are not set, they are handled by a reject message */
static confirmed_function Confirmed_Function[MAX_BACNET_CONFIRMED_SERVICE];

void apdu_set_confirmed_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    confirmed_function pFunction)
{
    if (service_choice < MAX_BACNET_CONFIRMED_SERVICE)
        Confirmed_Function[service_choice] = pFunction;
}

/* Allow the APDU handler to automatically reject */
static confirmed_function Unrecognized_Service_Handler;

void apdu_set_unrecognized_service_handler_handler(
    confirmed_function pFunction)
{
    Unrecognized_Service_Handler = pFunction;
}

/* Unconfirmed Function Handlers */
/* If they are not set, they are not handled */
static unconfirmed_function
    Unconfirmed_Function[MAX_BACNET_UNCONFIRMED_SERVICE];

void apdu_set_unconfirmed_handler(
    BACNET_UNCONFIRMED_SERVICE service_choice,
    unconfirmed_function pFunction)
{
    if (service_choice < MAX_BACNET_UNCONFIRMED_SERVICE)
        Unconfirmed_Function[service_choice] = pFunction;
}

bool apdu_service_supported(
    BACNET_SERVICES_SUPPORTED service_supported)
{
    int i = 0;
    bool status = false;
    bool found = false;

    if (service_supported < MAX_BACNET_SERVICES_SUPPORTED) {
        /* is it a confirmed service? */
        for (i = 0; i < MAX_BACNET_CONFIRMED_SERVICE; i++) {
            if (confirmed_service_supported[i] == service_supported) {
                found = true;
                if (Confirmed_Function[i] != NULL) {
#if BAC_ROUTING
                    /* Check to see if the current Device supports this service. */
                    int len =
                        Routed_Device_Service_Approval(service_supported, 0,
                        NULL, 0);
                    if (len > 0)
                        break;  /* Not supported - return false */
#endif

                    status = true;
                }
                break;
            }
        }

        if (!found) {
            /* is it an unconfirmed service? */
            for (i = 0; i < MAX_BACNET_UNCONFIRMED_SERVICE; i++) {
                if (unconfirmed_service_supported[i] == service_supported) {
                    if (Unconfirmed_Function[i] != NULL)
                        status = true;
                    break;
                }
            }
        }
    }
    return status;
}

/** Function to translate a SERVICE_SUPPORTED_ enum to its SERVICE_CONFIRMED_
 *  or SERVICE_UNCONFIRMED_ index.
 *  Useful with the bactext_confirmed_service_name() functions.
 *
 * @param service_supported [in] The SERVICE_SUPPORTED_ enum value to convert.
 * @param index [out] The SERVICE_CONFIRMED_ or SERVICE_UNCONFIRMED_ index,
 *                    if found.
 * @param bIsConfirmed [out] True if index is a SERVICE_CONFIRMED_ type.
 * @return True if a match was found and index and bIsConfirmed are valid.
 */
bool apdu_service_supported_to_index(
    BACNET_SERVICES_SUPPORTED service_supported,
    size_t * index,
    bool * bIsConfirmed)
{
    int i = 0;
    bool found = false;

    *bIsConfirmed = false;
    if (service_supported < MAX_BACNET_SERVICES_SUPPORTED) {
        /* is it a confirmed service? */
        for (i = 0; i < MAX_BACNET_CONFIRMED_SERVICE; i++) {
            if (confirmed_service_supported[i] == service_supported) {
                found = true;
                *index = (size_t) i;
                *bIsConfirmed = true;
                break;
            }
        }

        if (!found) {
            /* is it an unconfirmed service? */
            for (i = 0; i < MAX_BACNET_UNCONFIRMED_SERVICE; i++) {
                if (unconfirmed_service_supported[i] == service_supported) {
                    found = true;
                    *index = (size_t) i;
                    break;
                }
            }
        }
    }
    return found;
}

/* Confirmed ACK Function Handlers */
static confirmed_ack_function
    Confirmed_ACK_Function[MAX_BACNET_CONFIRMED_SERVICE];

void apdu_set_confirmed_simple_ack_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    confirmed_simple_ack_function pFunction)
{
    switch (service_choice) {
        case SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM:
        case SERVICE_CONFIRMED_COV_NOTIFICATION:
        case SERVICE_CONFIRMED_EVENT_NOTIFICATION:
        case SERVICE_CONFIRMED_SUBSCRIBE_COV:
        case SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY:
        case SERVICE_CONFIRMED_LIFE_SAFETY_OPERATION:
            /* Object Access Services */
        case SERVICE_CONFIRMED_ADD_LIST_ELEMENT:
        case SERVICE_CONFIRMED_REMOVE_LIST_ELEMENT:
        case SERVICE_CONFIRMED_DELETE_OBJECT:
        case SERVICE_CONFIRMED_WRITE_PROPERTY:
        case SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE:
            /* Remote Device Management Services */
        case SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL:
        case SERVICE_CONFIRMED_TEXT_MESSAGE:
        case SERVICE_CONFIRMED_REINITIALIZE_DEVICE:
            /* Virtual Terminal Services */
        case SERVICE_CONFIRMED_VT_CLOSE:
            /* Security Services */
        case SERVICE_CONFIRMED_REQUEST_KEY:
            Confirmed_ACK_Function[service_choice] =
                (confirmed_ack_function) pFunction;
            break;
        default:
            break;
    }
}

void apdu_set_confirmed_ack_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    confirmed_ack_function pFunction)
{
    switch (service_choice) {
        case SERVICE_CONFIRMED_GET_ALARM_SUMMARY:
        case SERVICE_CONFIRMED_GET_ENROLLMENT_SUMMARY:
        case SERVICE_CONFIRMED_GET_EVENT_INFORMATION:
            /* File Access Services */
        case SERVICE_CONFIRMED_ATOMIC_READ_FILE:
        case SERVICE_CONFIRMED_ATOMIC_WRITE_FILE:
            /* Object Access Services */
        case SERVICE_CONFIRMED_CREATE_OBJECT:
        case SERVICE_CONFIRMED_READ_PROPERTY:
        case SERVICE_CONFIRMED_READ_PROP_CONDITIONAL:
        case SERVICE_CONFIRMED_READ_PROP_MULTIPLE:
        case SERVICE_CONFIRMED_READ_RANGE:
            /* Remote Device Management Services */
        case SERVICE_CONFIRMED_PRIVATE_TRANSFER:
            /* Virtual Terminal Services */
        case SERVICE_CONFIRMED_VT_OPEN:
        case SERVICE_CONFIRMED_VT_DATA:
            /* Security Services */
        case SERVICE_CONFIRMED_AUTHENTICATE:
            Confirmed_ACK_Function[service_choice] = pFunction;
            break;
        default:
            break;
    }
}

static error_function Error_Function[MAX_BACNET_CONFIRMED_SERVICE];

void apdu_set_error_handler(
    BACNET_CONFIRMED_SERVICE service_choice,
    error_function pFunction)
{
    if (service_choice < MAX_BACNET_CONFIRMED_SERVICE)
        Error_Function[service_choice] = pFunction;
}

static abort_function Abort_Function;

void apdu_set_abort_handler(
    abort_function pFunction)
{
    Abort_Function = pFunction;
}

static reject_function Reject_Function;

void apdu_set_reject_handler(
    reject_function pFunction)
{
    Reject_Function = pFunction;
}

uint16_t apdu_decode_confirmed_service_request(
    uint8_t * apdu,     /* APDU data */
    uint16_t apdu_len,
    BACNET_CONFIRMED_SERVICE_DATA * service_data,
    uint8_t * service_choice,
    uint8_t ** service_request,
    uint16_t * service_request_len)
{
    uint16_t len = 0;   /* counts where we are in PDU */

    service_data->segmented_message = (apdu[0] & BIT3) ? true : false;
    service_data->more_follows = (apdu[0] & BIT2) ? true : false;
    service_data->segmented_response_accepted =
        (apdu[0] & BIT1) ? true : false;
    service_data->max_segs = decode_max_segs(apdu[1]);
    service_data->max_resp = decode_max_apdu(apdu[1]);
    service_data->invoke_id = apdu[2];
    len = 3;
    if (service_data->segmented_message) {
        service_data->sequence_number = apdu[len++];
        service_data->proposed_window_number = apdu[len++];
    }
    *service_choice = apdu[len++];
    *service_request = &apdu[len];
    *service_request_len = apdu_len - len;

    return len;
}

uint16_t apdu_timeout(
    void)
{
    return Timeout_Milliseconds;
}

void apdu_timeout_set(
    uint16_t milliseconds)
{
    Timeout_Milliseconds = milliseconds;
}

uint8_t apdu_retries(
    void)
{
    return Number_Of_Retries;
}

void apdu_retries_set(
    uint8_t value)
{
    Number_Of_Retries = value;
}


/* When network communications are completely disabled,
   only DeviceCommunicationControl and ReinitializeDevice APDUs
   shall be processed and no messages shall be initiated.
   When the initiation of communications is disabled, 
   all APDUs shall be processed and responses returned as 
   required... */
static bool apdu_confirmed_dcc_disabled(
    uint8_t service_choice)
{
    bool status = false;

    if (dcc_communication_disabled()) {
        switch (service_choice) {
            case SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL:
            case SERVICE_CONFIRMED_REINITIALIZE_DEVICE:
                break;
            default:
                status = true;
                break;
        }
    }

    return status;
}

/* When network communications are completely disabled,
   only DeviceCommunicationControl and ReinitializeDevice APDUs
   shall be processed and no messages shall be initiated. */
/* If the request is valid and the 'Enable/Disable' parameter is
   DISABLE_INITIATION, the responding BACnet-user shall
   discontinue the initiation of messages except for I-Am
   requests issued in accordance with the Who-Is service procedure.*/
static bool apdu_unconfirmed_dcc_disabled(
    uint8_t service_choice)
{
    bool status = false;

    if (dcc_communication_disabled()) {
        /* there are no Unconfirmed messages that 
           can be processed in this state */
        status = true;
    } else if (dcc_communication_initiation_disabled()) {
        /* WhoIs will be processed and I-Am initiated as response. */
        switch (service_choice) {
            case SERVICE_UNCONFIRMED_WHO_IS:
                break;
            default:
                status = true;
                break;
        }
    }

    return status;
}

/** Process the APDU header and invoke the appropriate service handler
 * to manage the received request.
 * Almost all requests and ACKs invoke this function.
 * @ingroup MISCHNDLR
 *
 * @param src [in] The BACNET_ADDRESS of the message's source.
 * @param apdu [in] The apdu portion of the request, to be processed.
 * @param apdu_len [in] The total (remaining) length of the apdu.
 */
void apdu_handler(
    BACNET_ADDRESS * src,
    uint8_t * apdu,     /* APDU data */
    uint16_t apdu_len)
{
    BACNET_CONFIRMED_SERVICE_DATA service_data = { 0 };
    BACNET_CONFIRMED_SERVICE_ACK_DATA service_ack_data = { 0 };
    uint8_t invoke_id = 0;
    uint8_t service_choice = 0;
    uint8_t *service_request = NULL;
    uint16_t service_request_len = 0;
    int len = 0;        /* counts where we are in PDU */
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t error_code = 0;
    uint32_t error_class = 0;
    uint8_t reason = 0;
    bool server = false;

    if (apdu) {
        /* PDU Type */
        switch (apdu[0] & 0xF0) {
            case PDU_TYPE_CONFIRMED_SERVICE_REQUEST:
                len =
                    (int) apdu_decode_confirmed_service_request(&apdu[0],
                    apdu_len, &service_data, &service_choice, &service_request,
                    &service_request_len);
                if (apdu_confirmed_dcc_disabled(service_choice)) {
                    /* When network communications are completely disabled,
                       only DeviceCommunicationControl and ReinitializeDevice APDUs
                       shall be processed and no messages shall be initiated. */
                    break;
                }
                if ((service_choice < MAX_BACNET_CONFIRMED_SERVICE) &&
                    (Confirmed_Function[service_choice]))
                    Confirmed_Function[service_choice] (service_request,
                        service_request_len, src, &service_data);
                else if (Unrecognized_Service_Handler)
                    Unrecognized_Service_Handler(service_request,
                        service_request_len, src, &service_data);
                break;
            case PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST:
                service_choice = apdu[1];
                service_request = &apdu[2];
                service_request_len = apdu_len - 2;
                if (apdu_unconfirmed_dcc_disabled(service_choice)) {
                    /* When network communications are disabled,
                       only DeviceCommunicationControl and ReinitializeDevice APDUs
                       shall be processed and no messages shall be initiated.
                       If communications have been initiation disabled, then
                       WhoIs may be processed. */
                    break;
                }
                if (service_choice < MAX_BACNET_UNCONFIRMED_SERVICE) {
                    if (Unconfirmed_Function[service_choice])
                        Unconfirmed_Function[service_choice] (service_request,
                            service_request_len, src);
                }
                break;
            case PDU_TYPE_SIMPLE_ACK:
                invoke_id = apdu[1];
                service_choice = apdu[2];
                switch (service_choice) {
                    case SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM:
                    case SERVICE_CONFIRMED_COV_NOTIFICATION:
                    case SERVICE_CONFIRMED_EVENT_NOTIFICATION:
                    case SERVICE_CONFIRMED_SUBSCRIBE_COV:
                    case SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY:
                    case SERVICE_CONFIRMED_LIFE_SAFETY_OPERATION:
                        /* Object Access Services */
                    case SERVICE_CONFIRMED_ADD_LIST_ELEMENT:
                    case SERVICE_CONFIRMED_REMOVE_LIST_ELEMENT:
                    case SERVICE_CONFIRMED_DELETE_OBJECT:
                    case SERVICE_CONFIRMED_WRITE_PROPERTY:
                    case SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE:
                        /* Remote Device Management Services */
                    case SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL:
                    case SERVICE_CONFIRMED_REINITIALIZE_DEVICE:
                    case SERVICE_CONFIRMED_TEXT_MESSAGE:
                        /* Virtual Terminal Services */
                    case SERVICE_CONFIRMED_VT_CLOSE:
                        /* Security Services */
                    case SERVICE_CONFIRMED_REQUEST_KEY:
                        if (Confirmed_ACK_Function[service_choice] != NULL) {
                            ((confirmed_simple_ack_function)
                                Confirmed_ACK_Function[service_choice]) (src,
                                invoke_id);
                        }
                        tsm_free_invoke_id(invoke_id);
                        break;
                    default:
                        break;
                }
                break;
            case PDU_TYPE_COMPLEX_ACK:
                service_ack_data.segmented_message =
                    (apdu[0] & BIT3) ? true : false;
                service_ack_data.more_follows =
                    (apdu[0] & BIT2) ? true : false;
                invoke_id = service_ack_data.invoke_id = apdu[1];
                len = 2;
                if (service_ack_data.segmented_message) {
                    service_ack_data.sequence_number = apdu[len++];
                    service_ack_data.proposed_window_number = apdu[len++];
                }
                service_choice = apdu[len++];
                service_request = &apdu[len];
                service_request_len = apdu_len - (uint16_t) len;
                switch (service_choice) {
                    case SERVICE_CONFIRMED_GET_ALARM_SUMMARY:
                    case SERVICE_CONFIRMED_GET_ENROLLMENT_SUMMARY:
                    case SERVICE_CONFIRMED_GET_EVENT_INFORMATION:
                        /* File Access Services */
                    case SERVICE_CONFIRMED_ATOMIC_READ_FILE:
                    case SERVICE_CONFIRMED_ATOMIC_WRITE_FILE:
                        /* Object Access Services */
                    case SERVICE_CONFIRMED_CREATE_OBJECT:
                    case SERVICE_CONFIRMED_READ_PROPERTY:
                    case SERVICE_CONFIRMED_READ_PROP_CONDITIONAL:
                    case SERVICE_CONFIRMED_READ_PROP_MULTIPLE:
                    case SERVICE_CONFIRMED_READ_RANGE:
                    case SERVICE_CONFIRMED_PRIVATE_TRANSFER:
                        /* Virtual Terminal Services */
                    case SERVICE_CONFIRMED_VT_OPEN:
                    case SERVICE_CONFIRMED_VT_DATA:
                        /* Security Services */
                    case SERVICE_CONFIRMED_AUTHENTICATE:
                        if (Confirmed_ACK_Function[service_choice] != NULL) {
                            (Confirmed_ACK_Function[service_choice])
                                (service_request, service_request_len, src,
                                &service_ack_data);
                        }
                        tsm_free_invoke_id(invoke_id);
                        break;
                    default:
                        break;
                }
                break;
            case PDU_TYPE_SEGMENT_ACK:
                /* FIXME: what about a denial of service attack here?
                   we could check src to see if that matched the tsm */
                tsm_free_invoke_id(invoke_id);
                break;
            case PDU_TYPE_ERROR:
                invoke_id = apdu[1];
                service_choice = apdu[2];
                len = 3;

                /* FIXME: Currently special case for C_P_T but there are others which may
                   need consideration such as ChangeList-Error, CreateObject-Error,
                   WritePropertyMultiple-Error and VTClose_Error but they may be left as
                   is for now until support for these services is added */

                if (service_choice == SERVICE_CONFIRMED_PRIVATE_TRANSFER) {     /* skip over opening tag 0 */
                    if (decode_is_opening_tag_number(&apdu[len], 0)) {
                        len++;  /* a tag number of 0 is not extended so only one octet */
                    }
                }
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                /* FIXME: we could validate that the tag is enumerated... */
                len += decode_enumerated(&apdu[len], len_value, &error_class);
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                /* FIXME: we could validate that the tag is enumerated... */
                len += decode_enumerated(&apdu[len], len_value, &error_code);

                if (service_choice == SERVICE_CONFIRMED_PRIVATE_TRANSFER) {     /* skip over closing tag 0 */
                    if (decode_is_closing_tag_number(&apdu[len], 0)) {
                        len++;  /* a tag number of 0 is not extended so only one octet */
                    }
                }
                if (service_choice < MAX_BACNET_CONFIRMED_SERVICE) {
                    if (Error_Function[service_choice])
                        Error_Function[service_choice] (src, invoke_id,
                            (BACNET_ERROR_CLASS) error_class,
                            (BACNET_ERROR_CODE) error_code);
                }
                tsm_free_invoke_id(invoke_id);
                break;
            case PDU_TYPE_REJECT:
                invoke_id = apdu[1];
                reason = apdu[2];
                if (Reject_Function)
                    Reject_Function(src, invoke_id, reason);
                tsm_free_invoke_id(invoke_id);
                break;
            case PDU_TYPE_ABORT:
                server = apdu[0] & 0x01;
                invoke_id = apdu[1];
                reason = apdu[2];
                if (Abort_Function)
                    Abort_Function(src, invoke_id, reason, server);
                tsm_free_invoke_id(invoke_id);
                break;
            default:
                break;
        }
    }
    return;
}
