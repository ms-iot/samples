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
#include "bacerror.h"
#include "bacdcode.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "arf.h"
/* demo objects */
#if defined(BACFILE)
#include "bacfile.h"
#endif
#include "handlers.h"

/** @file h_arf.c  Handles Atomic Read File request. */

/*
from BACnet SSPC-135-2004

14. FILE ACCESS SERVICES

This clause defines the set of services used to access and
manipulate files contained in BACnet devices. The concept of files
is used here as a network-visible representation for a collection
of octets of arbitrary length and meaning. This is an abstract
concept only and does not imply the use of disk, tape or other
mass storage devices in the server devices. These services may
be used to access vendor-defined files as well as specific
files defined in the BACnet protocol standard.
Every file that is accessible by File Access Services shall
have a corresponding File object in the BACnet device. This File
object is used to identify the particular file by name. In addition,
the File object provides access to "header information," such
as the file's total size, creation date, and type. File Access
Services may model files in two ways: as a continuous stream of
octets or as a contiguous sequence of numbered records.
The File Access Services provide atomic read and write operations.
In this context "atomic" means that during the execution
of a read or write operation, no other AtomicReadFile or
AtomicWriteFile operations are allowed for the same file.
Synchronization of these services with internal operations
of the BACnet device is a local matter and is not defined by this
standard.

14.1 AtomicReadFile Service

14.1.5 Service Procedure

The responding BACnet-user shall first verify the validity
of the 'File Identifier' parameter and return a 'Result(-)' response
with the appropriate error class and code if the File object
is unknown, if there is currently another AtomicReadFile or
AtomicWriteFile service in progress, or if the File object is
currently inaccessible for another reason. If the 'File Start
Position' parameter or the 'File Start Record' parameter is
either less than 0 or exceeds the actual file size, then the appropriate
error is returned in a 'Result(-)' response. If not, then the
responding BACnet-user shall read the number of octets specified by
'Requested Octet Count' or the number of records specified by
'Requested Record Count'. If the number of remaining octets or
records is less than the requested amount, then the length of
the 'File Data' returned or 'Returned Record Count' shall indicate
the actual number read. If the returned response contains the
last octet or record of the file, then the 'End Of File' parameter
shall be TRUE, otherwise FALSE.
*/

#if defined(BACFILE)
void handler_atomic_read_file(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_ATOMIC_READ_FILE_DATA data;
    int len = 0;
    int pdu_len = 0;
    bool error = false;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;
    BACNET_ERROR_CLASS error_class = ERROR_CLASS_OBJECT;
    BACNET_ERROR_CODE error_code = ERROR_CODE_UNKNOWN_OBJECT;

#if PRINT_ENABLED
    fprintf(stderr, "Received Atomic-Read-File Request!\n");
#endif
    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
        &npdu_data);
    if (service_data->segmented_message) {
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
#if PRINT_ENABLED
        fprintf(stderr, "ARF: Segmented Message. Sending Abort!\n");
#endif
        goto ARF_ABORT;
    }
    len = arf_decode_service_request(service_request, service_len, &data);
    /* bad decoding - send an abort */
    if (len < 0) {
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
#if PRINT_ENABLED
        fprintf(stderr, "Bad Encoding. Sending Abort!\n");
#endif
        goto ARF_ABORT;
    }
    if (data.object_type == OBJECT_FILE) {
        if (!bacfile_valid_instance(data.object_instance)) {
            error = true;
        } else if (data.access == FILE_STREAM_ACCESS) {
            if (data.type.stream.requestedOctetCount <
                octetstring_capacity(&data.fileData[0])) {
                if (bacfile_read_stream_data(&data)) {
#if PRINT_ENABLED
                    fprintf(stderr, "ARF: Stream offset %d, %d octets.\n",
                        data.type.stream.fileStartPosition,
                        data.type.stream.requestedOctetCount);
#endif
                    len =
                        arf_ack_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                        service_data->invoke_id, &data);
                } else {
                    error = true;
                    error_class = ERROR_CLASS_OBJECT;
                    error_code = ERROR_CODE_FILE_ACCESS_DENIED;
                }
            } else {
                len =
                    abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
#if PRINT_ENABLED
                fprintf(stderr, "Too Big To Send (%d >= %d). Sending Abort!\n",
                    data.type.stream.requestedOctetCount,
                    (int)octetstring_capacity(&data.fileData[0]));
#endif
            }
        } else if (data.access == FILE_RECORD_ACCESS) {
            if (data.type.record.fileStartRecord >=
                BACNET_READ_FILE_RECORD_COUNT) {
                error_class = ERROR_CLASS_SERVICES;
                error_code = ERROR_CODE_INVALID_FILE_START_POSITION;
                error = true;
            } else if (bacfile_read_stream_data(&data)) {
#if PRINT_ENABLED
                fprintf(stderr, "ARF: fileStartRecord %d, %u RecordCount.\n",
                    data.type.record.fileStartRecord,
                    data.type.record.RecordCount);
#endif
                len =
                    arf_ack_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id, &data);
            } else {
                error = true;
                error_class = ERROR_CLASS_OBJECT;
                error_code = ERROR_CODE_FILE_ACCESS_DENIED;
            }
        } else {
            error = true;
            error_class = ERROR_CLASS_SERVICES;
            error_code = ERROR_CODE_INVALID_FILE_ACCESS_METHOD;
#if PRINT_ENABLED
            fprintf(stderr, "Record Access Requested. Sending Error!\n");
#endif
        }
    } else {
        error = true;
        error_class = ERROR_CLASS_SERVICES;
        error_code = ERROR_CODE_INCONSISTENT_OBJECT_TYPE;
    }
    if (error) {
        len =
            bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, SERVICE_CONFIRMED_ATOMIC_READ_FILE,
            error_class, error_code);
    }
  ARF_ABORT:
    pdu_len += len;
    bytes_sent =
        datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0) {
        fprintf(stderr, "Failed to send PDU (%s)!\n", strerror(errno));
    }
#endif

    return;
}
#endif
