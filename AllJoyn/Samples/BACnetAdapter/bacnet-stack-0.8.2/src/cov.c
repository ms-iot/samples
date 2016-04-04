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
#include "cov.h"

/** @file cov.c  Encode/Decode Change of Value (COV) services */

/* Change-Of-Value Services
COV Subscribe
COV Subscribe Property
COV Notification
Unconfirmed COV Notification
*/
static int notify_encode_apdu(
    uint8_t * apdu,
    BACNET_COV_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */
    BACNET_PROPERTY_VALUE *value = NULL;        /* value in list */
	BACNET_APPLICATION_DATA_VALUE *app_data = NULL;

    if (apdu) {
        /* tag 0 - subscriberProcessIdentifier */
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            data->subscriberProcessIdentifier);
        apdu_len += len;
        /* tag 1 - initiatingDeviceIdentifier */
        len =
            encode_context_object_id(&apdu[apdu_len], 1, OBJECT_DEVICE,
            data->initiatingDeviceIdentifier);
        apdu_len += len;
        /* tag 2 - monitoredObjectIdentifier */
        len =
            encode_context_object_id(&apdu[apdu_len], 2,
            (int) data->monitoredObjectIdentifier.type,
            data->monitoredObjectIdentifier.instance);
        apdu_len += len;
        /* tag 3 - timeRemaining */
        len = encode_context_unsigned(&apdu[apdu_len], 3, data->timeRemaining);
        apdu_len += len;
        /* tag 4 - listOfValues */
        len = encode_opening_tag(&apdu[apdu_len], 4);
        apdu_len += len;
        /* the first value includes a pointer to the next value, etc */
        /* FIXME: for small implementations, we might try a partial
           approach like the rpm.c where the values are encoded with
           a separate function */
        value = data->listOfValues;
        while (value != NULL) {
            /* tag 0 - propertyIdentifier */
            len =
                encode_context_enumerated(&apdu[apdu_len], 0,
                value->propertyIdentifier);
            apdu_len += len;
            /* tag 1 - propertyArrayIndex OPTIONAL */
            if (value->propertyArrayIndex != BACNET_ARRAY_ALL) {
                len =
                    encode_context_unsigned(&apdu[apdu_len], 1,
                    value->propertyArrayIndex);
                apdu_len += len;
            }
            /* tag 2 - value */
            /* abstract syntax gets enclosed in a context tag */
            len = encode_opening_tag(&apdu[apdu_len], 2);
            apdu_len += len;
            app_data = &value->value;
			while (app_data != NULL)
			{
                len =
                bacapp_encode_application_data(&apdu[apdu_len], app_data);
                apdu_len += len;
				app_data = app_data->next;
            }

            len = encode_closing_tag(&apdu[apdu_len], 2);
            apdu_len += len;
            /* tag 3 - priority OPTIONAL */
            if (value->priority != BACNET_NO_PRIORITY) {
                len =
                    encode_context_unsigned(&apdu[apdu_len], 3,
                    value->priority);
                apdu_len += len;
            }
            /* is there another one to encode? */
            /* FIXME: check to see if there is room in the APDU */
            value = value->next;
        }
        len = encode_closing_tag(&apdu[apdu_len], 4);
        apdu_len += len;
    }

    return apdu_len;
}

int ccov_notify_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_COV_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_COV_NOTIFICATION;
        apdu_len = 4;
        len = notify_encode_apdu(&apdu[apdu_len], data);
        apdu_len += len;
    }

    return apdu_len;
}

int ucov_notify_encode_apdu(
    uint8_t * apdu,
    BACNET_COV_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && data) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_COV_NOTIFICATION; /* service choice */
        apdu_len = 2;
        len = notify_encode_apdu(&apdu[apdu_len], data);
        apdu_len += len;
    }

    return apdu_len;
}

/* decode the service request only */
/* COV and Unconfirmed COV are the same */
int cov_notify_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_COV_DATA * data)
{
    int len = 0;        /* return value */
    int app_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t decoded_value = 0; /* for decoding */
    uint16_t decoded_type = 0;  /* for decoding */
    uint32_t property = 0;      /* for decoding */
    BACNET_PROPERTY_VALUE *value = NULL;        /* value in list */
	BACNET_APPLICATION_DATA_VALUE *app_data = NULL;

    if (apdu_len && data) {
        /* tag 0 - subscriberProcessIdentifier */
        if (decode_is_context_tag(&apdu[len], 0)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            data->subscriberProcessIdentifier = decoded_value;
        } else {
            return BACNET_STATUS_ERROR;
        }
        /* tag 1 - initiatingDeviceIdentifier */
        if (decode_is_context_tag(&apdu[len], 1)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->initiatingDeviceIdentifier);
            if (decoded_type != OBJECT_DEVICE) {
                return BACNET_STATUS_ERROR;
            }
        } else {
            return BACNET_STATUS_ERROR;
        }
        /* tag 2 - monitoredObjectIdentifier */
        if (decode_is_context_tag(&apdu[len], 2)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->monitoredObjectIdentifier.instance);
            data->monitoredObjectIdentifier.type = decoded_type;
        } else {
            return BACNET_STATUS_ERROR;
        }
        /* tag 3 - timeRemaining */
        if (decode_is_context_tag(&apdu[len], 3)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            data->timeRemaining = decoded_value;
        } else {
            return BACNET_STATUS_ERROR;
        }
        /* tag 4: opening context tag - listOfValues */
        if (!decode_is_opening_tag_number(&apdu[len], 4)) {
            return BACNET_STATUS_ERROR;
        }
        /* a tag number of 4 is not extended so only one octet */
        len++;
        /* the first value includes a pointer to the next value, etc */
        value = data->listOfValues;
        if (value == NULL) {
            /* no space to store any values */
            return BACNET_STATUS_ERROR;
        }
        while (value != NULL) {
            /* tag 0 - propertyIdentifier */
            if (decode_is_context_tag(&apdu[len], 0)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len += decode_enumerated(&apdu[len], len_value, &property);
                value->propertyIdentifier = (BACNET_PROPERTY_ID) property;
            } else {
                return BACNET_STATUS_ERROR;
            }
            /* tag 1 - propertyArrayIndex OPTIONAL */
            if (decode_is_context_tag(&apdu[len], 1)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len += decode_unsigned(&apdu[len], len_value, &decoded_value);
                value->propertyArrayIndex = decoded_value;
            } else {
                value->propertyArrayIndex = BACNET_ARRAY_ALL;
            }
            /* tag 2: opening context tag - value */
            if (!decode_is_opening_tag_number(&apdu[len], 2)) {
                return BACNET_STATUS_ERROR;
            }
            /* a tag number of 2 is not extended so only one octet */
            len++;
            app_data = &value->value;
			while (!decode_is_closing_tag_number(&apdu[len], 2))
			{
                if (app_data == NULL) {
                    /* out of room to store more values */
                    return BACNET_STATUS_ERROR;
                }
                app_len =
                bacapp_decode_application_data(&apdu[len], apdu_len - len, app_data);
				if (app_len < 0)
				{
					return BACNET_STATUS_ERROR;
				}
                len += app_len;

                app_data = app_data->next;
            }
            /* a tag number of 2 is not extended so only one octet */
            len++;
            /* tag 3 - priority OPTIONAL */
            if (decode_is_context_tag(&apdu[len], 3)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len += decode_unsigned(&apdu[len], len_value, &decoded_value);
                value->priority = (uint8_t) decoded_value;
            } else {
                value->priority = BACNET_NO_PRIORITY;
            }
            /* end of list? */
            if (decode_is_closing_tag_number(&apdu[len], 4)) {
                value->next = NULL;
                break;
            }
            /* is there another one to decode? */
            value = value->next;
            if (value == NULL) {
                /* out of room to store more values */
                return BACNET_STATUS_ERROR;
            }
        }
    }

    return len;
}

/*
12.11.38Active_COV_Subscriptions
The Active_COV_Subscriptions property is a List of BACnetCOVSubscription,
each of which consists of a Recipient, a Monitored Property Reference,
an Issue Confirmed Notifications flag, a Time Remaining value and an
optional COV Increment. This property provides a network-visible indication
of those COV subscriptions that are active at any given time.
Whenever a COV Subscription is created with the SubscribeCOV or
SubscribeCOVProperty service, a new entry is added to
the Active_COV_Subscriptions list. Similarly, whenever a COV Subscription
is terminated, the corresponding entry shall be
removed from the Active_COV_Subscriptions list.
*/
/*
SubscribeCOV-Request ::= SEQUENCE {
        subscriberProcessIdentifier  [0] Unsigned32,
        monitoredObjectIdentifier    [1] BACnetObjectIdentifier,
        issueConfirmedNotifications  [2] BOOLEAN OPTIONAL,
        lifetime                     [3] Unsigned OPTIONAL
        }
*/

int cov_subscribe_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && data) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_SUBSCRIBE_COV;
        apdu_len = 4;
        /* tag 0 - subscriberProcessIdentifier */
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            data->subscriberProcessIdentifier);
        apdu_len += len;
        /* tag 1 - monitoredObjectIdentifier */
        len =
            encode_context_object_id(&apdu[apdu_len], 1,
            (int) data->monitoredObjectIdentifier.type,
            data->monitoredObjectIdentifier.instance);
        apdu_len += len;
        /*
           If both the 'Issue Confirmed Notifications' and
           'Lifetime' parameters are absent, then this shall
           indicate a cancellation request.
         */
        if (!data->cancellationRequest) {
            /* tag 2 - issueConfirmedNotifications */
            len =
                encode_context_boolean(&apdu[apdu_len], 2,
                data->issueConfirmedNotifications);
            apdu_len += len;
            /* tag 3 - lifetime */
            len = encode_context_unsigned(&apdu[apdu_len], 3, data->lifetime);
            apdu_len += len;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int cov_subscribe_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    int len = 0;        /* return value */
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t decoded_value = 0; /* for decoding */
    uint16_t decoded_type = 0;  /* for decoding */

    if (apdu_len && data) {
        /* tag 0 - subscriberProcessIdentifier */
        if (decode_is_context_tag(&apdu[len], 0)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            data->subscriberProcessIdentifier = decoded_value;
        } else {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* tag 1 - monitoredObjectIdentifier */
        if (decode_is_context_tag(&apdu[len], 1)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->monitoredObjectIdentifier.instance);
            data->monitoredObjectIdentifier.type = decoded_type;
        } else {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* optional parameters - if missing, means cancellation */
        if ((unsigned) len < apdu_len) {
            /* tag 2 - issueConfirmedNotifications - optional */
            if (decode_is_context_tag(&apdu[len], 2)) {
                data->cancellationRequest = false;
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                data->issueConfirmedNotifications =
                    decode_context_boolean(&apdu[len]);
                len += len_value;
            } else {
                data->cancellationRequest = true;
            }
            /* tag 3 - lifetime - optional */
            if (decode_is_context_tag(&apdu[len], 3)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len += decode_unsigned(&apdu[len], len_value, &decoded_value);
                data->lifetime = decoded_value;
            } else {
                data->lifetime = 0;
            }
        } else {
            data->cancellationRequest = true;
        }
    }

    return len;
}


/*
SubscribeCOVProperty-Request ::= SEQUENCE {
        subscriberProcessIdentifier  [0] Unsigned32,
        monitoredObjectIdentifier    [1] BACnetObjectIdentifier,
        issueConfirmedNotifications  [2] BOOLEAN OPTIONAL,
        lifetime                     [3] Unsigned OPTIONAL,
        monitoredPropertyIdentifier  [4] BACnetPropertyReference,
        covIncrement                 [5] REAL OPTIONAL
        }

BACnetPropertyReference ::= SEQUENCE {
      propertyIdentifier      [0] BACnetPropertyIdentifier,
      propertyArrayIndex      [1] Unsigned OPTIONAL
      -- used only with array datatype
      -- if omitted with an array the entire array is referenced
      }

*/

int cov_subscribe_property_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && data) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY;
        apdu_len = 4;
        /* tag 0 - subscriberProcessIdentifier */
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            data->subscriberProcessIdentifier);
        apdu_len += len;
        /* tag 1 - monitoredObjectIdentifier */
        len =
            encode_context_object_id(&apdu[apdu_len], 1,
            (int) data->monitoredObjectIdentifier.type,
            data->monitoredObjectIdentifier.instance);
        apdu_len += len;
        if (!data->cancellationRequest) {
            /* tag 2 - issueConfirmedNotifications */
            len =
                encode_context_boolean(&apdu[apdu_len], 2,
                data->issueConfirmedNotifications);
            apdu_len += len;
            /* tag 3 - lifetime */
            len = encode_context_unsigned(&apdu[apdu_len], 3, data->lifetime);
            apdu_len += len;
        }
        /* tag 4 - monitoredPropertyIdentifier */
        len = encode_opening_tag(&apdu[apdu_len], 4);
        apdu_len += len;
        len =
            encode_context_enumerated(&apdu[apdu_len], 0,
            data->monitoredProperty.propertyIdentifier);
        apdu_len += len;
        if (data->monitoredProperty.propertyArrayIndex != BACNET_ARRAY_ALL) {
            len =
                encode_context_unsigned(&apdu[apdu_len], 1,
                data->monitoredProperty.propertyArrayIndex);
            apdu_len += len;

        }
        len = encode_closing_tag(&apdu[apdu_len], 4);
        apdu_len += len;

        /* tag 5 - covIncrement */
        if (data->covIncrementPresent) {
            len = encode_context_real(&apdu[apdu_len], 5, data->covIncrement);
            apdu_len += len;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int cov_subscribe_property_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    int len = 0;        /* return value */
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t decoded_value = 0; /* for decoding */
    uint16_t decoded_type = 0;  /* for decoding */
    uint32_t property = 0;      /* for decoding */

    if (apdu_len && data) {
        /* tag 0 - subscriberProcessIdentifier */
        if (decode_is_context_tag(&apdu[len], 0)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            data->subscriberProcessIdentifier = decoded_value;
        } else {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* tag 1 - monitoredObjectIdentifier */
        if (decode_is_context_tag(&apdu[len], 1)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->monitoredObjectIdentifier.instance);
            data->monitoredObjectIdentifier.type = decoded_type;
        } else {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* tag 2 - issueConfirmedNotifications - optional */
        if (decode_is_context_tag(&apdu[len], 2)) {
            data->cancellationRequest = false;
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            data->issueConfirmedNotifications =
                decode_context_boolean(&apdu[len]);
            len++;
        } else {
            data->cancellationRequest = true;
        }
        /* tag 3 - lifetime - optional */
        if (decode_is_context_tag(&apdu[len], 3)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            data->lifetime = decoded_value;
        } else {
            data->lifetime = 0;
        }
        /* tag 4 - monitoredPropertyIdentifier */
        if (!decode_is_opening_tag_number(&apdu[len], 4)) {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* a tag number of 4 is not extended so only one octet */
        len++;
        /* the propertyIdentifier is tag 0 */
        if (decode_is_context_tag(&apdu[len], 0)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_enumerated(&apdu[len], len_value, &property);
            data->monitoredProperty.propertyIdentifier =
                (BACNET_PROPERTY_ID) property;
        } else {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* the optional array index is tag 1 */
        if (decode_is_context_tag(&apdu[len], 1)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            data->monitoredProperty.propertyArrayIndex = decoded_value;
        } else {
            data->monitoredProperty.propertyArrayIndex = BACNET_ARRAY_ALL;
        }

        if (!decode_is_closing_tag_number(&apdu[len], 4)) {
            data->error_code = ERROR_CODE_REJECT_INVALID_TAG;
            return BACNET_STATUS_REJECT;
        }
        /* a tag number of 4 is not extended so only one octet */
        len++;
        /* tag 5 - covIncrement - optional */
        if (decode_is_context_tag(&apdu[len], 5)) {
            data->covIncrementPresent = true;
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_real(&apdu[len], &data->covIncrement);
        } else {
            data->covIncrementPresent = false;
        }
    }

    return len;
}

/** Link an array or buffer of BACNET_PROPERTY_VALUE elements and add them
 * to the BACNET_COV_DATA structure.  It is used prior to encoding or
 * decoding the APDU data into the structure.
 *
 * @param data - The BACNET_COV_DATA structure that holds the data to
 * be encoded or decoded.
 * @param value_list - One or more BACNET_PROPERTY_VALUE elements in
 * a buffer or array.
 * @param count - number of BACNET_PROPERTY_VALUE elements
 */
void cov_data_value_list_link(
    BACNET_COV_DATA *data,
    BACNET_PROPERTY_VALUE *value_list,
    size_t count)
{
    BACNET_PROPERTY_VALUE *current_value_list = NULL;

    if (data && value_list) {
        data->listOfValues = value_list;
        while (count) {
            if (count > 1) {
                current_value_list = value_list;
                value_list++;
                current_value_list->next = value_list;
            } else {
                value_list->next = NULL;
            }
            count--;
        }
    }
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"
#include "bacapp.h"

int ccov_notify_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_COV_DATA * data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu) {
        return -1;
    }
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -2;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_COV_NOTIFICATION)
        return -3;
    offset = 4;

    /* optional limits - must be used as a pair */
    if (apdu_len > offset) {
        len =
            cov_notify_decode_service_request(&apdu[offset], apdu_len - offset,
            data);
    }

    return len;
}

int ucov_notify_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_COV_DATA * data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST)
        return -2;
    if (apdu[1] != SERVICE_UNCONFIRMED_COV_NOTIFICATION)
        return -3;
    /* optional limits - must be used as a pair */
    offset = 2;
    if (apdu_len > offset) {
        len =
            cov_notify_decode_service_request(&apdu[offset], apdu_len - offset,
            data);
    }

    return len;
}

int cov_subscribe_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -2;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_SUBSCRIBE_COV)
        return -3;
    offset = 4;

    /* optional limits - must be used as a pair */
    if (apdu_len > offset) {
        len =
            cov_subscribe_decode_service_request(&apdu[offset],
            apdu_len - offset, data);
    }

    return len;
}

int cov_subscribe_property_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -2;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY)
        return -3;
    offset = 4;

    /* optional limits - must be used as a pair */
    if (apdu_len > offset) {
        len =
            cov_subscribe_property_decode_service_request(&apdu[offset],
            apdu_len - offset, data);
    }

    return len;
}

void testCOVNotifyData(
    Test * pTest,
    BACNET_COV_DATA * data,
    BACNET_COV_DATA * test_data)
{
    BACNET_PROPERTY_VALUE *value = NULL;
    BACNET_PROPERTY_VALUE *test_value = NULL;

    ct_test(pTest,
        test_data->subscriberProcessIdentifier ==
        data->subscriberProcessIdentifier);
    ct_test(pTest,
        test_data->initiatingDeviceIdentifier ==
        data->initiatingDeviceIdentifier);
    ct_test(pTest,
        test_data->monitoredObjectIdentifier.type ==
        data->monitoredObjectIdentifier.type);
    ct_test(pTest,
        test_data->monitoredObjectIdentifier.instance ==
        data->monitoredObjectIdentifier.instance);
    ct_test(pTest, test_data->timeRemaining == data->timeRemaining);
    /* test the listOfValues in some clever manner */
    value = data->listOfValues;
    test_value = test_data->listOfValues;
    while (value) {
        ct_test(pTest, test_value);
        if (test_value) {
            ct_test(pTest,
                test_value->propertyIdentifier == value->propertyIdentifier);
            ct_test(pTest,
                test_value->propertyArrayIndex == value->propertyArrayIndex);
            ct_test(pTest,
                test_value->priority == value->priority);
            ct_test(pTest,
                bacapp_same_value(&test_value->value, &value->value));
            test_value = test_value->next;
        }
        value = value->next;
    }
}

void testUCOVNotifyData(
    Test * pTest,
    BACNET_COV_DATA * data)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_COV_DATA test_data;
    BACNET_PROPERTY_VALUE value_list[5] = {{0}};

    len = ucov_notify_encode_apdu(&apdu[0], data);
    ct_test(pTest, len > 0);
    apdu_len = len;

    cov_data_value_list_link(&test_data, &value_list[0], 5);
    len = ucov_notify_decode_apdu(&apdu[0], apdu_len, &test_data);
    ct_test(pTest, len != -1);
    testCOVNotifyData(pTest, data, &test_data);
}

void testCCOVNotifyData(
    Test * pTest,
    uint8_t invoke_id,
    BACNET_COV_DATA * data)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_COV_DATA test_data;
    BACNET_PROPERTY_VALUE value_list[2] = {{0}};
    uint8_t test_invoke_id = 0;

    len = ccov_notify_encode_apdu(&apdu[0], invoke_id, data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    cov_data_value_list_link(&test_data, &value_list[0], 2);
    len =
        ccov_notify_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_data);
    ct_test(pTest, len > 0);
    ct_test(pTest, test_invoke_id == invoke_id);
    testCOVNotifyData(pTest, data, &test_data);
}

void testCOVNotify(
    Test * pTest)
{
    uint8_t invoke_id = 12;
    BACNET_COV_DATA data;
    BACNET_PROPERTY_VALUE value_list[2] = {{0}};

    data.subscriberProcessIdentifier = 1;
    data.initiatingDeviceIdentifier = 123;
    data.monitoredObjectIdentifier.type = OBJECT_ANALOG_INPUT;
    data.monitoredObjectIdentifier.instance = 321;
    data.timeRemaining = 456;

    cov_data_value_list_link(&data, &value_list[0], 2);
    /* first value */
    value_list[0].propertyIdentifier = PROP_PRESENT_VALUE;
    value_list[0].propertyArrayIndex = BACNET_ARRAY_ALL;
    bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, "21.0",
        &value_list[0].value);
    value_list[0].priority = 0;
    /* second value */
    value_list[1].propertyIdentifier = PROP_STATUS_FLAGS;
    value_list[1].propertyArrayIndex = BACNET_ARRAY_ALL;
    bacapp_parse_application_data(BACNET_APPLICATION_TAG_BIT_STRING, "0000",
        &value_list[1].value);
    value_list[1].priority = 0;

    testUCOVNotifyData(pTest, &data);
    testCCOVNotifyData(pTest, invoke_id, &data);
}

void testCOVSubscribeData(
    Test * pTest,
    BACNET_SUBSCRIBE_COV_DATA * data,
    BACNET_SUBSCRIBE_COV_DATA * test_data)
{
    ct_test(pTest,
        test_data->subscriberProcessIdentifier ==
        data->subscriberProcessIdentifier);
    ct_test(pTest,
        test_data->monitoredObjectIdentifier.type ==
        data->monitoredObjectIdentifier.type);
    ct_test(pTest,
        test_data->monitoredObjectIdentifier.instance ==
        data->monitoredObjectIdentifier.instance);
    ct_test(pTest,
        test_data->cancellationRequest == data->cancellationRequest);
    if (test_data->cancellationRequest != data->cancellationRequest) {
        printf("cancellation request failed!\n");
    }
    if (!test_data->cancellationRequest) {
        ct_test(pTest,
            test_data->issueConfirmedNotifications ==
            data->issueConfirmedNotifications);
        ct_test(pTest, test_data->lifetime == data->lifetime);
    }
}

void testCOVSubscribePropertyData(
    Test * pTest,
    BACNET_SUBSCRIBE_COV_DATA * data,
    BACNET_SUBSCRIBE_COV_DATA * test_data)
{
    testCOVSubscribeData(pTest, data, test_data);
    ct_test(pTest,
        test_data->monitoredProperty.propertyIdentifier ==
        data->monitoredProperty.propertyIdentifier);
    ct_test(pTest,
        test_data->monitoredProperty.propertyArrayIndex ==
        data->monitoredProperty.propertyArrayIndex);
    ct_test(pTest,
        test_data->covIncrementPresent == data->covIncrementPresent);
    if (test_data->covIncrementPresent) {
        ct_test(pTest, test_data->covIncrement == data->covIncrement);
    }
}

void testCOVSubscribeEncoding(
    Test * pTest,
    uint8_t invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_SUBSCRIBE_COV_DATA test_data;
    uint8_t test_invoke_id = 0;

    len = cov_subscribe_encode_apdu(&apdu[0], invoke_id, data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        cov_subscribe_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_data);
    ct_test(pTest, len > 0);
    ct_test(pTest, test_invoke_id == invoke_id);
    testCOVSubscribeData(pTest, data, &test_data);
}

void testCOVSubscribePropertyEncoding(
    Test * pTest,
    uint8_t invoke_id,
    BACNET_SUBSCRIBE_COV_DATA * data)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_SUBSCRIBE_COV_DATA test_data;
    uint8_t test_invoke_id = 0;

    len = cov_subscribe_property_encode_apdu(&apdu[0], invoke_id, data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        cov_subscribe_property_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_data);
    ct_test(pTest, len > 0);
    ct_test(pTest, test_invoke_id == invoke_id);
    testCOVSubscribePropertyData(pTest, data, &test_data);
}

void testCOVSubscribe(
    Test * pTest)
{
    uint8_t invoke_id = 12;
    BACNET_SUBSCRIBE_COV_DATA data;

    data.subscriberProcessIdentifier = 1;
    data.monitoredObjectIdentifier.type = OBJECT_ANALOG_INPUT;
    data.monitoredObjectIdentifier.instance = 321;
    data.cancellationRequest = false;
    data.issueConfirmedNotifications = true;
    data.lifetime = 456;

    testCOVSubscribeEncoding(pTest, invoke_id, &data);
    data.cancellationRequest = true;
    testCOVSubscribeEncoding(pTest, invoke_id, &data);
}

void testCOVSubscribeProperty(
    Test * pTest)
{
    uint8_t invoke_id = 12;
    BACNET_SUBSCRIBE_COV_DATA data;

    data.subscriberProcessIdentifier = 1;
    data.monitoredObjectIdentifier.type = OBJECT_ANALOG_INPUT;
    data.monitoredObjectIdentifier.instance = 321;
    data.cancellationRequest = false;
    data.issueConfirmedNotifications = true;
    data.lifetime = 456;
    data.monitoredProperty.propertyIdentifier = PROP_FILE_SIZE;
    data.monitoredProperty.propertyArrayIndex = BACNET_ARRAY_ALL;
    data.covIncrementPresent = true;
    data.covIncrement = 1.0;

    testCOVSubscribePropertyEncoding(pTest, invoke_id, &data);

    data.cancellationRequest = true;
    testCOVSubscribePropertyEncoding(pTest, invoke_id, &data);

    data.cancellationRequest = false;
    data.covIncrementPresent = false;
    testCOVSubscribePropertyEncoding(pTest, invoke_id, &data);
}

#ifdef TEST_COV
int main(
    int argc,
    char *argv[])
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet COV", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testCOVNotify);
    assert(rc);
    rc = ct_addTestFunction(pTest, testCOVSubscribe);
    assert(rc);
    rc = ct_addTestFunction(pTest, testCOVSubscribeProperty);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_COV */
#endif /* TEST */
