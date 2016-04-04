/**************************************************************************
*
* Copyright (C) 2008 John Minack
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
#include <errno.h>
#include "event.h"
#include "datalink.h"
#include "client.h"
#include "device.h"

/** @file s_uevent.c  Send an Unconfirmed Event Notification. */

/** Sends an Unconfirmed Alarm/Event Notification.
 * @ingroup EVNOTFCN
 *
 * @param buffer [in,out] The buffer to build the message in for sending.
 * @param data [in] The information about the Event to be sent.
 * @param dest [in] The destination address information (may be a broadcast).
 * @return Size of the message sent (bytes), or a negative value on error.
 */
int Send_UEvent_Notify(
    uint8_t * buffer,
    BACNET_EVENT_NOTIFICATION_DATA * data,
    BACNET_ADDRESS * dest)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(buffer, dest, &my_address, &npdu_data);
    /* encode the APDU portion of the packet */
    len = uevent_notify_encode_apdu(&buffer[pdu_len], data);
    pdu_len += len;
    /* send the data */
    bytes_sent = datalink_send_pdu(dest, &npdu_data, &buffer[0], pdu_len);

    return bytes_sent;
}
