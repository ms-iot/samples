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
#include "timesync.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"

/** @file s_ts.c  Send TimeSync requests. */

void Send_TimeSync_Remote(
    BACNET_ADDRESS * dest,
    BACNET_DATE * bdate,
    BACNET_TIME * btime)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    if (!dcc_communication_enabled())
        return;

    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], dest, &my_address,
        &npdu_data);
    /* encode the APDU portion of the packet */
    len =
        timesync_encode_apdu(&Handler_Transmit_Buffer[pdu_len], bdate, btime);
    pdu_len += len;
    /* send it out the datalink */
    bytes_sent =
        datalink_send_pdu(dest, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr, "Failed to Send Time-Synchronization Request (%s)!\n",
            strerror(errno));
#endif
}

void Send_TimeSync(
    BACNET_DATE * bdate,
    BACNET_TIME * btime)
{
    BACNET_ADDRESS dest;

    datalink_get_broadcast_address(&dest);
    Send_TimeSync_Remote(&dest, bdate, btime);
}

void Send_TimeSyncUTC(
    BACNET_DATE * bdate,
    BACNET_TIME * btime)
{
    int pdu_len = 0;
    BACNET_ADDRESS dest;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    if (!dcc_communication_enabled())
        return;

    /* we could use unicast or broadcast */
    datalink_get_broadcast_address(&dest);
    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
        &npdu_data);
    /* encode the APDU portion of the packet */
    pdu_len =
        timesync_utc_encode_apdu(&Handler_Transmit_Buffer[pdu_len], bdate, btime);
    bytes_sent =
        datalink_send_pdu(&dest, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr,
            "Failed to Send UTC-Time-Synchronization Request (%s)!\n",
            strerror(errno));
#endif
}

/**
 * Sends a UTC TimeSync message using the local time from the device.
 */
void Send_TimeSyncUTC_Device(void)
{
    int32_t utc_offset_minutes = 0;
    bool dst = false;
    BACNET_DATE_TIME local_time;
    BACNET_DATE_TIME utc_time;

    Device_getCurrentDateTime(&local_time);
    dst = Device_Daylight_Savings_Status();
    utc_offset_minutes = Device_UTC_Offset();
    datetime_copy(&utc_time, &local_time);
    datetime_add_minutes(&utc_time, utc_offset_minutes);
    if (dst) {
        datetime_add_minutes(&utc_time, -60);
    }
    Send_TimeSyncUTC(&utc_time.date, &utc_time.time);
}

/**
 * Sends a TimeSync message using the local time from the device.
 */
void Send_TimeSync_Device(void)
{
    BACNET_DATE_TIME local_time;

    Device_getCurrentDateTime(&local_time);
    Send_TimeSync(&local_time.date, &local_time.time);
}
