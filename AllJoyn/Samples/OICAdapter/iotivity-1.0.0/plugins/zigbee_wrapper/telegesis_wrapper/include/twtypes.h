//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * @file
 *
 * This API only works with:
 *      Telegesis ETRX357
 *      CICIE R310 B110615
 *
 */

#ifndef TWTYPES_H_
#define TWTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stdbool.h>

#define DEVICE_BAUDRATE                 (B19200)
#define MAX_ZIGBEE_BYTES                (512)
#define MAX_ZIGBEE_ENROLLED_DEVICES     (255)

#define TIME_OUT_00_SECOND      (0)
#define TIME_OUT_01_SECOND      (1)
#define TIME_OUT_05_SECONDS     (5)
#define TIME_OUT_07_SECONDS     (7)
#define TIME_OUT_10_SECONDS     (10)
#define TIME_OUT_15_SECONDS     (15)

#define SIMPLEDESC_RESPONSE_EXPECTED_LINES (6)

#define AT_STR_ERROR_OK             "00"
#define AT_STR_ERROR_INVALID_OP     "70"

#define AT_STR_ERROR_EVERYTHING_OK                              "00"
#define AT_STR_ERROR_NODE_IS_PART_OF_PAN                        "28"
#define AT_STR_ERROR_MESSAGE_NOT_SENT_TO_TARGET_SUCCESSFULLY    "66"
#define AT_STR_ERROR_INVALID_OPERATION                          "70"


#define SENDMODE                "0"
#define SEPARATOR               ","
#define SEPARATOR_LENGTH        strlen(SEPARATOR)

#define AT_CMD_RESET                                    "AT&F"
#define AT_CMD_GET_NETWORK_INFO                         "AT+N"
#define AT_CMD_ESTABLISH_NETWORK                        "AT+EN"
#define AT_CMD_PERMIT_JOIN                              "AT+PJOIN:"
#define AT_CMD_MATCH_REQUEST                            "AT+MATCHREQ:"
#define AT_CMD_SIMPLE_DESC                              "AT+SIMPLEDESC:"
#define AT_CMD_WRITE_ATR                                "AT+WRITEATR:"
#define AT_CMD_READ_ATR                                 "AT+READATR:"
#define AT_CMD_RUN_ON_OFF                               "AT+RONOFF:"
#define AT_CMD_MOVE_TO_LEVEL                            "AT+LCMVTOLEV:"
#define AT_CMD_DOOR_LOCK                                "AT+DRLOCK:"
#define AT_CMD_COLOR_CTRL_MOVE_TO_COLOR_TEMPERATURE     "AT+CCMVTOCT:"
#define AT_CMD_GET_LOCAL_EUI                            "ATS04?"
#define AT_CMD_REMOTE_EUI_REQUEST                       "AT+EUIREQ:"

#define TW_ENDCONTROL_ERROR_STRING                      "ERROR:"

#define SIZE_EUI                    (17)
#define SIZE_NODEID                 (5)
#define SIZE_CLUSTERID              (5)
#define SIZE_ENDPOINTID             (3)

#define SIZE_ZONESTATUS             (5)
#define SIZE_ZONESTATUS_EXTENDED    (3)
#define SIZE_ZONEID                 (3)
#define SIZE_ZONETYPE               (5)
#define SIZE_UPDATE_DELAY_TIME      (5)

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef enum
{
    ZB_STATE_UNKNOWN,
    ZB_STATE_INIT
} TWState;

typedef struct
{
    TWState state;
    uint64_t panId;
    uint64_t extPanId;

    char*   remoteAttributeValueRead;
    uint8_t remoteAtrributeValueReadLength;
} TWStatus;

typedef enum
{
    TW_RESULT_OK = 0,

    TW_RESULT_ERROR_LINE_COUNT,
    TW_RESULT_NO_LOCAL_PAN,
    TW_RESULT_HAS_LOCAL_PAN,
    TW_RESULT_NEW_LOCAL_PAN_ESTABLISHED,
    TW_RESULT_DEVICE_JOINED,
    TW_RESULT_FOUND_NO_MATCHED_DEVICE,
    TW_RESULT_FOUND_MATCHED_DEVICES,
    TW_RESULT_HAS_CLUSTERS,
    TW_RESULT_HAS_NO_CLUSTER,
    TW_RESULT_REMOTE_ATTR_HAS_VALUE,

    TW_RESULT_UNKNOWN,

    TW_RESULT_ERROR_INVALID_PARAMS,
    TW_RESULT_ERROR_INVALID_PORT,
    TW_RESULT_ERROR_NO_MEMORY,
    TW_RESULT_ERROR_INVALID_OP,
    TW_RESULT_ERROR_NOTIMPL,

    TW_RESULT_ERROR = 255

} TWResultCode;

typedef enum
{
    AT_ERROR_EVERYTHING_OK  = 0,
    AT_ERROR_NODE_IS_PART_OF_PAN = 28,
    AT_ERROR_MESSAGE_NOT_SENT_TO_TARGET_SUCCESSFULLY   = 66,
    AT_ERROR_INVALID_OPERATION  = 70,

} TWATErrorCode;

typedef enum
{
    TW_ENDCONTROL_OK = 0,
    TW_ENDCONTROL_ERROR,
    TW_ENDCONTROL_ACK,
    TW_ENDCONTROL_SEQ,
    TW_ENDCONTROL_MAX_VALUE

} TWEndControl;

typedef struct
{
    const char * endStr;
    TWEndControl endControl;

} TWEndControlMap;

typedef TWResultCode (*TWATResultHandler)(int count, char** tokens);

typedef struct
{
    const char *resultTxt;
    TWATResultHandler handler;

} TWATResultHandlerPair;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TWTYPES_H_ */
