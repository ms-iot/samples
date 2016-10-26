/**************************************************************************
*
* Copyright (C) 2009 Peter Mc Shane
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
*********************************************************************/
#ifndef READRANGE_H
#define READRANGE_H

#include "bacstr.h"
#include "datetime.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    struct BACnet_Read_Range_Data;
    typedef struct BACnet_Read_Range_Data {
        BACNET_OBJECT_TYPE object_type;
        uint32_t object_instance;
        BACNET_PROPERTY_ID object_property;
        uint32_t array_index;
        uint8_t *application_data;
        int application_data_len;
        BACNET_BIT_STRING ResultFlags;  /**<  FIRST_ITEM, LAST_ITEM, MORE_ITEMS. */
        int RequestType;/**< Index, sequence or time based request. */
        int Overhead;    /**< How much space the baggage takes in the response. */
        uint32_t ItemCount;
        uint32_t FirstSequence;
        union { /**< Pick the appropriate data type. */
            uint32_t RefIndex;
            uint32_t RefSeqNum;
            BACNET_DATE_TIME RefTime;
        } Range;
        int32_t Count;  /**< SIGNED value as +ve vs -ve  is important. */
        BACNET_ERROR_CLASS error_class;
        BACNET_ERROR_CODE error_code;
    } BACNET_READ_RANGE_DATA;

/** Defines to indicate which type of read range request it is.
   Not really a bit map but we do it like this to allow quick
   checking of request against capabilities for the property */

#define RR_BY_POSITION    1
#define RR_BY_SEQUENCE    2
#define RR_BY_TIME        4
#define RR_READ_ALL       8    /**< Read all of array - so don't send any range in the request */
#define RR_ARRAY_OF_LISTS 16   /**< For info functionality indicates array of lists if set */

/** Bit String Enumerations */
    typedef enum {
        RESULT_FLAG_FIRST_ITEM = 0,
        RESULT_FLAG_LAST_ITEM = 1,
        RESULT_FLAG_MORE_ITEMS = 2
    } BACNET_RESULT_FLAGS;

/** Defines for ReadRange packet overheads to allow us to determine how
 * much space is left for actual payload:
 *
 * Overhead is comprised of:
 * - 1. PDU Type + invoke ID + service type = 3 bytes
 * - 2. Object ID = 5 bytes
 * - 3. Object Property = 2 bytes if property is 0-255, 3 if property is
 *    256-65535 � theoretical max of 5 bytes but how likely is that?
 * - 4. Optional array index = 2 bytes if index is 0-255, 3 if index is
 *    256-65535 � theoretical max of 5 bytes but how likely is that?
 * - 5. Flags = 3 bytes
 * - 6. Opening and closing tag for data = 2 bytes
 * - 7. firstSequenceNumber [6] Unsigned32 OPTIONAL -- used only if 'Item Count' > 0
 *    and the request was either of  type 'By Sequence Number' or 'By Time'
 *    = minimum of 2 bytes, maximum of 5 bytes.
 *
 * These figures give an absolute worst-case overhead of 28 bytes. A less
 * conservative value (if we assume object property is 3 bytes and array
 * index is 3 bytes) is 24. */

/* This is the fixed part of the overhead before we check for array and
 * first sequence number requirements. again if you are really paranoid
 * use a value of 18 */

#define RR_OVERHEAD         16
#define RR_1ST_SEQ_OVERHEAD 5
#define RR_INDEX_OVERHEAD   3   /* or 5 if paranoid */

/** Define pointer to function type for handling ReadRange request.
   This function will take the following parameters:
  - 1. A pointer to a buffer of at least MAX_APDU bytes to build the response in.
  - 2. A pointer to a BACNET_READ_RANGE_DATA structure with all the request
      information in it. The function is responsible for applying the request
      to the property in question and returning the response. */

    typedef int (
        *rr_handler_function) (
        uint8_t * apdu,
        BACNET_READ_RANGE_DATA * pRequest);

/** Structure to return the type of requests a given object property can
 * accept and the address of the function to handle the request */

    typedef struct rrpropertyinfo {
        int RequestTypes;
        rr_handler_function Handler;
    } RR_PROP_INFO;

/** Function template for ReadRange information retrieval function.
 * A function template; @see device.c for assignment to object types.
 * @ingroup ObjHelpers
 * @param pRequest [in]	Info on the request.
 * @param pInfo [out]   Where to write the response to.
 * @return True on success, False on error or failure.
 */
    typedef bool(
        *rr_info_function) (
        BACNET_READ_RANGE_DATA * pRequest,      /* Info on the request */
        RR_PROP_INFO * pInfo);  /* Where to write the response to */

    int rr_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_READ_RANGE_DATA * rrdata);

    int rr_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_READ_RANGE_DATA * rrdata);

    int rr_ack_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_READ_RANGE_DATA * rrdata);

    int rr_ack_decode_service_request(
        uint8_t * apdu,
        int apdu_len,   /* total length of the apdu */
        BACNET_READ_RANGE_DATA * rrdata);

    uint8_t Send_ReadRange_Request(
        uint32_t device_id,     /* destination device */
        BACNET_READ_RANGE_DATA * read_access_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup Trend Trending BIBBs
 * These BIBBs prescribe the BACnet capabilities required to interoperably
 * perform the trending functions enumerated in clause 22.2.1.4 for the
     * BACnet devices defined therein.
*//** @defgroup TrendReadRange Trending -Read Range Service (eg, in T-VMT)
 * @ingroup Trend
 * 15.8 ReadRange Service <br>
 * The ReadRange service is used by a client BACnet-user to read a specific
 * range of data items representing a subset of data available within a
 * specified object property.
 * The service may be used with any list or array of lists property.
 */
#endif
