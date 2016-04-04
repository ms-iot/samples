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
#include <stdlib.h>
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
#include "ptransfer.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "mydata.h"
#include "client.h"

/** @file s_ptransfer.c  Send a Private Transfer request. */

/* This function is exported. Hence this unnecessary prototype. */
uint8_t Send_Private_Transfer_Request(
    uint32_t device_id,
    uint16_t vendor_id,
    uint32_t service_number,
    char block_number,
    DATABLOCK * block);

uint8_t Send_Private_Transfer_Request(
    uint32_t device_id,
    uint16_t vendor_id,
    uint32_t service_number,
    char block_number,
    DATABLOCK * block)
{       /* NULL=optional */
    BACNET_ADDRESS dest;
    BACNET_ADDRESS my_address;
    unsigned max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    static uint8_t pt_req_buffer[300];  /* Somewhere to build the request packet */
    BACNET_PRIVATE_TRANSFER_DATA pt_block;
    BACNET_CHARACTER_STRING bsTemp;

    /* if we are forbidden to send, don't send! */
    if (!dcc_communication_enabled())
        return 0;

    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status)
        invoke_id = tsm_next_free_invokeID();
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        datalink_get_my_address(&my_address);
        npdu_encode_npdu_data(&npdu_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
            &npdu_data);
        /* encode the APDU portion of the packet */

        pt_block.vendorID = vendor_id;
        pt_block.serviceNumber = service_number;
        if (service_number == MY_SVC_READ) {
            len += encode_application_unsigned(&pt_req_buffer[len], block_number);      /* The block number we want to retrieve */
        } else {
            len += encode_application_unsigned(&pt_req_buffer[len], block_number);      /* The block number */
            len += encode_application_unsigned(&pt_req_buffer[len], block->cMyByte1);   /* And Then the block contents */
            len +=
                encode_application_unsigned(&pt_req_buffer[len],
                block->cMyByte2);
            len +=
                encode_application_real(&pt_req_buffer[len], block->fMyReal);
            characterstring_init_ansi(&bsTemp, (char *) block->sMyString);
            len +=
                encode_application_character_string(&pt_req_buffer[len],
                &bsTemp);
        }

        pt_block.serviceParameters = &pt_req_buffer[0];
        pt_block.serviceParametersLen = len;
        len =
            ptransfer_encode_apdu(&Handler_Transmit_Buffer[pdu_len], invoke_id,
            &pt_block);
        pdu_len += len;

        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */

        if ((unsigned) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(invoke_id, &dest,
                &npdu_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);
            bytes_sent =
                datalink_send_pdu(&dest, &npdu_data,
                &Handler_Transmit_Buffer[0], pdu_len);
#if PRINT_ENABLED
            if (bytes_sent <= 0)
                fprintf(stderr,
                    "Failed to Send Private Transfer Request (%s)!\n",
                    strerror(errno));
#endif
        } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send Private Transfer Request "
                "(exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}
