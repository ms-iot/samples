/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

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
#include "awf.h"

/** @file awf.c  Atomic Write File */

/* encode service */
int awf_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    uint32_t i = 0;

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_ATOMIC_WRITE_FILE;  /* service choice */
        apdu_len = 4;
        apdu_len +=
            encode_application_object_id(&apdu[apdu_len], data->object_type,
            data->object_instance);
        switch (data->access) {
            case FILE_STREAM_ACCESS:
                apdu_len += encode_opening_tag(&apdu[apdu_len], 0);
                apdu_len +=
                    encode_application_signed(&apdu[apdu_len],
                    data->type.stream.fileStartPosition);
                apdu_len +=
                    encode_application_octet_string(&apdu[apdu_len],
                    &data->fileData[0]);
                apdu_len += encode_closing_tag(&apdu[apdu_len], 0);
                break;
            case FILE_RECORD_ACCESS:
                apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
                apdu_len +=
                    encode_application_signed(&apdu[apdu_len],
                    data->type.record.fileStartRecord);
                apdu_len +=
                    encode_application_unsigned(&apdu[apdu_len],
                    data->type.record.returnedRecordCount);
                for (i = 0; i < data->type.record.returnedRecordCount; i++) {
                    apdu_len +=
                        encode_application_octet_string(&apdu[apdu_len],
                        &data->fileData[i]);
                }
                apdu_len += encode_closing_tag(&apdu[apdu_len], 1);
                break;
            default:
                break;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int awf_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    int len = 0;
    int tag_len = 0;
    int decoded_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    int32_t signed_value = 0;
    uint32_t unsigned_value = 0;
    uint16_t type = 0;  /* for decoding */
    uint32_t i = 0;

    /* check for value pointers */
    if (apdu_len && data) {
        len =
            decode_tag_number_and_value(&apdu[0], &tag_number,
            &len_value_type);
        if (tag_number != BACNET_APPLICATION_TAG_OBJECT_ID)
            return -1;
        len += decode_object_id(&apdu[len], &type, &data->object_instance);
        data->object_type = (BACNET_OBJECT_TYPE) type;
        if (decode_is_opening_tag_number(&apdu[len], 0)) {
            data->access = FILE_STREAM_ACCESS;
            /* a tag number of 2 is not extended so only one octet */
            len++;
            /* fileStartPosition */
            tag_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_SIGNED_INT)
                return -1;
            len += decode_signed(&apdu[len], len_value_type, &signed_value);
            data->type.stream.fileStartPosition = signed_value;
            /* fileData */
            tag_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING)
                return -1;
            decoded_len =
                decode_octet_string(&apdu[len], len_value_type,
                &data->fileData[0]);
            if (decoded_len != len_value_type) {
                return -1;
            }
            len += decoded_len;
            if (!decode_is_closing_tag_number(&apdu[len], 0))
                return -1;
            /* a tag number is not extended so only one octet */
            len++;
        } else if (decode_is_opening_tag_number(&apdu[len], 1)) {
            data->access = FILE_RECORD_ACCESS;
            /* a tag number is not extended so only one octet */
            len++;
            /* fileStartRecord */
            tag_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_SIGNED_INT)
                return -1;
            len += decode_signed(&apdu[len], len_value_type, &signed_value);
            data->type.record.fileStartRecord = signed_value;
            /* returnedRecordCount */
            tag_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += tag_len;
            if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT)
                return -1;
            len +=
                decode_unsigned(&apdu[len], len_value_type, &unsigned_value);
            data->type.record.returnedRecordCount = unsigned_value;
            /* fileData */
            for (i = 0; i < data->type.record.returnedRecordCount; i++) {
                tag_len =
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value_type);
                len += tag_len;
                if (tag_number != BACNET_APPLICATION_TAG_OCTET_STRING)
                    return -1;
                decoded_len =
                    decode_octet_string(&apdu[len], len_value_type,
                    &data->fileData[i]);
                if (decoded_len != len_value_type) {
                    return -1;
                }
                len += decoded_len;
            }
            if (!decode_is_closing_tag_number(&apdu[len], 1))
                return -1;
            /* a tag number is not extended so only one octet */
            len++;
        } else
            return -1;
    }

    return len;
}

int awf_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -1;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_ATOMIC_WRITE_FILE)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            awf_decode_service_request(&apdu[offset], apdu_len - offset, data);
    }

    return len;
}

int awf_ack_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_COMPLEX_ACK;
        apdu[1] = invoke_id;
        apdu[2] = SERVICE_CONFIRMED_ATOMIC_WRITE_FILE;  /* service choice */
        apdu_len = 3;
        switch (data->access) {
            case FILE_STREAM_ACCESS:
                apdu_len +=
                    encode_context_signed(&apdu[apdu_len], 0,
                    data->type.stream.fileStartPosition);
                break;
            case FILE_RECORD_ACCESS:
                apdu_len +=
                    encode_context_signed(&apdu[apdu_len], 1,
                    data->type.record.fileStartRecord);
                break;
            default:
                break;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int awf_ack_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    int len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;

    /* check for value pointers */
    if (apdu_len && data) {
        len =
            decode_tag_number_and_value(&apdu[0], &tag_number,
            &len_value_type);
        if (tag_number == 0) {
            data->access = FILE_STREAM_ACCESS;
            len +=
                decode_signed(&apdu[len], len_value_type,
                &data->type.stream.fileStartPosition);
        } else if (tag_number == 1) {
            data->access = FILE_RECORD_ACCESS;
            len +=
                decode_signed(&apdu[len], len_value_type,
                &data->type.record.fileStartRecord);
        } else
            return -1;
    }

    return len;
}

int awf_ack_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_COMPLEX_ACK)
        return -1;
    *invoke_id = apdu[1];       /* invoke id - filled in by net layer */
    if (apdu[2] != SERVICE_CONFIRMED_ATOMIC_WRITE_FILE)
        return -1;
    offset = 3;

    if (apdu_len > offset) {
        len =
            awf_ack_decode_service_request(&apdu[offset], apdu_len - offset,
            data);
    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testAtomicWriteFileAccess(
    Test * pTest,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    BACNET_ATOMIC_WRITE_FILE_DATA test_data = { 0 };
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;

    len = awf_encode_apdu(&apdu[0], invoke_id, data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len = awf_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.object_type == data->object_type);
    ct_test(pTest, test_data.object_instance == data->object_instance);
    ct_test(pTest, test_data.access == data->access);
    if (test_data.access == FILE_STREAM_ACCESS) {
        ct_test(pTest,
            test_data.type.stream.fileStartPosition ==
            data->type.stream.fileStartPosition);
    } else if (test_data.access == FILE_RECORD_ACCESS) {
        ct_test(pTest,
            test_data.type.record.fileStartRecord ==
            data->type.record.fileStartRecord);
        ct_test(pTest,
            test_data.type.record.returnedRecordCount ==
            data->type.record.returnedRecordCount);
    }
    ct_test(pTest,
        octetstring_length(&test_data.fileData[0]) ==
        octetstring_length(&data->fileData[0]));
    ct_test(pTest, memcmp(octetstring_value(&test_data.fileData[0]),
            octetstring_value(&data->fileData[0]),
            octetstring_length(&test_data.fileData[0])) == 0);
}

void testAtomicWriteFile(
    Test * pTest)
{
    BACNET_ATOMIC_WRITE_FILE_DATA data = { 0 };
    uint8_t test_octet_string[32] = "Joshua-Mary-Anna-Christopher";

    data.object_type = OBJECT_FILE;
    data.object_instance = 1;
    data.access = FILE_STREAM_ACCESS;
    data.type.stream.fileStartPosition = 0;
    octetstring_init(&data.fileData[0], test_octet_string,
        sizeof(test_octet_string));
    testAtomicWriteFileAccess(pTest, &data);

    data.object_type = OBJECT_FILE;
    data.object_instance = 1;
    data.access = FILE_RECORD_ACCESS;
    data.type.record.fileStartRecord = 1;
    data.type.record.returnedRecordCount = 1;
    octetstring_init(&data.fileData[0], test_octet_string,
        sizeof(test_octet_string));
    testAtomicWriteFileAccess(pTest, &data);

    return;
}

void testAtomicWriteFileAckAccess(
    Test * pTest,
    BACNET_ATOMIC_WRITE_FILE_DATA * data)
{
    BACNET_ATOMIC_WRITE_FILE_DATA test_data = { 0 };
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;

    len = awf_encode_apdu(&apdu[0], invoke_id, data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len = awf_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.access == data->access);
    if (test_data.access == FILE_STREAM_ACCESS) {
        ct_test(pTest,
            test_data.type.stream.fileStartPosition ==
            data->type.stream.fileStartPosition);
    } else if (test_data.access == FILE_RECORD_ACCESS) {
        ct_test(pTest,
            test_data.type.record.fileStartRecord ==
            data->type.record.fileStartRecord);
    }
}

void testAtomicWriteFileAck(
    Test * pTest)
{
    BACNET_ATOMIC_WRITE_FILE_DATA data = { 0 };

    data.access = FILE_STREAM_ACCESS;
    data.type.stream.fileStartPosition = 42;
    testAtomicWriteFileAckAccess(pTest, &data);

    data.access = FILE_RECORD_ACCESS;
    data.type.record.fileStartRecord = 54;
    testAtomicWriteFileAckAccess(pTest, &data);

    return;
}

#ifdef TEST_ATOMIC_WRITE_FILE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet AtomicWriteFile", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAtomicWriteFile);
    assert(rc);
    rc = ct_addTestFunction(pTest, testAtomicWriteFileAck);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_WRITE_PROPERTY */
#endif /* TEST */
