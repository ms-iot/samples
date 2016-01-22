/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

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
#include "handlers.h"

bool apdu_service_supported(
    BACNET_SERVICES_SUPPORTED service_supported)
{
    bool status = false;

    if (service_supported == SERVICE_SUPPORTED_READ_PROPERTY) {
        status = true;
    }
    if (service_supported == SERVICE_SUPPORTED_WHO_IS) {
        status = true;
    }
#ifdef WRITE_PROPERTY
    if (service_supported == SERVICE_SUPPORTED_WRITE_PROPERTY) {
        status = true;
    }
#endif

    return status;
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

void apdu_handler(
    BACNET_ADDRESS * src,
    uint8_t * apdu,     /* APDU data */
    uint16_t apdu_len)
{
    BACNET_CONFIRMED_SERVICE_DATA service_data = { 0 };
    uint8_t service_choice = 0;
    uint8_t *service_request = NULL;
    uint16_t service_request_len = 0;
    uint16_t len = 0;   /* counts where we are in PDU */

    if (apdu) {
        /* PDU Type */
        switch (apdu[0] & 0xF0) {
            case PDU_TYPE_CONFIRMED_SERVICE_REQUEST:
                len = apdu_decode_confirmed_service_request(&apdu[0],   /* APDU data */
                    apdu_len, &service_data, &service_choice, &service_request,
                    &service_request_len);
                if (service_choice == SERVICE_CONFIRMED_READ_PROPERTY) {
                    handler_read_property(service_request, service_request_len,
                        src, &service_data);
                }
#ifdef WRITE_PROPERTY
                else if (service_choice == SERVICE_CONFIRMED_WRITE_PROPERTY) {
                    handler_write_property(service_request,
                        service_request_len, src, &service_data);
                }
#endif
                else {
                    handler_unrecognized_service(service_request,
                        service_request_len, src, &service_data);
                }
                break;
            case PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST:
                service_choice = apdu[1];
                service_request = &apdu[2];
                service_request_len = apdu_len - 2;
                if (service_choice == SERVICE_UNCONFIRMED_WHO_IS) {
                    handler_who_is(service_request, service_request_len, src);
                }
                break;
            case PDU_TYPE_SIMPLE_ACK:
            case PDU_TYPE_COMPLEX_ACK:
            case PDU_TYPE_SEGMENT_ACK:
            case PDU_TYPE_ERROR:
            case PDU_TYPE_REJECT:
            case PDU_TYPE_ABORT:
            default:
                break;
        }
    }
    return;
}
