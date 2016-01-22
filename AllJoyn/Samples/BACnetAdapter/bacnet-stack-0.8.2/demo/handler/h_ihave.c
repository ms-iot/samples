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
#include <stdio.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bactext.h"
#include "ihave.h"
#include "handlers.h"

/** @file h_ihave.c  Handles incoming I-Have messages. */

/** Simple Handler for I-Have responses (just validates response).
 * @ingroup DMDOB
 * @param service_request [in] The received message to be handled.
 * @param service_len [in] Length of the service_request message.
 * @param src [in] The BACNET_ADDRESS of the message's source.
 */
void handler_i_have(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    int len = 0;
    BACNET_I_HAVE_DATA data;

    (void) service_len;
    (void) src;
    len = ihave_decode_service_request(service_request, service_len, &data);
    if (len != -1) {
#if PRINT_ENABLED
        fprintf(stderr, "I-Have: %s %lu from %s %lu!\r\n",
            bactext_object_type_name(data.object_id.type),
            (unsigned long) data.object_id.instance,
            bactext_object_type_name(data.device_id.type),
            (unsigned long) data.device_id.instance);
#endif
    } else {
#if PRINT_ENABLED
        fprintf(stderr, "I-Have: received, but unable to decode!\n");
#endif
    }

    return;
}
