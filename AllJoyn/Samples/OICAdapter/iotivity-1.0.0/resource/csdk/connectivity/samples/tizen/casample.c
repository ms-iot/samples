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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "cacommon.h"
#include "cainterface.h"

/**
 * @def MAX_BUF_LEN
 * @brief maximum buffer length
 */
#define MAX_BUF_LEN 1024

/**
 * @def MAX_OPT_LEN
 * @brief maximum option length
 */
#define MAX_OPT_LEN 16

/**
 * @def PORT_LENGTH
 * @brief maximum port length
 */
#define PORT_LENGTH 5

/**
 * @def SECURE_DEFAULT_PORT
 * @brief default secured port
 */
#define SECURE_DEFAULT_PORT 5684

#define RESOURCE_URI_LENGTH 14

#define COAP_PREFIX          "coap://"
#define COAP_PREFIX_LEN      7
#define COAPS_PREFIX         "coaps://"
#define COAPS_PREFIX_LEN     8

// Iotivity Device Identity.
const unsigned char IDENTITY[] = ("1111111111111111");

// PSK between this device and peer device.
const unsigned char RS_CLIENT_PSK[] = ("AAAAAAAAAAAAAAAA");

/**
 * Max size for big payload.
 */
#define BIG_PAYLOAD_SIZE 15000

static GMainLoop *g_mainloop = NULL;
pthread_t thread;

int g_received;
uint16_t g_local_secure_port = SECURE_DEFAULT_PORT;
CATransportAdapter_t g_selected_nw_type = CA_ADAPTER_IP;
const char *MESSAGE_TYPE[] = {"CON", "NON", "ACK", "RESET"};

typedef struct
{
    char ipAddress[CA_IPADDR_SIZE];
    uint16_t port;
} addressSet_t;

char get_menu();
void process();
CAResult_t get_network_type();
CAResult_t get_input_data(char *buf, int32_t length);

bool select_payload();
void populate_binary_payload(uint8_t *bigBuffer, size_t bigBufferLen);

void start_listening_server();
void start_discovery_server();
void send_request();
void send_request_all();
void send_notification();
void select_network();
void unselect_network();
void handle_request_response();
void get_network_info();
void send_secure_request();

void request_handler(const CAEndpoint_t *object, const CARequestInfo_t *requestInfo);
void response_handler(const CAEndpoint_t *object, const CAResponseInfo_t *responseInfo);
void error_handler(const CAEndpoint_t *object, const CAErrorInfo_t* errorInfo);
void send_response(const CAEndpoint_t *endpoint, const CAInfo_t *info);
void get_resource_uri(char *URI, char *resourceURI, int length);
int get_secure_information(CAPayload_t payLoad);
int get_address_set(const char *uri, addressSet_t* outAddress);
void parse_coap_uri(const char* uri, addressSet_t* address, CATransportFlags_t *flags);

static CAToken_t g_last_request_token = NULL;
static const char SECURE_COAPS_PREFIX[] = "coaps://";
static const char SECURE_INFO_DATA[] =
    "{\"oc\":[{\"href\":\"%s\",\"prop\":{\"rt\":[\"core.led\"],"
    "\"if\":[\"oic.if.baseline\"],\"obs\":1,\"sec\":1,\"port\":%d}}]}";
static const char NORMAL_INFO_DATA[] =
    "{\"oc\":[{\"href\":\"%s\",\"prop\":{\"rt\":[\"core.led\"],"
    "\"if\":[\"oic.if.baseline\"],\"obs\":1}}]}";

#ifdef __WITH_DTLS__
// Internal API. Invoked by CA stack to retrieve credentials from this module.
int32_t CAGetDtlsPskCredentials( CADtlsPskCredType_t type,
              const unsigned char *desc, size_t desc_len,
              unsigned char *result, size_t result_length)
{
    printf("CAGetDtlsPskCredentials IN\n");

    int32_t ret = -1;

    if (NULL == result)
    {
        return ret;
    }

    switch (type)
    {
        case CA_DTLS_PSK_HINT:
        case CA_DTLS_PSK_IDENTITY:

            if (result_length < sizeof(IDENTITY))
            {
                printf("ERROR : Wrong value for result for storing IDENTITY");
                return ret;
            }

            memcpy(result, IDENTITY, sizeof(IDENTITY));
            ret = sizeof(IDENTITY);
            break;

        case CA_DTLS_PSK_KEY:

            if ((desc_len == sizeof(IDENTITY)) &&
                memcmp(desc, IDENTITY, sizeof(IDENTITY)) == 0)
            {
                if (result_length < sizeof(RS_CLIENT_PSK))
                {
                    printf("ERROR : Wrong value for result for storing RS_CLIENT_PSK");
                    return ret;
                }

                memcpy(result, RS_CLIENT_PSK, sizeof(RS_CLIENT_PSK));
                ret = sizeof(RS_CLIENT_PSK);
            }
            break;

        default:

            printf("Wrong value passed for PSK_CRED_TYPE.");
            ret = -1;
    }

    printf("CAGetDtlsPskCredentials OUT\n");
    return ret;
}
#endif

void GMainLoopThread()
{
    g_main_loop_run(g_mainloop);
}

CAResult_t Initialize()
{
    g_mainloop = g_main_loop_new(NULL, FALSE);
    if(!g_mainloop)
    {
        printf("g_main_loop_new failed\n");
        return CA_STATUS_FAILED;
    }

    int result = pthread_create(&thread, NULL, (void *) &GMainLoopThread, NULL);
    if (result < 0)
    {
        printf("pthread_create failed in initialize\n");
        return CA_STATUS_FAILED;
    }

    CAResult_t res = CAInitialize();
    if (res != CA_STATUS_OK)
    {
        printf("CAInitialize fail\n");
    }
    return res;
}

int main()
{
    int ret = system("clear");
    // shell invoke error: 127, others: -1
    if (127 == ret || -1 == ret)
    {
        printf("Terminal Clear Error: %d\n", ret);
    }

    printf("=============================================\n");
    printf("\t\tsample main\n");
    printf("=============================================\n");

    CAResult_t res = Initialize();
    if (CA_STATUS_OK != res)
    {
        printf("Initialization is  failed\n");
        return -1;
    }

    // Set the PSK Credentials callback handler.
#ifdef __WITH_DTLS__
    res = CARegisterDTLSCredentialsHandler(CAGetDtlsPskCredentials);
    if (CA_STATUS_OK != res)
    {
        printf("Set credential handler fail\n");
        return -1;
    }
#endif

    // set handler.
    CARegisterHandler(request_handler, response_handler, error_handler);

    process();

    CADestroyToken(g_last_request_token);

    CATerminate();
    return 0;
}

void process()
{
    while (1)
    {
        char menu = get_menu();

        switch (menu)
        {
            case 'm': // menu
            case 'M':
                break;

            case 'q': // quit
            case 'Q':
                printf("quit..!!\n");
                return;

            case 's': // start server
            case 'S':
                start_listening_server();
                break;

            case 't': // send request
            case 'T':
                send_request_all();
                break;

            case 'c': // start client
            case 'C':
                start_discovery_server();
                break;

            case 'r': // send request
            case 'R':
                send_request();
                break;

            case 'b': // send notification
            case 'B':
                send_notification();
                break;

            case 'n': // select network
            case 'N':
                select_network();
                break;

            case 'x': // unselect network
            case 'X':
                unselect_network();
                break;

            case 'h': // handle request response
            case 'H':
                handle_request_response();
                break;

            case 'w':
            case 'W':
                g_received = 0;
                start_discovery_server();
                send_secure_request();
                while (g_received == 0)
                {
                    sleep(1);
                    handle_request_response();
                }
                break;

            case 'z':
            case 'Z':
                start_listening_server();
                while (1)
                {
                    sleep(1);
                    handle_request_response();
                }
                break;

            case 'g': // get network information
            case 'G':
                get_network_info();
                break;

            default:
                printf("Not supported menu!!\n");
                break;
        }
    }

}

void start_listening_server()
{
    printf("Start listening server!!\n");

    CAResult_t res = CAStartListeningServer();
    if (CA_STATUS_OK != res)
    {
        printf("Start listening server fail, error code : %d\n", res);
    }
    else
    {
        printf("Start listening server success\n");
    }
}

void start_discovery_server()
{
    printf("Start discovery client!!\n");

    CAResult_t res = CAStartDiscoveryServer();
    if (CA_STATUS_OK != res)
    {
        printf("Start discovery client fail, error code : %d\n", res);
    }
    else
    {
        printf("Start discovery client success\n");
    }
}

bool select_payload()
{
    char buf[MAX_BUF_LEN]={0};
    printf("\n=============================================\n");
    printf("0:Normal Payload\n1:Big Payload(~15KB)\n");
    printf("select Payload type : ");

    CAResult_t res = get_input_data(buf, sizeof(buf));
    if (CA_STATUS_OK != res)
    {
        printf("Payload type selection error\n");
        printf("Default: Using normal Payload\n");
        return false;
    }

    return (buf[0] == '1') ? true : false;
}

void populate_binary_payload(uint8_t *bigBuffer, size_t bigBufferLen)
{
/**
 * bigBuffer to be filled with binary data. For our sample application to verify, we may fill with
 * any arbitrary value. Hence filling with '1' here.
 */
    memset(bigBuffer, '1', bigBufferLen-1);
    //Last byte making NULL
    bigBuffer[bigBufferLen-1] = '\0';
}

void send_request()
{
    CAResult_t res = get_network_type();
    if (CA_STATUS_OK != res)
    {
        return;
    }

    printf("Do you want to send secure request ?.... enter (0/1): ");

    char secureRequest[MAX_BUF_LEN] = {0};
    if (CA_STATUS_OK != get_input_data(secureRequest, MAX_BUF_LEN))
    {
        return;
    }

    if (strcmp(secureRequest, "1") == 0)
    {
        printf("Enter the URI like below....\n");
        printf("coaps://10.11.12.13:4545/resource_uri ( for IP secure)\n");
    }
    else if (strcmp(secureRequest, "0") == 0)
    {
        printf("Enter the URI like below....\n");
        printf("coap://10.11.12.13:4545/resource_uri ( for IP )\n");
        printf("coap://10:11:12:13:45:45/resource_uri ( for BT )\n");
    }
    else
    {
        printf("Input data is wrong value\n");
        return;
    }

    char uri[MAX_BUF_LEN] = {'\0'};
    if (CA_STATUS_OK != get_input_data(uri, MAX_BUF_LEN))
    {
        return;
    }

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    CATransportFlags_t flags;

    printf("URI : %s\n", uri);
    addressSet_t address = {};
    parse_coap_uri(uri, &address, &flags);

    res = CACreateEndpoint(flags, g_selected_nw_type,
                           (const char*)address.ipAddress, address.port, &endpoint);
    if (CA_STATUS_OK != res || !endpoint)
    {
        printf("Failed to create remote endpoint, error code : %d\n", res);
        return;
    }

    printf("\n=============================================\n");
    printf("0:CON, 1:NON\n");
    printf("select message type : ");

    char buf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(buf, MAX_BUF_LEN))
    {
        CADestroyEndpoint(endpoint);
        return;
    }

    CAMessageType_t msgType = (buf[0] == '1') ? 1 : 0;

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        printf("Token generate error, error code : %d\n", res);
        CADestroyEndpoint(endpoint);
        return;
    }

    printf("Generated token %s\n", token);

    // extract relative resourceuri from give uri
    char resourceURI[RESOURCE_URI_LENGTH + 1] = {0};
    get_resource_uri(uri, resourceURI, RESOURCE_URI_LENGTH);
    printf("resourceURI : %s\n", resourceURI);

    // create request data
    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.resourceUri = (CAURI_t)resourceURI;

    if (strcmp(secureRequest, "1") == 0)
    {
        size_t length = sizeof(SECURE_INFO_DATA) + strlen(resourceURI);
        requestData.payload = (CAPayload_t) calloc(length,  sizeof(char));
        if (NULL == requestData.payload)
        {
            printf("Memory allocation fail\n");
            CADestroyEndpoint(endpoint);
            CADestroyToken(token);
            return;
        }
        snprintf((char *) requestData.payload, length, SECURE_INFO_DATA,
                 (const char *) resourceURI, g_local_secure_port);
    }
    else
    {
        bool useBigPayload = select_payload();
        if (useBigPayload)
        {
            requestData.payload = (CAPayload_t) calloc(BIG_PAYLOAD_SIZE, sizeof(char));
            if (NULL == requestData.payload)
            {
                printf("Memory allocation fail\n");
                CADestroyEndpoint(endpoint);
                CADestroyToken(token);
                return;
            }
            populate_binary_payload(requestData.payload, BIG_PAYLOAD_SIZE);
        }
        else
        {
            size_t length = sizeof(NORMAL_INFO_DATA) + strlen(resourceURI);
            requestData.payload = (CAPayload_t) calloc(length, sizeof(char));
            if (NULL == requestData.payload)
            {
                printf("Memory allocation fail\n");
                CADestroyEndpoint(endpoint);
                CADestroyToken(token);
                return;
            }
            snprintf((char *) requestData.payload, length, NORMAL_INFO_DATA,
                     (const char *) resourceURI);
        }
    }
    requestData.payloadSize = strlen((char *)requestData.payload)+1;
    requestData.type = msgType;

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.info = requestData;
    requestInfo.isMulticast = false;

    // send request
    res = CASendRequest(endpoint, &requestInfo);
    if (CA_STATUS_OK != res)
    {
        printf("Could not send request : %d\n", res);
    }

    //destroy token
    CADestroyToken(token);
    // destroy remote endpoint
    CADestroyEndpoint(endpoint);
    free(requestData.payload);


    printf("=============================================\n");
}

void send_secure_request()
{
    char ipv4addr[CA_IPADDR_SIZE];

    printf("\n=============================================\n");
    printf("Enter IPv4 address of the source hosting secure resource (Ex: 11.12.13.14)\n");

    if (CA_STATUS_OK != get_input_data(ipv4addr, CA_IPADDR_SIZE))
    {
        return;
    }
    printf("%s%s:5684/a/light", SECURE_COAPS_PREFIX, ipv4addr);

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    CAResult_t res = CACreateEndpoint(0, CA_ADAPTER_IP, ipv4addr, SECURE_DEFAULT_PORT, &endpoint);
    if (CA_STATUS_OK != res)
    {
        printf("Failed to create remote endpoint, error code: %d\n", res);
        goto exit;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        printf("Token generate error, error code : %d\n", res);
        goto exit;
    }

    printf("Generated token %s\n", token);

    // create request data
    CAMessageType_t msgType = CA_MSG_NONCONFIRM;
    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.type = msgType;
    requestData.payload = "Temp Json Payload";
    requestData.payloadSize = strlen(requestData.payload)+1;

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.info = requestData;
    requestInfo.isMulticast = false;

    // send request
    CASendRequest(endpoint, &requestInfo);

exit:
    // cleanup
    CADestroyToken(token);
    CADestroyEndpoint(endpoint);
    printf("=============================================\n");
}


void send_request_all()
{
    CAResult_t res = get_network_type();
    if (CA_STATUS_OK != res)
    {
        return;
    }

    printf("\n=============================================\n");
    printf("ex) /a/light\n");
    printf("resource uri : ");

    char resourceURI[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(resourceURI, MAX_BUF_LEN))
    {
        return;
    }

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    res = CACreateEndpoint(CA_IPV4, g_selected_nw_type, NULL, 0, &endpoint);
    if (CA_STATUS_OK != res)
    {
        printf("Create remote endpoint error, error code: %d\n", res);
        return;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        printf("Token generate error!!\n");
        CADestroyEndpoint(endpoint);
        return;
    }

    printf("generated token %s\n", token);

    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.payload = (CAPayload_t) "TempJsonPayload";
    requestData.payloadSize = strlen((const char *) requestData.payload);
    requestData.type = CA_MSG_NONCONFIRM;
    requestData.resourceUri = (CAURI_t)resourceURI;

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.info = requestData;
    requestInfo.isMulticast = true;

    // send request
    res = CASendRequest(endpoint, &requestInfo);
    if (CA_STATUS_OK != res)
    {
        printf("Could not send request to all\n");
    }
    else
    {
        CADestroyToken(g_last_request_token);
        g_last_request_token = token;
    }

    // destroy remote endpoint
    CADestroyEndpoint(endpoint);

    printf("=============================================\n");
}

void send_notification()
{
    CAResult_t res = get_network_type();
    if (CA_STATUS_OK != res)
    {
        return;
    }

    printf("\n=============================================\n");
    printf("Enter the URI like below....\n");
    printf("coap://10.11.12.13:4545/resource_uri ( for IP )\n");
    printf("coap://10:11:12:13:45:45/resource_uri ( for BT )\n");
    printf("uri : ");

    char uri[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(uri, MAX_BUF_LEN))
    {
        return;
    }

    printf("\n=============================================\n");
    printf("\tselect message type\n");
    printf("CON     : 0\n");
    printf("NON     : 1\n");
    printf("ACK     : 2\n");
    printf("RESET   : 3\n");

    printf("select : ");

    char messageTypeBuf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(messageTypeBuf, MAX_BUF_LEN))
    {
        return;
    }

    int messageType = messageTypeBuf[0] - '0';

    switch(messageType)
    {
        case 0:
                printf("CONFIRM messagetype is selected\n");
                break;
        case 1:
                printf("NONCONFIRM messagetype is selected\n");
                break;
        default:
                printf("Invalid Selection\n");
                return;
    }

    CATransportFlags_t flags;
    addressSet_t address = {};
    parse_coap_uri(uri, &address, &flags);

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    res = CACreateEndpoint(flags, g_selected_nw_type, address.ipAddress, address.port, &endpoint);
    if (CA_STATUS_OK != res)
    {
        printf("Create remote endpoint error, error code: %d\n", res);
        return;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        printf("Token generate error!!\n");
        CADestroyEndpoint(endpoint);
        return;
    }

    printf("Generated token %s\n", token);

    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.payload = (CAPayload_t) "TempNotificationData";
    requestData.payloadSize = strlen((const char *) requestData.payload);
    requestData.type = messageType;
    requestData.resourceUri = (CAURI_t)uri;

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.info = requestData;

    // send notification
    res = CASendRequest(endpoint, &requestInfo);
    if (CA_STATUS_OK != res)
    {
        printf("Send notification error, error code: %d\n", res);
    }
    else
    {
        printf("Send notification success\n");
    }

    // destroy token
    CADestroyToken(token);
    // destroy remote endpoint
    CADestroyEndpoint(endpoint);

    printf("\n=============================================\n");
}

void select_network()
{
    printf("\n=============================================\n");
    printf("\tselect network\n");
    printf("IP     : 0\n");
    printf("GATT   : 1\n");
    printf("RFCOMM : 2\n");
    printf("select : ");

    char buf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(buf, MAX_BUF_LEN))
    {
        return;
    }

    int number = buf[0] - '0';

    if (number < 0 || number > 3)
    {
        printf("Invalid network type\n");
        return;
    }

    CAResult_t res = CASelectNetwork(1 << number);
    if (CA_STATUS_OK != res)
    {
        printf("Select network error\n");
        g_selected_nw_type = 1 << number;
    }
    else
    {
        printf("Select network success\n");
    }
    printf("=============================================\n");
}

void unselect_network()
{
    printf("\n=============================================\n");
    printf("\tunselect enabled network\n");
    printf("IP     : 0\n");
    printf("GATT   : 1\n");
    printf("RFCOMM : 2\n");
    printf("select : ");

    char buf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(buf, MAX_BUF_LEN))
    {
        return;
    }

    int number = buf[0] - '0';

    if (number < 0 || number > 3)
    {
        printf("Invalid network type\n");
        return;
    }

    CAResult_t res = CAUnSelectNetwork(1 << number);
    if (CA_STATUS_OK != res)
    {
        printf("Unselect network error\n");
    }
    else
    {
        printf("Unselect network success\n");
    }

    printf("=============================================\n");
}

char get_menu()
{
    printf("\n=============================================\n");
    printf("\t\tMenu\n");
    printf("\ts : start server\n");
    printf("\tc : start client\n");
    printf("\tr : send request\n");
    printf("\tt : send request to all\n");
    printf("\tb : send notification\n");
    printf("\tn : select network\n");
    printf("\tx : unselect network\n");
    printf("\tg : get network information\n");
    printf("\th : handle request response\n");
    printf("\tz : run static server\n");
    printf("\tw : send secure request\n");
    printf("\tq : quit\n");
    printf("=============================================\n");
    printf("select : ");

    char buf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(buf, MAX_BUF_LEN))
    {
        printf("Failed to get input data\n");
    }

    return buf[0];
}

void handle_request_response()
{
    printf("Handle_request_response\n");

    CAResult_t res = CAHandleRequestResponse();
    if (CA_STATUS_OK != res)
    {
        printf("Handle request error, error code: %d\n", res);
    }
    else
    {
        printf("Handle request success\n");
    }
}

void get_network_info()
{
    CAEndpoint_t *tempInfo = NULL;
    uint32_t tempSize = 0;

    CAResult_t res = CAGetNetworkInformation(&tempInfo, &tempSize);
    if (CA_STATUS_OK != res || NULL == tempInfo || 0 >= tempSize)
    {
        printf("Network not connected\n");
        free(tempInfo);
        return;
    }

    printf("################## Network Information #######################\n");
    printf("Network info total size is %d\n\n", tempSize);

    int index;
    for (index = 0; index < tempSize; index++)
    {
        printf("Type: %d\n", tempInfo[index].adapter);
        if (CA_ADAPTER_IP == tempInfo[index].adapter)
        {
            printf("Address: %s\n", tempInfo[index].addr);
            printf("Port: %d\n", tempInfo[index].port);
        }
        else
        {
            printf("Address: %s\n", tempInfo[index].addr);
        }

        printf("Secured: %s\n\n", (tempInfo[index].flags & CA_SECURE) ? "true" : "false");

        if (tempInfo[index].flags & CA_SECURE)
        {
            g_local_secure_port = tempInfo[index].port;
            printf("Secured: in global %d\n\n", g_local_secure_port);
        }
    }

    free(tempInfo);
    printf("##############################################################");
}

void request_handler(const CAEndpoint_t *object, const CARequestInfo_t *requestInfo)
{
    if (NULL == object || NULL == requestInfo)
    {
        printf("Input parameter is NULL\n");
        return;
    }

    if ((NULL != g_last_request_token) && (NULL != requestInfo->info.token)
        && (strncmp(g_last_request_token, requestInfo->info.token,
                    requestInfo->info.tokenLength) == 0))
    {
        printf("Token is same. received request of it's own. skip.. \n");
        return;
    }

    printf("##########received request from remote device #############\n");
    if (CA_ADAPTER_IP == object->adapter)
    {
        printf("Remote Address: %s Port: %d secured:%d\n", object->addr,
               object->port, object->flags & CA_SECURE);
    }
    else
    {
        printf("Remote Address: %s \n", object->addr);
    }
    printf("Data: %s\n", requestInfo->info.payload);
    printf("Message type: %s\n", MESSAGE_TYPE[requestInfo->info.type]);

    if (requestInfo->info.options)
    {
        uint32_t len = requestInfo->info.numOptions;
        uint32_t i;
        for (i = 0; i < len; i++)
        {
            printf("Option %d\n", i + 1);
            printf("ID : %d\n", requestInfo->info.options[i].optionID);
            printf("Data[%d]: %s\n", requestInfo->info.options[i].optionLength,
                   requestInfo->info.options[i].optionData);
        }
    }
    printf("############################################################\n");

    //Check if this has secure communication information
    if (requestInfo->info.payload &&
            (CA_ADAPTER_IP == object->adapter))
    {
        int securePort = get_secure_information(requestInfo->info.payload);
        if (0 < securePort) //Set the remote endpoint secure details and send response
        {
            printf("This is secure resource...\n");

            CAEndpoint_t *endpoint = NULL;
            if (CA_STATUS_OK != CACreateEndpoint(0, object->adapter, object->addr,
                                                 object->port, &endpoint))
            {
                printf("Failed to create duplicate of remote endpoint!\n");
                return;
            }
            endpoint->flags = CA_SECURE;
            object = endpoint;
        }
    }

    printf("Send response with URI\n");
    send_response(object, &requestInfo->info);

    g_received = 1;
}

void response_handler(const CAEndpoint_t *object, const CAResponseInfo_t *responseInfo)
{
    printf("##########Received response from remote device #############\n");
    if (CA_ADAPTER_IP == object->adapter)
    {
        printf("Remote Address: %s Port: %d secured:%d\n", object->addr,
               object->port, object->flags & CA_SECURE);
    }
    else
    {
        printf("Remote Address: %s \n", object->addr);
    }

    printf("resource uri : %s\n", responseInfo->info.resourceUri);
    printf("response result : %d\n", responseInfo->result);
    printf("Data: %s\n", responseInfo->info.payload);
    printf("Message type: %s\n", MESSAGE_TYPE[responseInfo->info.type]);
    printf("Token: %s\n", responseInfo->info.token);
    if (responseInfo->info.options)
    {
        uint32_t len = responseInfo->info.numOptions;
        uint32_t i;
        for (i = 0; i < len; i++)
        {
            printf("Option %d\n", i + 1);
            printf("ID : %d\n", responseInfo->info.options[i].optionID);
            printf("Data[%d]: %s\n", responseInfo->info.options[i].optionLength,
                   responseInfo->info.options[i].optionData);
        }
    }
    printf("############################################################\n");
    g_received = 1;

    //Check if this has secure communication information
    if (responseInfo->info.payload)
    {
        int securePort = get_secure_information(responseInfo->info.payload);
        if (0 < securePort) //Set the remote endpoint secure details and send response
        {
            printf("This is secure resource...\n");
        }
    }
}

void error_handler(const CAEndpoint_t *rep, const CAErrorInfo_t* errorInfo)
{
    printf("+++++++++++++++++++++++++++++++++++ErrorInfo+++++++++++++++++++++++++++++++++++\n");

    if(errorInfo)
    {
        const CAInfo_t *info = &errorInfo->info;
        printf("Error Handler, ErrorInfo :\n");
        printf("Error Handler result    : %d\n", errorInfo->result);
        printf("Error Handler token     : %s\n", info->token);
        printf("Error Handler messageId : %d\n", (uint16_t) info->messageId);
        printf("Error Handler type      : %d\n", info->type);
        printf("Error Handler resourceUri : %s\n", info->resourceUri);
        printf("Error Handler payload   : %s\n", info->payload);

        if(CA_ADAPTER_NOT_ENABLED == errorInfo->result)
        {
            printf("CA_ADAPTER_NOT_ENABLED, enable the adapter\n");
        }
        else if(CA_SEND_FAILED == errorInfo->result)
        {
            printf("CA_SEND_FAILED, unable to send the message, check parameters\n");
        }
        else if(CA_MEMORY_ALLOC_FAILED == errorInfo->result)
        {
            printf("CA_MEMORY_ALLOC_FAILED, insufficient memory\n");
        }
        else if(CA_SOCKET_OPERATION_FAILED == errorInfo->result)
        {
            printf("CA_SOCKET_OPERATION_FAILED, socket operation failed\n");
        }
        else if(CA_STATUS_FAILED == errorInfo->result)
        {
            printf("CA_STATUS_FAILED, message could not be delivered, internal error\n");
        }
    }
    printf("++++++++++++++++++++++++++++++++End of ErrorInfo++++++++++++++++++++++++++++++++\n");

    return;
}

void send_response(const CAEndpoint_t *endpoint, const CAInfo_t *info)
{
    printf("entering send_response\n");

    printf("\n=============================================\n");
    printf("\tselect message type\n");
    printf("CON     : 0\n");
    printf("NON     : 1\n");
    printf("ACK     : 2\n");
    printf("RESET   : 3\n");
    printf("select : ");

    char buf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(buf, MAX_BUF_LEN))
    {
        return;
    }

    int messageType = buf[0] - '0';
    int responseCode = 0 ;
    char responseCodeBuf[MAX_BUF_LEN] = { 0 };
    if (CA_MSG_RESET != messageType)
    {
        printf("\n=============================================\n");
        printf("\tselect response code\n");
        printf("EMPTY                    :   0\n");
        printf("CREATED                  : 201\n");
        printf("DELETED                  : 202\n");
        printf("VALID                    : 203\n");
        printf("CHANGED                  : 204\n");
        printf("CONTENT                  : 205\n");
        printf("BAD_REQ                  : 400\n");
        printf("BAD_OPT                  : 402\n");
        printf("NOT_FOUND                : 404\n");
        printf("INTERNAL_SERVER_ERROR    : 500\n");
        printf("RETRANSMIT_TIMEOUT       : 504\n");
        printf("select : ");

        if (CA_STATUS_OK != get_input_data(responseCodeBuf, MAX_BUF_LEN))
        {
            return;
        }
        responseCode = atoi(responseCodeBuf);
    }
    CAInfo_t responseData = { 0 };
    responseData.type = messageType;
    responseData.messageId = (info != NULL) ? info->messageId : 0;
    responseData.resourceUri = (info != NULL) ? info->resourceUri : 0;

    if(CA_MSG_RESET != messageType)
    {
        responseData.token = (info != NULL) ? info->token : NULL;
        responseData.tokenLength = (info != NULL) ? info->tokenLength : 0;

        if (endpoint->flags & CA_SECURE)
        {
            printf("Sending response on secure communication\n");

            uint32_t length = sizeof(SECURE_INFO_DATA) + strlen(responseData.resourceUri)
                              + sizeof(g_local_secure_port);
            responseData.payload = (CAPayload_t) calloc(length,  sizeof(char));
            if (NULL == responseData.payload)
            {
                printf("Memory allocation fail\n");
                return;
            }
            snprintf((char *) responseData.payload, length, SECURE_INFO_DATA,
                     (const char *) responseData.resourceUri, g_local_secure_port);
        }
        else
        {
            printf("Sending response on non-secure communication\n");

            bool useBigPayload = select_payload();
            if (useBigPayload)
            {
                responseData.payload = (CAPayload_t) calloc(BIG_PAYLOAD_SIZE, sizeof(char));
                if (NULL == responseData.payload)
                {
                    printf("Memory allocation fail\n");
                    return;
                }
                populate_binary_payload(responseData.payload, BIG_PAYLOAD_SIZE);
            }
            else
            {
                size_t length = sizeof(NORMAL_INFO_DATA) + strlen(responseData.resourceUri);
                responseData.payload = (CAPayload_t) calloc(length, sizeof(char));
                if (NULL == responseData.payload)
                {
                    printf("Memory allocation fail\n");
                    return;
                }
                snprintf((char *) responseData.payload, length, NORMAL_INFO_DATA,
                         (const char *) responseData.resourceUri);
            }
        }
    }
    responseData.payloadSize = strlen((char *)responseData.payload)+1;
    CAResponseInfo_t responseInfo = { 0 };
    responseInfo.result = responseCode;
    responseInfo.info = responseData;

    // send response (transportType from remoteEndpoint of request Info)
    CAResult_t res = CASendResponse(endpoint, &responseInfo);
    if (CA_STATUS_OK != res)
    {
        printf("Send response error\n");
    }
    else
    {
        printf("Send response success\n");
    }

    printf("=============================================\n");
}

int get_secure_information(CAPayload_t payLoad)
{
    printf("Entering get_secure_information\n");

    if (!payLoad)
    {
        printf("Payload is NULL\n");
        return -1;
    }

    char *subString = NULL;
    if (NULL == (subString = strstr((const char *) payLoad, "\"sec\":1")))
    {
        printf("This is not secure resource\n");
        return -1;
    }

    if (NULL == (subString = strstr((const char *) payLoad, "\"port\":")))
    {
        printf("This secure resource does not have port information\n");
        return -1;
    }

    char *startPos = strstr(subString, ":");
    if (!startPos)
    {
        printf("Parsing failed !\n");
        return -1;
    }

    char *endPos = strstr(startPos, "}");
    if (!endPos)
    {
        printf("Parsing failed !\n");
        return -1;
    }

    char portStr[6] = {0};
    memcpy(portStr, startPos + 1, (endPos - 1) - startPos);

    printf("secured port is: %s\n", portStr);
    return atoi(portStr);
}

void get_resource_uri(char *URI, char *resourceURI, int length)
{
    char *startPos = URI;
    char *temp = NULL;
    if (NULL != (temp = strstr(URI, "://")))
    {
        startPos = strchr(temp + 3, '/');
        if (!startPos)
        {
            printf("Resource URI is missing\n");
            return;
        }
    }

    char *endPos = strchr(startPos, '?');
    if (!endPos)
    {
        endPos = URI + strlen(URI);
    }
    endPos -= 1;

    if (endPos - startPos <= length)
    {
        memcpy(resourceURI, startPos + 1, endPos - startPos);
    }

    printf("URI: %s, ResourceURI:%s\n", URI, resourceURI);
}

CAResult_t get_network_type()
{
    printf("\n=============================================\n");
    printf("\tselect network type\n");
    printf("IP     : 0\n");
    printf("GATT   : 1\n");
    printf("RFCOMM : 2\n");
    printf("select : ");

    char buf[MAX_BUF_LEN] = { 0 };
    if (CA_STATUS_OK != get_input_data(buf, MAX_BUF_LEN))
    {
        return CA_NOT_SUPPORTED ;
    }

    int number = buf[0] - '0';
    if (0 > number || 2 < number)
    {
        printf("\nInvalid Network type");
        return CA_NOT_SUPPORTED;
    }

    g_selected_nw_type = 1 << number;

    return CA_STATUS_OK;
}

CAResult_t get_input_data(char *buf, int32_t length)
{
    if (!fgets(buf, length, stdin))
    {
        printf("fgets error\n");
        return CA_STATUS_FAILED;
    }

    char *p = NULL;
    if ((p = strchr(buf, '\n')) != NULL)
    {
        *p = '\0';
    }

    return CA_STATUS_OK;
}


void parse_coap_uri(const char* uri, addressSet_t* address, CATransportFlags_t *flags)
{
    if (NULL == uri)
    {
        printf("parameter is null\n");
        return;
    }

    // parse uri
    // #1. check prefix
    uint8_t startIndex = 0;
    if (strncmp(COAPS_PREFIX, uri, COAPS_PREFIX_LEN) == 0)
    {
        printf("uri has '%s' prefix\n", COAPS_PREFIX);
        startIndex = COAPS_PREFIX_LEN;
        *flags = CA_SECURE;
    }
    else if (strncmp(COAP_PREFIX, uri, COAP_PREFIX_LEN) == 0)
    {
        printf("uri has '%s' prefix\n", COAP_PREFIX);
        startIndex = COAP_PREFIX_LEN;
        *flags = CA_IPV4;
    }

    // #2. copy uri for parse
    int32_t len = strlen(uri) - startIndex;

    if (len <= 0)
    {
        printf("uri length is 0!\n");
        return;
    }

    int res = get_address_set(uri + startIndex, address);
    if (res == -1)
    {
        printf("address parse error\n");
        return;
    }

    return;
}

int get_address_set(const char *uri, addressSet_t* outAddress)
{
    if (NULL == uri || NULL == outAddress)
    {
        printf("parameter is null !\n");
        return -1;
    }

    int32_t len = strlen(uri);
    if (len <= 0)
    {
        printf("uri length is 0!\n");
        return -1;
    }

    int32_t isIp = 0;
    int32_t ipLen = 0;
    for (int i = 0; i < len; i++)
    {
        if (uri[i] == '.')
        {
            isIp = 1;
        }

        // found port number start index
        if (isIp && uri[i] == ':')
        {
            ipLen = i;
            outAddress->port = atoi(uri + ipLen + 1);
            break;
        }

        if (uri[i] == '/')
        {
            break;
        }

        outAddress->ipAddress[i] = uri[i];
    }

    return isIp;
}
