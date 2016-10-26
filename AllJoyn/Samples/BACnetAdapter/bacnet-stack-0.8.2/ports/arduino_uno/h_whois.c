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

bool Send_I_Am_Flag = true;

void sendIamUnicast(uint8_t * buffer,
    BACNET_ADDRESS * src)
{
    BACNET_ADDRESS dest;
    int pdu_len = 0;
    BACNET_NPDU_DATA npdu_data;

    /* encode the data */
    int npdu_len = 0;
    int apdu_len = 0;
    BACNET_ADDRESS my_address;
    /* The destination will be the same as the src, so copy it over. */
    memcpy(&dest, src, sizeof(BACNET_ADDRESS));
    /* dest->net = 0; - no, must direct back to src->net to meet BTL tests */

    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    npdu_len = npdu_encode_pdu(&buffer[0], &dest, &my_address, &npdu_data);
    /* encode the APDU portion of the packet */
    apdu_len =
        iam_encode_apdu(&buffer[npdu_len], Device_Object_Instance_Number(),
        MAX_APDU, SEGMENTATION_NONE, Device_Vendor_Identifier());
    /* send data */
    pdu_len = npdu_len + apdu_len;
    int bytes = datalink_send_pdu(&dest, &npdu_data, &buffer[0], pdu_len);
}

void handler_who_is(uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    int len = 0;
    int32_t low_limit = 0;
    int32_t high_limit = 0;
    int32_t target_device;

    len =
        whois_decode_service_request(service_request, service_len, &low_limit,
        &high_limit);
    if (len == 0) {
        sendIamUnicast(&Handler_Transmit_Buffer[0], src);
    } else if (len != -1) {
        /* is my device id within the limits? */
        target_device = Device_Object_Instance_Number();
        if (((target_device >= low_limit) && (target_device <= high_limit)) {
            sendIamUnicast(&Handler_Transmit_Buffer[0], src);
        }
    }

    return;
}
