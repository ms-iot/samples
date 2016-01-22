/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2006 Steve Karg

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
#include <stdint.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "bacapp.h"
#include "timesync.h"

/** @file timesync.c  Encode/Decode TimeSync APDUs  */
#if BACNET_SVC_TS_A
/* encode service */
int timesync_encode_apdu_service(
    uint8_t * apdu,
    BACNET_UNCONFIRMED_SERVICE service,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && my_date && my_time) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = service;
        apdu_len = 2;
        len = encode_application_date(&apdu[apdu_len], my_date);
        apdu_len += len;
        len = encode_application_time(&apdu[apdu_len], my_time);
        apdu_len += len;
    }

    return apdu_len;
}

int timesync_utc_encode_apdu(
    uint8_t * apdu,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    return timesync_encode_apdu_service(apdu,
        SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION, my_date, my_time);
}

int timesync_encode_apdu(
    uint8_t * apdu,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    return timesync_encode_apdu_service(apdu,
        SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION, my_date, my_time);
}
#endif

/* decode the service request only */
int timesync_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    int len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;

    if (apdu_len && my_date && my_time) {
        /* date */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        if (tag_number == BACNET_APPLICATION_TAG_DATE) {
            len += decode_date(&apdu[len], my_date);
        } else
            return -1;
        /* time */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        if (tag_number == BACNET_APPLICATION_TAG_TIME) {
            len += decode_bacnet_time(&apdu[len], my_time);
        } else
            return -1;
    }

    return len;
}

/** Handle a request to encode the list of timesync recipients.
 *
 *  Invoked by a request to read the Device object's
 *  PROP_TIME_SYNCHRONIZATION_RECIPIENTS.
 *  Loops through the list of timesync recipients, and, for each one,
 *  adds its data to the APDU.
 *
 *   BACnetRecipient ::= CHOICE {
 *       device [0] BACnetObjectIdentifier,
 *       address [1] BACnetAddress
 *   }
 *
 *   BACnetAddress ::= SEQUENCE {
 *       network-number Unsigned16, -- A value of 0 indicates the local network
 *       mac-address OCTET STRING -- A string of length 0 indicates a broadcast
 *   }
 *
 *  @param apdu [out] Buffer in which the APDU contents are built.
 *  @param max_apdu [in] Max length of the APDU buffer.
 *  @param recipient [in] BACNET_RECIPIENT_LIST type linked list of recipients.
 *
 *  @return How many bytes were encoded in the buffer, or
 *   BACNET_STATUS_ABORT if the response would not fit within the buffer.
 */
int timesync_encode_timesync_recipients(
    uint8_t * apdu,
    unsigned max_apdu,
    BACNET_RECIPIENT_LIST * recipient)
{
    int len = 0;
    int apdu_len = 0;
    BACNET_OCTET_STRING octet_string;
    BACNET_RECIPIENT_LIST *pRecipient;

    pRecipient = recipient;
    while (pRecipient != NULL) {
        if (pRecipient->tag == 0) {
            if (max_apdu >= (1 + 4)) {
                /* CHOICE - device [0] BACnetObjectIdentifier */
                len =
                    encode_context_object_id(&apdu[apdu_len], 0,
                    pRecipient->type.device.type,
                    pRecipient->type.device.instance);
                apdu_len += len;
            } else {
                return BACNET_STATUS_ABORT;
            }
        } else if (pRecipient->tag == 1) {
            if (pRecipient->type.address.net) {
                len = 1 + 3 + 2 + pRecipient->type.address.len + 1;
            } else {
                len = 1 + 3 + 2 + pRecipient->type.address.mac_len + 1;
            }
            if (max_apdu >= (unsigned)len) {
                /* CHOICE - address [1] BACnetAddress - opening */
                len = encode_opening_tag(&apdu[apdu_len], 1);
                apdu_len += len;
                /* network-number Unsigned16, */
                /* -- A value of 0 indicates the local network */
                len =
                    encode_application_unsigned(&apdu[apdu_len],
                    pRecipient->type.address.net);
                apdu_len += len;
                /* mac-address OCTET STRING */
                /* -- A string of length 0 indicates a broadcast */
                if (pRecipient->type.address.net == BACNET_BROADCAST_NETWORK) {
                    octetstring_init(&octet_string, NULL, 0);
                } else if (pRecipient->type.address.net) {
                    octetstring_init(&octet_string,
                        &pRecipient->type.address.adr[0],
                        pRecipient->type.address.len);
                } else {
                    octetstring_init(&octet_string,
                        &pRecipient->type.address.mac[0],
                        pRecipient->type.address.mac_len);
                }
                len =
                    encode_application_octet_string(&apdu[apdu_len],
                    &octet_string);
                apdu_len += len;
                /* CHOICE - address [1] BACnetAddress - closing */
                len = encode_closing_tag(&apdu[apdu_len], 1);
                apdu_len += len;
            } else {
                /* not a valid tag - don't encode this one */
            }
        }
        pRecipient = pRecipient->next;
    }

    return apdu_len;
}

/** Handle a request to decode a list of timesync recipients.
 *
 *  Invoked by a request to write the Device object's
 *  PROP_TIME_SYNCHRONIZATION_RECIPIENTS.
 *  Loops through the list of timesync recipients, and, for each one,
 *  adds its data from the APDU.
 *
 *   BACnetRecipient ::= CHOICE {
 *       device [0] BACnetObjectIdentifier,
 *       address [1] BACnetAddress
 *   }
 *
 *   BACnetAddress ::= SEQUENCE {
 *       network-number Unsigned16, -- A value of 0 indicates the local network
 *       mac-address OCTET STRING -- A string of length 0 indicates a broadcast
 *   }
 *
 *  @param apdu [in] Buffer in which the APDU contents are read
 *  @param max_apdu [in] length of the APDU buffer.
 *  @param recipient [out] BACNET_RECIPIENT_LIST type linked list of recipients.
 *
 *  @return How many bytes were decoded from the buffer, or
 *   BACNET_STATUS_ABORT if there was a problem decoding the buffer
 */
int timesync_decode_timesync_recipients(
    uint8_t * apdu,
    unsigned max_apdu,
    BACNET_RECIPIENT_LIST * recipient)
{
    int len = 0;
    int apdu_len = 0;
    int tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t unsigned_value = 0;
    BACNET_OCTET_STRING octet_string;
    BACNET_RECIPIENT_LIST *pRecipient;

    pRecipient = recipient;
    while (pRecipient != NULL) {
        /* device [0] BACnetObjectIdentifier */
        if (decode_is_context_tag(&apdu[apdu_len], 0)) {
            pRecipient->tag = 0;
            len =
                decode_context_object_id(&apdu[apdu_len], 0,
                &pRecipient->type.device.type,
                &pRecipient->type.device.instance);
            if (len < 0) {
                return BACNET_STATUS_ABORT;
            }
            apdu_len += len;
        } else if (decode_is_context_tag(&apdu[apdu_len], 1)) {
            apdu_len += 1;
            pRecipient->tag = 1;
            /* network-number Unsigned16 */
            tag_len =
                decode_tag_number_and_value(&apdu[apdu_len], &tag_number,
                &len_value_type);
            apdu_len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT) {
                return BACNET_STATUS_ABORT;
            }
            len =
                decode_unsigned(&apdu[apdu_len], len_value_type,
                &unsigned_value);
            pRecipient->type.address.net = unsigned_value;
            apdu_len += len;
            /* mac-address OCTET STRING */
            tag_len =
                decode_tag_number_and_value(&apdu[apdu_len], &tag_number,
                &len_value_type);
            apdu_len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING) {
                return BACNET_STATUS_ABORT;
            }
            len = decode_octet_string(&apdu[0], len_value_type, &octet_string);
            apdu_len += len;
            if (octetstring_length(&octet_string) == 0) {
                /* -- A string of length 0 indicates a broadcast */
            } else if (pRecipient->type.address.net) {
                pRecipient->type.address.len =
                    octetstring_copy_value(&pRecipient->type.address.adr[0],
                    sizeof(pRecipient->type.address.adr), &octet_string);
            } else {
                pRecipient->type.address.mac_len =
                    octetstring_copy_value(&pRecipient->type.address.mac[0],
                    sizeof(pRecipient->type.address.mac), &octet_string);
            }
        } else {
            return BACNET_STATUS_ABORT;
        }
        pRecipient = pRecipient->next;
    }

    return apdu_len;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testTimeSyncRecipientData(
    Test * pTest,
    BACNET_RECIPIENT_LIST * recipient1,
    BACNET_RECIPIENT_LIST * recipient2)
{
    unsigned i = 0;

    if (recipient1 && recipient2) {
        ct_test(pTest, recipient1->tag == recipient2->tag);
        if (recipient1->tag == 0) {
            ct_test(pTest,
                recipient1->type.device.type == recipient2->type.device.type);
            ct_test(pTest,
                recipient1->type.device.instance ==
                recipient2->type.device.instance);
        } else if (recipient1->tag == 1) {
            ct_test(pTest,
                recipient1->type.address.net == recipient2->type.address.net);
            if (recipient1->type.address.net == BACNET_BROADCAST_NETWORK) {
                ct_test(pTest,
                    recipient1->type.address.mac_len ==
                    recipient2->type.address.mac_len);
            } else if (recipient1->type.address.net) {
                ct_test(pTest,
                    recipient1->type.address.len ==
                    recipient2->type.address.len);
                for (i = 0; i < recipient1->type.address.len; i++) {
                    ct_test(pTest,
                        recipient1->type.address.adr[i] ==
                        recipient2->type.address.adr[i]);
                }
            } else {
                ct_test(pTest,
                    recipient1->type.address.mac_len ==
                    recipient2->type.address.mac_len);
                for (i = 0; i < recipient1->type.address.mac_len; i++) {
                    ct_test(pTest,
                        recipient1->type.address.mac[i] ==
                        recipient2->type.address.mac[i]);
                }
            }
        } else {
            ct_test(pTest, recipient1->tag <= 1);
        }
    }
}

void testTimeSyncRecipient(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    BACNET_RECIPIENT_LIST recipient[4];
    BACNET_RECIPIENT_LIST test_recipient[4];

    /* link the recipient list */
    recipient[0].next = &recipient[1];
    recipient[1].next = &recipient[2];
    recipient[2].next = &recipient[3];
    recipient[3].next = NULL;
    /* link the test recipient list */
    test_recipient[0].next = &test_recipient[1];
    test_recipient[1].next = &test_recipient[2];
    test_recipient[2].next = &test_recipient[3];
    test_recipient[3].next = NULL;
    /* load the test data - device */
    recipient[0].tag = 0;
    recipient[0].type.device.type = OBJECT_DEVICE;
    recipient[0].type.device.instance = 1234;
    /* load the test data - address */
    /* network = broadcast */
    recipient[1].tag = 1;
    recipient[1].type.address.net = BACNET_BROADCAST_NETWORK;
    recipient[2].type.address.mac_len = 0;
    /* network = non-zero */
    recipient[1].tag = 1;
    recipient[2].type.address.net = 4201;
    recipient[2].type.address.adr[0] = 127;
    recipient[2].type.address.len = 1;
    /* network = zero */
    recipient[2].type.address.net = 0;
    recipient[2].type.address.mac[0] = 10;
    recipient[2].type.address.mac[1] = 1;
    recipient[2].type.address.mac[2] = 0;
    recipient[2].type.address.mac[3] = 86;
    recipient[2].type.address.mac[4] = 0xBA;
    recipient[2].type.address.mac[5] = 0xC1;
    recipient[2].type.address.mac_len = 6;
    /* perform positive test */
    len =
        timesync_encode_timesync_recipients(&apdu[0], sizeof(apdu),
        &recipient[0]);
    ct_test(pTest, len != BACNET_STATUS_ABORT);
    ct_test(pTest, len > 0);
    len =
        timesync_decode_timesync_recipients(&apdu[0], sizeof(apdu),
        &test_recipient[0]);
    ct_test(pTest, len != BACNET_STATUS_ABORT);
    ct_test(pTest, len > 0);
    testTimeSyncRecipientData(pTest, &recipient[0], &test_recipient[0]);
}

int timesync_decode_apdu_service(
    uint8_t * apdu,
    BACNET_UNCONFIRMED_SERVICE service,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    int len = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST)
        return -1;
    if (apdu[1] != service)
        return -1;
    /* optional limits - must be used as a pair */
    if (apdu_len > 2) {
        len =
            timesync_decode_service_request(&apdu[2], apdu_len - 2, my_date,
            my_time);
    }

    return len;
}

int timesync_utc_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    return timesync_decode_apdu_service(apdu,
        SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION, apdu_len, my_date,
        my_time);
}

int timesync_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    return timesync_decode_apdu_service(apdu,
        SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION, apdu_len, my_date, my_time);
}

void testTimeSyncData(
    Test * pTest,
    BACNET_DATE * my_date,
    BACNET_TIME * my_time)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_DATE test_date;
    BACNET_TIME test_time;

    len = timesync_encode_apdu(&apdu[0], my_date, my_time);
    ct_test(pTest, len != 0);
    apdu_len = len;
    len = timesync_decode_apdu(&apdu[0], apdu_len, &test_date, &test_time);
    ct_test(pTest, len != -1);
    ct_test(pTest, datetime_compare_time(my_time, &test_time) == 0);
    ct_test(pTest, datetime_compare_date(my_date, &test_date) == 0);

    len = timesync_utc_encode_apdu(&apdu[0], my_date, my_time);
    ct_test(pTest, len != 0);
    apdu_len = len;
    len = timesync_utc_decode_apdu(&apdu[0], apdu_len, &test_date, &test_time);
    ct_test(pTest, len != -1);
    ct_test(pTest, datetime_compare_time(my_time, &test_time) == 0);
    ct_test(pTest, datetime_compare_date(my_date, &test_date) == 0);
}

void testTimeSync(
    Test * pTest)
{
    BACNET_DATE bdate;
    BACNET_TIME btime;

    bdate.year = 2006;  /* AD */
    bdate.month = 4;    /* 1=Jan */
    bdate.day = 11;     /* 1..31 */
    bdate.wday = 1;     /* 1=Monday */

    btime.hour = 7;
    btime.min = 0;
    btime.sec = 3;
    btime.hundredths = 1;

    testTimeSyncData(pTest, &bdate, &btime);
}

#ifdef TEST_TIMESYNC
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Time-Sync", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testTimeSync);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_WHOIS */
#endif /* TEST */
