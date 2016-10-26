//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "ocstack.h"
#include "logger.h"
#include "occlientslow.h"
#include "oic_string.h"
#include "ocpayload.h"
#include "payload_logging.h"

// Tracking user input
static int UnicastDiscovery = 0;
static int TestCase = 0;
static int ConnectivityType = 0;

static std::string coapServerResource = "/a/led";

//The following variable determines the interface protocol (IP, etc)
//to be used for sending unicast messages. Default set to IP.
static OCConnectivityType AdapterType = CT_ADAPTER_IP;
static OCDevAddr endpoint;
static const char *RESOURCE_DISCOVERY_QUERY = "%s/oic/res";
void StripNewLineChar(char* str);

int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : occlient -c <0|1> -u <0|1> -t <1|2|3>");
    OC_LOG(INFO, TAG, "-c 0 : Default auto-selection");
    OC_LOG(INFO, TAG, "-c 1 : IP Connectivity Type");
    OC_LOG(INFO, TAG, "-u <0|1> : Perform multicast/unicast discovery of resources");
    OC_LOG(INFO, TAG, "-t 1 : Discover Resources");
    OC_LOG(INFO, TAG, "-t 2 : Discover Resources and Initiate Nonconfirmable Get Request");
    OC_LOG(INFO, TAG, "-t 3 : Discover Resources and Initiate Confirmable Get Request");
    OC_LOG(INFO, TAG, "-t 4 : Discover Resources and Initiate NonConfirmable Put Request");
    OC_LOG(INFO, TAG, "-t 5 : Discover Resources and Initiate Confirmable Put Request");
}

OCPayload* putPayload()
{
    OCRepPayload* payload = OCRepPayloadCreate();

    if(!payload)
    {
        std::cout << "Failed to create put payload object"<<std::endl;
        std::exit(1);
    }

    OCRepPayloadSetPropInt(payload, "power", 15);
    OCRepPayloadSetPropBool(payload, "state", true);

    return (OCPayload*) payload;
}

OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCDevAddr *dest, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions)
{
    OCStackResult ret;
    OCCallbackData cbData;

    cbData.cb = cb;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, method, query.str().c_str(), dest,
            (method == OC_REST_PUT) ? putPayload() : NULL,
            AdapterType, qos, &cbData, options, numOptions);

    if (ret != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCDoResource returns error %d with method %d", ret, method);
    }

    return ret;
}

OCStackApplicationResult getReqCB(void* ctx,
        OCDoHandle /*handle*/, OCClientResponse * clientResponse)
{
    if(clientResponse == NULL)
    {
        OC_LOG(INFO, TAG, "The clientResponse is NULL");
        return   OC_STACK_DELETE_TRANSACTION;
    }
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");
    }

    OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
    OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
    OC_LOG(INFO, TAG, "Get Response =============> ");
    OC_LOG_PAYLOAD(INFO, clientResponse->payload);

    if(clientResponse->rcvdVendorSpecificHeaderOptions &&
            clientResponse->numRcvdVendorSpecificHeaderOptions)
    {
        OC_LOG (INFO, TAG, "Received vendor specific options");
        uint8_t i = 0;
        OCHeaderOption * rcvdOptions = clientResponse->rcvdVendorSpecificHeaderOptions;
        for( i = 0; i < clientResponse->numRcvdVendorSpecificHeaderOptions; i++)
        {
            if(((OCHeaderOption)rcvdOptions[i]).protocolID == OC_COAP_ID)
            {
                OC_LOG_V(INFO, TAG, "Received option with OC_COAP_ID and ID %u with",
                        ((OCHeaderOption)rcvdOptions[i]).optionID );

                OC_LOG_BUFFER(INFO, TAG, ((OCHeaderOption)rcvdOptions[i]).optionData,
                    MAX_HEADER_OPTION_DATA_LENGTH);
            }
        }
    }
    return OC_STACK_DELETE_TRANSACTION;
}

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle /*handle*/,
        OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for DISCOVER query recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));

        OC_LOG_V(INFO, TAG, "Discovered @ %s:%u =============> ",
            clientResponse->devAddr.addr, clientResponse->devAddr.port);
        OC_LOG_PAYLOAD (INFO, clientResponse->payload);

        endpoint = clientResponse->devAddr;

        switch(TestCase)
        {
            case TEST_NON_CON_OP:
                InitGetRequest(OC_LOW_QOS);
                break;
            case TEST_CON_OP:
                InitGetRequest(OC_HIGH_QOS);
                break;
            case TEST_NON_CON_PUT:
                InitPutRequest(OC_LOW_QOS);
                break;
            case TEST_CON_PUT:
                InitPutRequest(OC_HIGH_QOS);
                break;
            default:
                PrintUsage();
                break;
        }
    }

    return UnicastDiscovery ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION ;

}

int InitGetRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    OC_LOG_V (INFO, TAG, "Performing GET with query : %s", query.str().c_str());
    return (InvokeOCDoResource(query, OC_REST_GET, &endpoint,
                               (qos == OC_HIGH_QOS)? OC_HIGH_QOS : OC_LOW_QOS,
                               getReqCB, NULL, 0));
}

int InitPutRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    OC_LOG_V (INFO, TAG, "Performing PUT with query : %s", query.str().c_str());
    return (InvokeOCDoResource(query, OC_REST_PUT, &endpoint,
                               (qos == OC_HIGH_QOS)?  OC_HIGH_QOS:OC_LOW_QOS,
                               getReqCB, NULL, 0));
}

int InitDiscovery()
{
    OCStackResult ret;
    OCCallbackData cbData;
    char queryUri[200];
    char ipaddr[100] = { '\0' };

    if (UnicastDiscovery)
    {
        OC_LOG(INFO, TAG, "Enter IP address (with optional port) of the Server hosting resource\n");
        OC_LOG(INFO, TAG, "IPv4: 192.168.0.15:45454\n");
        OC_LOG(INFO, TAG, "IPv6: [fe80::20c:29ff:fe1b:9c5]:45454\n");

        if (fgets(ipaddr, sizeof (ipaddr), stdin))
        {
            StripNewLineChar(ipaddr); //Strip newline char from ipaddr
        }
        else
        {
            OC_LOG(ERROR, TAG, "!! Bad input for IP address. !!");
            return OC_STACK_INVALID_PARAM;
        }
    }

    snprintf(queryUri, sizeof (queryUri), RESOURCE_DISCOVERY_QUERY, ipaddr);

    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, queryUri, 0, 0, CT_DEFAULT,
                       OC_LOW_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "u:t:c:")) != -1)
    {
        switch(opt)
        {
            case 'u':
                UnicastDiscovery = atoi(optarg);
                break;
            case 't':
                TestCase = atoi(optarg);
                break;
            case 'c':
                ConnectivityType = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((UnicastDiscovery != 0 && UnicastDiscovery != 1) ||
            (TestCase < TEST_DISCOVER_REQ || TestCase >= MAX_TESTS) ||
            (ConnectivityType < CT_ADAPTER_DEFAULT || ConnectivityType >= MAX_CT))
    {
        PrintUsage();
        return -1;
    }

    /* Initialize OCStack*/
    if (OCInit(NULL, 0, OC_CLIENT) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    if(ConnectivityType == CT_ADAPTER_DEFAULT || ConnectivityType == CT_IP)
    {
        AdapterType = CT_ADAPTER_IP;
    }
    else
    {
        OC_LOG(INFO, TAG, "Default Connectivity type selected...");
        AdapterType = CT_ADAPTER_IP;
    }

    InitDiscovery();

    // Break from loop with Ctrl+C
    OC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }

        sleep(2);
    }
    OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

std::string getQueryStrForGetPut(OCClientResponse * /*clientResponse*/)
{
    return "/a/led";
}

