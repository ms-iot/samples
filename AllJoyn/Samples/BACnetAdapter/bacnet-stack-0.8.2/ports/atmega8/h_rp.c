/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include <string.h>
#include <errno.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "rp.h"
/* demo objects */
#include "device.h"
#if MAX_ANALOG_VALUES
#include "av.h"
#endif
#if MAX_BINARY_VALUES
#include "bv.h"
#endif

/* Encodes the property APDU and returns the length,
   or sets the error, and returns -1 */
int Encode_Property_APDU(
    uint8_t * apdu,
    BACNET_READ_PROPERTY_DATA * rp_data,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    int apdu_len = -1;

    /* handle each object type */
    switch (rp_data->object_type) {
        case OBJECT_DEVICE:
            if (Device_Valid_Object_Instance_Number(rp_data->object_instance)) {
                apdu_len =
                    Device_Encode_Property_APDU(&apdu[0],
                    rp_data->object_instance, rp_data->object_property,
                    rp_data->array_index, error_class, error_code);
            }
            break;
#if MAX_ANALOG_VALUES
        case OBJECT_ANALOG_VALUE:
            if (Analog_Value_Valid_Instance(rp_data->object_instance)) {
                apdu_len =
                    Analog_Value_Encode_Property_APDU(&apdu[0],
                    rp_data->object_instance, rp_data->object_property,
                    rp_data->array_index, error_class, error_code);
            }
            break;
#endif
#if MAX_BINARY_VALUES
        case OBJECT_BINARY_VALUE:
            if (Binary_Value_Valid_Instance(rp_data->object_instance)) {
                apdu_len =
                    Binary_Value_Encode_Property_APDU(&apdu[0],
                    rp_data->object_instance, rp_data->object_property,
                    rp_data->array_index, error_class, error_code);
            }
            break;
#endif
        default:
            *error_class = ERROR_CLASS_OBJECT;
            *error_code = ERROR_CODE_UNKNOWN_OBJECT;
            break;
    }

    return apdu_len;
}

void handler_read_property(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_READ_PROPERTY_DATA data;
    int len = 0;
    int ack_len = 0;
    int property_len = 0;
    int pdu_len = 0;
    BACNET_NPDU_DATA npdu_data;
    int bytes_sent = 0;
    BACNET_ERROR_CLASS error_class = ERROR_CLASS_OBJECT;
    BACNET_ERROR_CODE error_code = ERROR_CODE_UNKNOWN_OBJECT;
    BACNET_ADDRESS my_address;

    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
        &npdu_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
        goto RP_ABORT;
    }
    len = rp_decode_service_request(service_request, service_len, &data);
    if (len < 0) {
        /* bad decoding - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
        goto RP_ABORT;
    }
    /* most cases will be error */
    ack_len =
        rp_ack_encode_apdu_init(&Handler_Transmit_Buffer[pdu_len],
        service_data->invoke_id, &data);
    /* FIXME: add buffer len as passed into function or use smart buffer */
    property_len =
        Encode_Property_APDU(&Handler_Transmit_Buffer[pdu_len + ack_len],
        &data, &error_class, &error_code);
    if (property_len >= 0) {
        len =
            rp_ack_encode_apdu_object_property_end(&Handler_Transmit_Buffer
            [pdu_len + property_len + ack_len]);
        len += ack_len + property_len;
    } else {
        switch (property_len) {
                /* BACnet APDU too small to fit data, so proper response is Abort */
            case BACNET_STATUS_ABORT:
                len =
                    abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
                break;
            default:
                len =
                    bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id, SERVICE_CONFIRMED_READ_PROPERTY,
                    error_class, error_code);
                break;
        }
    }
  RP_ABORT:
    pdu_len += len;
    bytes_sent =
        datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);

    return;
}
