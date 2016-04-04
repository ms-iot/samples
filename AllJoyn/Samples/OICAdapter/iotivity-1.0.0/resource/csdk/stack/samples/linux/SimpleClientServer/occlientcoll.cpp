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
#include <ocstack.h>
#include <iostream>
#include <sstream>
#include "ocpayload.h"
#include "payload_logging.h"
#include "logger.h"
const char *getResult(OCStackResult result);
std::string getQueryStrForGetPut();

#define TAG ("occlient")
#define DEFAULT_CONTEXT_VALUE 0x99
#ifndef MAX_LENGTH_IPv4_ADDR
#define MAX_LENGTH_IPv4_ADDR 16
#endif

typedef enum
{
    TEST_INVALID = 0,
    TEST_GET_DEFAULT,
    TEST_GET_BATCH,
    TEST_GET_LINK_LIST,
    TEST_PUT_DEFAULT,
    TEST_PUT_BATCH,
    TEST_PUT_LINK_LIST,
    TEST_UNKNOWN_RESOURCE_GET_DEFAULT,
    TEST_UNKNOWN_RESOURCE_GET_BATCH,
    TEST_UNKNOWN_RESOURCE_GET_LINK_LIST,
    MAX_TESTS
} CLIENT_TEST;

/**
 * List of connectivity types that can be initiated from the client
 * Required for user input validation
 */
typedef enum {
    CT_ADAPTER_DEFAULT = 0,
    CT_IP,
    MAX_CT
} CLIENT_ConnectivityType_TYPE;

unsigned static int TestType = TEST_INVALID;
unsigned static int ConnectivityType = 0;

typedef struct
{
    char text[30];
    CLIENT_TEST test;
} testToTextMap;

testToTextMap queryInterface[] = {
        {"invalid", TEST_INVALID},
        {"?if=oic.if.baseline", TEST_GET_DEFAULT},
        {"?if=oic.if.b", TEST_GET_BATCH},
        {"?if=oic.if.ll", TEST_GET_LINK_LIST},
        {"?if=oic.if.baseline", TEST_UNKNOWN_RESOURCE_GET_DEFAULT},
        {"?if=oic.if.b", TEST_UNKNOWN_RESOURCE_GET_BATCH},
        {"?if=oic.if.ll", TEST_UNKNOWN_RESOURCE_GET_LINK_LIST},
        {"?if=oic.if.baseline", TEST_PUT_DEFAULT},
        {"?if=oic.if.b", TEST_PUT_BATCH},
        {"?if=oic.if.ll", TEST_PUT_LINK_LIST},
};


//The following variable determines the interface protocol (IP, etc)
//to be used for sending unicast messages. Default set to IP.
static OCConnectivityType ConnType = CT_ADAPTER_IP;
static const char * RESOURCE_DISCOVERY_QUERY = "/oic/res";

// The handle for the observe registration
OCDoHandle gObserveDoHandle;
// After this crosses a threshold client deregisters for further observations
int gNumObserveNotifies = 1;

int gQuitFlag = 0;
/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

// Forward Declaration
OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);
int InitGetRequestToUnavailableResource(OCClientResponse * clientResponse);
int InitObserveRequest(OCClientResponse * clientResponse);
int InitPutRequest(OCClientResponse * clientResponse);
int InitGetRequest(OCClientResponse * clientResponse);
int InitDiscovery();

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

void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : occlientcoll -t <Test Case> -c <CA connectivity Type>");
    OC_LOG(INFO, TAG, "-c 0 : Default auto-selection");
    OC_LOG(INFO, TAG, "-c 1 : IP Connectivity Type");
    OC_LOG(INFO, TAG, "Test Case 1 : Discover Resources && Initiate GET Request on an "\
            "available resource using default interface.");
    OC_LOG(INFO, TAG, "Test Case 2 : Discover Resources && Initiate GET Request on an "\
                 "available resource using batch interface.");
    OC_LOG(INFO, TAG, "Test Case 3 : Discover Resources && Initiate GET Request on an "\
                 "available resource using link list interface.");
    OC_LOG(INFO, TAG, "Test Case 4 : Discover Resources && Initiate GET & PUT Request on an "\
                 "available resource using default interface.");
    OC_LOG(INFO, TAG, "Test Case 5 : Discover Resources && Initiate GET & PUT Request on an "\
                 "available resource using batch interface.");
    OC_LOG(INFO, TAG, "Test Case 6 : Discover Resources && Initiate GET & PUT Request on an "\
                 "available resource using link list interface.");
    OC_LOG(INFO, TAG, "Test Case 7 : Discover Resources && Initiate GET Request on an "\
                 "unavailable resource using default interface.");
    OC_LOG(INFO, TAG, "Test Case 8 : Discover Resources && Initiate GET Request on an "\
                 "unavailable resource using batch interface.");
    OC_LOG(INFO, TAG, "Test Case 9 : Discover Resources && Initiate GET Request on an "\
                 "unavailable resource using link list interface.");
}

OCStackApplicationResult putReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    if(clientResponse == NULL)
    {
        OC_LOG(INFO, TAG, "The clientResponse is NULL");
        return   OC_STACK_DELETE_TRANSACTION;
    }
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG_V(INFO, TAG, "Callback Context for PUT query recvd successfully");
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    }

    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    OC_LOG_V(INFO, TAG, "StackResult: %s",
            getResult(clientResponse->result));
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        if(clientResponse->sequenceNumber == 0)
        {
            OC_LOG_V(INFO, TAG, "Callback Context for GET query recvd successfully");
            OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        }
        else
        {
            OC_LOG_V(INFO, TAG, "Callback Context for Get recvd successfully %d",
                    gNumObserveNotifies);
            OC_LOG_PAYLOAD(INFO, clientResponse->payload);;
            gNumObserveNotifies++;
            if (gNumObserveNotifies == 3)
            {
                if (OCCancel (gObserveDoHandle, OC_LOW_QOS, NULL, 0) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Observe cancel error");
                }
            }
        }
    }
    if(TestType == TEST_PUT_DEFAULT || TestType == TEST_PUT_BATCH || TestType == TEST_PUT_LINK_LIST)
    {
        InitPutRequest(clientResponse);
    }
    return OC_STACK_KEEP_TRANSACTION;
}

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle /*handle*/,
                                        OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG,
            "Entering discoveryReqCB (Application Layer CB)");
    OC_LOG_V(INFO, TAG, "StackResult: %s",
            getResult(clientResponse->result));

    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG_V(INFO, TAG, "Callback Context recvd successfully");
    }

    OC_LOG_V(INFO, TAG,
            "Device =============> Discovered @ %s:%d",
            clientResponse->devAddr.addr,
            clientResponse->devAddr.port);
    OC_LOG_PAYLOAD(INFO, clientResponse->payload);

    ConnType = clientResponse->connType;

    if(TestType == TEST_UNKNOWN_RESOURCE_GET_DEFAULT || TestType == TEST_UNKNOWN_RESOURCE_GET_BATCH ||\
            TestType == TEST_UNKNOWN_RESOURCE_GET_LINK_LIST)
    {
        InitGetRequestToUnavailableResource(clientResponse);
    }
    else
    {
        InitGetRequest(clientResponse);
    }
    return OC_STACK_KEEP_TRANSACTION;
}


int InitGetRequestToUnavailableResource(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    std::ostringstream getQuery;
    getQuery << "/SomeUnknownResource";
    cbData.cb = getReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_GET, getQuery.str().c_str(),
                       &clientResponse->devAddr, 0, ConnType, OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}


int InitObserveRequest(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;
    std::ostringstream obsReg;
    obsReg << getQueryStrForGetPut();
    cbData.cb = getReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    OC_LOG_V(INFO, TAG, "OBSERVE payload from client =");
    OCPayload* payload = putPayload();
    OC_LOG_PAYLOAD(INFO, payload);
    OCPayloadDestroy(payload);

    ret = OCDoResource(&handle, OC_REST_OBSERVE, obsReg.str().c_str(),
                       &clientResponse->devAddr, 0, ConnType,
                       OC_LOW_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    else
    {
        gObserveDoHandle = handle;
    }
    return ret;
}


int InitPutRequest(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;
    //* Make a PUT query*/
    std::ostringstream getQuery;
    getQuery << "coap://" << clientResponse->devAddr.addr << ":" <<
            clientResponse->devAddr.port <<
            "/a/room" << queryInterface[TestType].text;
    cbData.cb = putReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    OC_LOG_V(INFO, TAG, "PUT payload from client = ");
    OCPayload* payload = putPayload();
    OC_LOG_PAYLOAD(INFO, payload);
    OCPayloadDestroy(payload);

    ret = OCDoResource(NULL, OC_REST_PUT, getQuery.str().c_str(),
                       &clientResponse->devAddr, putPayload(), ConnType,
                       OC_LOW_QOS, &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}


int InitGetRequest(OCClientResponse * clientResponse)
{
    OCStackResult ret;
    OCCallbackData cbData;

    //* Make a GET query*/
    std::ostringstream getQuery;
    getQuery << "/a/room" << queryInterface[TestType].text;

    std::cout << "Get Query: " << getQuery.str() << std::endl;

    cbData.cb = getReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    ret = OCDoResource(NULL, OC_REST_GET, getQuery.str().c_str(),
                       &clientResponse->devAddr, 0, ConnType, OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int InitDiscovery()
{
    OCStackResult ret;
    OCCallbackData cbData;
    /* Start a discovery query*/
    char szQueryUri[64] = { 0 };

    strcpy(szQueryUri, RESOURCE_DISCOVERY_QUERY);

    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, ConnType,
                        OC_LOW_QOS,
            &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "t:c:")) != -1)
    {
        switch (opt)
        {
            case 't':
                TestType = atoi(optarg);
                break;
            case 'c':
                ConnectivityType = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }
    if ((TestType <= TEST_INVALID || TestType >= MAX_TESTS) ||
        ConnectivityType >= MAX_CT)
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
        ConnType = CT_ADAPTER_IP;
    }
    else
    {
        OC_LOG(INFO, TAG, "Default Connectivity type selected...");
        ConnType = CT_ADAPTER_IP;
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
    } OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
    }

    return 0;
}

std::string getQueryStrForGetPut()
{
    return "/a/room";
}

