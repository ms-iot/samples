/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "whois.h"
#include "iam.h"
#include "device.h"

#include "client.h"
#include "txbuf.h"
#include "handlers.h"

/** @file h_whois.c  Handles Who-Is requests. */

/** Handler for Who-Is requests, with broadcast I-Am response.
 * @ingroup DMDDB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source (ignored).
 */
void handler_who_is(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    int len = 0;
    int32_t low_limit = 0;
    int32_t high_limit = 0;

    (void) src;
    len =
        whois_decode_service_request(service_request, service_len, &low_limit,
        &high_limit);
    if (len == 0) {
        Send_I_Am(&Handler_Transmit_Buffer[0]);
    } else if (len != BACNET_STATUS_ERROR) {
        /* is my device id within the limits? */
        if ((Device_Object_Instance_Number() >= (uint32_t) low_limit) &&
                (Device_Object_Instance_Number() <= (uint32_t) high_limit)) {
            Send_I_Am(&Handler_Transmit_Buffer[0]);
        }
    }

    return;
}

/** Handler for Who-Is requests, with Unicast I-Am response (per Addendum 135-2004q).
 * @ingroup DMDDB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source that the
 *                 response will be sent back to.
 */
void handler_who_is_unicast(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    int len = 0;
    int32_t low_limit = 0;
    int32_t high_limit = 0;

    len =
        whois_decode_service_request(service_request, service_len, &low_limit,
        &high_limit);
    /* If no limits, then always respond */
    if (len == 0) {
        Send_I_Am_Unicast(&Handler_Transmit_Buffer[0], src);
    } else if (len != BACNET_STATUS_ERROR) {
        /* is my device id within the limits? */
        if ((Device_Object_Instance_Number() >= (uint32_t) low_limit) &&
                (Device_Object_Instance_Number() <= (uint32_t) high_limit)) {
            Send_I_Am_Unicast(&Handler_Transmit_Buffer[0], src);
        }
    }

    return;
}


#ifdef BAC_ROUTING      /* was for BAC_ROUTING - delete in 2/2012 if still unused */
                                                /* EKH: I restored this to BAC_ROUTING (from DEPRECATED) because I found that the server demo with the built-in
                                                   virtual Router did not insert the SADRs of the virtual devices on the virtual network without it */


/** Local function to check Who-Is requests against our Device IDs.
 * Will check the gateway (root Device) and all virtual routed
 * Devices against the range and respond for each that matches.
 *
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source.
 * @param is_unicast [in] True if should send unicast response(s)
 * 			back to the src, else False if should broadcast response(s).
 */
static void check_who_is_for_routing(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    bool is_unicast)
{
    int len = 0;
    int32_t low_limit = 0;
    int32_t high_limit = 0;
    int32_t dev_instance;
    int cursor = 0;     /* Starting hint */
    int my_list[2] = { 0, -1 }; /* Not really used, so dummy values */
    BACNET_ADDRESS bcast_net;

    len =
        whois_decode_service_request(service_request, service_len, &low_limit,
        &high_limit);
    if (len == BACNET_STATUS_ERROR) {
        /* Invalid; just leave */
        return;
    }
    /* Go through all devices, starting with the root gateway Device */
    memset(&bcast_net, 0, sizeof(BACNET_ADDRESS));
    bcast_net.net = BACNET_BROADCAST_NETWORK;   /* That's all we have to set */

    while (Routed_Device_GetNext(&bcast_net, my_list, &cursor)) {
        dev_instance = Device_Object_Instance_Number();
        /* If len == 0, no limits and always respond */
        if ((len == 0) || ((dev_instance >= low_limit) &&
                (dev_instance <= high_limit))) {
            if (is_unicast)
                Send_I_Am_Unicast(&Handler_Transmit_Buffer[0], src);
            else
                Send_I_Am(&Handler_Transmit_Buffer[0]);
        }
    }

}


/** Handler for Who-Is requests in the virtual routing setup,
 * with broadcast I-Am response(s).
 * @ingroup DMDDB
 * Will check the gateway (root Device) and all virtual routed
 * Devices against the range and respond for each that matches.
 *
 * @ingroup DMDDB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source (ignored).
 */
void handler_who_is_bcast_for_routing(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    check_who_is_for_routing(service_request, service_len, src, false);
}


/** Handler for Who-Is requests in the virtual routing setup,
 * with unicast I-Am response(s) returned to the src.
 * Will check the gateway (root Device) and all virtual routed
 * Devices against the range and respond for each that matches.
 *
 * @ingroup DMDDB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source that the
 *                 response will be sent back to.
 */
void handler_who_is_unicast_for_routing(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    check_who_is_for_routing(service_request, service_len, src, true);
}
#endif /* BAC_ROUTING */
