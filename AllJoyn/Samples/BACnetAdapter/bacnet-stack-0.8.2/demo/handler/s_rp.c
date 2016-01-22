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
#include "rp.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"

/** @file s_rp.c  Send Read Property request. */

/** Sends a Read Property request
 * @ingroup DSRP
 *
 * @param dest [in] BACNET_ADDRESS of the destination device
 * @param max_apdu [in]
 * @param object_type [in]  Type of the object whose property is to be read.
 * @param object_instance [in] Instance # of the object to be read.
 * @param object_property [in] Property to be read, but not ALL, REQUIRED, or OPTIONAL.
 * @param array_index [in] Optional: if the Property is an array,
 *   - 0 for the array size
 *   - 1 to n for individual array members
 *   - BACNET_ARRAY_ALL (~0) for the full array to be read.
 * @return invoke id of outgoing message, or 0 if device is not bound or no tsm available
 */
uint8_t Send_Read_Property_Request_Address(
    BACNET_ADDRESS * dest,
    uint16_t max_apdu,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_ID object_property,
    uint32_t array_index)
{
    BACNET_ADDRESS my_address;
    uint8_t invoke_id = 0;
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_READ_PROPERTY_DATA data;
    BACNET_NPDU_DATA npdu_data;

    if (!dcc_communication_enabled()) {
        return 0;
    }
    if (!dest) {
        return 0;
    }
    /* is there a tsm available? */
    invoke_id = tsm_next_free_invokeID();
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        datalink_get_my_address(&my_address);
        npdu_encode_npdu_data(&npdu_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], dest, &my_address,
            &npdu_data);
        /* encode the APDU portion of the packet */
        data.object_type = object_type;
        data.object_instance = object_instance;
        data.object_property = object_property;
        data.array_index = array_index;
        len =
            rp_encode_apdu(&Handler_Transmit_Buffer[pdu_len], invoke_id,
            &data);
        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((uint16_t) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(invoke_id, dest,
                &npdu_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);
            bytes_sent =
                datalink_send_pdu(dest, &npdu_data,
                &Handler_Transmit_Buffer[0], pdu_len);
#if PRINT_ENABLED
            if (bytes_sent <= 0)
                fprintf(stderr, "Failed to Send ReadProperty Request (%s)!\n",
                    strerror(errno));
#endif
        } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send ReadProperty Request "
                "(exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}

/** Sends a Read Property request.
 * @ingroup DSRP
 *
 * @param device_id [in] ID of the destination device
 * @param object_type [in]  Type of the object whose property is to be read.
 * @param object_instance [in] Instance # of the object to be read.
 * @param object_property [in] Property to be read, but not ALL, REQUIRED, or OPTIONAL.
 * @param array_index [in] Optional: if the Property is an array,
 *   - 0 for the array size
 *   - 1 to n for individual array members
 *   - BACNET_ARRAY_ALL (~0) for the full array to be read.
 * @return invoke id of outgoing message, or 0 if device is not bound or no tsm available
 */
uint8_t Send_Read_Property_Request(
    uint32_t device_id, /* destination device */
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_ID object_property,
    uint32_t array_index)
{
    BACNET_ADDRESS dest = { 0 };
    unsigned max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    if (status) {
        invoke_id =
            Send_Read_Property_Request_Address(&dest, max_apdu, object_type,
            object_instance, object_property, array_index);
    }

    return invoke_id;
}
