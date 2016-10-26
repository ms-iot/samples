/**
* @file
* @author Andriy Sukhynyuk, Vasyl Tkhir, Andriy Ivasiv
* @date 2012
* @brief Network layer for BACnet routing
*
* @section LICENSE
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
*/
#ifndef NETWORK_LAYER_H
#define NETWORK_LAYER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bacenum.h"
#include "bacdef.h"
#include "npdu.h"
#include "net.h"
#include "portthread.h"

uint16_t process_network_message(
    BACMSG * msg,
    MSG_DATA * data,
    uint8_t ** buff);

uint16_t create_network_message(
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    MSG_DATA * data,
    uint8_t ** buff,
    void *val);

void send_network_message(
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    MSG_DATA * data,
    uint8_t ** buff,
    void *val);

void init_npdu(
    BACNET_NPDU_DATA * npdu_data,
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    bool data_expecting_reply);

#endif /* end of NETWORK_LAYER_H */
