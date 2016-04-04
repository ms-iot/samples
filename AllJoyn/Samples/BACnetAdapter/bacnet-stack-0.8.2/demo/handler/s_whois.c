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
#include <errno.h>
#include <string.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "dcc.h"
#include "whois.h"
#include "bacenum.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"

/** @file s_whois.c  Send a Who-Is request. */

/** Send a Who-Is request to a remote network for a specific device, a range,
 * or any device.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param target_address [in] BACnet address of target router
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_To_Network(
    BACNET_ADDRESS * target_address,
    int32_t low_limit,
    int32_t high_limit)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);

    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], target_address,
        &my_address, &npdu_data);
    /* encode the APDU portion of the packet */
    len =
        whois_encode_apdu(&Handler_Transmit_Buffer[pdu_len], low_limit,
        high_limit);
    pdu_len += len;
    bytes_sent =
        datalink_send_pdu(target_address, &npdu_data,
        &Handler_Transmit_Buffer[0], pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr, "Failed to Send Who-Is Request (%s)!\n",
            strerror(errno));
#endif
}

/** Send a global Who-Is request for a specific device, a range, or any device.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_Global(
    int32_t low_limit,
    int32_t high_limit)
{
    BACNET_ADDRESS dest;

    if (!dcc_communication_enabled())
        return;

    /* Who-Is is a global broadcast */
    datalink_get_broadcast_address(&dest);

    Send_WhoIs_To_Network(&dest, low_limit, high_limit);
}

/** Send a local Who-Is request for a specific device, a range, or any device.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_Local(
    int32_t low_limit,
    int32_t high_limit)
{
    BACNET_ADDRESS dest;
    char temp[6];
    int loop;

    if (!dcc_communication_enabled())
        return;

    /* Who-Is is a global broadcast */
    datalink_get_broadcast_address(&dest);
    /* encode the NPDU portion of the packet */

    /* I added this to make it a local broadcast */
    dest.net = 0;

    /* Not sure why this happens but values are backwards so they need to be reversed */

    temp[0] = dest.mac[3];
    temp[1] = dest.mac[2];
    temp[2] = dest.mac[1];
    temp[3] = dest.mac[0];
    temp[4] = dest.mac[5];
    temp[5] = dest.mac[4];


    for (loop = 0; loop < 6; loop++) {
        dest.mac[loop] = temp[loop];
    }

    Send_WhoIs_To_Network(&dest, low_limit, high_limit);
}

/** Send a Who-Is request to a remote network for a specific device, a range,
 * or any device.
 * @ingroup DMDDB
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param target_address [in] BACnet address of target router
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs_Remote(
    BACNET_ADDRESS * target_address,
    int32_t low_limit,
    int32_t high_limit)
{
    if (!dcc_communication_enabled())
        return;

    Send_WhoIs_To_Network(target_address, low_limit, high_limit);
}

/** Send a global Who-Is request for a specific device, a range, or any device.
 * @ingroup DMDDB
 * This was the original Who-Is broadcast but the code was moved to the more
 * descriptive Send_WhoIs_Global when Send_WhoIs_Local and Send_WhoIsRemote was
 * added.
 * If low_limit and high_limit both are -1, then the range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 */
void Send_WhoIs(
    int32_t low_limit,
    int32_t high_limit)
{
    Send_WhoIs_Global(low_limit, high_limit);
}
