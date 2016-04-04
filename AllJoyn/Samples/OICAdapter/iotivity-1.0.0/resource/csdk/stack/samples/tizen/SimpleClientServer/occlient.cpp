//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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
#include "occlient.h"
#include "ocpayload.h"
#include "payload_logging.h"

using namespace std;

#ifdef ROUTING_GATEWAY
/**
 * Maximum number of gateway requests to form the routing table.
 */
#define MAX_NUM_GATEWAY_REQUEST 20

/**
 * Sleep duration after every OCProcess().
 */
#define SLEEP_DURATION 100000
#endif

// Tracking user input
static int g_unicastDiscovery = 0;
static int g_testCase = 0;
static int g_connectivity = 0;

static const char *DEVICE_DISCOVERY_QUERY = "%s/oic/d";
static const char *PLATFORM_DISCOVERY_QUERY = "%s/oic/p";
static const char *RESOURCE_DISCOVERY_QUERY = "%s/oic/res";

//The following variable determines the interface protocol (IPv4, IPv6, etc)
//to be used for sending unicast messages. Default set to IP dual stack.
static OCConnectivityType g_connType = CT_ADAPTER_IP;
static OCDevAddr g_serverAddr;
static char g_discoveryAddr[100];
static std::string coapServerResource = "/a/light";

void StripNewLineChar(char* str);

// The handle for the observe registration
OCDoHandle gObserveDoHandle;
#ifdef WITH_PRESENCE
// The handle for observe registration
OCDoHandle gPresenceHandle;
#endif
// After this crosses a threshold client deregisters for further notifications
int gNumObserveNotifies = 0;

#ifdef WITH_PRESENCE
int gNumPresenceNotifies = 0;
#endif

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
        exit(1);
    }

    OCRepPayloadSetPropInt(payload, "power", 15);
    OCRepPayloadSetPropBool(payload, "state", true);

    return (OCPayload*) payload;
}

static void PrintUsage()
{
    cout << "Hello";
    cout << "\nUsage : occlient -u <0|1> -t <1..17> -c <0|1|2>";
    cout << "\n-u <0|1> : Perform multicast/unicast discovery of resources";
    cout << "\n-c 0 : Default IP selection";
    cout << "\n-c 1 : IP Connectivity Type";
    cout << "\n-c 2 : EDR Connectivity Type (IPv6 not currently supported)";
    cout << "\n-t 1  :  Discover Resources";
    cout << "\n-t 2  :  Discover Resources and Initiate Nonconfirmable Get Request";
    cout << "\n-t 3  :  Discover Resources and Initiate Nonconfirmable Get Request with query filter";
    cout << "\n-t 4  :  Discover Resources and Initiate Nonconfirmable Put Requests";
    cout << "\n-t 5  :  Discover Resources and Initiate Nonconfirmable Post Requests";
    cout << "\n-t 6  :  Discover Resources and Initiate Nonconfirmable Delete Requests";
    cout << "\n-t 7  :  Discover Resources and Initiate Nonconfirmable Observe Requests";
    cout << "\n-t 8  :  Discover Resources and Initiate Nonconfirmable Get Request for a resource";
    cout << "which is unavailable";
    cout << "\n-t 9  :  Discover Resources and Initiate Confirmable Get Request";
    cout << "\n-t 10 :  Discover Resources and Initiate Confirmable Post Request";
    cout << "\n-t 11 :  Discover Resources and Initiate Confirmable Delete Requests";
    cout << "\n-t 12 :  Discover Resources and Initiate Confirmable Observe Requests and";
    cout << "cancel with Low QoS";

#ifdef WITH_PRESENCE
    cout << "\n-t 13 :  Discover Resources and Initiate Nonconfirmable presence";
    cout << "\n-t 14 :  Discover Resources and Initiate Nonconfirmable presence with filter";
    cout << "\n-t 15 :  Discover Resources and Initiate Nonconfirmable presence with 2 filters";
    cout << "\n-t 16 :  Discover Resources and Initiate Nonconfirmable multicast presence.";
#endif

    cout << "\n-t 17 :  Discover Resources and Initiate Nonconfirmable Observe Requests";
    cout << "then cancel immediately with High QOS";
    cout << "\n-t 18 :  Discover Resources and Initiate Nonconfirmable Get Request and add";
    cout << "vendor specific header options";
    cout << "\n-t 19 :  Discover Platform";
    cout << "\n-t 20 :  Discover Devices";
}

OCStackResult InvokeOCDoResource(std::ostringstream &query,
                                 OCDevAddr *remoteAddr,
                                 OCMethod method,
                                 OCQualityOfService qos,
                                 OCClientResponseHandler cb,
                                 OCHeaderOption * options,
                                 uint8_t numOptions)
{
    OCStackResult ret;
    OCCallbackData cbData;
    OCDoHandle handle;

    cbData.cb = cb;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(&handle, method, query.str().c_str(), remoteAddr,
                       (method == OC_REST_PUT) ? putPayload() : NULL,
                       (g_connType), qos, &cbData, options, numOptions);

    if (ret != OC_STACK_OK)
    {
        cout << "\nOCDoResource returns error "<< ret;
        cout << " with method " << method;
    }
    else if (method == OC_REST_OBSERVE || method == OC_REST_OBSERVE_ALL)
    {
        gObserveDoHandle = handle;
    }
#ifdef WITH_PRESENCE
    else if (method == OC_REST_PRESENCE)
    {
        gPresenceHandle = handle;
    }
#endif

    return ret;
}

OCStackApplicationResult putReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for PUT recvd successfully";
    }

    if (clientResponse)
    {
        cout << "\nStackResult: " << getResult(clientResponse->result);
        cout << "\nJSON = " << clientResponse->payload;
    }
    else
    {
        cout << "\nputReqCB received Null clientResponse";
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult postReqCB(void *ctx, OCDoHandle /*handle*/,
                                   OCClientResponse *clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for POST recvd successfully";
    }

    if (clientResponse)
    {
        cout << "\nStackResult: " << getResult(clientResponse->result);
        cout << "\nJSON = " << clientResponse->payload;
    }
    else
    {
        cout << "\npostReqCB received Null clientResponse";
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult deleteReqCB(void *ctx,
                                     OCDoHandle /*handle*/,
                                     OCClientResponse *clientResponse)
{
    if(ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for DELETE recvd successfully";
    }

    if(clientResponse)
    {
        cout << "\nStackResult: " << getResult(clientResponse->result);
        //OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    }
    else
    {
        cout << "\ndeleteReqCB received Null clientResponse";
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    if (clientResponse == NULL)
    {
        cout << "\ngetReqCB received NULL clientResponse";
        return   OC_STACK_DELETE_TRANSACTION;
    }

    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for GET query recvd successfully";
    }

    cout << "\nStackResult: " << getResult(clientResponse->result);
    cout << "\nSEQUENCE NUMBER: " << clientResponse->sequenceNumber;
    //OC_LOG_PAYLOAD(INFO, TAG, clientResponse->payload);

    if (clientResponse->numRcvdVendorSpecificHeaderOptions > 0)
    {
        cout << "\nReceived vendor specific options";
        uint8_t i = 0;
        OCHeaderOption * rcvdOptions = clientResponse->rcvdVendorSpecificHeaderOptions;
        for( i = 0; i < clientResponse->numRcvdVendorSpecificHeaderOptions; i++)
        {
            if (((OCHeaderOption)rcvdOptions[i]).protocolID == OC_COAP_ID)
            {
                cout << "\nReceived option ID " << ((OCHeaderOption)rcvdOptions[i]).optionID;
            }
        }
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult obsReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for OBS query recvd successfully";
    }

    if (clientResponse)
    {
        cout << "\nStackResult: " << getResult(clientResponse->result);
        cout << "\nSEQUENCE NUMBER: " << clientResponse->sequenceNumber;
        cout << "\nCallback Context for OBSERVE notification recvd successfully ";
        //OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        gNumObserveNotifies++;
        if (gNumObserveNotifies == 15) //large number to test observing in DELETE case.
        {
            if (g_testCase == TEST_OBS_REQ_NON || g_testCase == TEST_OBS_REQ_CON)
            {
                if (OCCancel (gObserveDoHandle, OC_LOW_QOS, NULL, 0) != OC_STACK_OK)
                {
                    cout << "\nObserve cancel error";
                }
                return OC_STACK_DELETE_TRANSACTION;
            }
            else if (g_testCase == TEST_OBS_REQ_NON_CANCEL_IMM)
            {
                if (OCCancel (gObserveDoHandle, OC_HIGH_QOS, NULL, 0) != OC_STACK_OK)
                {
                    cout << "\nObserve cancel error";
                }
            }
        }
        if (clientResponse->sequenceNumber == OC_OBSERVE_REGISTER)
        {
            cout << "\nThis also serves as a registration confirmation";
        }
        else if (clientResponse->sequenceNumber == OC_OBSERVE_DEREGISTER)
        {
            cout << "\nThis also serves as a deregistration confirmation";
            return OC_STACK_DELETE_TRANSACTION;
        }
        else if (clientResponse->sequenceNumber == OC_OBSERVE_NO_OPTION)
        {
            cout << "\nThis also tells you that registration/deregistration failed";
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    else
    {
        cout << "\nobsReqCB received Null clientResponse";
    }
    return OC_STACK_KEEP_TRANSACTION;
}
#ifdef WITH_PRESENCE
OCStackApplicationResult presenceCB(void* ctx, OCDoHandle /*handle*/,
                                    OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for Presence recvd successfully";
    }

    if (clientResponse)
    {
        cout << "\nStackResult: " << getResult(clientResponse->result);
        cout << "\nNONCE NUMBER: " << clientResponse->sequenceNumber;
        cout << "\nCallback Context for Presence notification recvd successfully ";
        //OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        gNumPresenceNotifies++;
        if (gNumPresenceNotifies == 20)
        {
            if (OCCancel(gPresenceHandle, OC_LOW_QOS, NULL, 0) != OC_STACK_OK)
            {
                cout << "\nPresence cancel error";
            }
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    else
    {
        cout << "\npresenceCB received Null clientResponse";
    }
    return OC_STACK_KEEP_TRANSACTION;
}
#endif

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle /*handle*/,
                                        OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for DISCOVER query recvd successfully";
    }

    if (clientResponse)
    {
        if (NULL == clientResponse->payload)
        {
            cout << "\nPayload is NULL, No resources found";
            return OC_STACK_KEEP_TRANSACTION;
        }

        cout << "\nStackResult: " << getResult(clientResponse->result);

        std::string connectionType = getConnectivityType (clientResponse->connType);
        cout << "\nDiscovered on " << connectionType.c_str();
        cout << "\nDevice ======> Discovered ";
        cout << clientResponse->devAddr.addr;
        if (CT_ADAPTER_IP == clientResponse->connType)
        {
            cout << ":" << clientResponse->devAddr.port;
        }
        //OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        cout << "\nConnectivity type: " << clientResponse->connType;
        g_connType = clientResponse->connType;
        g_serverAddr = clientResponse->devAddr;
        parseClientResponse(clientResponse);

        switch(g_testCase)
        {
            case TEST_GET_REQ_NON:
                InitGetRequest(OC_LOW_QOS, 0, 0);
                break;
            case TEST_GET_REQ_NON_WITH_FILTERS:
                InitGetRequest(OC_LOW_QOS, 0, 1);
                break;
            case TEST_PUT_REQ_NON:
                InitPutRequest(OC_LOW_QOS);
                break;
            case TEST_POST_REQ_NON:
                InitPostRequest(OC_LOW_QOS);
                break;
            case TEST_DELETE_REQ_NON:
                InitDeleteRequest(OC_LOW_QOS);
                break;
            case TEST_OBS_REQ_NON:
            case TEST_OBS_REQ_NON_CANCEL_IMM:
                InitObserveRequest(OC_LOW_QOS);
                break;
            case TEST_GET_UNAVAILABLE_RES_REQ_NON:
                InitGetRequestToUnavailableResource(OC_LOW_QOS);
                break;
            case TEST_GET_REQ_CON:
                InitGetRequest(OC_HIGH_QOS, 0, 0);
                break;
            case TEST_POST_REQ_CON:
                InitPostRequest(OC_HIGH_QOS);
                break;
            case TEST_DELETE_REQ_CON:
                InitDeleteRequest(OC_HIGH_QOS);
                break;
            case TEST_OBS_REQ_CON:
                InitObserveRequest(OC_HIGH_QOS);
                break;
#ifdef WITH_PRESENCE
            case TEST_OBS_PRESENCE:
            case TEST_OBS_PRESENCE_WITH_FILTER:
            case TEST_OBS_PRESENCE_WITH_FILTERS:
            case TEST_OBS_MULTICAST_PRESENCE:
                InitPresence();
                break;
#endif
            case TEST_GET_REQ_NON_WITH_VENDOR_HEADER_OPTIONS:
                InitGetRequest(OC_LOW_QOS, 1, 0);
                break;
            case TEST_DISCOVER_PLATFORM_REQ:
                InitPlatformDiscovery(OC_LOW_QOS);
                break;
            case TEST_DISCOVER_DEV_REQ:
                InitDeviceDiscovery(OC_LOW_QOS);
                break;
            default:
                PrintUsage();
                break;
        }
    }
    else
    {
        cout << "\ndiscoveryReqCB received Null clientResponse";
    }
    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult PlatformDiscoveryReqCB(void* ctx,
                                                OCDoHandle /*handle*/,
                                                OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for Platform DISCOVER query recvd successfully";
    }

    if (clientResponse)
    {
        //OC_LOG truncates the response as it is too long.
        //OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    }
    else
    {
        cout << "\nPlatformDiscoveryReqCB received Null clientResponse";
    }

    return (g_unicastDiscovery) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult DeviceDiscoveryReqCB(void* ctx, OCDoHandle /*handle*/,
                                              OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        cout << "\nCallback Context for Device DISCOVER query recvd successfully";
    }

    if (clientResponse)
    {
        //OC_LOG truncates the response as it is too long.
        cout << "\nDiscovery response: ";
        cout << clientResponse->payload;
    }
    else
    {
        cout << "\nPlatformDiscoveryReqCB received Null clientResponse";
    }

    return (g_unicastDiscovery) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION;
}

#ifdef WITH_PRESENCE
int InitPresence()
{
    OCStackResult result = OC_STACK_OK;
    cout << "\nExecuting " << __func__;
    std::ostringstream query;
    std::ostringstream querySuffix;
    query << OC_RSRVD_PRESENCE_URI;
    if (g_testCase == TEST_OBS_PRESENCE)
    {
        result = InvokeOCDoResource(query, &g_serverAddr, OC_REST_PRESENCE,
                OC_LOW_QOS, presenceCB, NULL, 0);
    }
    if (g_testCase == TEST_OBS_PRESENCE_WITH_FILTER || g_testCase == TEST_OBS_PRESENCE_WITH_FILTERS)
    {
        querySuffix.str("");
        querySuffix << query.str() << "?rt=core.led";
        result = InvokeOCDoResource(querySuffix, &g_serverAddr, OC_REST_PRESENCE,
                OC_LOW_QOS, presenceCB, NULL, 0);
    }
    if (g_testCase == TEST_OBS_PRESENCE_WITH_FILTERS)
    {
        if (result == OC_STACK_OK)
        {
            querySuffix.str("");
            querySuffix << query.str() << "?rt=core.fan";
            result = InvokeOCDoResource(querySuffix, &g_serverAddr, OC_REST_PRESENCE, OC_LOW_QOS,
                    presenceCB, NULL, 0);
        }
    }
    if (g_testCase == TEST_OBS_MULTICAST_PRESENCE)
    {
        if (result == OC_STACK_OK)
        {
            std::ostringstream multicastPresenceQuery;
            multicastPresenceQuery.str("");
            multicastPresenceQuery << "coap://" << OC_MULTICAST_PREFIX << OC_RSRVD_PRESENCE_URI;
            result = InvokeOCDoResource(multicastPresenceQuery, &g_serverAddr, OC_REST_PRESENCE, OC_LOW_QOS,
                    presenceCB, NULL, 0);
        }
    }
    return result;
}
#endif

int InitGetRequestToUnavailableResource(OCQualityOfService qos)
{
    cout << "\nExecuting " << __func__;
    std::ostringstream query;
    query << "/SomeUnknownResource";
    return (InvokeOCDoResource(query, &g_serverAddr, OC_REST_GET, (qos == OC_HIGH_QOS)? OC_HIGH_QOS:OC_LOW_QOS,
            getReqCB, NULL, 0));
}

int InitObserveRequest(OCQualityOfService qos)
{
    cout << "\nExecuting " << __func__;
    std::ostringstream query;
    query << coapServerResource;
    return (InvokeOCDoResource(query, &g_serverAddr, OC_REST_OBSERVE,
              (qos == OC_HIGH_QOS)? OC_HIGH_QOS:OC_LOW_QOS, obsReqCB, NULL, 0));
}

int InitPutRequest(OCQualityOfService qos)
{
    cout << "\nExecuting " << __func__;
    std::ostringstream query;
    query << coapServerResource;
    return (InvokeOCDoResource(query, &g_serverAddr, OC_REST_PUT, (qos == OC_HIGH_QOS)? OC_HIGH_QOS:OC_LOW_QOS,
            putReqCB, NULL, 0));
}

int InitPostRequest(OCQualityOfService qos)
{
    OCStackResult result;
    cout << "\nExecuting " << __func__;
    std::ostringstream query;
    query << coapServerResource;

    // First POST operation (to create an Light instance)
    result = InvokeOCDoResource(query, &g_serverAddr, OC_REST_POST,
                               ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                               postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        // Error can happen if for example, network connectivity is down
        cout << "\nFirst POST call did not succeed";
    }

    // Second POST operation (to create an Light instance)
    result = InvokeOCDoResource(query, &g_serverAddr, OC_REST_POST,
                               ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                               postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        cout << "\nSecond POST call did not succeed";
    }

    // This POST operation will update the original resourced /a/light
    return (InvokeOCDoResource(query, &g_serverAddr, OC_REST_POST,
                               ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                               postReqCB, NULL, 0));
}

void* RequestDeleteDeathResourceTask(void* myqos)
{
    sleep (30);//long enough to give the server time to finish deleting the resource.
    std::ostringstream query;
    query << coapServerResource;

    cout << "\nExecuting " << __func__;

    // Second DELETE operation to delete the resource that might have been removed already.
    OCQualityOfService qos;
    if (myqos == NULL)
    {
        qos = OC_LOW_QOS;
    }
    else
    {
        qos = OC_HIGH_QOS;
    }

    OCStackResult result = InvokeOCDoResource(query, &g_serverAddr, OC_REST_DELETE,
                               qos,
                               deleteReqCB, NULL, 0);

    if (OC_STACK_OK != result)
    {
        cout << "\nSecond DELETE call did not succeed";
    }

    return NULL;
}

int InitDeleteRequest(OCQualityOfService qos)
{
    OCStackResult result;
    std::ostringstream query;
    query << coapServerResource;

    cout << "\nExecuting " << __func__;

    // First DELETE operation
    result = InvokeOCDoResource(query, &g_serverAddr, OC_REST_DELETE,
                               qos,
                               deleteReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        // Error can happen if for example, network connectivity is down
        cout << "\nFirst DELETE call did not succeed";
    }
    else
    {
        //Create a thread to delete this resource again
        pthread_t threadId;
        pthread_create (&threadId, NULL, RequestDeleteDeathResourceTask, (void*)qos);
    }

    cout << "\nExit  " << __func__;
    return result;
}

int InitGetRequest(OCQualityOfService qos, uint8_t withVendorSpecificHeaderOptions, bool getWithQuery)
{

    OCHeaderOption options[MAX_HEADER_OPTIONS];

    cout << "\nExecuting " << __func__;
    std::ostringstream query;
    query << coapServerResource;

    // ocserver is written to only process "power<X" query.
    if (getWithQuery)
    {
        cout << "\nUsing query power<30";
        query << "?power<50";
    }

    if (withVendorSpecificHeaderOptions)
    {
        uint8_t option0[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        uint8_t option1[] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
        memset(options, 0, sizeof(OCHeaderOption) * MAX_HEADER_OPTIONS);
        options[0].protocolID = OC_COAP_ID;
        options[0].optionID = 2048;
        memcpy(options[0].optionData, option0, sizeof(option0));
        options[0].optionLength = 10;
        options[1].protocolID = OC_COAP_ID;
        options[1].optionID = 3000;
        memcpy(options[1].optionData, option1, sizeof(option1));
        options[1].optionLength = 10;
    }
    if (withVendorSpecificHeaderOptions)
    {
        return (InvokeOCDoResource(query, &g_serverAddr, OC_REST_GET,
                (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS, getReqCB, options, 2));
    }
    else
    {
        return (InvokeOCDoResource(query, &g_serverAddr, OC_REST_GET,
                (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS, getReqCB, NULL, 0));
    }
}

int InitPlatformDiscovery(OCQualityOfService qos)
{
    cout << "\nExecuting " << __func__;

    OCStackResult ret;
    OCCallbackData cbData;
    char szQueryUri[64] = { 0 };

    snprintf(szQueryUri, sizeof (szQueryUri) - 1, PLATFORM_DISCOVERY_QUERY, g_discoveryAddr);

    cbData.cb = PlatformDiscoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, CT_DEFAULT,
                       (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        cout << "\nOCStack device error";
    }

    return ret;
}

int InitDeviceDiscovery(OCQualityOfService qos)
{
    cout << "\nExecuting " << __func__;

    OCStackResult ret;
    OCCallbackData cbData;
    char szQueryUri[100] = { 0 };

    snprintf(szQueryUri, sizeof (szQueryUri) - 1, DEVICE_DISCOVERY_QUERY, g_discoveryAddr);

    cbData.cb = DeviceDiscoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, CT_DEFAULT,
                       (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        cout << "\nOCStack device error";
    }

    return ret;
}

int InitDiscovery(OCQualityOfService qos)
{
    OCStackResult ret;
    OCCallbackData cbData;
    char szQueryUri[100] = { 0 };

    snprintf(szQueryUri, sizeof (szQueryUri) - 1, RESOURCE_DISCOVERY_QUERY, g_discoveryAddr);

    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, CT_DEFAULT,
                       (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        cout << "\nOCStack resource error";
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
                g_unicastDiscovery = atoi(optarg);
                break;
            case 't':
                g_testCase = atoi(optarg);
                break;
            case 'c':
                g_connectivity = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((g_unicastDiscovery != 0 && g_unicastDiscovery != 1) ||
            (g_testCase < TEST_DISCOVER_REQ || g_testCase >= MAX_TESTS) ||
            (g_connectivity < CT_ADAPTER_DEFAULT || g_connectivity >= MAX_CT))
    {
        PrintUsage();
        return -1;
    }

    cout << "\nEntering occlient main loop...\n";

    if (OCInit1(OC_CLIENT, OC_DEFAULT_FLAGS, OC_DEFAULT_FLAGS) != OC_STACK_OK)
    {
        cout << "\nOCStack init error";
        return 0;
    }

#ifdef ROUTING_GATEWAY
    /*
     * Before invoking Discover resource, we process the gateway requests
     * and form the routing table.
     */
    for (int index = 0; index < MAX_NUM_GATEWAY_REQUEST; index++)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }
        usleep(SLEEP_DURATION);
    }
#endif

    if (g_connectivity == CT_ADAPTER_DEFAULT || g_connectivity == CT_IP)
    {
        g_connType = CT_ADAPTER_IP;
    }
    else if(g_connectivity == CT_EDR)
    {
        cout << "\nSelected EDR Adapter\n";
        g_connType = CT_ADAPTER_RFCOMM_BTEDR;
    }
    else
    {
        cout << "\nDefault Connectivity type selected...";
        PrintUsage();
    }

    g_discoveryAddr[0] = '\0';

    if (g_unicastDiscovery)
    {
        cout << "\nEnter address of Server hosting resource as given below:";
        cout << "\nIP Adapter: 192.168.0.15:45454(IP:Port)";
        cout << "\nEDR/BLE Adapter: AB:BC:CD:DE:EF:FG(MAC Address)";
        cout << "\nInput:  ";

        if (fgets(g_discoveryAddr, sizeof (g_discoveryAddr), stdin))
        {
            //Strip newline char from unicastAddr
            StripNewLineChar(g_discoveryAddr);
        }
        else
        {
            cout << "\n!! Bad input for IPV4 address. !!";
            return OC_STACK_INVALID_PARAM;
        }
    }

    if (g_unicastDiscovery == 0 && g_testCase == TEST_DISCOVER_DEV_REQ)
    {
        InitDeviceDiscovery(OC_LOW_QOS);
    }
    else if (g_unicastDiscovery == 0 && g_testCase == TEST_DISCOVER_PLATFORM_REQ)
    {
        InitPlatformDiscovery(OC_LOW_QOS);
    }
    else
    {
        InitDiscovery(OC_LOW_QOS);
    }

    // Break from loop with Ctrl+C
    OC_LOG(INFO, TAG, "Entering occlient main loop...");
    signal(SIGINT, handleSigInt);
    while (!gQuitFlag)
    {

        if (OCProcess() != OC_STACK_OK)
        {
            cout << "\nOCStack process error\n";
            return 0;
        }
#ifndef ROUTING_GATEWAY
        sleep(1);
#endif
    }

    cout << "\nExiting occlient main loop...\n";

    if (OCStop() != OC_STACK_OK)
    {
        cout << "\nOCStack stop error\n";
    }

    return 0;
}

std::string getConnectivityType (OCConnectivityType connType)
{
    switch (connType & CT_MASK_ADAPTER)
    {
        case CT_ADAPTER_IP:
            return "IP";

        case CT_IP_USE_V4:
            return "IPv4";

        case CT_IP_USE_V6:
            return "IPv6";

        case CT_ADAPTER_GATT_BTLE:
            return "GATT";

        case CT_ADAPTER_RFCOMM_BTEDR:
            return "RFCOMM";

        default:
            return "Incorrect connectivity";
    }
}

std::string getQueryStrForGetPut(OCClientResponse * /*clientResponse*/)
{
    return "/a/light";
}

void parseClientResponse(OCClientResponse * clientResponse)
{
    coapServerResource = getQueryStrForGetPut(clientResponse);
}

