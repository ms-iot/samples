/******************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include <ctype.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cacommon.h"
#include "cainterface.h"
#include "Arduino.h"

#ifdef ARDUINOWIFI
#include "WiFi.h"
#elif defined ARDUINOETH
#include "Ethernet.h"
#endif

#include "oic_malloc.h"

#define MAX_BUF_LEN 100 //1024
#define MAX_OPT_LEN 16
#define PORT_LENGTH 5

static bool g_isLeSelected = false;

static void PrintMenu();
static void Process();
static void Initialize();
static void StartListeningServer();
static void StartDiscoveryServer();
static void SendRequest();
static void SendRequestAll();
static void SendResponse(CAEndpoint_t *endpoint, const CAInfo_t* info);
static void SendNotification();
static void SelectNetwork();
static void UnselectNetwork();
static void HandleRequestResponse();
static void GetNetworkInfo();

static void RequestHandler(const CAEndpoint_t *object, const CARequestInfo_t *requestInfo);
static void ResponseHandler(const CAEndpoint_t *object, const CAResponseInfo_t *responseInfo);
static void ErrorHandler(const CAEndpoint_t *object, const CAErrorInfo_t* errorInfo);
static void Terminate();

void GetData(char *readInput, size_t bufferLength, size_t *dataLength)
{
    if (!readInput || bufferLength == 0 || !dataLength)
    {
        Serial.println("Invalid buffer");
        return;
    }

    while (!Serial.available())
    {
        delay(500);
    }
    int len = 0;
    while (Serial.available())
    {
        delay(100);
        char c = Serial.read();
        if ('\n' != c && '\r' != c && len < bufferLength - 1)
        {
            readInput[len++] = c;
        }
        else
        {
            break;
        }
    }

    readInput[len] = '\0';
    Serial.flush();
    Serial.print("PD: ");
    Serial.println(readInput);
    (*dataLength) = len;
}

bool ParseData(char *buf, char *url, char *port, char *resourceUri)
{
    char *slash = strchr(buf, '/');
    if (!slash)
    {
        return false;
    }

    strcpy(resourceUri, slash);

    char *dot = strchr(buf, '.');
    if (dot && dot < slash)
    {
        char *colon = strchr(buf, ':');

        if (colon)
        {
            strncpy(port, colon, slash - colon);
            memmove(port, port+1, strlen(port));
        }
        if (colon && colon < slash)
        {
            strncpy(url, buf, colon - buf);
            return true;
        }
    }

    strncpy(url, buf, slash - buf);
    return true;
}

CATransportAdapter_t GetConnectivityType()
{
    char type[2] = {0};
    Serial.println("Select network");
    Serial.println("IP: 0");
    Serial.println("GATT (BLE): 1");
    Serial.println("RFCOMM (EDR): 2");

    size_t typeLen = 0;
    GetData(type, sizeof(type), &typeLen);
    if (0 >= typeLen)
    {
        Serial.println("i/p err,default ethernet");
        return CA_ADAPTER_IP;
    }
    switch (type[0])
    {
        case '0':
            return CA_ADAPTER_IP;
        case '1':
            return CA_ADAPTER_GATT_BTLE;
        case '2':
            return CA_ADAPTER_RFCOMM_BTEDR;
    }
    return CA_ADAPTER_IP;
}

void setup()
{
    Serial.begin (115200);

    Serial.println("============");
    Serial.println("CA SAMPLE");
    Serial.println("============");
    PrintMenu();
}

void loop()
{
    char buffer[5] = {0};
    size_t len;
    if (Serial.available() > 0)
    {
        GetData(buffer, sizeof(buffer), &len);
        if (0 >= len)
        {
            Serial.println("i/p err");
            return;
        }
        switch (toupper(buffer[0]))
        {
            case 'M': // menu
                PrintMenu();
                break;

            case 'Q': // quit
                Serial.println("quit");
                return;

            case 'I': // Initialize interface
                Initialize();
                break;

            case 'S': // start listening server
                StartListeningServer();
                break;

            case 'D': // start discovery server
                StartDiscoveryServer();
                break;

            case 'R': // send request
                SendRequest();
                break;

            case 'E': //send request to all
                SendRequestAll();
                break;
            case 'B': // send notification
                SendNotification();
                break;
            case 'G': // Get network info
                GetNetworkInfo();
                break;

            case 'N': // select network
                SelectNetwork();
                break;

            case 'X': // unselect network
                UnselectNetwork();
                break;

            case 'H': // handle request response
                HandleRequestResponse();
                break;

            case 'T': // handle request response
                Terminate();
                break;

            default:
                Serial.println("wrong menu");
                break;
        }
    }
    //1:Add check for startserver before calling below api
    if (g_isLeSelected)
    {
        HandleRequestResponse();
    }
    delay(1000);
}

void Initialize()
{
    if(CAInitialize() != CA_STATUS_OK)
    {
        Serial.println("Initialize failed");
        return;
    }
    SelectNetwork();
    // set handler.
    CARegisterHandler(RequestHandler, ResponseHandler, ErrorHandler);
}

void StartListeningServer()
{
    Serial.println("listening server");
    CAResult_t ret = CAStartListeningServer();
    if(ret != CA_STATUS_OK)
    {
        Serial.print("listening failed: ");
        Serial.println(ret);
        return;
    }
}

void StartDiscoveryServer()
{
    Serial.println("discovery server");
    CAResult_t ret = CAStartDiscoveryServer();
    if(ret != CA_STATUS_OK)
    {
        Serial.print("discovery failed: ");
        Serial.println(ret);
        return;
    }
}

void SendRequest()
{
    char buf[MAX_BUF_LEN] = {0};
    char address[MAX_BUF_LEN] = {0};
    char resourceUri[MAX_BUF_LEN] = {0};
    char port[PORT_LENGTH] = {0};
    CATransportAdapter_t selectedNetwork;
    selectedNetwork = GetConnectivityType();

    Serial.println("============");
    Serial.println("10.11.12.13:4545/res_uri (for IP)");
    Serial.println("10:11:12:13:45:45/res_uri (for BT)");
    Serial.println("uri: ");

    size_t len = 0;
    GetData(buf, sizeof(buf), &len);
    if (0 >= len)
    {
        Serial.println("i/p err");
        return;
    }

    if (!ParseData(buf, address, port, resourceUri))
    {
        Serial.println("bad uri");
        return;
    }

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    CAResult_t res = CACreateEndpoint(CA_DEFAULT_FLAGS, selectedNetwork, address, atoi(port),
                                      &endpoint);
    if (res != CA_STATUS_OK)
    {
        Serial.println("Out of memory");
        CADestroyEndpoint(endpoint);
        return;
    }

    memset(buf, 0, sizeof(buf));

    Serial.println("\n=============================================\n");
    Serial.println("0:CON, 1:NON\n");
    Serial.println("select message type : ");
    GetData(buf, sizeof(buf), &len);
    CAMessageType_t msgType = CA_MSG_CONFIRM;

    if (0 >= len)
    {
        Serial.println("i/p err,default: 0");
    }
    else if(buf[0] == '1')
    {
        msgType = CA_MSG_NONCONFIRM;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if (res != CA_STATUS_OK || (!token))
    {
        Serial.println("token error");
        CADestroyEndpoint(endpoint);
        return;
    }

    Serial.println(token);
    CAInfo_t requestData = {CA_MSG_RESET};
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.payload = (CAPayload_t)"Json Payload";
    requestData.payloadSize = strlen((const char *) requestData.payload);

    requestData.type = msgType;
    requestData.resourceUri = (char *)OICMalloc(strlen(resourceUri) + 1);
    strcpy(requestData.resourceUri, resourceUri);

    CARequestInfo_t requestInfo = {CA_GET, {CA_MSG_RESET}};
    requestInfo.method = CA_GET;
    requestInfo.isMulticast = false;
    requestInfo.info = requestData;
    requestInfo.isMulticast = false;

    // send request
    CASendRequest(endpoint, &requestInfo);
    if (NULL != token)
    {
        CADestroyToken(token);
    }

    // destroy remote endpoint
    if (endpoint != NULL)
    {
        CADestroyEndpoint(endpoint);
    }

    Serial.println("============");
}

void SendRequestAll()
{
    char buf[MAX_BUF_LEN] = {0};
    char address[MAX_BUF_LEN] = {0};
    char resourceUri[MAX_BUF_LEN] = {0};
    char port[PORT_LENGTH] = {0};

    CATransportAdapter_t selectedNetwork;
    selectedNetwork = GetConnectivityType();

    Serial.println("\n=============================================\n");
    Serial.println("ex) /a/light\n");
    Serial.println("resource uri : ");

    size_t len = 0;
    GetData(buf, sizeof(buf), &len);
    if (0 >= len)
    {
        Serial.println("i/p err");
        return;
    }

    if (!ParseData(buf, address, port, resourceUri))
    {
        Serial.println("bad uri");
        return;
    }

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    CAResult_t res = CACreateEndpoint(CA_IPV4, selectedNetwork, address, atoi(port),
                                        &endpoint);

    if (res != CA_STATUS_OK)
    {
        Serial.println("create remote endpoint error");
        CADestroyEndpoint(endpoint);
        return;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if (res != CA_STATUS_OK || (!token))
    {
        Serial.println("token error");
        CADestroyEndpoint(endpoint);
        return;
    }

    Serial.println(token);

    CAInfo_t requestData = {CA_MSG_RESET};
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.payload = (CAPayload_t)"Temp Json Payload";
    requestData.payloadSize = strlen((const char *) requestData.payload);
    requestData.type = CA_MSG_NONCONFIRM;
    requestData.resourceUri = (char *)OICMalloc(strlen(resourceUri) + 1);
    strcpy(requestData.resourceUri, resourceUri);

    CARequestInfo_t requestInfo = {CA_GET, {CA_MSG_RESET}};
    requestInfo.method = CA_GET;
    requestInfo.isMulticast = true;
    requestInfo.info = requestData;

    // send request
    CASendRequest(endpoint, &requestInfo);

    if (NULL != token)
    {
        CADestroyToken(token);
    }

    // destroy remote endpoint
    if (endpoint != NULL)
    {
        CADestroyEndpoint(endpoint);
    }

    Serial.println("==========");
}

void SendNotification()
{
    char buf[MAX_BUF_LEN] = {0};
    char address[MAX_BUF_LEN] = {0};
    char resourceUri[MAX_BUF_LEN] = {0};
    char port[PORT_LENGTH] = {0};
    CATransportAdapter_t selectedNetwork;
    selectedNetwork = GetConnectivityType();

    Serial.println("============");
    Serial.println("10.11.12.13:4545/res_uri (for IP)");
    Serial.println("10:11:12:13:45:45/res_uri (for BT)");
    Serial.println("uri: ");

    size_t len = 0;
    GetData(buf, sizeof(buf), &len);
    if (0 >= len)
    {
        Serial.println("i/p err");
        return;
    }

    if (!ParseData(buf, address, port, resourceUri))
    {
        Serial.println("bad uri");
        return;
    }

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    CAResult_t res = CACreateEndpoint(CA_DEFAULT_FLAGS, selectedNetwork, address, atoi(port),
                                      &endpoint);
    if (CA_STATUS_OK != res)
    {
        Serial.println("Out of memory");
        CADestroyEndpoint(endpoint);
        return;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if (res != CA_STATUS_OK || (!token))
    {
        Serial.println("token error");
        CADestroyEndpoint(endpoint);
        return;
    }

    CAInfo_t requestData = {CA_MSG_NONCONFIRM};
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.payload = (CAPayload_t)"Notification Data";
    requestData.payloadSize = strlen((const char *) requestData.payload);
    requestData.resourceUri = (char *)OICMalloc(strlen(resourceUri) + 1);
    strcpy(requestData.resourceUri, resourceUri);

    CARequestInfo_t requestInfo = {CA_GET, {CA_MSG_RESET}};
    requestInfo.method = CA_GET;
    requestInfo.info = requestData;

    // send request
    CASendRequest(endpoint, &requestInfo);
    // destroy remote endpoint
    if (NULL != endpoint)
    {
        CADestroyEndpoint(endpoint);
    }

    CADestroyToken(token);
    Serial.println("============");
}

void SelectNetwork()
{
    char buf[MAX_BUF_LEN] = {0};

    Serial.println("============");
    Serial.println("Select network");
    Serial.println("IP: 0");
    Serial.println("LE: 1");
    Serial.println("EDR: 2\n");

    size_t len = 0;
    GetData(buf, sizeof(buf), &len);
    int number = buf[0] - '0';
    if (0 >= len || number < 0 || number > 3)
    {
        Serial.println("Wrong i/p. WIFI selected");
        number = 1;
    }

    switch (number)
    {
        case 0:
            {
#ifdef ARDUINOWIFI
                const char ssid[] = "SSID";              // your network SSID (name)
                const char pass[] = "SSID_Password";     // your network password
                int16_t status = WL_IDLE_STATUS;         // the Wifi radio's status

                if (WiFi.status() == WL_NO_SHIELD)
                {
                    Serial.println("ERROR:No Shield");
                    return;
                }

                while (status != WL_CONNECTED)
                {
                    Serial.print("connecting: ");
                    Serial.println(ssid);
                    // WiFi.begin api is weird. ssid should also be taken as const char *
                    // Connect to WPA/WPA2 network:
                    status = WiFi.begin((char *)ssid, pass);
                }
#elif defined ARDUINOETH
                // Note: ****Update the MAC address here with your shield's MAC address****
                uint8_t ETHERNET_MAC[] = {0x90, 0xA2, 0xDA, 0x0E, 0xC4, 0x05};
                uint8_t error = Ethernet.begin(ETHERNET_MAC);
                if (error  == 0)
                {
                    Serial.print("Failed: ");
                    Serial.println(error);
                    return;
                }
#endif
            }
            break;
        case 1:
            g_isLeSelected = true;
            break;
        case 2:
            // Nothing TBD here
            break;
    }

    CASelectNetwork(CATransportAdapter_t(1<<number));
    Serial.println("============");
}

void UnselectNetwork()
{
    char buf[MAX_BUF_LEN] = {0};

    Serial.println("============");
    Serial.println("Unselect network");
    Serial.println("IPv4: 0");
    Serial.println("LE: 1");
    Serial.println("EDR: 2\n");

    size_t len = 0;
    GetData(buf, sizeof(buf), &len);
    int number = buf[0] - '0';
    Serial.println(number);
    if (0 >= len || number < 0 || number > 3)
    {
        Serial.println("Wrong i/p. WIFI selected");
        number = 1;
    }
    if (number == 3)
    {
        g_isLeSelected = false;
    }
    CAUnSelectNetwork((CATransportAdapter_t)(1 << number));
    Serial.println("Terminate");
    CATerminate();
    Serial.println("============");
}

void GetNetworkInfo()
{
    CAEndpoint_t *tempInfo = NULL;
    uint32_t tempSize = 0;
    CAResult_t res = CAGetNetworkInformation(&tempInfo, &tempSize);
    if (CA_STATUS_OK != res || NULL == tempInfo || 0 >= tempSize)
    {
        Serial.println("Network not connected");
        free(tempInfo);
        return;
    }
    Serial.println("=========");
    Serial.print("Network info total size is ");
    Serial.println(tempSize);
    int index;
    for (index = 0; index < tempSize; index++)
    {
        Serial.print("Type: ");
        Serial.println(tempInfo[index].adapter);
        if (CA_ADAPTER_IP == tempInfo[index].adapter)
        {
            Serial.print("Address: ");
            Serial.println(tempInfo[index].addr);
            Serial.print("Port: ");
            Serial.println(tempInfo[index].port);
        }
    }
    free(tempInfo);
    Serial.println("=======");
}

void PrintMenu()
{

    Serial.println("============");
    Serial.println("i: Initialize");
    Serial.println("s: start listening server");
    Serial.println("d: start discovery server");
    Serial.println("r: send request");
    Serial.println("e: send request to all");
    Serial.println("b: send notification");
    Serial.println("g: get network info");
    Serial.println("n: select network");
    Serial.println("x: unselect network");
    Serial.println("h: handle request response");
    Serial.println("t: terminate");
    Serial.println("q: quit");
    Serial.println("============");
}

void HandleRequestResponse()
{
    CAHandleRequestResponse();
}

void RequestHandler(const CAEndpoint_t *object, const CARequestInfo_t *requestInfo)
{
    if (!object)
    {
        Serial.println("endpoint is NULL!");
        return;
    }

    if (!requestInfo)
    {
        Serial.println("Request info is NULL!");
        return;
    }

    Serial.print("RAddr: ");
    Serial.println(object->addr);
    Serial.print("Port: ");
    Serial.println(object->port);
    Serial.print("uri: ");
    Serial.println(requestInfo->info.resourceUri);
    Serial.print("data: ");
    Serial.println((char*)requestInfo->info.payload);
    Serial.print("data size: ");
    Serial.println(requestInfo->info.payloadSize);
    Serial.print("Type: ");
    Serial.println(requestInfo->info.type);

    if (requestInfo->info.options)
    {
        uint32_t len = requestInfo->info.numOptions;
        uint32_t i;
        for (i = 0; i < len; i++)
        {
            Serial.print("Option: ");
            Serial.println(i+1);
            Serial.print("ID: ");
            Serial.println(requestInfo->info.options[i].optionID);
            Serial.print("Data: ");
            Serial.println((char*)requestInfo->info.options[i].optionData);
        }
    }
    Serial.println("send response");
    SendResponse((CAEndpoint_t *)object, (requestInfo != NULL) ? &requestInfo->info : NULL);
}

void ResponseHandler(const CAEndpoint_t *object, const CAResponseInfo_t *responseInfo)
{
    if (object)
    {
        Serial.print("uri: ");
        Serial.println(object->addr);
    }

    if (responseInfo)
    {
        Serial.print("uri: ");
        Serial.println(responseInfo->info.resourceUri);
        Serial.print("data: ");
        Serial.println((char*)responseInfo->info.payload);
        Serial.print("data size: ");
        Serial.println(responseInfo->info.payloadSize);
        Serial.print("Type: ");
        Serial.println(responseInfo->info.type);
        Serial.print("res result=");
        Serial.println(responseInfo->result);
    }
}

void ErrorHandler(const CAEndpoint_t *rep, const CAErrorInfo_t* errorInfo)
{
    Serial.println("ErrorInfo");

    if(errorInfo)
    {
        const CAInfo_t *info = &errorInfo->info;
        Serial.print("result: ");
        Serial.println(errorInfo->result);
        Serial.print("token: ");
        Serial.println(info->token);
        Serial.print("messageId: ");
        Serial.println(info->messageId);
        Serial.print("type: ");
        Serial.println(info->type);
        Serial.print("resourceUri: ");
        Serial.println(info->resourceUri);
        Serial.print("payload: ");
        Serial.println((char*)info->payload);
    }

    return;
}

void SendResponse(CAEndpoint_t *endpoint, const CAInfo_t* info)
{
    char buf[MAX_BUF_LEN] = {0};

    Serial.println("============");
    Serial.println("Select Message Type");
    Serial.println("CON: 0");
    Serial.println("NON: 1");
    Serial.println("ACK: 2");
    Serial.println("RESET: 3");

    size_t len = 0;
    int messageType = 0;
    while(1)
    {
        GetData(buf, sizeof(buf), &len);
        if(len >= 1)
        {
            messageType = buf[0] - '0';
            if (messageType >= 0 && messageType <= 3)
            {
                break;
            }
        }
        Serial.println("Invalid type");
    }

    int respCode = 0;
    if(messageType != 3)
    {
        Serial.println("============");
        Serial.println("Enter Resp Code:");
        Serial.println("For Ex: Empty  : 0");
        Serial.println("Created: 201");
        Serial.println("Deleted: 202");
        Serial.println("Valid  : 203");
        Serial.println("Changed: 204");
        Serial.println("Content: 205");
        Serial.println("BadReq : 400");
        Serial.println("BadOpt : 402");
        Serial.println("NotFnd : 404");
        Serial.println("Internal Srv Err:500");
        Serial.println("Timeout: 504");
        while(1)
        {
            GetData(buf, sizeof(buf), &len);
            if(len >= 1)
            {
                respCode = atoi(buf);
                if (respCode >= 0 && respCode <= 504)
                {
                    break;
                }
            }
            Serial.println("Invalid response");
        }
    }

    CAInfo_t responseData = {CA_MSG_RESET};
    responseData.type = static_cast<CAMessageType_t>(messageType);
    responseData.messageId = (info != NULL) ? info->messageId : 0;
    responseData.resourceUri = (info != NULL) ? info->resourceUri : 0;
    if(messageType != 3)
    {
        responseData.token = (info != NULL) ? info->token : NULL;
        responseData.tokenLength = (info != NULL) ? info->tokenLength : 0;
        responseData.payload = reinterpret_cast<CAPayload_t>(const_cast<char*>("response payload"));
        responseData.payloadSize = strlen((const char *) responseData.payload);
    }
    CAResponseInfo_t responseInfo = {CA_BAD_REQ, {CA_MSG_RESET}};
    responseInfo.result = static_cast<CAResponseResult_t>(respCode);
    responseInfo.info = responseData;
    // send request (transportType from remoteEndpoint of request Info)
    CAResult_t res = CASendResponse(endpoint, &responseInfo);
    if(res != CA_STATUS_OK)
    {
        Serial.println("Snd Resp error");
    }
    else
    {
        Serial.println("Snd Resp success");
    }

    Serial.println("============");
}

void Terminate()
{
    UnselectNetwork();
}

