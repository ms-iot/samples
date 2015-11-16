/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg
 Corrections by Ferran Arumi, 2007, Barcelona, Spain

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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bits.h"
#include "apdu.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "tsm.h"
#include "config.h"
#include "datalink.h"
#include "handlers.h"
#include "address.h"
#include "bacaddr.h"

/** @file tsm.c  BACnet Transaction State Machine operations  */

#if (MAX_TSM_TRANSACTIONS)
/* Really only needed for segmented messages */
/* and a little for sending confirmed messages */
/* If we are only a server and only initiate broadcasts, */
/* then we don't need a TSM layer. */

/* FIXME: not coded for segmentation */

/* declare space for the TSM transactions, and set it up in the init. */
/* table rules: an Invoke ID = 0 is an unused spot in the table */
static BACNET_TSM_DATA TSM_List[MAX_TSM_TRANSACTIONS];

/* invoke ID for incrementing between subsequent calls. */
static uint8_t Current_Invoke_ID = 1;

static tsm_timeout_function Timeout_Function;

void tsm_set_timeout_handler(
    tsm_timeout_function pFunction)
{
    Timeout_Function = pFunction;
}

/* returns MAX_TSM_TRANSACTIONS if not found */
static uint8_t tsm_find_invokeID_index(
    uint8_t invokeID)
{
    unsigned i = 0;     /* counter */
    uint8_t index = MAX_TSM_TRANSACTIONS;       /* return value */

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (TSM_List[i].InvokeID == invokeID) {
            index = (uint8_t) i;
            break;
        }
    }

    return index;
}

static uint8_t tsm_find_first_free_index(
    void)
{
    unsigned i = 0;     /* counter */
    uint8_t index = MAX_TSM_TRANSACTIONS;       /* return value */

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (TSM_List[i].InvokeID == 0) {
            index = (uint8_t) i;
            break;
        }
    }

    return index;
}

bool tsm_transaction_available(
    void)
{
    bool status = false;        /* return value */
    unsigned i = 0;     /* counter */

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (TSM_List[i].InvokeID == 0) {
            /* one is available! */
            status = true;
            break;
        }
    }

    return status;
}

uint8_t tsm_transaction_idle_count(
    void)
{
    uint8_t count = 0;  /* return value */
    unsigned i = 0;     /* counter */

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if ((TSM_List[i].InvokeID == 0) &&
            (TSM_List[i].state == TSM_STATE_IDLE)) {
            /* one is available! */
            count++;
        }
    }

    return count;
}

/* sets the invokeID */

void tsm_invokeID_set(
    uint8_t invokeID)
{
    if (invokeID == 0) {
        invokeID = 1;
    }
    Current_Invoke_ID = invokeID;
}

/* gets the next free invokeID,
   and reserves a spot in the table
   returns 0 if none are available */
uint8_t tsm_next_free_invokeID(
    void)
{
    uint8_t index = 0;
    uint8_t invokeID = 0;
    bool found = false;

    /* is there even space available? */
    if (tsm_transaction_available()) {
        while (!found) {
            index = tsm_find_invokeID_index(Current_Invoke_ID);
            if (index == MAX_TSM_TRANSACTIONS) {
                /* Not found, so this invokeID is not used */
                found = true;
                /* set this id into the table */
                index = tsm_find_first_free_index();
                if (index != MAX_TSM_TRANSACTIONS) {
                    TSM_List[index].InvokeID = invokeID = Current_Invoke_ID;
                    TSM_List[index].state = TSM_STATE_IDLE;
                    TSM_List[index].RequestTimer = apdu_timeout();
                    /* update for the next call or check */
                    Current_Invoke_ID++;
                    /* skip zero - we treat that internally as invalid or no free */
                    if (Current_Invoke_ID == 0) {
                        Current_Invoke_ID = 1;
                    }
                }
            } else {
                /* found! This invokeID is already used */
                /* try next one */
                Current_Invoke_ID++;
                /* skip zero - we treat that internally as invalid or no free */
                if (Current_Invoke_ID == 0) {
                    Current_Invoke_ID = 1;
                }
            }
        }
    }

    return invokeID;
}

void tsm_set_confirmed_unsegmented_transaction(
    uint8_t invokeID,
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * ndpu_data,
    uint8_t * apdu,
    uint16_t apdu_len)
{
    uint16_t j = 0;
    uint8_t index;

    if (invokeID) {
        index = tsm_find_invokeID_index(invokeID);
        if (index < MAX_TSM_TRANSACTIONS) {
            /* SendConfirmedUnsegmented */
            TSM_List[index].state = TSM_STATE_AWAIT_CONFIRMATION;
            TSM_List[index].RetryCount = 0;
            /* start the timer */
            TSM_List[index].RequestTimer = apdu_timeout();
            /* copy the data */
            for (j = 0; j < apdu_len; j++) {
                TSM_List[index].apdu[j] = apdu[j];
            }
            TSM_List[index].apdu_len = apdu_len;
            npdu_copy_data(&TSM_List[index].npdu_data, ndpu_data);
            bacnet_address_copy(&TSM_List[index].dest, dest);
        }
    }

    return;
}

/* used to retrieve the transaction payload */
/* if we wanted to find out what we sent (i.e. when we get an ack) */
bool tsm_get_transaction_pdu(
    uint8_t invokeID,
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * ndpu_data,
    uint8_t * apdu,
    uint16_t * apdu_len)
{
    uint16_t j = 0;
    uint8_t index;
    bool found = false;

    if (invokeID) {
        index = tsm_find_invokeID_index(invokeID);
        /* how much checking is needed?  state?  dest match? just invokeID? */
        if (index < MAX_TSM_TRANSACTIONS) {
            /* FIXME: we may want to free the transaction so it doesn't timeout */
            /* retrieve the transaction */
            /* FIXME: bounds check the pdu_len? */
            *apdu_len = (uint16_t) TSM_List[index].apdu_len;
            for (j = 0; j < *apdu_len; j++) {
                apdu[j] = TSM_List[index].apdu[j];
            }
            npdu_copy_data(ndpu_data, &TSM_List[index].npdu_data);
            bacnet_address_copy(dest, &TSM_List[index].dest);
            found = true;
        }
    }

    return found;
}

/* called once a millisecond or slower */
void tsm_timer_milliseconds(
    uint16_t milliseconds)
{
    unsigned i = 0;     /* counter */

    for (i = 0; i < MAX_TSM_TRANSACTIONS; i++) {
        if (TSM_List[i].state == TSM_STATE_AWAIT_CONFIRMATION) {
            if (TSM_List[i].RequestTimer > milliseconds)
                TSM_List[i].RequestTimer -= milliseconds;
            else
                TSM_List[i].RequestTimer = 0;
            /* AWAIT_CONFIRMATION */
            if (TSM_List[i].RequestTimer == 0) {
                if (TSM_List[i].RetryCount < apdu_retries()) {
                    TSM_List[i].RequestTimer = apdu_timeout();
                    TSM_List[i].RetryCount++;
                    datalink_send_pdu(&TSM_List[i].dest,
                        &TSM_List[i].npdu_data, &TSM_List[i].apdu[0],
                        TSM_List[i].apdu_len);
                } else {
                    /* note: the invoke id has not been cleared yet
                       and this indicates a failed message:
                       IDLE and a valid invoke id */
                    TSM_List[i].state = TSM_STATE_IDLE;
                    if (TSM_List[i].InvokeID != 0) {
                        if (Timeout_Function) {
                            Timeout_Function(TSM_List[i].InvokeID);
                        }
                    }
                }
            }
        }
    }
}

/* frees the invokeID and sets its state to IDLE */
void tsm_free_invoke_id(
    uint8_t invokeID)
{
    uint8_t index;

    index = tsm_find_invokeID_index(invokeID);
    if (index < MAX_TSM_TRANSACTIONS) {
        TSM_List[index].state = TSM_STATE_IDLE;
        TSM_List[index].InvokeID = 0;
    }
}

/** Check if the invoke ID has been made free by the Transaction State Machine.
 * @param invokeID [in] The invokeID to be checked, normally of last message sent.
 * @return True if it is free (done with), False if still pending in the TSM.
 */
bool tsm_invoke_id_free(
    uint8_t invokeID)
{
    bool status = true;
    uint8_t index;

    index = tsm_find_invokeID_index(invokeID);
    if (index < MAX_TSM_TRANSACTIONS)
        status = false;

    return status;
}

/** See if we failed get a confirmation for the message associated
 *  with this invoke ID.
 * @param invokeID [in] The invokeID to be checked, normally of last message sent.
 * @return True if already failed, False if done or segmented or still waiting
 *         for a confirmation.
 */
bool tsm_invoke_id_failed(
    uint8_t invokeID)
{
    bool status = false;
    uint8_t index;

    index = tsm_find_invokeID_index(invokeID);
    if (index < MAX_TSM_TRANSACTIONS) {
        /* a valid invoke ID and the state is IDLE is a
           message that failed to confirm */
        if (TSM_List[index].state == TSM_STATE_IDLE)
            status = true;
    }

    return status;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* flag to send an I-Am */
bool I_Am_Request = true;

/* dummy function stubs */
int datalink_send_pdu(
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * npdu_data,
    uint8_t * pdu,
    unsigned pdu_len)
{
    (void) dest;
    (void) npdu_data;
    (void) pdu;
    (void) pdu_len;

    return 0;
}

/* dummy function stubs */
void datalink_get_broadcast_address(
    BACNET_ADDRESS * dest)
{
    (void) dest;
}

void testTSM(
    Test * pTest)
{
    /* FIXME: add some unit testing... */
    return;
}

#ifdef TEST_TSM
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet TSM", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testTSM);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_TSM */
#endif /* TEST */
#endif /* MAX_TSM_TRANSACTIONS */
