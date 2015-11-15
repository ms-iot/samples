/**************************************************************************
*
* Copyright (C) 2005-2006 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef HANDLERS_H
#define HANDLERS_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "apdu.h"
#include "bacapp.h"
#include "ptransfer.h"
#include "rd.h"
#include "rp.h"
#include "rpm.h"
#include "wp.h"
#include "readrange.h"
#include "getevent.h"
#include "get_alarm_sum.h"
#include "alarm_ack.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void handler_unrecognized_service(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * dest,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void npdu_handler(
        BACNET_ADDRESS * src,   /* source address */
        uint8_t * pdu,  /* PDU data */
        uint16_t pdu_len);      /* length PDU  */

    void routing_npdu_handler(
        BACNET_ADDRESS * src,
        int *DNET_list,
        uint8_t * pdu,
        uint16_t pdu_len);

    void handler_who_is(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_who_is_unicast(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_who_is_bcast_for_routing(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_who_is_unicast_for_routing(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_who_has(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_who_has_for_routing(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_i_am_add(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_i_am_bind(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void handler_read_property(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_read_property_ack(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

    void handler_write_property(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_write_property_multiple(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    bool WPValidateString(
        BACNET_APPLICATION_DATA_VALUE * pValue,
        int iMaxLen,
        bool bEmptyAllowed,
        BACNET_ERROR_CLASS * pErrorClass,
        BACNET_ERROR_CODE * pErrorCode);

    bool WPValidateArgType(
        BACNET_APPLICATION_DATA_VALUE * pValue,
        uint8_t ucExpectedType,
        BACNET_ERROR_CLASS * pErrorClass,
        BACNET_ERROR_CODE * pErrorCode);

    void handler_atomic_read_file(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_atomic_read_file_ack(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

    void handler_atomic_write_file(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_reinitialize_device(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_device_communication_control(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);
    void handler_dcc_password_set(
        char *new_password);

    void handler_i_have(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    /* time synchronization handlers */
    void handler_timesync(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);
    void handler_timesync_utc(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);
    /* time sync master features */
    int handler_timesync_encode_recipients(
        uint8_t * apdu,
        int max_apdu);
    void handler_timesync_task(
        void);
    void handler_timesync_init(
        void);
    bool handler_timesync_recipient_write(
        BACNET_WRITE_PROPERTY_DATA * wp_data);
    bool handler_timesync_interval_set(
        uint32_t minutes);
    bool handler_timesync_recipient_address_set(
        unsigned index,
        BACNET_ADDRESS * address);

    void handler_read_property_multiple(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_read_property_multiple_ack(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

    /* Decode the received RPM data and make a linked list of the results. */
    int rpm_ack_decode_service_request(
        uint8_t * apdu,
        int apdu_len,
        BACNET_READ_ACCESS_DATA * read_access_data);
    /* print the RP Ack data to stdout */
    void rp_ack_print_data(
        BACNET_READ_PROPERTY_DATA * data);
    /* print the GE Ack data to stdout */
    void ge_ack_print_data(BACNET_GET_EVENT_INFORMATION_DATA * data, uint32_t device_id);
    /* print the RPM Ack data to stdout */
    void rpm_ack_print_data(
        BACNET_READ_ACCESS_DATA * rpm_data);

    void handler_cov_subscribe(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);
    void handler_cov_task(
        void);
    void handler_cov_timer_seconds(
        uint32_t elapsed_seconds);
    void handler_cov_init(
        void);
    int handler_cov_encode_subscriptions(
        uint8_t * apdu,
        int max_apdu);

    void handler_ucov_notification(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);
    void handler_ccov_notification(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_lso(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_alarm_ack(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_alarm_ack_set(
        BACNET_OBJECT_TYPE object_type,
        alarm_ack_function pFunction);

    void handler_conf_private_trans(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_conf_private_trans_ack(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

    void handler_unconfirmed_private_transfer(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src);

    void private_transfer_print_data(
        BACNET_PRIVATE_TRANSFER_DATA *private_data);

    void handler_read_range(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_read_range_ack(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

    void handler_get_event_information_set(
        BACNET_OBJECT_TYPE object_type,
        get_event_info_function pFunction);

    void handler_get_event_information(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void handler_get_alarm_summary_set(
        BACNET_OBJECT_TYPE object_type,
        get_alarm_summary_function pFunction);

    void handler_get_alarm_summary(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_DATA * service_data);

    void get_alarm_summary_ack_handler(
        uint8_t * service_request,
        uint16_t service_len,
        BACNET_ADDRESS * src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data);

    void get_event_ack_handler(
        uint8_t *service_request,
        uint16_t service_len,
        BACNET_ADDRESS *src,
        BACNET_CONFIRMED_SERVICE_ACK_DATA *service_data);


#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup MISCHNDLR Miscellaneous Handler Utilities
 * Various utilities and functions to support the Handlers.
 */
#endif
