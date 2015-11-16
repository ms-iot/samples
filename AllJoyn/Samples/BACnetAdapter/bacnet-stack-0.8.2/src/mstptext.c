/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#include <stdio.h>
#include "mstp.h"
#include "indtext.h"
#include "bacenum.h"
#include "mstptext.h"

/** @file mstptext.c  Text mapping functions for BACnet MS/TP */

static INDTEXT_DATA mstp_receive_state_text[] = {
    {MSTP_RECEIVE_STATE_IDLE, "IDLE"},
    {MSTP_RECEIVE_STATE_PREAMBLE, "PREAMBLE"},
    {MSTP_RECEIVE_STATE_HEADER, "HEADER"},
    {MSTP_RECEIVE_STATE_DATA, "DATA"},
    {0, NULL}
};

const char *mstptext_receive_state(
    unsigned index)
{
    return indtext_by_index_default(mstp_receive_state_text, index, "unknown");
}

static INDTEXT_DATA mstp_master_state_text[] = {
    {MSTP_MASTER_STATE_INITIALIZE, "INITIALIZE"},
    {MSTP_MASTER_STATE_IDLE, "IDLE"},
    {MSTP_MASTER_STATE_USE_TOKEN, "USE_TOKEN"},
    {MSTP_MASTER_STATE_WAIT_FOR_REPLY, "WAIT_FOR_REPLY"},
    {MSTP_MASTER_STATE_DONE_WITH_TOKEN, "DONE_WITH_TOKEN"},
    {MSTP_MASTER_STATE_PASS_TOKEN, "PASS_TOKEN"},
    {MSTP_MASTER_STATE_NO_TOKEN, "NO_TOKEN"},
    {MSTP_MASTER_STATE_POLL_FOR_MASTER, "POLL_FOR_MASTER"},
    {MSTP_MASTER_STATE_ANSWER_DATA_REQUEST, "ANSWER_DATA_REQUEST"},
    {0, NULL}
};

const char *mstptext_master_state(
    unsigned index)
{
    return indtext_by_index_default(mstp_master_state_text, index, "unknown");
}

static INDTEXT_DATA mstp_frame_type_text[] = {
    {FRAME_TYPE_TOKEN, "TOKEN"},
    {FRAME_TYPE_POLL_FOR_MASTER, "POLL_FOR_MASTER"},
    {FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER, "REPLY_TO_POLL_FOR_MASTER"},
    {FRAME_TYPE_TEST_REQUEST, "TEST_REQUEST"},
    {FRAME_TYPE_TEST_RESPONSE, "TEST_RESPONSE"},
    {FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY, "BACNET_DATA_EXPECTING_REPLY"},
    {FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY,
        "BACNET_DATA_NOT_EXPECTING_REPLY"},
    {FRAME_TYPE_REPLY_POSTPONED, "REPLY_POSTPONED"},
    {0, NULL}
};

const char *mstptext_frame_type(
    unsigned index)
{
    return indtext_by_index_split_default(mstp_frame_type_text, index,
        FRAME_TYPE_PROPRIETARY_MIN, "UNKNOWN", "PROPRIETARY");
}
