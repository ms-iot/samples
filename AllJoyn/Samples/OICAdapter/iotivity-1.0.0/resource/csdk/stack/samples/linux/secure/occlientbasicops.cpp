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
#include "occlientbasicops.h"
#include "ocpayload.h"
#include "payload_logging.h"
#include "oic_string.h"
#include "common.h"

#define TAG "occlientbasicops"
static int UnicastDiscovery = 0;
static int TestCase = 0;
static int ConnType = 0;

static char DISCOVERY_QUERY[] = "%s/oic/res";
OCConnectivityType discoveryReqConnType = CT_ADAPTER_IP;
static OCDevAddr endpoint;

static std::string coapServerResource;
static int coapSecureResource;
static OCConnectivityType ocConnType;

//Secure Virtual Resource database for Iotivity Client application
//It contains Client's Identity and the PSK credentials
//of other devices which the client trusts
static char CRED_FILE[] = "oic_svr_db_client.json";


int gQuitFlag = 0;

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
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

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : occlient -u <0|1> -t <1|2|3> -c <0|1>");
    OC_LOG(INFO, TAG, "-u <0|1> : Perform multicast/unicast discovery of resources");
    OC_LOG(INFO, TAG, "-t 1 : Discover Resources");
    OC_LOG(INFO, TAG, "-t 2 : Discover Resources and"
            " Initiate Nonconfirmable Get/Put/Post Requests");
    OC_LOG(INFO, TAG, "-t 3 : Discover Resources and Initiate Confirmable Get/Put/Post Requests");
    OC_LOG(INFO, TAG, "-c 0 : Default auto-selection");
    OC_LOG(INFO, TAG, "-c 1 : IP Connectivity Type");
}

OCStackResult InvokeOCDoResource(std::ostringstream &query,
                                 OCMethod method,
                                 const OCDevAddr *dest,
                                 OCQualityOfService qos,
                                 OCClientResponseHandler cb,
                                 OCHeaderOption * options, uint8_t numOptions)
{
    OCStackResult ret;
    OCCallbackData cbData;

    cbData.cb = cb;
    cbData.context = NULL;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, method, query.str().c_str(), dest,
            (method == OC_REST_PUT || method == OC_REST_POST) ? putPayload() : NULL,
            ocConnType, qos, &cbData, options, numOptions);

    if (ret != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCDoResource returns error %d with method %d", ret, method);
    }

    return ret;
}

OCStackApplicationResult putReqCB(void*, OCDoHandle, OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for PUT recvd successfully");

    if(clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Put Response"));
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult postReqCB(void *, OCDoHandle, OCClientResponse *clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for POST recvd successfully");

    if(clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Post Response"));
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult getReqCB(void*, OCDoHandle, OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");

    if(clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Get Response"));
    }
    return OC_STACK_DELETE_TRANSACTION;
}

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void*, OCDoHandle,
        OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for DISCOVER query recvd successfully");

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG,
                "Device =============> Discovered @ %s:%d",
                clientResponse->devAddr.addr,
                clientResponse->devAddr.port);

        if (clientResponse->result == OC_STACK_OK)
        {
            OC_LOG_PAYLOAD(INFO, clientResponse->payload);

            ocConnType = clientResponse->connType;
            endpoint = clientResponse->devAddr;

            if (parseClientResponse(clientResponse) != -1)
            {
                switch(TestCase)
                {
                    case TEST_NON_CON_OP:
                        InitGetRequest(OC_LOW_QOS);
                        InitPutRequest(OC_LOW_QOS);
                        //InitPostRequest(OC_LOW_QOS);
                        break;
                    case TEST_CON_OP:
                        InitGetRequest(OC_HIGH_QOS);
                        InitPutRequest(OC_HIGH_QOS);
                        //InitPostRequest(OC_HIGH_QOS);
                        break;
                }
            }
        }
    }

    return (UnicastDiscovery) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION ;

}

int InitPutRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    endpoint.flags = (OCTransportFlags)(endpoint.flags|OC_SECURE);
    return (InvokeOCDoResource(query, OC_REST_PUT, &endpoint,
            ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS), putReqCB, NULL, 0));
}

int InitPostRequest(OCQualityOfService qos)
{
    OCStackResult result;
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    endpoint.flags = (OCTransportFlags)(endpoint.flags|OC_SECURE);

    // First POST operation (to create an LED instance)
    result = InvokeOCDoResource(query, OC_REST_POST, &endpoint,
            ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
            postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        // Error can happen if for example, network connectivity is down
        OC_LOG(INFO, TAG, "First POST call did not succeed");
    }

    // Second POST operation (to create an LED instance)
    result = InvokeOCDoResource(query, OC_REST_POST, &endpoint,
            ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
            postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        OC_LOG(INFO, TAG, "Second POST call did not succeed");
    }

    // This POST operation will update the original resourced /a/led
    return (InvokeOCDoResource(query, OC_REST_POST, &endpoint,
                ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                postReqCB, NULL, 0));
}

int InitGetRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    endpoint.flags = (OCTransportFlags)(endpoint.flags|OC_SECURE);

    return (InvokeOCDoResource(query, OC_REST_GET, &endpoint,
                ((qos == OC_HIGH_QOS)?  OC_HIGH_QOS:OC_LOW_QOS),
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
    snprintf(queryUri, sizeof (queryUri), DISCOVERY_QUERY, ipaddr);

    cbData.cb = discoveryReqCB;
    cbData.context = NULL;
    cbData.cd = NULL;

    /* Start a discovery query*/
    OC_LOG_V(INFO, TAG, "Initiating %s Resource Discovery : %s\n",
        (UnicastDiscovery) ? "Unicast" : "Multicast",
        queryUri);

    ret = OCDoResource(NULL, OC_REST_DISCOVER, queryUri, 0, 0, CT_DEFAULT,
                       OC_LOW_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

FILE* client_fopen(const char *path, const char *mode)
{
    (void)path;
    return fopen(CRED_FILE, mode);
}

int main(int argc, char* argv[])
{
    int opt;
    struct timespec timeout;

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
                ConnType = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((UnicastDiscovery != 0 && UnicastDiscovery != 1) ||
            (TestCase < TEST_DISCOVER_REQ || TestCase >= MAX_TESTS)||
            (ConnType < CT_ADAPTER_DEFAULT || ConnType >= MAX_CT))
    {
        PrintUsage();
        return -1;
    }


    if(ConnType == CT_ADAPTER_DEFAULT || ConnType ==  CT_IP)
    {
        discoveryReqConnType = CT_DEFAULT;
    }
    else
    {
        OC_LOG(INFO, TAG, "Using Default Connectivity type");
        PrintUsage();
    }


    // Initialize Persistent Storage for SVR database
    OCPersistentStorage ps = { client_fopen, fread, fwrite, fclose, unlink };
    OCRegisterPersistentStorageHandler(&ps);

    /* Initialize OCStack*/
    if (OCInit(NULL, 0, OC_CLIENT_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    InitDiscovery();

    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L;

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

        nanosleep(&timeout, NULL);
    }
    OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

int parseClientResponse(OCClientResponse * clientResponse)
{
    OCResourcePayload* res = ((OCDiscoveryPayload*)clientResponse->payload)->resources;

    // Initialize all global variables
    coapServerResource.clear();
    coapSecureResource = 0;

    while (res)
    {
        coapServerResource.assign(res->uri);
        OC_LOG_V(INFO, TAG, "Uri -- %s", coapServerResource.c_str());

        if (res->secure)
        {
            endpoint.port = res->port;
            coapSecureResource = 1;
        }

        OC_LOG_V(INFO, TAG, "Secure -- %s", coapSecureResource == 1 ? "YES" : "NO");

        // If we discovered a secure resource, exit from here
        if (coapSecureResource)
        {
            break;
        }

        res = res->next;
    }

    return 0;
}

