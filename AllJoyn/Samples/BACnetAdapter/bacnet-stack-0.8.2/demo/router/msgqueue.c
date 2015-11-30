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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "msgqueue.h"

pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;

MSGBOX_ID create_msgbox(
    )
{
    MSGBOX_ID msgboxid;

    msgboxid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (msgboxid == INVALID_MSGBOX_ID) {
        return INVALID_MSGBOX_ID;
    }

    return msgboxid;
}

bool send_to_msgbox(
    MSGBOX_ID dest,
    BACMSG * msg)
{

    int err;

    err = msgsnd(dest, msg, sizeof(BACMSG), 0);
    if (err) {
        return false;
    }
    return true;
}

BACMSG *recv_from_msgbox(
    MSGBOX_ID src,
    BACMSG * msg)
{

    int recv_bytes;

    recv_bytes = msgrcv(src, msg, sizeof(BACMSG), 0, IPC_NOWAIT);
    if (recv_bytes > 0) {
        return msg;
    } else {
        return NULL;
    }
}

void del_msgbox(
    MSGBOX_ID msgboxid)
{

    if (msgboxid == INVALID_MSGBOX_ID)
        return;
    else
        msgctl(msgboxid, IPC_RMID, NULL);
}

void free_data(
    MSG_DATA * data)
{

    if (data->pdu) {
        free(data->pdu);
        data->pdu = NULL;
    }
    if (data) {
        free(data);
        data = NULL;
    }
}

void check_data(
    MSG_DATA * data)
{

    /* lock and decrement messages reference count */
    pthread_mutex_lock(&msg_lock);
    if (--data->ref_count == 0) {
        free_data(data);
    }
    pthread_mutex_unlock(&msg_lock);
}
