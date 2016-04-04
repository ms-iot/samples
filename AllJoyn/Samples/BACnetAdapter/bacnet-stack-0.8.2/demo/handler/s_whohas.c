/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#include "whohas.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"

/** @file s_whohas.c  Send Who-Has requests. */

/** Send a Who-Has request for a device which has a named Object.
 * @ingroup DMDOB
 * If low_limit and high_limit both are -1, then the device ID range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit for a range.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 * @param object_name [in] The Name of the desired Object.
 */
void Send_WhoHas_Name(
    int32_t low_limit,
    int32_t high_limit,
    const char *object_name)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_ADDRESS dest;
    int bytes_sent = 0;
    BACNET_WHO_HAS_DATA data;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled())
        return;
    /* Who-Has is a global broadcast */
    datalink_get_broadcast_address(&dest);
    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
        &npdu_data);

    /* encode the APDU portion of the packet */
    data.low_limit = low_limit;
    data.high_limit = high_limit;
    data.is_object_name = true;
    characterstring_init_ansi(&data.object.name, object_name);
    len = whohas_encode_apdu(&Handler_Transmit_Buffer[pdu_len], &data);
    pdu_len += len;
    /* send the data */
    bytes_sent =
        datalink_send_pdu(&dest, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr, "Failed to Send Who-Has Request (%s)!\n",
            strerror(errno));
#endif
}

/** Send a Who-Has request for a device which has a specific Object type and ID.
 * @ingroup DMDOB
 * If low_limit and high_limit both are -1, then the device ID range is unlimited.
 * If low_limit and high_limit have the same non-negative value, then only
 * that device will respond.
 * Otherwise, low_limit must be less than high_limit for a range.
 * @param low_limit [in] Device Instance Low Range, 0 - 4,194,303 or -1
 * @param high_limit [in] Device Instance High Range, 0 - 4,194,303 or -1
 * @param object_type [in] The BACNET_OBJECT_TYPE of the desired Object.
 * @param object_instance [in] The ID of the desired Object.
 */
void Send_WhoHas_Object(
    int32_t low_limit,
    int32_t high_limit,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_ADDRESS dest;
    int bytes_sent = 0;
    BACNET_WHO_HAS_DATA data;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled())
        return;
    /* Who-Has is a global broadcast */
    datalink_get_broadcast_address(&dest);
    datalink_get_my_address(&my_address);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
        &npdu_data);

    /* encode the APDU portion of the packet */
    data.low_limit = low_limit;
    data.high_limit = high_limit;
    data.is_object_name = false;
    data.object.identifier.type = object_type;
    data.object.identifier.instance = object_instance;
    len = whohas_encode_apdu(&Handler_Transmit_Buffer[pdu_len], &data);
    pdu_len += len;
    bytes_sent =
        datalink_send_pdu(&dest, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr, "Failed to Send Who-Has Request (%s)!\n",
            strerror(errno));
#endif
}
