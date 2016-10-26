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


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>

#include <termios.h>
#include <string.h>
#include <errno.h>

#include "oic_string.h"
#include "oic_malloc.h"
#include "logger.h"

#include "twtypes.h"
#include "telegesis_socket.h"
#include "telegesis_wrapper.h"


#define TAG PCF("telegesiswrapper")     // Module Name

#define ARRAY_LENGTH    100

#define RESPONSE_PARAMS_COUNT_NETWORK_INFO_1                (1)
#define RESPONSE_PARAMS_COUNT_NETWORK_INFO_5                (5)
#define RESPONSE_PARAMS_COUNT_JPAN                          (3)
#define RESPONSE_PARAMS_COUNT_DEVICE_JOINED                 (2)
#define RESPONSE_PARAMS_COUNT_MATCH_DESC                    (3)
#define RESPONSE_PARAMS_COUNT_SIMPLE_DESC                   (2)
#define RESPONSE_PARAMS_COUNT_SIMPLE_DESC_IN_CLUSTER_MIN    (1)
#define RESPONSE_PARAMS_COUNT_WRITE_ATTR_4                  (4)
#define RESPONSE_PARAMS_COUNT_WRITE_ATTR_5                  (5)
#define RESPONSE_PARAMS_COUNT_TEMPERATURE                   (5)
#define RESPONSE_PARAMS_COUNT_RESPATTR                      (6)
#define RESPONSE_PARAMS_COUNT_ADDRESS_RESPONSE              (3)
#define RESPONSE_PARAMS_COUNT_DFTREP                        (5)
#define RESPONSE_PARAMS_COUNT_DRLOCKUNLOCKRSP               (3)
#define RESPONSE_PARAMS_COUNT_ZENROLLREQ                    (4)
#define RESPONSE_PARAMS_COUNT_ENROLLED                      (3)
#define RESPONSE_PARAMS_COUNT_ZONESTATUS_4                  (4)
#define RESPONSE_PARAMS_COUNT_ZONESTATUS_6                  (6)

//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------

static TWResultCode HandleATResponse(TWEntry* entry);

static TWResultCode processEntry(TWEntry* entry);
static TWResultCode processEntryNETWORK_INFO(TWEntry* entry);
static TWResultCode processEntryJPAN(TWEntry* entry);
static TWResultCode processEntryEndDevice(TWEntry* entry);
static TWResultCode processEntryMatchDesc(TWEntry* entry);
static TWResultCode processEntrySimpleDesc(TWEntry* entry);
static TWResultCode processEntryWriteAttr(TWEntry* entry);
static TWResultCode processEntryReadAttr(TWEntry* entry);
static TWResultCode processEntryTemperature(TWEntry* entry);
static TWResultCode processEntrySwitchDoorLockState(TWEntry* entry);
static TWResultCode processEntryZCLDefaultResponse(TWEntry* entry);
static TWResultCode processEntryZoneEnrollRequest(TWEntry* entry);
static TWResultCode processEntryEnrolled(TWEntry* entry);
static TWResultCode processEntryZoneStatus(TWEntry* entry);
static TWResultCode processEntryAddressResponse(TWEntry* entry);

static TWResultCode Reset();
static TWResultCode GetRemoteEUI();
static TWResultCode CreatePAN();
static TWResultCode EnableJoin(bool isKeyEncrypted);
static TWResultCode FindMatchNodes();
static TWResultCode FindClusters(char nodeId[], char endpoint[]);

static TWResultCode TelNetworkInfoHandler(int count, char* tokens[]);
static TWResultCode TelJpanHandler(int count, char* tokens[]);
static TWResultCode TelEndDeviceJoinHandler(int count, char* tokens[]);
static TWResultCode TelMatchDescHandler(int count, char* tokens[]);
static TWResultCode TelAddressResponseHandler(int count, char* tokens[]);
static TWResultCode TelSimpleDescHandler(int count, char* tokens[]);
static TWResultCode TelSimpleDescInClusterHandler(int count, char* tokens[]);
static TWResultCode TelWriteAttrHandler(int count, char* tokens[]);
static TWResultCode TelReadAttrHandler(int count, char* tokens[]);
static TWResultCode TelReadAttrHandlerTemperature(int count, char* tokens[]);
static TWResultCode TelZCLDefaultResponseHandler(int count, char* tokens[]);
static TWResultCode TelSwitchDoorLockStateHandler(int count, char* tokens[]);
static TWResultCode TelZoneEnrollRequestHandler(int count, char* tokens[]);
static TWResultCode TelEnrolledHandler(int count, char* tokens[]);
static TWResultCode TelZoneStatusHandler(int count, char* tokens[]);

static TWResultCode AsciiHexToValue(char* hexString, int length, uint64_t* value);
static int AsciiToHex(char c);
static int Tokenize(const char *input, const char* delimiters, char* output[]);

static void DeallocateTWDeviceList();

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

static TWATResultHandlerPair g_TWATResultHandlerPairArray[] =
{
    {"+N=",         TelNetworkInfoHandler},
    {"JPAN:",       TelJpanHandler},
    {"RFD:",        TelEndDeviceJoinHandler},       //e.g SmartThings Open/Closed Sensor
    {"FFD:",        TelEndDeviceJoinHandler},       //e.g SmartThings Plug
    {"SED:",        TelEndDeviceJoinHandler},
    {"ZED:",        TelEndDeviceJoinHandler},
    {"MatchDesc:",  TelMatchDescHandler},
    {"SimpleDesc:", TelSimpleDescHandler},
    {"InCluster:",  TelSimpleDescInClusterHandler},
    {"WRITEATTR:",  TelWriteAttrHandler},
    {"RESPATTR:",   TelReadAttrHandler},
    {"TEMPERATURE:",TelReadAttrHandlerTemperature},
    {"DFTREP",      TelZCLDefaultResponseHandler},
    {"DRLOCRSP:",   TelSwitchDoorLockStateHandler},
    {"DRUNLOCKRSP:",TelSwitchDoorLockStateHandler},
    {"ZENROLLREQ:", TelZoneEnrollRequestHandler},
    {"ENROLLED:",   TelEnrolledHandler},
    {"ZONESTATUS:", TelZoneStatusHandler},
    {"AddrResp:",   TelAddressResponseHandler},
    {"Unknown:",    TelNetworkInfoHandler}
};

//TODO: Take care of all global variables
static PIPlugin* g_plugin = (PIPlugin*)1;
static const char* g_port = NULL;

static char g_LocalEUI[SIZE_EUI] = "";
static char g_WIPRemoteEUI[SIZE_EUI] = "";
static char g_WIPRemoteNodeId[SIZE_NODEID] = "";

static TWStatus g_ZigBeeStatus = {ZB_STATE_UNKNOWN,0,0,NULL,0};
static TWDeviceList* g_FoundMatchedDeviceList = NULL;
static TWDevice* g_WIPDevice = NULL;

static TWDeviceFoundCallback g_DeviceFoundCallback = NULL;
static TWEnrollmentSucceedCallback g_EnrollmentSucceedCallback = NULL;
static TWDeviceStatusUpdateCallback g_DeviceStatusUpdateCallback = NULL;

/*****************************************************************************/
/*                                                                           */
/* Public functions                                                          */
/*                                                                           */
/*****************************************************************************/
OCStackResult TWInitialize(const char* deviceDevPath)
{
    OC_LOG_V(INFO, TAG, "Enter TWInitialize()");

    TWResultCode twCode = TW_RESULT_ERROR;

    g_port = deviceDevPath;
    OC_LOG_V(INFO, TAG, "Attempt to open %s", deviceDevPath);

    twCode = TWStartSock(g_plugin, deviceDevPath);  //TODO:

    if (twCode != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Failed to open %s because of error: %d", deviceDevPath, twCode);
        return OC_STACK_ERROR;
    }

    char* eui = NULL;
    twCode = TWGetEUI(g_plugin, &eui);
    if (twCode != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Failed to get EUI because of error: %d", twCode);
        return OC_STACK_ERROR;
    }
    OICStrcpy(g_LocalEUI, SIZE_EUI, eui);
    OC_LOG_V(INFO, TAG, "LocalEUI=%s", g_LocalEUI);
    OICFree(eui);

    bool wantReset = false;     //TODO:
    if (wantReset)
    {
        twCode = Reset();
        if (twCode != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "ZigBee Initialization - Reset");
            return OC_STACK_ERROR;
        }
    }

    twCode = CreatePAN();
    if (twCode != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "CreatePan Failed");
        OC_LOG_V(ERROR, TAG, "TWInitialize() - MUST STOP NOW");
        g_ZigBeeStatus.state = ZB_STATE_UNKNOWN;
        return OC_STACK_ERROR;
    }
    else
    {
        OC_LOG_V(INFO, TAG, "CreatePan Succeed");
        OC_LOG_V(INFO, TAG, "TWInitialize() Succeed");
        return OC_STACK_OK;
    }
}

OCStackResult TWDiscover()
{
    OC_LOG_V(INFO, TAG, "Enter TWDiscover()");

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_ERROR;

    if (g_DeviceFoundCallback == NULL)
    {
        OC_LOG_V(INFO, TAG, "Required TWDeviceFoundCallback.");
        return OC_STACK_ERROR;
    }

    twRet = EnableJoin(false);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "EnableJoin");
        return OC_STACK_ERROR;
    }

    twRet = FindMatchNodes();
    if (twRet == TW_RESULT_OK)
    {
        OC_LOG_V(INFO, TAG, "FindMatchNodes");
        ret = OC_STACK_OK;
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "FindMatchNodes");
        ret = OC_STACK_ERROR;
    }

    OC_LOG_V(INFO, TAG, "Leave TWDiscover() with ret=%d", ret);
    return ret;
}

OCStackResult TWSetAttribute(char* extendedUniqueId, char* nodeId, char* endpointId,
                             char* clusterId, char* attributeId, char* attributeType,
                             char* newValue)
{
    //Ask:  AT+WRITEATR:5DA7,01,0,0003,0000,21,01

    OC_LOG_V(INFO, TAG, "Enter TWSetAttribute()");

    (void)extendedUniqueId;

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_ERROR;

    int size =  strlen(AT_CMD_WRITE_ATR) + strlen(nodeId) +
                SEPARATOR_LENGTH + strlen(endpointId) +
                SEPARATOR_LENGTH + strlen(SENDMODE) +
                SEPARATOR_LENGTH + strlen(clusterId) +
                SEPARATOR_LENGTH + strlen(attributeId) +
                SEPARATOR_LENGTH + strlen(attributeType) +
                SEPARATOR_LENGTH + strlen(newValue) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    snprintf(cmdString, size, "%s%s,%s,%s,%s,%s,%s,%s",
             AT_CMD_WRITE_ATR, nodeId, endpointId, SENDMODE,
             clusterId, attributeId, attributeType, newValue);

    twRet = TWIssueATCommand(g_plugin, cmdString);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    twRet = TWDequeueEntry(g_plugin, &entry, TW_WRITEATTR);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(INFO, TAG, "TWEntry is NULL.");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = processEntry(entry);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "processEntry");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    ret = OC_STACK_OK;

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave TWSetAttribute() with ret=%d", ret);
    return ret;
}

OCStackResult TWGetAttribute(char* extendedUniqueId, char* nodeId, char* endpointId,
                             char* clusterId, char* attributeId,
                             char** outValue, uint8_t* outValueLength)
{
    //Ask:  AT+READATR:FE5A,01,0,0402,0002

    OC_LOG_V(INFO, TAG, "Enter TWGetAttribute()");

    (void)extendedUniqueId;

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_ERROR;

    int size =  strlen(AT_CMD_READ_ATR) + strlen(nodeId) +
                SEPARATOR_LENGTH + strlen(endpointId) +
                SEPARATOR_LENGTH + strlen(SENDMODE) +
                SEPARATOR_LENGTH + strlen(clusterId) +
                SEPARATOR_LENGTH + strlen(attributeId) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_NO_MEMORY;
        goto exit;
    }
    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s%s%s%s%s",
                             AT_CMD_READ_ATR, nodeId,
                             SEPARATOR, endpointId,
                             SEPARATOR, SENDMODE,
                             SEPARATOR, clusterId,
                             SEPARATOR, attributeId);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    twRet = TWIssueATCommand(g_plugin, cmdString);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    twRet = TWDequeueEntry(g_plugin, &entry, TW_RESPATTR);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(INFO, TAG, "TWEntry is NULL");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = processEntry(entry);
    if (twRet != TW_RESULT_REMOTE_ATTR_HAS_VALUE)
    {
        OC_LOG_V(ERROR, TAG, "processEntry.");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    size = strlen(g_ZigBeeStatus.remoteAttributeValueRead) + 1;
    *outValue = (char*)OICMalloc(sizeof(char) * size);
    if (*outValue == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_NO_MEMORY;
        goto exit;
    }

    OICStrcpy(*outValue, size, g_ZigBeeStatus.remoteAttributeValueRead);
    *outValueLength = g_ZigBeeStatus.remoteAtrributeValueReadLength;
    OICFree(g_ZigBeeStatus.remoteAttributeValueRead);
    g_ZigBeeStatus.remoteAttributeValueRead = NULL;
    g_ZigBeeStatus.remoteAtrributeValueReadLength = 0;
    OC_LOG_V(INFO, TAG, "TWGetAttribute() gets an attribute value.");
    ret = OC_STACK_OK;

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave TWGetAttribute() with ret=%d", ret);
    return ret;
}

OCStackResult TWSwitchOnOff(char* nodeId, char* endpointId, char* newState)
{
    //AT+RONOFF:<Address>,<EP>,<SendMode>[,<ON/OFF>]
    //AT+RONOFF:9E2B,01,0,1
    //      OK
    //      DFTREP:9E2B,01,0006,01,00

    OC_LOG_V(INFO, TAG, "Enter TWSwitchOnOff()");

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_UNKNOWN;

    int size = 0;
    if (newState == NULL)
    {
        size =  strlen(AT_CMD_RUN_ON_OFF) + strlen(nodeId) +
                SEPARATOR_LENGTH + strlen(endpointId) +
                SEPARATOR_LENGTH + strlen(SENDMODE) + 1;
    }
    else
    {
        size =  strlen(AT_CMD_RUN_ON_OFF) + strlen(nodeId) +
                SEPARATOR_LENGTH + strlen(endpointId) +
                SEPARATOR_LENGTH + strlen(SENDMODE) +
                SEPARATOR_LENGTH + strlen(newState) + 1;
    }

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s",
                             AT_CMD_RUN_ON_OFF, nodeId, SEPARATOR,
                             endpointId, SEPARATOR, SENDMODE);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    if (newState != NULL)
    {
        OICStrcat(cmdString, size, SEPARATOR);
        OICStrcat(cmdString, size, newState);
    }

    twRet = TWIssueATCommand(g_plugin, cmdString);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    twRet = TWDequeueEntry(g_plugin, &entry, TW_DFTREP);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = processEntry(entry);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "processEntry - %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    ret = OC_STACK_OK;

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave TWSwitchOnOff() with ret=%d", ret);
    return ret;
}

OCStackResult TWMoveToLevel(char* nodeId, char* endpointId,
                            char* onOffState, char* level, char* transTime)
{
    //AT+LCMVTOLEV:<Address>,<EP>,<SendMode>,<ON/OFF>,<LevelValue>,<TransTime>

    OC_LOG_V(INFO, TAG, "Enter TWMoveToLevel()");

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_UNKNOWN;

    int size = 0;
    size =  strlen(AT_CMD_MOVE_TO_LEVEL) + strlen(nodeId) +
            SEPARATOR_LENGTH + strlen(endpointId) +
            SEPARATOR_LENGTH + strlen(SENDMODE) +
            SEPARATOR_LENGTH + strlen(onOffState) +
            SEPARATOR_LENGTH + strlen(level) +
            SEPARATOR_LENGTH + strlen(transTime) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s%s%s%s%s%s%s",
                             AT_CMD_MOVE_TO_LEVEL, nodeId,
                             SEPARATOR, endpointId,
                             SEPARATOR, SENDMODE,
                             SEPARATOR, onOffState,
                             SEPARATOR, level,
                             SEPARATOR, transTime);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = TWIssueATCommand(g_plugin, cmdString);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    twRet = TWDequeueEntry(g_plugin, &entry, TW_DFTREP);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = processEntry(entry);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "processEntry - %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    ret = OC_STACK_OK;

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave TWMoveToLevel() with ret=%d", ret);
    return ret;
}

OCStackResult TWSwitchDoorLockState(char* nodeId, char* endpointId, char* newState)
{
    //AT+DRLOCK:<Address>,<EP>,<SendMode>,<Lock/Unlock>

    OC_LOG_V(INFO, TAG, "Enter TWSwitchDoorLockState()");

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_UNKNOWN;

    int size = 0;
    size =  strlen(AT_CMD_DOOR_LOCK) + strlen(nodeId) +
            SEPARATOR_LENGTH + strlen(endpointId) +
            SEPARATOR_LENGTH + strlen(SENDMODE) +
            SEPARATOR_LENGTH + strlen(newState) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s%s%s",
                             AT_CMD_DOOR_LOCK, nodeId,
                             SEPARATOR, endpointId,
                             SEPARATOR, SENDMODE,
                             SEPARATOR, newState);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = TWIssueATCommand(g_plugin, cmdString);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    twRet = TWDequeueEntry(g_plugin, &entry, TW_DFTREP);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL");
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = processEntry(entry);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "processEntry - %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    ret = OC_STACK_OK;

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave TWSwitchDoorLockState() with ret=%d", ret);
    return ret;
}

OCStackResult TWColorMoveToColorTemperature(char* nodeId, char* endpointId,
                                            char* colorTemperature, char* transTime)
{

    //AT+CCMVTOCT:<Address>,<EP>,<SendMode>,<ColorTemperature>,<TransTime>
    //  OK
    //  ERROR:<errorcode>

    OC_LOG_V(INFO, TAG, "Enter TWColorMoveToColorTemperature()");

    OCStackResult ret = OC_STACK_ERROR;
    TWResultCode twRet = TW_RESULT_UNKNOWN;

    int size = 0;
    size =  strlen(AT_CMD_COLOR_CTRL_MOVE_TO_COLOR_TEMPERATURE) + strlen(nodeId) +
            SEPARATOR_LENGTH + strlen(endpointId) +
            SEPARATOR_LENGTH + strlen(SENDMODE) +
            SEPARATOR_LENGTH + strlen(colorTemperature) +
            SEPARATOR_LENGTH + strlen(transTime) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = OC_STACK_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s%s%s%s%s",
                             AT_CMD_COLOR_CTRL_MOVE_TO_COLOR_TEMPERATURE, nodeId,
                             SEPARATOR, endpointId,
                             SEPARATOR, SENDMODE,
                             SEPARATOR, colorTemperature,
                             SEPARATOR, transTime);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    twRet = TWIssueATCommand(g_plugin, cmdString);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    twRet = TWDequeueEntry(g_plugin, &entry, TW_DFTREP);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry - %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL - %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    twRet = processEntry(entry);
    if (twRet != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "processEntry - %s", cmdString);
        ret = OC_STACK_ERROR;
        goto exit;
    }

    ret = OC_STACK_OK;

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave TWColorMoveToColorTemperature() with ret=%d", ret);
    return ret;
}

OCStackResult TWSetDiscoveryCallback(const TWDeviceFoundCallback callback)
{
    OC_LOG_V(INFO, TAG, "Enter TWSetDiscoveryCallback()");
    if (callback != NULL)
    {
        g_DeviceFoundCallback= callback;
    }
    else
    {
        g_DeviceFoundCallback = NULL;
    }

    OC_LOG_V(INFO, TAG, "Leave TWSetDiscoveryCallback() with ret=%d", OC_STACK_OK);
    return OC_STACK_OK;
}

OCStackResult TWSetStatusUpdateCallback(TWDeviceStatusUpdateCallback callback)
{
    OC_LOG_V(INFO, TAG, "Enter TWSetStatusUpdateCallback()");
    if (callback != NULL)
    {
        g_DeviceStatusUpdateCallback= callback;
    }
    else
    {
        g_DeviceStatusUpdateCallback = NULL;
    }

    OC_LOG_V(INFO, TAG, "Leave TWSetStatusUpdateCallback() with ret=%d", OC_STACK_OK);
    return OC_STACK_OK;
}

OCStackResult TWListenForStatusUpdates(char* nodeId, char* endpointId)
{
    OC_LOG_V(INFO, TAG, "Enter TWListenForStatusUpdates()");

    char* zoneClusterID = "0500";
    char* zoneAttributeID = "0010";
    char* attributeDateType = "F0";

    OCStackResult ret = TWSetAttribute(NULL, nodeId, endpointId,
                                       zoneClusterID, zoneAttributeID, attributeDateType,
                                       g_LocalEUI);

    OC_LOG_V(INFO, TAG, "Leave TWListenForStatusUpdates() with ret=%d", ret);
    return ret;
}

OCStackResult TWProcess()
{

    TWResultCode ret = TW_RESULT_UNKNOWN;

    while (true)
    {
        TWEntry* entry = NULL;
        ret = TWDequeueEntry(g_plugin, &entry, TW_NONE);
        if (ret != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "TWDequeueEntry");
            ret = OC_STACK_ERROR;
            break;
        }
        if (entry == NULL)
        {
            ret = OC_STACK_OK;
            break;
        }

        ret = processEntry(entry);
        if (ret != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "processEntry");
            ret = TWDeleteEntry(g_plugin, entry);
            if(ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "Failed to delete entry.");
                ret = OC_STACK_ERROR;
                break;
            }
        }
        else
        {
            OC_LOG_V(INFO, TAG, "processEntry");
            ret = TWDeleteEntry(g_plugin, entry);
            if(ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "Failed to delete entry.");
                ret = OC_STACK_ERROR;
                break;
            }
        }
    }

    return ret;
}

OCStackResult TWUninitialize()
{
    OC_LOG_V(INFO, TAG, "Enter TWUninitializeZigBee()");
    OCStackResult ret = OC_STACK_ERROR;

    TWResultCode twRet = TWStopSock(g_plugin);
    if (twRet == TW_RESULT_OK)
    {
        OC_LOG_V(INFO, TAG, "TWStopSock");
        ret = OC_STACK_OK;
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "TWStopSock");
        ret = OC_STACK_ERROR;
    }

    DeallocateTWDeviceList();

    OC_LOG_V(INFO, TAG, "Leave TWUninitializeZigBee() with ret=%d", ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------

TWResultCode processEntry(TWEntry *entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntry()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    switch(entry->type)
    {
        /*
        TW_OK,              TODO: Joey to return an TWEntry for OK
        TW_ERROR,           TODO: Joey to return an TWEntry for ERROR

        TW_INCLUSTER,

        TW_ACK,
        TW_NACK,
        TW_SEQ,
        TW_MAX_ENTRY
         */

        /*
        TODO: Joey?
        //Ask:          AT+PJOIN
        //Response:     OK

        //Ask:          AT+PJOIN
        //Response:     ERROR:70

         */

        case TW_NETWORK_INFO:
            ret = processEntryNETWORK_INFO(entry);
            if ((ret != TW_RESULT_NO_LOCAL_PAN) &&
                (ret != TW_RESULT_HAS_LOCAL_PAN))
            {
                OC_LOG_V(ERROR, TAG, "processEntryNETWORK_INFO.");
            }
            break;
        case TW_JPAN:
            ret = processEntryJPAN(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryJPAN.");
            }
            break;
        case TW_SED:
            ret = processEntryEndDevice(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntrySED.");
            }
            break;
        case TW_RFD:
            ret = processEntryEndDevice(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryRFD.");
            }
            break;
        case TW_FFD:
            ret = processEntryEndDevice(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryFFD.");
            }
            break;
        case TW_ZED:
            ret = processEntryEndDevice(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryZED.");
            }
            break;
        case TW_MATCHDESC:
            ret = processEntryMatchDesc(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryMatchDesc.");
            }
            break;
        case TW_SIMPLEDESC:
            ret = processEntrySimpleDesc(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntrySimpleDesc.");
            }
            break;
        case TW_WRITEATTR:
            ret = processEntryWriteAttr(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryWriteAttr.");
            }
            break;
        case TW_RESPATTR:
            ret = processEntryReadAttr(entry);
            if (ret != TW_RESULT_REMOTE_ATTR_HAS_VALUE)
            {
                OC_LOG_V(ERROR, TAG, "processEntryReadAttr.");
            }
            break;
        case TW_TEMPERATURE:
            ret = processEntryTemperature(entry);
            if (ret != TW_RESULT_REMOTE_ATTR_HAS_VALUE)
            {
                OC_LOG_V(ERROR, TAG, "processEntryTemperature.");
            }
            break;
        case TW_DRLOCRSP:
            ret = processEntrySwitchDoorLockState(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntrySwitchDoorLockState.");
            }
            break;
        case TW_DRUNLOCKRSP:
            ret = processEntrySwitchDoorLockState(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntrySwitchDoorLockState.");
            }
            break;
        case TW_DFTREP:
            ret = processEntryZCLDefaultResponse(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryZCLDefaultResponse.");
            }
            break;
        case TW_ZENROLLREQ:
            ret = processEntryZoneEnrollRequest(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryZoneEnrollRequest.");
            }
            break;
        case TW_ENROLLED:
            ret = processEntryEnrolled(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryEnrolled.");
            }
            break;
        case TW_ZONESTATUS:
            ret = processEntryZoneStatus(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryZoneStatus.");
            }
            break;
        case TW_ADDRESS_RESPONSE:
            ret = processEntryAddressResponse(entry);
            if (ret != TW_RESULT_OK)
            {
                OC_LOG_V(ERROR, TAG, "processEntryAddressResponse.");
            }
            break;
        default:
            OC_LOG_V(ERROR, TAG, "processEntry() doesn't receive an valid entry.");
            ret = TW_RESULT_ERROR;
            break;
    }

    OC_LOG_V(INFO, TAG, "Leave processEntry() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryNETWORK_INFO(TWEntry* entry)
{
    /*
    //at+n
    //      +N=NoPAN
    //      OK

    //at+n
    //      +N=COO,26,-6,7306,133F04EA669C6B24
    //      OK
    */

    OC_LOG_V(INFO, TAG, "Enter processEntryNETWORK_INFO()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR: %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    ret = HandleATResponse(entry);

exit:
    OC_LOG_V(INFO, TAG, "Leave processEntryNETWORK_INFO() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryJPAN(TWEntry* entry)
{
    /*
    //at+en
    //      OK
    //      JPAN:26,7306,133F04EA669C6B24

    //at+en
    //      ERROR:28
    */

    OC_LOG_V(INFO, TAG, "Enter processEntryJPAN()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) == 0)
    {
        ret = HandleATResponse(entry);
        if (ret == TW_RESULT_NEW_LOCAL_PAN_ESTABLISHED)
        {
            OC_LOG_V(INFO, TAG, "New Local PAN established.");
            ret =  TW_RESULT_OK;
        }
        else
        {
            ret = TW_RESULT_ERROR;
        }
    }
    else if (strcmp(entry->atErrorCode, AT_STR_ERROR_NODE_IS_PART_OF_PAN) == 0)
    {
        OC_LOG_V(INFO, TAG, "Already Established PAN.");
        ret = TW_RESULT_OK;
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR: %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryJPAN() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryEndDevice(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryEndDevice()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    ret = HandleATResponse(entry);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "HandleATResponse");
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryEndDevice() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryMatchDesc(TWEntry* entry)
{
    //MatchDesc:0B4A,00,01

    OC_LOG_V(INFO, TAG, "Enter processEntryMatchDesc()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
        if (ret == TW_RESULT_OK)
        {
            OC_LOG_V(INFO, TAG, "HandleATResponse");
            ret = FindClusters(g_WIPDevice->nodeId,
                               g_WIPDevice->endpointOfInterest->endpointId);
            if (ret == TW_RESULT_OK)
            {
                OC_LOG_V(INFO, TAG, "FindClusters - Found a match node");
                if (g_DeviceFoundCallback != NULL)
                {
                    OC_LOG_V(INFO, TAG, "Found a match node -- invoke callback");
                    g_DeviceFoundCallback(g_WIPDevice);
                }
                ret =  TW_RESULT_OK;
            }
            else
            {
                OC_LOG_V(ERROR, TAG, "FindClusters");
                ret = TW_RESULT_ERROR;
            }
        }
        else
        {
            OC_LOG_V(ERROR, TAG, "HandleATResponse");
            ret = TW_RESULT_ERROR;
        }

        g_WIPDevice = NULL; //reset and do not deallocate it
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryMatchDesc() with ret=%d", ret);
    return ret;
}

TWResultCode processEntrySimpleDesc(TWEntry* entry)
{
    /*
    //AT+SIMPLEDESC:3746,3746,01
    //      SEQ:97
    //      OK
    //
    //      SimpleDesc:3746,00
    //      EP:01
    //      ProfileID:0104
    //      DeviceID:0402v00
    //      InCluster:0000,0001,0003,0402,0500,0020,0B05
    //      OutCluster:0019
    //
    //      ACK:97
    */
    OC_LOG_V(INFO, TAG, "Enter processEntrySimpleDesc()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp((entry->atErrorCode), AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", (entry->atErrorCode));
        ret = TW_RESULT_ERROR;
    }
    else
    {
        if (entry->count == 6)   //must be 6 as it is the number of lines to expect
        {
            ret = HandleATResponse(entry);
            if (ret == TW_RESULT_HAS_CLUSTERS)
            {
                OC_LOG_V(INFO, TAG, "has clusters.");
                ret = TW_RESULT_OK;
            }
        }
        else
        {
            OC_LOG_V(INFO, TAG, "Received an invalid Simple Descriptor.");
            ret = TW_RESULT_ERROR;
        }
    }

    OC_LOG_V(INFO, TAG, "Leave processEntrySimpleDesc() returns with ret=%d", ret);
    return ret;
}

TWResultCode processEntryWriteAttr(TWEntry* entry)
{
    //AT+WRITEATR:3A3D,01,0,0003,0000,21,00
    //      OK
    //      WRITEATTR:3A3D,01,0003,,00

    OC_LOG_V(INFO, TAG, "Enter processEntryWriteAttr()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp((entry->atErrorCode), AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryWriteAttr() returns with ret=%d", ret);
    return ret;
}

TWResultCode processEntryReadAttr(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryWriteAttr()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp((entry->atErrorCode), AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryWriteAttr() returns with ret=%d", ret);
    return ret;
}

TWResultCode processEntryTemperature(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryTemperature()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp((entry->atErrorCode), AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryTemperature() returns with ret=%d", ret);
    return ret;
}

TWResultCode processEntrySwitchDoorLockState(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntrySwitchDoorLockState()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp((entry->atErrorCode), AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntrySwitchDoorLockState() returns with ret=%d", ret);
    return ret;
}

TWResultCode processEntryZCLDefaultResponse(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryZCLDefaultResponse()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if (strcmp((entry->atErrorCode), AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        if (strcmp(entry->atErrorCode, AT_STR_ERROR_MESSAGE_NOT_SENT_TO_TARGET_SUCCESSFULLY) == 0)
        {
            OC_LOG_V(ERROR, TAG, "Send to the target not succeed.");
            ret = TW_RESULT_ERROR;
        }
        else
        {
            OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR = %s", entry->atErrorCode);
            ret = TW_RESULT_ERROR;
        }
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryZCLDefaultResponse() returns with ret=%d", ret);
    return ret;
}

TWResultCode processEntryZoneEnrollRequest(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryZoneEnrollRequest()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR: %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryZoneEnrollRequest() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryEnrolled(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryEnrolled()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR: %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryEnrolled() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryZoneStatus(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryZoneStatus()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR: %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryZoneStatus() with ret=%d", ret);
    return ret;
}

TWResultCode processEntryAddressResponse(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter processEntryAddressResponse()");

    TWResultCode ret = TW_RESULT_UNKNOWN;
    if (strcmp(entry->atErrorCode, AT_STR_ERROR_EVERYTHING_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry contains AT_ERROR: %s", entry->atErrorCode);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = HandleATResponse(entry);
    }

    OC_LOG_V(INFO, TAG, "Leave processEntryAddressResponse() with ret=%d", ret);
    return ret;
}

TWResultCode Reset()
{
    OC_LOG_V(INFO, TAG, "Enter Reset()");

    TWResultCode ret = TW_RESULT_ERROR;
    ret = TWIssueATCommand(g_plugin, AT_CMD_RESET);
    if (ret == TW_RESULT_OK)
    {
        OC_LOG_V(INFO, TAG, "Write %s", AT_CMD_RESET);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "Write %s", AT_CMD_RESET);
    }
    OC_LOG_V(INFO, TAG, "Leave Reset() with ret=%d", ret);
    return ret;
}

TWResultCode CreatePAN()
{
    /*
    //at+n
    //      +N=NoPAN
    //      OK

    //at+n
    //      +N=COO,26,-6,7306,133F04EA669C6B24
    //      OK

    //at+en
    //      OK
    //      JPAN:26,7306,133F04EA669C6B24

    //at+en
    //      ERROR:28
    */

    OC_LOG_V(INFO, TAG, "Enter CreatePAN()");

    TWResultCode twRet1 = TW_RESULT_UNKNOWN;
    TWResultCode twRet2 = TW_RESULT_UNKNOWN;
    TWResultCode ret = TW_RESULT_UNKNOWN;
    ret = TWIssueATCommand(g_plugin, AT_CMD_GET_NETWORK_INFO);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", AT_CMD_GET_NETWORK_INFO);
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", AT_CMD_GET_NETWORK_INFO);
    TWEntry* entry = NULL;
    TWEntry* entry2 = NULL;
    ret = TWDequeueEntry(g_plugin, &entry, TW_NETWORK_INFO);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry - %s", AT_CMD_GET_NETWORK_INFO);
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL - %s", AT_CMD_GET_NETWORK_INFO);
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    ret = processEntry(entry);
    if (ret == TW_RESULT_HAS_LOCAL_PAN)
    {
        OC_LOG_V(INFO, TAG, "Has local PAN.");
        ret = TW_RESULT_OK;
    }
    else if (ret == TW_RESULT_NO_LOCAL_PAN)
    {
        OC_LOG_V(INFO, TAG, "Has no local PAN.");
        ret = TWIssueATCommand(g_plugin, AT_CMD_ESTABLISH_NETWORK);
        if (ret != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "Write %s", AT_CMD_ESTABLISH_NETWORK);
            goto exit;
        }
        OC_LOG_V(INFO, TAG, "Write %s", AT_CMD_ESTABLISH_NETWORK);

        ret = TWDequeueEntry(g_plugin, &entry2, TW_JPAN);
        if (ret != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "TWDequeueEntry - %s", AT_CMD_ESTABLISH_NETWORK);
            goto exit;
        }
        if (entry2 == NULL)
        {
            OC_LOG_V(ERROR, TAG, "TWEntry is NULL - %s", AT_CMD_ESTABLISH_NETWORK);
            ret = TW_RESULT_ERROR;
            goto exit;
        }
        ret = processEntry(entry2);
        if (ret == TW_RESULT_OK)
        {
            OC_LOG_V(INFO, TAG, "processEntry - %s", AT_CMD_ESTABLISH_NETWORK);
            g_ZigBeeStatus.state = ZB_STATE_INIT;
            ret = TW_RESULT_OK;
        }
        else
        {
            OC_LOG_V(ERROR, TAG, "processEntry - %s", AT_CMD_ESTABLISH_NETWORK);
            ret = TW_RESULT_ERROR;
        }
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "processEntry - unexpected return code: %d", ret);
        ret = TW_RESULT_ERROR;
    }

exit:
    if (entry)
    {
        twRet1 = TWDeleteEntry(g_plugin, entry);
        if(twRet1 != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "TWDeleteEntry 1 - ret=%d", twRet1);
        }
    }
    if (entry2)
    {
        twRet2 = TWDeleteEntry(g_plugin, entry2);
        if(twRet2 != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "TWDeleteEntry 2 - ret=%d", twRet2);
        }
    }

    OC_LOG_V(INFO, TAG, "Leave CreatePan with ret=%d", ret);
    return ret;
}

TWResultCode EnableJoin(bool isKeyEncrypted)
{
    //Ask:          AT+PJOIN
    //Response:     OK

    //Ask:          AT+PJOIN
    //Response:     ERROR:70

    (void)isKeyEncrypted;
    isKeyEncrypted = false;         //TODO: for now - don't encrypt

    TWResultCode ret = TW_RESULT_ERROR;
    char* joinTimeHex = "0F";       //TODO: for now - 15 seconds
    char* broadcast = "FFFC";

    int size =  strlen(AT_CMD_PERMIT_JOIN) + strlen(joinTimeHex) +
                SEPARATOR_LENGTH + strlen(broadcast) + 1;
    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }
    snprintf(cmdString, size, "%s%s%s%s",
             AT_CMD_PERMIT_JOIN, joinTimeHex, SEPARATOR, broadcast);
    ret = TWIssueATCommand(g_plugin, cmdString);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    sleep(15);  //must sleep here to permit joining for 15 seconds

exit:
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave EnableJoin() with ret=%d", ret);
    return ret;
}

TWResultCode FindMatchNodes()
{
    //AT+MATCHREQ:0104,03,0003,0006,0402,00
    //      OK
    //      MatchDesc:0B4A,00,01

    //AT+MATCHREQ:0104,03,0999,0999,0999,00
    //      OK

    OC_LOG_V(INFO, TAG, "Enter FindMatchNodes()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    char* profileHomeAutomation = "0104";
    char* inClusterCount = "04";
    char* outClusterCount = "00";

    //TODO: add more clusters
    char* clusterIdentify = "0003";
    char* clusterOnOff = "0006";
    char* clusterTemperatureMeasurement = "0402";
    char* clusterIASZone = "0500";

    int size = strlen(AT_CMD_MATCH_REQUEST) + strlen(profileHomeAutomation) +
               SEPARATOR_LENGTH + strlen(inClusterCount) +
               SEPARATOR_LENGTH + strlen(clusterIdentify) +
               SEPARATOR_LENGTH + strlen(clusterOnOff) +
               SEPARATOR_LENGTH + strlen(clusterTemperatureMeasurement) +
               SEPARATOR_LENGTH + strlen(clusterIASZone) +
               SEPARATOR_LENGTH + strlen(outClusterCount) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(INFO, TAG, "No Memory");
        ret = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
                             AT_CMD_MATCH_REQUEST, profileHomeAutomation,
                             SEPARATOR, inClusterCount,
                             SEPARATOR, clusterIdentify,
                             SEPARATOR, clusterOnOff,
                             SEPARATOR, clusterTemperatureMeasurement,
                             SEPARATOR, clusterIASZone,
                             SEPARATOR, outClusterCount);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    ret = TWIssueATCommand(g_plugin, cmdString);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s.", cmdString);
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s.", cmdString);

exit:
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave FindMatchNodes() with ret=%d", ret);
    return ret;
}

TWResultCode FindClusters(char nodeId[], char endpoint[])
{
    /*
    //AT+SIMPLEDESC:3746,3746,01
    //      SEQ:97
    //      OK
    //
    //      SimpleDesc:3746,00
    //      EP:01
    //      ProfileID:0104
    //      DeviceID:0402v00
    //      InCluster:0000,0001,0003,0402,0500,0020,0B05
    //      OutCluster:0019
    //
    //      ACK:97
    */

    OC_LOG_V(INFO, TAG, "Enter FindClusters()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    int size = strlen(AT_CMD_SIMPLE_DESC) + strlen(nodeId) +
               SEPARATOR_LENGTH + strlen(nodeId) +
               SEPARATOR_LENGTH + strlen(endpoint) + 1;

    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s%s%s",
                             AT_CMD_SIMPLE_DESC, nodeId,
                             SEPARATOR, nodeId,
                             SEPARATOR, endpoint);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    ret = TWIssueATCommand(g_plugin, cmdString);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    ret = TWDequeueEntry(g_plugin, &entry, TW_SIMPLEDESC);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry - %s", cmdString);
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL - %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    ret = processEntry(entry);
    if (ret == TW_RESULT_OK)
    {
        OC_LOG_V(INFO, TAG, "processEntry - %s", cmdString);
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "processEntry - %s", cmdString);
    }

exit:
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave FindClusters() with ret=%d", ret);
    return ret;
}

TWResultCode GetRemoteEUI(char *nodeId, char* outRemoteEUI)
{
    //AT+EUIREQ:< Address>,<NodeID>[,XX]
    //  AddrResp:<errorcode>[,<NodeID>,<EUI64>]
    //  AddrResp:00,15ED,000D6F00040574B8

    OC_LOG_V(INFO, TAG, "Enter GetRemoteEUI()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    int size = strlen(AT_CMD_REMOTE_EUI_REQUEST) + strlen(nodeId) +
               SEPARATOR_LENGTH + strlen(nodeId) + 1;
    char* cmdString = (char*)OICMalloc(size * sizeof(char));
    if (cmdString == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }

    int stringRet = snprintf(cmdString, size, "%s%s%s%s",
                         AT_CMD_REMOTE_EUI_REQUEST, nodeId,
                         SEPARATOR, nodeId);
    if(stringRet <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Build command error.");
        ret = OC_STACK_ERROR;
        goto exit;
    }
    ret = TWIssueATCommand(g_plugin, cmdString);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "Write %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Write %s", cmdString);

    TWEntry* entry = NULL;
    ret = TWDequeueEntry(g_plugin, &entry, TW_ADDRESS_RESPONSE);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "TWDequeueEntry - %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    if (entry == NULL)
    {
        OC_LOG_V(ERROR, TAG, "TWEntry is NULL - %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    ret = processEntry(entry);
    if (ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "processEntry - %s", cmdString);
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "Wanted   eui of NodeID=%s ", nodeId);
    OC_LOG_V(INFO, TAG, "Received eui of g_WIPRemoteNodeId=%s ", g_WIPRemoteNodeId);
    if (strcmp(nodeId, g_WIPRemoteNodeId) != 0)
    {
        OC_LOG_V(ERROR, TAG, "Received eui for an unexpected remote node id.");
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    OC_LOG_V(INFO, TAG, "Remote NodeId:%s has EUI: %s \n",
                        g_WIPRemoteNodeId, g_WIPRemoteEUI);
    OICStrcpy(outRemoteEUI, SIZE_EUI, g_WIPRemoteEUI);

    ret = TW_RESULT_OK;

exit:
    memset(g_WIPRemoteEUI, '\0', SIZE_EUI);
    memset(g_WIPRemoteNodeId, '\0', SIZE_NODEID);
    TWDeleteEntry(g_plugin, entry);
    OICFree(cmdString);
    OC_LOG_V(INFO, TAG, "Leave GetRemoteEUI() with ret=%d", ret);
    return ret;
}

TWResultCode HandleATResponse(TWEntry* entry)
{
    OC_LOG_V(INFO, TAG, "Enter HandleATResponse()");

    TWResultCode ret = TW_RESULT_ERROR;

    int32_t i = 0;
    for (; i < entry->count; i++)
    {
        uint32_t k = 0;
        for (; k < sizeof(g_TWATResultHandlerPairArray)/sizeof(TWATResultHandlerPair); ++k)
        {
            const char* line = (entry)->lines[i].line;
            if (strncmp(line,
                        g_TWATResultHandlerPairArray[k].resultTxt,
                        strlen(g_TWATResultHandlerPairArray[k].resultTxt)
                        ) == 0)
            {
                char* tokens[ARRAY_LENGTH] = {};
                const char* delimiters = ",\r\n";
                int paramCount = Tokenize((entry)->lines[i].line +
                                           strlen(g_TWATResultHandlerPairArray[k].resultTxt),
                                           delimiters, tokens);
                if (paramCount > 0)
                {
                    ret = g_TWATResultHandlerPairArray[k].handler(paramCount, tokens);
                }

                int n = 0;
                for (; n < paramCount; n++)
                {
                    OICFree(tokens[n]);
                }

                break;
            }
        }
    }

    OC_LOG_V(INFO, TAG, "Leave HandleATResponse() with ret=%d", ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Internal functions - AT Response/Prompt Handlers
//-----------------------------------------------------------------------------

TWResultCode TelAddressResponseHandler(int count, char* tokens[])
{
    //AT+EUIREQ:< Address>,<NodeID>[,XX]
    //  AddrResp:<errorcode>[,<NodeID>,<EUI64>]
    //  AddrResp:00,15ED,000D6F00040574B8

    OC_LOG_V(INFO, TAG, "Enter TelAddressResponseHandler()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_ADDRESS_RESPONSE)
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
    }
    else
    {
        if (strcmp(tokens[0], AT_STR_ERROR_OK) != 0)
        {
            OC_LOG_V(ERROR, TAG, "AddrResp prompt contained error status.");
            ret = TW_RESULT_ERROR;
        }
        else
        {
            OICStrcpy(g_WIPRemoteNodeId, SIZE_NODEID, tokens[1]);
            OICStrcpy(g_WIPRemoteEUI, SIZE_EUI, tokens[2]);
            OC_LOG_V(INFO, TAG, "Received eui %s for g_WIPRemoteNodeId=%s ",
                     g_WIPRemoteEUI,
                     g_WIPRemoteNodeId);
            ret = TW_RESULT_OK;
        }
    }

    OC_LOG_V(INFO, TAG, "Leave TelAddressResponseHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelNetworkInfoHandler(int count, char* tokens[])
{
    // Ask:         AT+N
    // Response:    +N=COO,24,-6,9726,12BB200F073AB573
    //                          or
    //              +N=NoPAN
    //
    //              +N=<devicetype>,<channel>,<power>,<PANID>,<EPANID>

    OC_LOG_V(INFO, TAG, "Enter TelNetworkInfoHandler()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens ||
       ((count != RESPONSE_PARAMS_COUNT_NETWORK_INFO_1) &&
        (count != RESPONSE_PARAMS_COUNT_NETWORK_INFO_5)))
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    int i = 0;
    for (; i < count; ++i)
    {
        if (tokens[i] != NULL)
        {
            OC_LOG_V(INFO, TAG, "Token[%d] = %s", i, tokens[i]);
        }
    }

    char* temp = tokens[0];
    if (strcmp(temp, "NoPAN") == 0)
    {
        OC_LOG_V(INFO, TAG, "It is NoPan.");
        ret = TW_RESULT_NO_LOCAL_PAN;
        goto exit;
    }

    OC_LOG_V(INFO, TAG, "Already have an established network.");
    ret = AsciiHexToValue(tokens[3], strlen(tokens[3]), &g_ZigBeeStatus.panId);
    if(ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "AsciiHexToValue - panId");
        goto exit;
    }
    ret = AsciiHexToValue(tokens[4], strlen(tokens[4]), &g_ZigBeeStatus.extPanId);
    if(ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "AsciiHexToValue - extPanId");
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "PanId=%" PRId64 , g_ZigBeeStatus.panId);
    OC_LOG_V(INFO, TAG, "ExtPanId=%" PRId64 , g_ZigBeeStatus.extPanId);
    OC_LOG_V(INFO, TAG, "PanId=%s", tokens[3]);
    OC_LOG_V(INFO, TAG, "ExtPanId=%s", tokens[4]);

    OC_LOG_V(INFO, TAG, "TelNetworkInfoHandler set ExtPanId to %08X%08X",
             (unsigned int)(g_ZigBeeStatus.extPanId >> 32),
             (unsigned int)(g_ZigBeeStatus.extPanId & 0xFFFFFFFF));

    ret = TW_RESULT_HAS_LOCAL_PAN;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelNetworkInfoHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelJpanHandler(int count, char* tokens[])
{
    //Ask:        AT+EN:[<channel>],[<POWER>],[<PANID>]
    //Response:   JPAN:<channel>,<PANID>,<EPANID>

    OC_LOG_V(INFO, TAG, "Enter TelJpanHandler()");

    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_JPAN)
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    ret = AsciiHexToValue(tokens[1], strlen(tokens[1]), &g_ZigBeeStatus.panId);
    if(ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "AsciiHexToValue - panId");
        goto exit;
    }
    ret = AsciiHexToValue(tokens[2], strlen(tokens[2]), &g_ZigBeeStatus.extPanId);
    if(ret != TW_RESULT_OK)
    {
        OC_LOG_V(ERROR, TAG, "AsciiHexToValue - extPanId");
        goto exit;
    }
    OC_LOG_V(INFO, TAG, "PanId = %" PRId64 "\n", g_ZigBeeStatus.panId);
    OC_LOG_V(INFO, TAG, "ExtPanId = %" PRId64 "\n", g_ZigBeeStatus.extPanId);
    ret = TW_RESULT_NEW_LOCAL_PAN_ESTABLISHED;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelJpanHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelEndDeviceJoinHandler(int count, char* tokens[])
{
    //Ask:      AT+PJOIN
    //
    //Prompt:   RFD:<IEEE Address>,<NodeID>
    //Prompt:   FFD:<IEEE Address>,<NodeID>
    //Prompt:   SED:<IEEE Address>,<NodeID>
    //Prompt:   ZED:<IEEE Address>,<NodeID>

    OC_LOG_V(INFO, TAG, "Enter TelEndDeviceJoinHandler()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_DEVICE_JOINED)
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    //TODO: Might need to add into the list if needed - log it for now.
    OC_LOG_V(INFO, TAG, "Just Joined - EUI:%s; NodeID:%s.\n", tokens[0], tokens[1]);
    ret = TW_RESULT_OK;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelEndDeviceJoinHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelMatchDescHandler(int count, char* tokens[])
{
    //Prompt:       MatchDesc:0B4A,00,01

    OC_LOG_V(INFO, TAG, "Enter TelMatchDescHandler()");
    TWResultCode ret = TW_RESULT_ERROR;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_MATCH_DESC)
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    if (strcmp(tokens[1], AT_STR_ERROR_OK) != 0)
    {
        OC_LOG_V(INFO, TAG, "MatchDesc prompt contained error status.");
        ret = TW_RESULT_ERROR;
        goto exit;
    }
    else
    {
        char remoteEUI[SIZE_EUI];
        ret = GetRemoteEUI(tokens[0], remoteEUI);
        if (ret != TW_RESULT_OK)
        {
            OC_LOG_V(ERROR, TAG, "GetRemoteEUI()");
            goto exit;
        }
        else
        {
            //Step 1: Create TWDevice
            TWDevice* device = (TWDevice*)OICCalloc(1, sizeof(TWDevice));
            if (device == NULL)
            {
                ret = TW_RESULT_ERROR_NO_MEMORY;
                goto exit;
            }
            else
            {
                device->endpointOfInterest = (TWEndpoint*)OICCalloc(1, sizeof(TWEndpoint));
                if (device->endpointOfInterest == NULL)
                {
                    OICFree(device);
                    ret = TW_RESULT_ERROR_NO_MEMORY;
                }
                else
                {
                    OICStrcpy(device->eui, SIZE_EUI, remoteEUI);
                    OICStrcpy(device->nodeId, SIZE_NODEID, tokens[0]);
                    OICStrcpy(device->endpointOfInterest->endpointId, SIZE_ENDPOINTID, tokens[2]);
                    g_WIPDevice = device;

                    //Step 2: Add to list
                    if (g_FoundMatchedDeviceList == NULL)
                    {
                        //Create a list of promptCount entries
                        g_FoundMatchedDeviceList = (TWDeviceList*)OICMalloc(sizeof(TWDeviceList));
                        if (g_FoundMatchedDeviceList == NULL)
                        {
                            OICFree(device->endpointOfInterest);
                            OICFree(device);
                            ret = TW_RESULT_ERROR_NO_MEMORY;
                        }
                        else
                        {
                            g_FoundMatchedDeviceList->count = 1;
                            g_FoundMatchedDeviceList->deviceList =
                                    (TWDevice*)OICMalloc(sizeof(TWDevice));
                            if (g_FoundMatchedDeviceList->deviceList == NULL)
                            {
                                OICFree(device->endpointOfInterest);
                                OICFree(device);
                                ret = TW_RESULT_ERROR_NO_MEMORY;
                            }
                            else
                            {
                                memcpy(g_FoundMatchedDeviceList->deviceList,
                                       device,
                                       sizeof(TWDevice));
                                ret = TW_RESULT_OK;
                            }
                        }
                    }
                    else
                    {
                        //Expand the list
                        int newSize = sizeof(TWDevice) * (g_FoundMatchedDeviceList->count + 1);
                        TWDevice* temp = (TWDevice*)realloc(g_FoundMatchedDeviceList->deviceList,
                                                            newSize);
                        if (temp == NULL)
                        {
                            OICFree(device->endpointOfInterest);
                            OICFree(device);
                            ret =TW_RESULT_ERROR_NO_MEMORY;
                        }
                        else
                        {
                            g_FoundMatchedDeviceList->deviceList = temp;

                            //Add to the end of list
                            int count = g_FoundMatchedDeviceList->count;
                            memcpy(&g_FoundMatchedDeviceList->deviceList[count],
                                   device,
                                   sizeof(TWDevice));

                            //Increase the count
                            g_FoundMatchedDeviceList->count++;

                            ret = TW_RESULT_OK;
                        }
                    }
                }
            }
        }
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelMatchDescHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelSimpleDescHandler(int count, char* tokens[])
{
    //AT+SIMPLEDESC:3746,3746,01
    //      SEQ:97
    //      OK
    //
    //      SimpleDesc:3746,00              <<<<<<<---------------------
    //      EP:01
    //      ProfileID:0104
    //      DeviceID:0402v00
    //      InCluster:0000,0001,0003,0402,0500,0020,0B05
    //      OutCluster:0019
    //      ACK:97

    OC_LOG_V(INFO, TAG, "Enter TelSimpleDescHandler().");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_SIMPLE_DESC)
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    if (g_WIPDevice == NULL)
    {
        OC_LOG_V(ERROR, TAG,
                 "Receive simple descriptor unexpectedly - %s", tokens[0]);
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    if (strcmp(tokens[1], AT_STR_ERROR_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "SimpleDesc: prompt contained error status %s.", tokens[1]);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        if (strcmp(tokens[0], g_WIPDevice->nodeId) == 0)
        {
            OC_LOG_V(INFO, TAG, "Got simple descriptor for nodeid %s", tokens[0]);
            ret = TW_RESULT_OK;
        }
        else
        {
            OC_LOG_V(ERROR, TAG,
                     "Finding simple descriptor for non existing nodeid %s.", tokens[0]);
            ret = TW_RESULT_ERROR;
        }
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelSimpleDescHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelSimpleDescInClusterHandler(int count, char* tokens[])
{
    //AT+SIMPLEDESC:3746,3746,01
    //      SEQ:97
    //      OK
    //
    //      SimpleDesc:3746,00
    //      EP:01
    //      ProfileID:0104
    //      DeviceID:0402v00
    //      InCluster:0000,0001,0003,0402,0500,0020,0B05        <<<<<<<<--------------
    //      OutCluster:0019
    //      ACK:97

    OC_LOG_V(INFO, TAG, "Enter TelSimpleDescInClusterHandler()");
    TWResultCode ret = TW_RESULT_ERROR;

    if (!tokens || count < RESPONSE_PARAMS_COUNT_SIMPLE_DESC_IN_CLUSTER_MIN )
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
	        goto exit;
    }

    if (g_WIPDevice == NULL)
    {
        OC_LOG_V(ERROR, TAG,
                 "Receive simple descriptor unexpectedly - %s", tokens[0]);
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    if (g_WIPDevice->endpointOfInterest->clusterList != NULL)
    {
        OC_LOG_V(ERROR, TAG, "Expected an empty cluster list.");
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    //Add found clusters for the node.
    g_WIPDevice->endpointOfInterest->clusterList =
            (TWClusterList*)OICMalloc(sizeof(TWClusterList));
    if (g_WIPDevice->endpointOfInterest->clusterList == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory - clusterList");
        ret = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }

    g_WIPDevice->endpointOfInterest->clusterList->clusterIds =
            (TWClusterId*)OICMalloc(sizeof(TWClusterId) * count);
    if (g_WIPDevice->endpointOfInterest->clusterList->clusterIds == NULL)
    {
        OICFree(g_WIPDevice->endpointOfInterest->clusterList);
        OC_LOG_V(ERROR, TAG, "No Memory - clusterIds");
        ret = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }

    int i = 0;
    for (; i < count; i++)
    {
        OICStrcpy(g_WIPDevice->endpointOfInterest->clusterList->
                    clusterIds[i].clusterId,
                    SIZE_CLUSTERID,
                    tokens[i]);

        OC_LOG_V(INFO, TAG, "ClusterIds[%d]=%s",
                 i,
                 g_WIPDevice->endpointOfInterest->
                 clusterList->clusterIds[i].clusterId);
    }
    g_WIPDevice->endpointOfInterest->clusterList->count = count;
    ret = TW_RESULT_HAS_CLUSTERS;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelSimpleDescInClusterHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelWriteAttrHandler(int count, char* tokens[])
{
    //AT+WRITEATR:3A3D,01,0,0003,0000,21,00
    //      OK
    //      WRITEATTR:3A3D,01,0003,,00
    //
    //AT+WRITEATR:B826,01,0,0500,0010,F0,000D6F0000D59E92
    //      OK
    //      WRITEATTR:B826,01,0500,0010,70

    OC_LOG_V(INFO, TAG, "Enter TelWriteAttrHandler()");

    TWResultCode ret = TW_RESULT_ERROR;

    if(!tokens ||
       (count < RESPONSE_PARAMS_COUNT_WRITE_ATTR_4) ||
       (count > RESPONSE_PARAMS_COUNT_WRITE_ATTR_5))
    {
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    if (count == RESPONSE_PARAMS_COUNT_WRITE_ATTR_4)
    {
        if (strcmp(tokens[3], AT_STR_ERROR_OK) == 0)
        {
            ret = TW_RESULT_OK;
        }
    }
    else if (count == RESPONSE_PARAMS_COUNT_WRITE_ATTR_5)
    {
        if (strcmp(tokens[4], AT_STR_ERROR_INVALID_OP) == 0)
        {
            ret = TW_RESULT_ERROR_INVALID_OP;
        }
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelWriteAttrHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelReadAttrHandlerTemperature(int count, char* tokens[])
{
    //AT+READATR:F2D7,01,0,0402,0002
    //      OK
    //      TEMPERATURE:F2D7,01,0002,00,1770
    //
    //AT+READATR:F2D7,01,0,0402,0002
    //      OK
    //      ERROR:66

    OC_LOG_V(INFO, TAG, "Enter TelReadAttrHandlerTemperature().");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_TEMPERATURE)
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    if (strcmp(tokens[3], AT_STR_ERROR_OK) != 0)
    {
        OC_LOG_V(ERROR, TAG, "TEMPERATURE prompt contained error status.");
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    // AttrInfo is 16-bit value representing (100 * Degrees Celsius)
    // so 0x812 = 20.66 C = 69.188 F
    if (g_ZigBeeStatus.remoteAttributeValueRead != NULL)
    {
        OICFree(g_ZigBeeStatus.remoteAttributeValueRead);
        g_ZigBeeStatus.remoteAttributeValueRead = NULL;
    }
    OC_LOG_V(INFO, TAG, "Read Attribute Value: %s", tokens[4]);
    g_ZigBeeStatus.remoteAttributeValueRead =
            (char*)OICMalloc(sizeof(char) * strlen(tokens[4]));
    if (g_ZigBeeStatus.remoteAttributeValueRead == NULL)
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = TW_RESULT_ERROR_NO_MEMORY;
    }
    else
    {
        strcpy(g_ZigBeeStatus.remoteAttributeValueRead, tokens[4]);
        g_ZigBeeStatus.remoteAtrributeValueReadLength = strlen(tokens[4]);
        ret = TW_RESULT_REMOTE_ATTR_HAS_VALUE;
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelReadAttrHandlerTemperature() with ret=%d", ret);
    return ret;
}

TWResultCode TelReadAttrHandler(int count, char* tokens[])
{
    //AT+READATR:F2D7,01,0,0402,0002
    //      OK
    //      RESPATTR:<NodeID>,<EP>,<ClusterID>,<AttrID>,<Status>,<AttrInfo>
    //
    //AT+READATR:F2D7,01,0,0402,0002
    //      OK
    //      ERROR:66

    OC_LOG_V(INFO, TAG, "Enter TelReadAttrHandler()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_RESPATTR)
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    if (strcmp(tokens[4], AT_STR_ERROR_OK) != 0)
    {
        OC_LOG_V(INFO, TAG, "READATTR prompt contained error status.");
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    if (g_ZigBeeStatus.remoteAttributeValueRead != NULL)
    {
        OICFree(g_ZigBeeStatus.remoteAttributeValueRead);
    }
    OC_LOG_V(INFO, TAG, "Read Attribute Value: %s.", tokens[5]);
    g_ZigBeeStatus.remoteAttributeValueRead =
            (char*)OICMalloc(sizeof(char) * strlen(tokens[5]));
    if (g_ZigBeeStatus.remoteAttributeValueRead != NULL)
    {
        strcpy(g_ZigBeeStatus.remoteAttributeValueRead, tokens[5]);
        g_ZigBeeStatus.remoteAtrributeValueReadLength = strlen(tokens[5]);
        ret = TW_RESULT_REMOTE_ATTR_HAS_VALUE;
    }
    else
    {
        OC_LOG_V(ERROR, TAG, "No Memory");
        ret = TW_RESULT_ERROR_NO_MEMORY;
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelReadAttrHandler().\n");
    return ret;
}

TWResultCode TelZCLDefaultResponseHandler(int count, char* tokens[])
{
    //AT+RONOFF:<Address>,<EP>,<SendMode>[,<ON/OFF>]
    //      DFTREP:<NodeID>,<EP>,<ClusterID>,<CMD>,<Status>
    //
    //AT+DRLOCK:<Address>,<EP>,<SendMode>,<Lock/Unlock>
    //      DFTREP:<NodeID>,<EP>,<ClusterID>,<CMD>,<Status>
    //
    //AT+LCMVTOLEV:<Address>,<EP>,<SendMode>,<ON/OFF>,<LevelValue>,<TransTime>
    //      DFTREP:<NodeID>,<EP>,<ClusterID>,<CMD>,<Status>

    OC_LOG_V(INFO, TAG, "Enter TelZCLDefaultResponseHandler()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_DFTREP)
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    OC_LOG_V(INFO, TAG,
             "DFTREP prompt succeed for NodeId:%s, EP:%s, ClusterId:%s, CMD:%s.\n",
             tokens[0], tokens[1], tokens[2], tokens[3]);

    if (strcmp(tokens[4], AT_STR_ERROR_OK) != 0)
    {
        ret = TW_RESULT_ERROR;
    }
    else
    {
        ret = TW_RESULT_OK;
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelZCLDefaultResponseHandler()");
    return ret;
}

TWResultCode TelSwitchDoorLockStateHandler(int count, char* tokens[])
{
    //AT+DRLOCK:<Address>,<EP>,<SendMode>,<Lock/Unlock>
    //      DRLOCRSP:<nodeID>,<ep>,<status>
    //      or
    //      DRUNLOCKRSP:<nodeID>,<ep>,<status>

    OC_LOG_V(INFO, TAG, "Enter TelSwitchDoorLockStateHandler()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_DRLOCKUNLOCKRSP)
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    if (strcmp(tokens[2], AT_STR_ERROR_OK) != 0)
    {
        OC_LOG_V(INFO, TAG,
                 "DRLOCRSP/DRUNLOCKRSP prompt contained error status %s.", tokens[4]);
        ret = TW_RESULT_ERROR;
    }
    else
    {
        OC_LOG_V(INFO, TAG, "DRLOCRSP/DRUNLOCKRSP prompt succeed for nodeId:%s, ep:%s.",
                 tokens[0], tokens[1]);
        ret = TW_RESULT_OK;
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave TelSwitchDoorLockStateHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelZoneEnrollRequestHandler(int count, char* tokens[])
{
    //ZENROLLREQ:<NodeID>,<EndPoint>,<ZoneType>,<ManufactureCode>

    OC_LOG_V(INFO, TAG, "Enter TelZoneEnrollRequestHandler()");
    TWResultCode ret = TW_RESULT_UNKNOWN;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_ZENROLLREQ)
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    OC_LOG_V(INFO, TAG, "Received zone request from:");
    OC_LOG_V(INFO, TAG, "Node:%s", tokens[0]);
    OC_LOG_V(INFO, TAG, "EP:%s", tokens[1]);
    OC_LOG_V(INFO, TAG, "ZoneType:%s", tokens[2]);
    OC_LOG_V(INFO, TAG, "ManufactureCode:%s", tokens[3]);
    ret = TW_RESULT_OK;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelZoneEnrollRequestHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelEnrolledHandler(int count, char* tokens[])
{
    //ENROLLED:<ZID>,<ZoneType>,<EUI>

    OC_LOG_V(INFO, TAG, "Enter TelEnrolledHandler()");
    TWResultCode ret = TW_RESULT_OK;

    if(!tokens || count != RESPONSE_PARAMS_COUNT_ENROLLED)
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    OC_LOG_V(INFO, TAG, "Received zone enrollment for:");
    OC_LOG_V(INFO, TAG, "ZID:%s", tokens[0]);
    OC_LOG_V(INFO, TAG, "ZoneType:%s", tokens[1]);
    OC_LOG_V(INFO, TAG, "EUI:%s", tokens[2]);

    TWEnrollee enrollee;
    OICStrcpy(enrollee.zoneId, SIZE_ZONEID, tokens[0]);
    OICStrcpy(enrollee.zoneType, SIZE_ZONETYPE, tokens[1]);
    OICStrcpy(enrollee.eui, SIZE_EUI, tokens[2]);

    if (g_EnrollmentSucceedCallback != NULL)
    {
        OC_LOG_V(INFO, TAG, "Enrolled - Invoke callback");
        g_EnrollmentSucceedCallback(&enrollee);
    }
    ret = TW_RESULT_OK;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelEnrolledHandler() with ret=%d", ret);
    return ret;
}

TWResultCode TelZoneStatusHandler(int count, char* tokens[])
{
    //ZONESTATUS:<NodeID>,<EP>,<ZoneStatus>,<ExtendStatus>[,<ZoneID>,<Delay>]
    //ZONESTATUS:5FBA,01,0021,00,01,00AF

    OC_LOG_V(INFO, TAG, "Enter TelZoneStatusHandler()");
    TWResultCode ret = TW_RESULT_UNKNOWN;
    if(!tokens ||
       ((count != RESPONSE_PARAMS_COUNT_ZONESTATUS_4) &&
        (count != RESPONSE_PARAMS_COUNT_ZONESTATUS_6)))
    {
        OC_LOG_V(ERROR, TAG, "Invalid Params");
        ret = TW_RESULT_ERROR_INVALID_PARAMS;
        goto exit;
    }

    TWUpdate update;
    OICStrcpy(update.nodeId, SIZE_NODEID, tokens[0]);
    OICStrcpy(update.endpoint, SIZE_NODEID, tokens[1]);
    OICStrcpy(update.status, SIZE_NODEID, tokens[2]);
    OICStrcpy(update.extendedStatus, SIZE_NODEID, tokens[3]);

    if (count == RESPONSE_PARAMS_COUNT_ZONESTATUS_6)
    {
        OICStrcpy(update.zoneId, SIZE_NODEID, tokens[4]);
        OICStrcpy(update.delay, SIZE_NODEID, tokens[5]);
    }

    if (g_DeviceStatusUpdateCallback != NULL)
    {
        OC_LOG_V(INFO, TAG, "device status update - invoke callback");
        g_DeviceStatusUpdateCallback(&update);
    }
    ret = TW_RESULT_OK;

exit:
    OC_LOG_V(INFO, TAG, "Leave TelZoneStatusHandler() with ret=%d", ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Internal functions - Helpers
//-----------------------------------------------------------------------------

/**
 *
 * Tokenize 'input' parameter by 'delimiters' into 'output' array.
 *
 */
int Tokenize(const char *input, const char* delimiters, char* output[])
{
    OC_LOG_V(INFO, TAG, "Enter Tokenize() - %s", input);

    if (output == NULL)
    {
        OC_LOG_V(INFO, TAG, "Invalid parameter.");
        return -1;
    }

    int length = strlen(input);
    char * str = (char *) OICCalloc(1, length + 1);
    OICStrcpy(str, length+1, input);

    char* savePtr = NULL;
    char* p   = strtok_r(str, delimiters, &savePtr);
    int index = 0;
    while (p && index <= ARRAY_LENGTH)
    {
        int size = strlen(p) + 1;   //for null char
        output[index] = (char*)OICCalloc(size, sizeof(char));
        OICStrcpy(output[index], size, p);
        OC_LOG_V(INFO, TAG, "Token[%d]=%s", index, output[index]);
        p = strtok_r (NULL, delimiters, &savePtr);
        index++;
    }

    OICFree(str);
    OC_LOG_V(INFO, TAG, "Leave Tokenize()");
    return index;
}

int AsciiToHex(char c)
{
    int num = (int) c;
    if(c >= '0' && c <= '9')
    {
        return num - '0';
    }

    if(num >= 'A' && num <= 'F')
    {
        return num - 'A' + 10;
    }
    return -1;
}

TWResultCode AsciiHexToValue(char* hexString, int length, uint64_t* value)
{
    if(!hexString || !value || length < 0)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    int retVal = AsciiToHex(hexString[0]);
    if(retVal == -1)
    {
        OC_LOG(ERROR, TAG, "Bad conversion from ASCII To Hex.");
        return TW_RESULT_ERROR;
    }
    *value = (uint64_t)retVal;
    for (int i = 1; i < length; ++i)
    {
        if (sizeof(hexString) > (uint32_t)i)
        {
            *value <<= 4;
            retVal = AsciiToHex(hexString[i]);
            if(retVal == -1)
            {
                OC_LOG(ERROR, TAG, "Bad conversion from ASCII To Hex.");
                return TW_RESULT_ERROR;
            }
            *value |= (uint64_t)retVal;
        }
    }
    return TW_RESULT_OK;
}

/**
 *
 * Deallocate device list.
 *
 */
void DeallocateTWDeviceList()
{
    if (g_FoundMatchedDeviceList == NULL)
    {
        return;
    }

    if (g_FoundMatchedDeviceList->deviceList == NULL)
    {
        OICFree(g_FoundMatchedDeviceList);
        g_FoundMatchedDeviceList = NULL;
        return;
    }

    if (g_FoundMatchedDeviceList->deviceList->endpointOfInterest == NULL)
    {
        OICFree(g_FoundMatchedDeviceList->deviceList);
        g_FoundMatchedDeviceList->deviceList = NULL;

        OICFree(g_FoundMatchedDeviceList);
        g_FoundMatchedDeviceList = NULL;
        return;
    }

    if (g_FoundMatchedDeviceList->deviceList->endpointOfInterest->clusterList == NULL)
    {
        OICFree(g_FoundMatchedDeviceList->deviceList->endpointOfInterest);
        g_FoundMatchedDeviceList->deviceList->endpointOfInterest = NULL;

        OICFree(g_FoundMatchedDeviceList->deviceList);
        g_FoundMatchedDeviceList->deviceList = NULL;

        OICFree(g_FoundMatchedDeviceList);
        g_FoundMatchedDeviceList = NULL;
        return;
    }

    if (g_FoundMatchedDeviceList->deviceList->endpointOfInterest-> clusterList->clusterIds == NULL)
    {
        OICFree(g_FoundMatchedDeviceList->deviceList->endpointOfInterest->clusterList);
        g_FoundMatchedDeviceList->deviceList->endpointOfInterest->clusterList = NULL;

        OICFree(g_FoundMatchedDeviceList->deviceList->endpointOfInterest);
        g_FoundMatchedDeviceList->deviceList->endpointOfInterest = NULL;

        OICFree(g_FoundMatchedDeviceList->deviceList);
        g_FoundMatchedDeviceList->deviceList = NULL;

        OICFree(g_FoundMatchedDeviceList);
        g_FoundMatchedDeviceList = NULL;
        return;
    }

    OICFree(g_FoundMatchedDeviceList->deviceList->endpointOfInterest-> clusterList->clusterIds);
    g_FoundMatchedDeviceList->deviceList->endpointOfInterest->clusterList->clusterIds = NULL;

    OICFree(g_FoundMatchedDeviceList->deviceList->endpointOfInterest->clusterList);
    g_FoundMatchedDeviceList->deviceList->endpointOfInterest->clusterList = NULL;

    OICFree(g_FoundMatchedDeviceList->deviceList->endpointOfInterest);
    g_FoundMatchedDeviceList->deviceList->endpointOfInterest = NULL;

    OICFree(g_FoundMatchedDeviceList->deviceList);
    g_FoundMatchedDeviceList->deviceList = NULL;

    OICFree(g_FoundMatchedDeviceList);
    g_FoundMatchedDeviceList = NULL;
}
