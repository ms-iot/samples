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
#include "bacerror.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "bacapp.h"
#include "memcopy.h"
#include "rpm.h"

/** @file rpm.c  Encode/Decode Read Property Multiple and RPM ACKs  */

#if BACNET_SVC_RPM_A
/* encode the initial portion of the service */
int rpm_encode_apdu_init(
    uint8_t * apdu,
    uint8_t invoke_id)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_READ_PROP_MULTIPLE; /* service choice */
        apdu_len = 4;
    }

    return apdu_len;
}

int rpm_encode_apdu_object_begin(
    uint8_t * apdu,
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu_len =
            encode_context_object_id(&apdu[0], 0, object_type,
            object_instance);
        /* Tag 1: sequence of ReadAccessSpecification */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
    }

    return apdu_len;
}

int rpm_encode_apdu_object_property(
    uint8_t * apdu,
    BACNET_PROPERTY_ID object_property,
    uint32_t array_index)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu_len = encode_context_enumerated(&apdu[0], 0, object_property);
        /* optional array index */
        if (array_index != BACNET_ARRAY_ALL)
            apdu_len +=
                encode_context_unsigned(&apdu[apdu_len], 1, array_index);
    }

    return apdu_len;
}

int rpm_encode_apdu_object_end(
    uint8_t * apdu)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu_len = encode_closing_tag(&apdu[0], 1);
    }

    return apdu_len;
}

/** Encode an RPM request, to be sent.
 *
 * @param apdu [in,out] Buffer to hold encoded bytes.
 * @param max_apdu [in] Length of apdu buffer.
 * @param invoke_id [in] The Invoke ID to use for this message.
 * @param read_access_data [in] The RPM data to be requested.
 * @return Length of encoded bytes, or 0 on failure.
 */
int rpm_encode_apdu(
    uint8_t * apdu,
    size_t max_apdu,
    uint8_t invoke_id,
    BACNET_READ_ACCESS_DATA * read_access_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;        /* length of the data */
    BACNET_READ_ACCESS_DATA *rpm_object;        /* current object */
    uint8_t apdu_temp[16];      /* temp for data before copy */
    BACNET_PROPERTY_REFERENCE *rpm_property;    /* current property */

    len = rpm_encode_apdu_init(&apdu_temp[0], invoke_id);
    len =
        (int) memcopy(&apdu[0], &apdu_temp[0], (size_t) apdu_len, (size_t) len,
        (size_t) max_apdu);
    if (len == 0) {
        return 0;
    }
    apdu_len += len;
    rpm_object = read_access_data;
    while (rpm_object) {
        len =
            encode_context_object_id(&apdu_temp[0], 0, rpm_object->object_type,
            rpm_object->object_instance);
        len =
            (int) memcopy(&apdu[0], &apdu_temp[0], (size_t) apdu_len,
            (size_t) len, (size_t) max_apdu);
        if (len == 0) {
            return 0;
        }
        apdu_len += len;
        /* Tag 1: sequence of ReadAccessSpecification */
        len = encode_opening_tag(&apdu_temp[0], 1);
        len =
            (int) memcopy(&apdu[0], &apdu_temp[0], (size_t) apdu_len,
            (size_t) len, (size_t) max_apdu);
        if (len == 0) {
            return 0;
        }
        apdu_len += len;
        rpm_property = rpm_object->listOfProperties;
        while (rpm_property) {
            /* stuff as many properties into it as APDU length will allow */
            len =
                encode_context_enumerated(&apdu_temp[0], 0,
                rpm_property->propertyIdentifier);
            len =
                (int) memcopy(&apdu[0], &apdu_temp[0], (size_t) apdu_len,
                (size_t) len, (size_t) max_apdu);
            if (len == 0) {
                return 0;
            }
            apdu_len += len;
            /* optional array index */
            if (rpm_property->propertyArrayIndex != BACNET_ARRAY_ALL) {
                len =
                    encode_context_unsigned(&apdu_temp[0], 1,
                    rpm_property->propertyArrayIndex);
                len =
                    (int) memcopy(&apdu[0], &apdu_temp[0], (size_t) apdu_len,
                    (size_t) len, (size_t) max_apdu);
                if (len == 0) {
                    return 0;
                }
                apdu_len += len;
            }
            rpm_property = rpm_property->next;
        }
        len = encode_closing_tag(&apdu_temp[0], 1);
        len =
            (int) memcopy(&apdu[0], &apdu_temp[0], (size_t) apdu_len,
            (size_t) len, (size_t) max_apdu);
        if (len == 0) {
            return 0;
        }
        apdu_len += len;
        rpm_object = rpm_object->next;
    }

    return apdu_len;
}

#endif

/* decode the object portion of the service request only. Bails out if
 * tags are wrong or missing/incomplete
 */
int rpm_decode_object_id(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_RPM_DATA * rpmdata)
{
    unsigned len = 0;
    uint16_t type = 0;  /* for decoding */

    /* check for value pointers */
    if (apdu && apdu_len && rpmdata) {
        if (apdu_len < 5) {     /* Must be at least 2 tags and an object id */
            rpmdata->error_code = ERROR_CODE_REJECT_MISSING_REQUIRED_PARAMETER;
            return BACNET_STATUS_REJECT;
        }
        /* Tag 0: Object ID */
        if (!decode_is_context_tag(&apdu[len++], 0)) {
            rpmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        len += decode_object_id(&apdu[len], &type, &rpmdata->object_instance);
        rpmdata->object_type = (BACNET_OBJECT_TYPE) type;
        /* Tag 1: sequence of ReadAccessSpecification */
        if (!decode_is_opening_tag_number(&apdu[len], 1)) {
            rpmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        len++;  /* opening tag is only one octet */
    }

    return (int) len;
}

int rpm_decode_object_end(
    uint8_t * apdu,
    unsigned apdu_len)
{
    int len = 0;        /* total length of the apdu, return value */

    if (apdu && apdu_len) {
        if (decode_is_closing_tag_number(apdu, 1) == true)
            len = 1;
    }

    return len;
}

/* decode the object property portion of the service request only */
/*  BACnetPropertyReference ::= SEQUENCE {
        propertyIdentifier [0] BACnetPropertyIdentifier,
        propertyArrayIndex [1] Unsigned OPTIONAL
        --used only with array datatype
        -- if omitted with an array the entire array is referenced
    }
*/
int rpm_decode_object_property(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_RPM_DATA * rpmdata)
{
    unsigned len = 0;
    unsigned option_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t property = 0;      /* for decoding */
    uint32_t array_value = 0;   /* for decoding */

    /* check for valid pointers */
    if (apdu && apdu_len && rpmdata) {
        /* Tag 0: propertyIdentifier */
        if (!IS_CONTEXT_SPECIFIC(apdu[len])) {
            rpmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }

        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        if (tag_number != 0) {
            rpmdata->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* Should be at least the unsigned value + 1 tag left */
        if ((len + len_value_type) >= apdu_len) {
            rpmdata->error_code = ERROR_CODE_REJECT_MISSING_REQUIRED_PARAMETER;
            return BACNET_STATUS_REJECT;
        }
        len += decode_enumerated(&apdu[len], len_value_type, &property);
        rpmdata->object_property = (BACNET_PROPERTY_ID) property;
        /* Assume most probable outcome */
        rpmdata->array_index = BACNET_ARRAY_ALL;
        /* Tag 1: Optional propertyArrayIndex */
        if (IS_CONTEXT_SPECIFIC(apdu[len]) && !IS_CLOSING_TAG(apdu[len])) {
            option_len =
                (unsigned) decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            if (tag_number == 1) {
                len += option_len;
                /* Should be at least the unsigned array index + 1 tag left */
                if ((len + len_value_type) >= apdu_len) {
                    rpmdata->error_code =
                        ERROR_CODE_REJECT_MISSING_REQUIRED_PARAMETER;
                    return BACNET_STATUS_REJECT;
                }
                len +=
                    decode_unsigned(&apdu[len], len_value_type, &array_value);
                rpmdata->array_index = array_value;
            }
        }
    }

    return (int) len;
}

int rpm_ack_encode_apdu_init(
    uint8_t * apdu,
    uint8_t invoke_id)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_COMPLEX_ACK; /* complex ACK service */
        apdu[1] = invoke_id;    /* original invoke id from request */
        apdu[2] = SERVICE_CONFIRMED_READ_PROP_MULTIPLE; /* service choice */
        apdu_len = 3;
    }

    return apdu_len;
}

int rpm_ack_encode_apdu_object_begin(
    uint8_t * apdu,
    BACNET_RPM_DATA * rpmdata)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        /* Tag 0: objectIdentifier */
        apdu_len =
            encode_context_object_id(&apdu[0], 0, rpmdata->object_type,
            rpmdata->object_instance);
        /* Tag 1: listOfResults */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
    }

    return apdu_len;
}

int rpm_ack_encode_apdu_object_property(
    uint8_t * apdu,
    BACNET_PROPERTY_ID object_property,
    uint32_t array_index)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        /* Tag 2: propertyIdentifier */
        apdu_len = encode_context_enumerated(&apdu[0], 2, object_property);
        /* Tag 3: optional propertyArrayIndex */
        if (array_index != BACNET_ARRAY_ALL)
            apdu_len +=
                encode_context_unsigned(&apdu[apdu_len], 3, array_index);
    }

    return apdu_len;
}

int rpm_ack_encode_apdu_object_property_value(
    uint8_t * apdu,
    uint8_t * application_data,
    unsigned application_data_len)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    unsigned len = 0;

    if (apdu) {
        /* Tag 4: propertyValue */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 4);
        if (application_data == &apdu[apdu_len]) {      /* Is Data already in place? */
            apdu_len += application_data_len;   /* Yes, step over data */
        } else {        /* No, copy data in */
            for (len = 0; len < application_data_len; len++) {
                apdu[apdu_len++] = application_data[len];
            }
        }
        apdu_len += encode_closing_tag(&apdu[apdu_len], 4);
    }

    return apdu_len;
}

int rpm_ack_encode_apdu_object_property_error(
    uint8_t * apdu,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        /* Tag 5: propertyAccessError */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 5);
        apdu_len +=
            encode_application_enumerated(&apdu[apdu_len], error_class);
        apdu_len += encode_application_enumerated(&apdu[apdu_len], error_code);
        apdu_len += encode_closing_tag(&apdu[apdu_len], 5);
    }

    return apdu_len;
}

int rpm_ack_encode_apdu_object_end(
    uint8_t * apdu)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu_len = encode_closing_tag(&apdu[0], 1);
    }

    return apdu_len;
}

#if BACNET_SVC_RPM_A

/* decode the object portion of the service request only */
int rpm_ack_decode_object_id(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_OBJECT_TYPE * object_type,
    uint32_t * object_instance)
{
    unsigned len = 0;
    uint16_t type = 0;  /* for decoding */

    /* check for value pointers */
    if (apdu && apdu_len && object_type && object_instance) {
        /* Tag 0: objectIdentifier */
        if (!decode_is_context_tag(&apdu[len++], 0))
            return -1;
        len += decode_object_id(&apdu[len], &type, object_instance);
        if (object_type)
            *object_type = (BACNET_OBJECT_TYPE) type;
        /* Tag 1: listOfResults */
        if (!decode_is_opening_tag_number(&apdu[len], 1))
            return -1;
        len++;  /* opening tag is only one octet */
    }

    return (int) len;
}

/* is this the end of the list of this objects properties values? */
int rpm_ack_decode_object_end(
    uint8_t * apdu,
    unsigned apdu_len)
{
    int len = 0;        /* total length of the apdu, return value */

    if (apdu && apdu_len) {
        if (decode_is_closing_tag_number(apdu, 1))
            len = 1;
    }

    return len;
}

int rpm_ack_decode_object_property(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_PROPERTY_ID * object_property,
    uint32_t * array_index)
{
    unsigned len = 0;
    unsigned tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t property = 0;      /* for decoding */
    uint32_t array_value = 0;   /* for decoding */

    /* check for valid pointers */
    if (apdu && apdu_len && object_property && array_index) {
        /* Tag 2: propertyIdentifier */
        if (!IS_CONTEXT_SPECIFIC(apdu[len]))
            return -1;
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        if (tag_number != 2)
            return -1;
        len += decode_enumerated(&apdu[len], len_value_type, &property);
        if (object_property)
            *object_property = (BACNET_PROPERTY_ID) property;
        /* Tag 3: Optional propertyArrayIndex */
        if ((len < apdu_len) && IS_CONTEXT_SPECIFIC(apdu[len]) &&
            (!IS_CLOSING_TAG(apdu[len]))) {
            tag_len =
                (unsigned) decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            if (tag_number == 3) {
                len += tag_len;
                len +=
                    decode_unsigned(&apdu[len], len_value_type, &array_value);
                *array_index = array_value;
            } else {
                *array_index = BACNET_ARRAY_ALL;
            }
        } else {
            *array_index = BACNET_ARRAY_ALL;
        }
    }

    return (int) len;
}

#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int rpm_ack_decode_apdu(
    uint8_t * apdu,
    int apdu_len,       /* total length of the apdu */
    uint8_t * invoke_id,
    uint8_t ** service_request,
    unsigned *service_request_len)
{
    int offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_COMPLEX_ACK)
        return -1;
    *invoke_id = apdu[1];
    if (apdu[2] != SERVICE_CONFIRMED_READ_PROP_MULTIPLE)
        return -1;
    offset = 3;
    if (apdu_len > offset) {
        if (service_request)
            *service_request = &apdu[offset];
        if (service_request_len)
            *service_request_len = apdu_len - offset;
    }

    return offset;
}

int rpm_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    uint8_t ** service_request,
    unsigned *service_request_len)
{
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -1;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_READ_PROP_MULTIPLE)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        if (service_request)
            *service_request = &apdu[offset];
        if (service_request_len)
            *service_request_len = apdu_len - offset;
    }

    return offset;
}

void testReadPropertyMultiple(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int test_len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 12;
    uint8_t test_invoke_id = 0;
    uint8_t *service_request = NULL;
    unsigned service_request_len = 0;
    BACNET_RPM_DATA rpmdata;

    rpmdata.object_type = OBJECT_DEVICE;
    rpmdata.object_instance = 0;
    rpmdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpmdata.array_index = 0;

    /* build the RPM - try to make it easy for the Application Layer development */
    /* IDEA: similar construction, but pass apdu, apdu_len pointer, size of apdu to
       let the called function handle the out of space problem that these get into
       by returning a boolean of success/failure.
       It almost needs to use the keylist library or something similar.
       Also check case of storing a backoff point (i.e. save enough room for object_end) */
    apdu_len = rpm_encode_apdu_init(&apdu[0], invoke_id);
    /* each object has a beginning and an end */
    apdu_len +=
        rpm_encode_apdu_object_begin(&apdu[apdu_len], OBJECT_DEVICE, 123);
    /* then stuff as many properties into it as APDU length will allow */
    apdu_len +=
        rpm_encode_apdu_object_property(&apdu[apdu_len],
        PROP_OBJECT_IDENTIFIER, BACNET_ARRAY_ALL);
    apdu_len +=
        rpm_encode_apdu_object_property(&apdu[apdu_len], PROP_OBJECT_NAME,
        BACNET_ARRAY_ALL);
    apdu_len += rpm_encode_apdu_object_end(&apdu[apdu_len]);
    /* each object has a beginning and an end */
    apdu_len +=
        rpm_encode_apdu_object_begin(&apdu[apdu_len], OBJECT_ANALOG_INPUT, 33);
    apdu_len +=
        rpm_encode_apdu_object_property(&apdu[apdu_len],
        PROP_OBJECT_IDENTIFIER, BACNET_ARRAY_ALL);
    apdu_len +=
        rpm_encode_apdu_object_property(&apdu[apdu_len], PROP_ALL,
        BACNET_ARRAY_ALL);
    apdu_len += rpm_encode_apdu_object_end(&apdu[apdu_len]);

    ct_test(pTest, apdu_len != 0);

    test_len = rpm_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &service_request,   /* will point to the service request in the apdu */
        &service_request_len);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, service_request != NULL);
    ct_test(pTest, service_request_len > 0);

    test_len =
        rpm_decode_object_id(service_request, service_request_len, &rpmdata);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, rpmdata.object_type == OBJECT_DEVICE);
    ct_test(pTest, rpmdata.object_instance == 123);
    len = test_len;
    /* decode the object property portion of the service request */
    test_len =
        rpm_decode_object_property(&service_request[len],
        service_request_len - len, &rpmdata);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, rpmdata.object_property == PROP_OBJECT_IDENTIFIER);
    ct_test(pTest, rpmdata.array_index == BACNET_ARRAY_ALL);
    len += test_len;
    test_len =
        rpm_decode_object_property(&service_request[len],
        service_request_len - len, &rpmdata);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, rpmdata.object_property == PROP_OBJECT_NAME);
    ct_test(pTest, rpmdata.array_index == BACNET_ARRAY_ALL);
    len += test_len;
    /* try again - we should fail */
    test_len =
        rpm_decode_object_property(&service_request[len],
        service_request_len - len, &rpmdata);
    ct_test(pTest, test_len < 0);
    /* is it the end of this object? */
    test_len =
        rpm_decode_object_end(&service_request[len],
        service_request_len - len);
    ct_test(pTest, test_len == 1);
    len += test_len;
    /* try to decode an object id */
    test_len =
        rpm_decode_object_id(&service_request[len], service_request_len - len,
        &rpmdata);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, rpmdata.object_type == OBJECT_ANALOG_INPUT);
    ct_test(pTest, rpmdata.object_instance == 33);
    len += test_len;
    /* decode the object property portion of the service request only */
    test_len =
        rpm_decode_object_property(&service_request[len],
        service_request_len - len, &rpmdata);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, rpmdata.object_property == PROP_OBJECT_IDENTIFIER);
    ct_test(pTest, rpmdata.array_index == BACNET_ARRAY_ALL);
    len += test_len;
    test_len =
        rpm_decode_object_property(&service_request[len],
        service_request_len - len, &rpmdata);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, rpmdata.object_property == PROP_ALL);
    ct_test(pTest, rpmdata.array_index == BACNET_ARRAY_ALL);
    len += test_len;
    test_len =
        rpm_decode_object_property(&service_request[len],
        service_request_len - len, &rpmdata);
    ct_test(pTest, test_len < 0);
    /* got an error -1, is it the end of this object? */
    test_len =
        rpm_decode_object_end(&service_request[len],
        service_request_len - len);
    ct_test(pTest, test_len == 1);
    len += test_len;
    ct_test(pTest, len == service_request_len);
}

void testReadPropertyMultipleAck(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int test_len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 12;
    uint8_t test_invoke_id = 0;
    uint8_t *service_request = NULL;
    unsigned service_request_len = 0;
    BACNET_OBJECT_TYPE object_type = OBJECT_DEVICE;
    uint32_t object_instance = 0;
    BACNET_PROPERTY_ID object_property = PROP_OBJECT_IDENTIFIER;
    uint32_t array_index = 0;
    BACNET_APPLICATION_DATA_VALUE application_data[4] = { {0} };
    BACNET_APPLICATION_DATA_VALUE test_application_data = { 0 };
    uint8_t application_data_buffer[MAX_APDU] = { 0 };
    int application_data_buffer_len = 0;
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
    BACNET_RPM_DATA rpmdata;

    /* build the RPM - try to make it easy for the
       Application Layer development */
    /* IDEA: similar construction, but pass apdu, apdu_len pointer,
       size of apdu to let the called function handle the out of
       space problem that these get into  by returning a boolean
       of success/failure.
       It almost needs to use the keylist library or something similar.
       Also check case of storing a backoff point
       (i.e. save enough room for object_end) */
    apdu_len = rpm_ack_encode_apdu_init(&apdu[0], invoke_id);
    /* object beginning */
    rpmdata.object_type = OBJECT_DEVICE;
    rpmdata.object_instance = 123;
    apdu_len += rpm_ack_encode_apdu_object_begin(&apdu[apdu_len], &rpmdata);
    /* reply property */
    apdu_len +=
        rpm_ack_encode_apdu_object_property(&apdu[apdu_len],
        PROP_OBJECT_IDENTIFIER, BACNET_ARRAY_ALL);
    /* reply value */
    application_data[0].tag = BACNET_APPLICATION_TAG_OBJECT_ID;
    application_data[0].type.Object_Id.type = OBJECT_DEVICE;
    application_data[0].type.Object_Id.instance = 123;
    application_data_buffer_len =
        bacapp_encode_application_data(&application_data_buffer[0],
        &application_data[0]);
    apdu_len +=
        rpm_ack_encode_apdu_object_property_value(&apdu[apdu_len],
        &application_data_buffer[0], application_data_buffer_len);
    /* reply property */
    apdu_len +=
        rpm_ack_encode_apdu_object_property(&apdu[apdu_len], PROP_OBJECT_TYPE,
        BACNET_ARRAY_ALL);
    /* reply value */
    application_data[1].tag = BACNET_APPLICATION_TAG_ENUMERATED;
    application_data[1].type.Enumerated = OBJECT_DEVICE;
    application_data_buffer_len =
        bacapp_encode_application_data(&application_data_buffer[0],
        &application_data[1]);
    apdu_len +=
        rpm_ack_encode_apdu_object_property_value(&apdu[apdu_len],
        &application_data_buffer[0], application_data_buffer_len);
    /* object end */
    apdu_len += rpm_ack_encode_apdu_object_end(&apdu[apdu_len]);

    /* object beginning */
    rpmdata.object_type = OBJECT_ANALOG_INPUT;
    rpmdata.object_instance = 33;
    apdu_len += rpm_ack_encode_apdu_object_begin(&apdu[apdu_len], &rpmdata);
    /* reply property */
    apdu_len +=
        rpm_ack_encode_apdu_object_property(&apdu[apdu_len],
        PROP_PRESENT_VALUE, BACNET_ARRAY_ALL);
    /* reply value */
    application_data[2].tag = BACNET_APPLICATION_TAG_REAL;
    application_data[2].type.Real = 0.0;
    application_data_buffer_len =
        bacapp_encode_application_data(&application_data_buffer[0],
        &application_data[2]);
    apdu_len +=
        rpm_ack_encode_apdu_object_property_value(&apdu[apdu_len],
        &application_data_buffer[0], application_data_buffer_len);
    /* reply property */
    apdu_len +=
        rpm_ack_encode_apdu_object_property(&apdu[apdu_len], PROP_DEADBAND,
        BACNET_ARRAY_ALL);
    /* reply error */
    apdu_len +=
        rpm_ack_encode_apdu_object_property_error(&apdu[apdu_len],
        ERROR_CLASS_PROPERTY, ERROR_CODE_UNKNOWN_PROPERTY);
    /* object end */
    apdu_len += rpm_ack_encode_apdu_object_end(&apdu[apdu_len]);
    ct_test(pTest, apdu_len != 0);

  /****** decode the packet ******/
    test_len = rpm_ack_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &service_request,       /* will point to the service request in the apdu */
        &service_request_len);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, service_request != NULL);
    ct_test(pTest, service_request_len > 0);
    /* the first part should be the first object id */
    test_len =
        rpm_ack_decode_object_id(service_request, service_request_len,
        &object_type, &object_instance);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, object_type == OBJECT_DEVICE);
    ct_test(pTest, object_instance == 123);
    len = test_len;
    /* extract the property */
    test_len =
        rpm_ack_decode_object_property(&service_request[len],
        service_request_len - len, &object_property, &array_index);
    ct_test(pTest, object_property == PROP_OBJECT_IDENTIFIER);
    ct_test(pTest, array_index == BACNET_ARRAY_ALL);
    len += test_len;
    /* what is the result? An error or a value? */
    ct_test(pTest, decode_is_opening_tag_number(&service_request[len], 4));
    len++;
    /* decode the object property portion of the service request */
    /* note: if this was an array, there could have been
       more than one element to decode */
    test_len =
        bacapp_decode_application_data(&service_request[len],
        service_request_len - len, &test_application_data);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, bacapp_same_value(&application_data[0],
            &test_application_data));
    len += test_len;
    ct_test(pTest, decode_is_closing_tag_number(&service_request[len], 4));
    len++;
    /* see if there is another property */
    test_len =
        rpm_ack_decode_object_property(&service_request[len],
        service_request_len - len, &object_property, &array_index);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, object_property == PROP_OBJECT_TYPE);
    ct_test(pTest, array_index == BACNET_ARRAY_ALL);
    len += test_len;
    /* what is the result value? */
    ct_test(pTest, decode_is_opening_tag_number(&service_request[len], 4));
    len++;
    /* decode the object property portion of the service request */
    test_len =
        bacapp_decode_application_data(&service_request[len],
        service_request_len - len, &test_application_data);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, bacapp_same_value(&application_data[1],
            &test_application_data));
    len += test_len;
    ct_test(pTest, decode_is_closing_tag_number(&service_request[len], 4));
    len++;
    /* see if there is another property */
    /* this time we should fail */
    test_len =
        rpm_ack_decode_object_property(&service_request[len],
        service_request_len - len, &object_property, &array_index);
    ct_test(pTest, test_len == -1);
    /* see if it is the end of this object */
    test_len =
        rpm_ack_decode_object_end(&service_request[len],
        service_request_len - len);
    ct_test(pTest, test_len == 1);
    len += test_len;
    /* try to decode another object id */
    test_len =
        rpm_ack_decode_object_id(&service_request[len],
        service_request_len - len, &object_type, &object_instance);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, object_type == OBJECT_ANALOG_INPUT);
    ct_test(pTest, object_instance == 33);
    len += test_len;
    /* decode the object property portion of the service request only */
    test_len =
        rpm_ack_decode_object_property(&service_request[len],
        service_request_len - len, &object_property, &array_index);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, object_property == PROP_PRESENT_VALUE);
    ct_test(pTest, array_index == BACNET_ARRAY_ALL);
    len += test_len;
    /* what is the result value? */
    ct_test(pTest, decode_is_opening_tag_number(&service_request[len], 4));
    len++;
    /* decode the object property portion of the service request */
    test_len =
        bacapp_decode_application_data(&service_request[len],
        service_request_len - len, &test_application_data);
    ct_test(pTest, test_len > 0);
    ct_test(pTest, bacapp_same_value(&application_data[2],
            &test_application_data));
    len += test_len;
    ct_test(pTest, decode_is_closing_tag_number(&service_request[len], 4));
    len++;
    /* see if there is another property */
    test_len =
        rpm_ack_decode_object_property(&service_request[len],
        service_request_len - len, &object_property, &array_index);
    ct_test(pTest, test_len != -1);
    ct_test(pTest, object_property == PROP_DEADBAND);
    ct_test(pTest, array_index == BACNET_ARRAY_ALL);
    len += test_len;
    /* what is the result value? */
    ct_test(pTest, decode_is_opening_tag_number(&service_request[len], 5));
    len++;
    /* it was an error reply */
    test_len =
        bacerror_decode_error_class_and_code(&service_request[len],
        service_request_len - len, &error_class, &error_code);
    ct_test(pTest, test_len != 0);
    ct_test(pTest, error_class == ERROR_CLASS_PROPERTY);
    ct_test(pTest, error_code == ERROR_CODE_UNKNOWN_PROPERTY);
    len += test_len;
    ct_test(pTest, decode_is_closing_tag_number(&service_request[len], 5));
    len++;
    /* is there another property? */
    test_len =
        rpm_ack_decode_object_property(&service_request[len],
        service_request_len - len, &object_property, &array_index);
    ct_test(pTest, test_len == -1);
    /* got an error -1, is it the end of this object? */
    test_len =
        rpm_ack_decode_object_end(&service_request[len],
        service_request_len - len);
    ct_test(pTest, test_len == 1);
    len += test_len;
    /* check for another object */
    test_len =
        rpm_ack_decode_object_id(&service_request[len],
        service_request_len - len, &object_type, &object_instance);
    ct_test(pTest, test_len == 0);
    ct_test(pTest, len == service_request_len);
}

#ifdef TEST_READ_PROPERTY_MULTIPLE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet ReadPropertyMultiple", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testReadPropertyMultiple);
    assert(rc);
    rc = ct_addTestFunction(pTest, testReadPropertyMultipleAck);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_READ_PROPERTY_MULTIPLE */

#endif /* TEST */
