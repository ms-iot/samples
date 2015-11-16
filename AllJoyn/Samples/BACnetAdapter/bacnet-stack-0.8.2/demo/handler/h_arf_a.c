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
#include "arf.h"
#if defined(BACFILE)
#include "bacfile.h"
#endif
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"

/** @file h_arf_a.c  Handles Acknowledgment of Atomic Read  File response. */

/* We performed an AtomicReadFile Request, */
/* and here is the data from the server */
/* Note: it does not have to be the same file=instance */
/* that someone can read from us.  It is common to */
/* use the description as the file name. */
#if defined(BACFILE)
void handler_atomic_read_file_ack(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_ATOMIC_READ_FILE_DATA data;
    uint32_t instance = 0;

    (void) src;
    /* get the file instance from the tsm data before freeing it */
    instance = bacfile_instance_from_tsm(service_data->invoke_id);
    len = arf_ack_decode_service_request(service_request, service_len, &data);
#if PRINT_ENABLED
    fprintf(stderr, "Received Read-File Ack!\n");
#endif
    if ((len > 0) && (instance <= BACNET_MAX_INSTANCE)) {
        /* write the data received to the file specified */
        if (data.access == FILE_STREAM_ACCESS) {
            bacfile_read_ack_stream_data(instance, &data);
        } else if (data.access == FILE_RECORD_ACCESS) {
            bacfile_read_ack_record_data(instance, &data);
        }
    }
}
#endif
