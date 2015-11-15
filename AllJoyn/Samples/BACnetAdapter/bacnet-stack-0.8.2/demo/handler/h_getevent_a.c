/**
 * @file
 * @author Daniel Blazevic <daniel.blazevic@gmail.com>
 * @date 2014
 * @brief GetEvent ACK service handling
 *
 * @section LICENSE
 *
 * Copyright (C) 2014 Daniel Blazevic <daniel.blazevic@gmail.com>
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
 * @section DESCRIPTION
 *
 * The GetEventInformation service ACK service handler is used by a client
 * BACnet-user to obtain a summary of all "active event states". The term
 * "active event states" refers to all event-initiating objects that have an
 * Event_State property whose value is not equal to NORMAL, or have an
 * Acked_Transitions property, which has at least one of the bits
 * (TO-OFFNORMAL, TO-FAULT, TONORMAL) set to FALSE.
 */
#include <assert.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "handlers.h"
#include "getevent.h"

/* 40 = min size of get event data in APDU */
#define MAX_NUMBER_OF_EVENTS ((MAX_APDU / 40) + 1)

/** Example function to handle a GetEvent ACK.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_ACK_DATA information
 * decoded from the APDU header of this message.
 */
void get_event_ack_handler(
    uint8_t *service_request,
    uint16_t service_len,
    BACNET_ADDRESS *src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA *service_data)
{
    uint8_t i = 0;
    uint16_t apdu_len = 0;
    bool more_events = false;
    /* initialize array big enough to accommodate
       multiple get event data in APDU */
    BACNET_GET_EVENT_INFORMATION_DATA get_event_data[MAX_NUMBER_OF_EVENTS];

    for (i = 1; i < MAX_NUMBER_OF_EVENTS; i++) {
        /* Create linked list */
        get_event_data[i - 1].next = &get_event_data[i];
    }

    apdu_len =
        getevent_ack_decode_service_request(&service_request[0],
        service_len, &get_event_data[0], &more_events);

    if (apdu_len > 0) {
        /* FIXME: Add code to process get_event_data */
    }
}
