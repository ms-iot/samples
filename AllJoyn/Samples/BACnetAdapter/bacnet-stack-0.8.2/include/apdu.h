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
*********************************************************************/
#ifndef APDU_H
#define APDU_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacenum.h"

typedef struct _confirmed_service_data {
    bool segmented_message;
    bool more_follows;
    bool segmented_response_accepted;
    int max_segs;
    int max_resp;
    uint8_t invoke_id;
    uint8_t sequence_number;
    uint8_t proposed_window_number;
} BACNET_CONFIRMED_SERVICE_DATA;

typedef struct _confirmed_service_ack_data {
    bool segmented_message;
    bool more_follows;
    uint8_t invoke_id;
    uint8_t sequence_number;
    uint8_t proposed_window_number;
} BACNET_CONFIRMED_SERVICE_ACK_DATA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* generic unconfirmed function handler */
/* Suitable to handle the following services: */
/* I_Am, Who_Is, Unconfirmed_COV_Notification, I_Have, */
/* Unconfirmed_Event_Notification, Unconfirmed_Private_Transfer, */
/* Unconfirmed_Text_Message, Time_Synchronization, Who_Has, */
/* UTC_Time_Synchronization */
    typedef void (
        *unconfirmed_function) (
        uint8_t * service_request,
        uint16_t len,
        BACNET_ADDRESS * src);

/* generic confirmed function handler */
/* Suitable to handle the following services: */
/* Acknowledge_Alarm, Confirmed_COV_Notification, */
/* Confirmed_Event_Notification, Get_Alarm_Summary, */
/* Get_Enrollment_Summary_Handler, Get_Event_Information, */
/* Subscribe_COV_Handler, Subscribe_COV_Property, */
/* Life_Safety_Operation, Atomic_Read_File, */
/* Confirmed_Atomic_Write_File, Add_List_Element, */
/* Remove_List_Element, Create_Object_Handler, */
/* Delete_Object_Handler, Read_Property, */
/* Read_Property_Conditional, Read_Property_Multiple, Read_Range, */
/* Write_Property, Write_Property_Multiple, */
/* Device_Communication_Control, Confirmed_Private_Transfer, */
/* Confirmed_Text_Message, Reinitialize_Device, */
/* VT_Open, VT_Close, VT_Data_Handler, */
/* Authenticate, Request_Key */
    typedef void (
        *confirmed_function) (
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

/* generic confirmed simple ack function handler */
    typedef void (
        *confirmed_simple_ack_function) (
        BACNET_ADDRESS * src,
        uint8_t invoke_id);

/* generic confirmed ack function handler */
    typedef void (
        *confirmed_ack_function) (
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

/* generic error reply function */
    typedef void (
        *error_function) (
        BACNET_ADDRESS * src,
        uint8_t invoke_id,
        BACNET_ERROR_CLASS error_class,
        BACNET_ERROR_CODE error_code);

/* generic abort reply function */
    typedef void (
        *abort_function) (
        BACNET_ADDRESS * src,
        uint8_t invoke_id,
        uint8_t abort_reason,
        bool server);

/* generic reject reply function */
    typedef void (
        *reject_function) (
        BACNET_ADDRESS * src,
        uint8_t invoke_id,
        uint8_t reject_reason);

    void apdu_set_confirmed_ack_handler(
        BACNET_CONFIRMED_SERVICE service_choice,
        confirmed_ack_function pFunction);

    void apdu_set_confirmed_simple_ack_handler(
        BACNET_CONFIRMED_SERVICE service_choice,
        confirmed_simple_ack_function pFunction);

/* configure reject for confirmed services that are not supported */
    void apdu_set_unrecognized_service_handler_handler(
        confirmed_function pFunction);

    void apdu_set_confirmed_handler(
        BACNET_CONFIRMED_SERVICE service_choice,
        confirmed_function pFunction);

    void apdu_set_unconfirmed_handler(
        BACNET_UNCONFIRMED_SERVICE service_choice,
        unconfirmed_function pFunction);

/* returns true if the service is supported by a handler */
    bool apdu_service_supported(
        BACNET_SERVICES_SUPPORTED service_supported);

/* Function to translate a SERVICE_SUPPORTED_ enum to its SERVICE_CONFIRMED_
 *  or SERVICE_UNCONFIRMED_ index.
 */
    bool apdu_service_supported_to_index(
        BACNET_SERVICES_SUPPORTED service_supported,
        size_t * index,
        bool * bIsConfirmed);


    void apdu_set_error_handler(
        BACNET_CONFIRMED_SERVICE service_choice,
        error_function pFunction);

    void apdu_set_abort_handler(
        abort_function pFunction);

    void apdu_set_reject_handler(
        reject_function pFunction);

    uint16_t apdu_decode_confirmed_service_request(
        uint8_t * apdu, /* APDU data */
        uint16_t apdu_len,
        BACNET_CONFIRMED_SERVICE_DATA * service_data,
        uint8_t * service_choice,
        uint8_t ** service_request,
        uint16_t * service_request_len);

    uint16_t apdu_timeout(
        void);
    void apdu_timeout_set(
        uint16_t value);
    uint8_t apdu_retries(
        void);
    void apdu_retries_set(
        uint8_t value);

    void apdu_handler(
        BACNET_ADDRESS * src,   /* source address */
        uint8_t * apdu, /* APDU data */
        uint16_t pdu_len);      /* for confirmed messages */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
