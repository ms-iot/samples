/**
* @file
* @author Andriy Sukhynyuk, Vasyl Tkhir, Andriy Ivasiv
* @date 2012
* @brief Message queue module
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
#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "bacdef.h"
#include "npdu.h"

extern pthread_mutex_t msg_lock;

#define INVALID_MSGBOX_ID -1

typedef int MSGBOX_ID;

typedef enum {
    DATA,
    SERVICE
} MSGTYPE;

typedef enum {
    SHUTDOWN,
    CHG_IP,
    CHG_MAC
} MSGSUBTYPE;

typedef struct _message {
    MSGBOX_ID origin;
    MSGTYPE type;
    MSGSUBTYPE subtype;
    void *data;
    /* add timestamp */
} BACMSG;

/* specific message type data structures */
typedef struct _msg_data {
    BACNET_ADDRESS dest;
    BACNET_ADDRESS src;
    uint8_t *pdu;
    uint16_t pdu_len;
    uint8_t ref_count;
} MSG_DATA;

MSGBOX_ID create_msgbox(
    );

/* returns sent byte count */
bool send_to_msgbox(
    MSGBOX_ID dest,
    BACMSG * msg);

/* returns received message */
BACMSG *recv_from_msgbox(
    MSGBOX_ID src,
    BACMSG * msg);

void del_msgbox(
    MSGBOX_ID msgboxid);

/* free message data structure */
void free_data(
    MSG_DATA * data);

/* check message reference counter and delete data if needed */
void check_data(
    MSG_DATA * data);

#endif /* end of MSGQUEUE_H */
