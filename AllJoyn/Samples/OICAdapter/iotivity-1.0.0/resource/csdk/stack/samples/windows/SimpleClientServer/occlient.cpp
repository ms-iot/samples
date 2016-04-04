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
#include <iostream>
#include <sstream>
#include <windows.h>
#include "ocstack.h"
#include "logger.h"
#include "occlient.h"
#include "ocpayload.h"
#include "payload_logging.h"

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
static int UnicastDiscovery = 0;
static int TestCase = 0;
static int Connectivity = 0;

static const char *DEVICE_DISCOVERY_QUERY = "%s/oic/d";
static const char *PLATFORM_DISCOVERY_QUERY = "%s/oic/p";
static const char *RESOURCE_DISCOVERY_QUERY = "%s/oic/res";

//The following variable determines the interface protocol (IPv4, IPv6, etc)
//to be used for sending unicast messages. Default set to IP dual stack.
static OCConnectivityType ConnType = CT_ADAPTER_IP;
static OCDevAddr serverAddr;
static char discoveryAddr[100];
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

    if (!payload)
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
    OC_LOG(INFO, TAG, "Usage : occlient -u <0|1> -t <1..17> -c <0|1>");
    OC_LOG(INFO, TAG, "-u <0|1> : Perform multicast/unicast discovery of resources");
    OC_LOG(INFO, TAG, "-c 0 : Use Default connectivity(IP)");
    OC_LOG(INFO, TAG, "-c 1 : IP Connectivity Type");
    OC_LOG(INFO, TAG, "-t 1  :  Discover Resources");
    OC_LOG(INFO, TAG, "-t 2  :  Discover Resources and Initiate Nonconfirmable Get Request");
    OC_LOG(INFO, TAG, "-t 3  :  Discover Resources and Initiate Nonconfirmable Get Request"
            " with query filter.");
    OC_LOG(INFO, TAG, "-t 4  :  Discover Resources and Initiate Nonconfirmable Put Requests");
    OC_LOG(INFO, TAG, "-t 5  :  Discover Resources and Initiate Nonconfirmable Post Requests");
    OC_LOG(INFO, TAG, "-t 6  :  Discover Resources and Initiate Nonconfirmable Delete Requests");
    OC_LOG(INFO, TAG, "-t 7  :  Discover Resources and Initiate Nonconfirmable Observe Requests");
    OC_LOG(INFO, TAG, "-t 8  :  Discover Resources and Initiate Nonconfirmable Get Request "\
            "for a resource which is unavailable");
    OC_LOG(INFO, TAG, "-t 9  :  Discover Resources and Initiate Confirmable Get Request");
    OC_LOG(INFO, TAG, "-t 10 :  Discover Resources and Initiate Confirmable Post Request");
    OC_LOG(INFO, TAG, "-t 11 :  Discover Resources and Initiate Confirmable Delete Requests");
    OC_LOG(INFO, TAG, "-t 12 :  Discover Resources and Initiate Confirmable Observe Requests"\
            " and cancel with Low QoS");

#ifdef WITH_PRESENCE
    OC_LOG(INFO, TAG, "-t 13 :  Discover Resources and Initiate Nonconfirmable presence");
    OC_LOG(INFO, TAG, "-t 14 :  Discover Resources and Initiate Nonconfirmable presence with "\
            "filter");
    OC_LOG(INFO, TAG, "-t 15 :  Discover Resources and Initiate Nonconfirmable presence with "\
            "2 filters");
    OC_LOG(INFO, TAG, "-t 16 :  Discover Resources and Initiate Nonconfirmable multicast presence.");
#endif

    OC_LOG(INFO, TAG, "-t 17 :  Discover Resources and Initiate Nonconfirmable Observe Requests "\
            "then cancel immediately with High QOS");
    OC_LOG(INFO, TAG, "-t 18 :  Discover Resources and Initiate Nonconfirmable Get Request and "\
            "add  vendor specific header options");
    OC_LOG(INFO, TAG, "-t 19 :  Discover Platform");
    OC_LOG(INFO, TAG, "-t 20 :  Discover Devices");
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
                       (ConnType), qos, &cbData, options, numOptions);

    if (ret != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCDoResource returns error %d with method %d", ret, method);
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
        OC_LOG(INFO, TAG, "Callback Context for PUT recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Put Response"));
    }
    else
    {
        OC_LOG_V(INFO, TAG, "putReqCB received Null clientResponse");
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult postReqCB(void *ctx, OCDoHandle /*handle*/,
                                   OCClientResponse *clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for POST recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Post Response"));
    }
    else
    {
        OC_LOG_V(INFO, TAG, "postReqCB received Null clientResponse");
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult deleteReqCB(void *ctx,
                                     OCDoHandle /*handle*/,
                                     OCClientResponse *clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for DELETE recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Delete Response"));
    }
    else
    {
        OC_LOG_V(INFO, TAG, "deleteReqCB received Null clientResponse");
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    if (clientResponse == NULL)
    {
        OC_LOG(INFO, TAG, "getReqCB received NULL clientResponse");
        return   OC_STACK_DELETE_TRANSACTION;
    }

    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");
    }

    OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
    OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
    OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    OC_LOG(INFO, TAG, ("=============> Get Response"));

    if (clientResponse->numRcvdVendorSpecificHeaderOptions > 0)
    {
        OC_LOG (INFO, TAG, "Received vendor specific options");
        uint8_t i = 0;
        OCHeaderOption * rcvdOptions = clientResponse->rcvdVendorSpecificHeaderOptions;
        for( i = 0; i < clientResponse->numRcvdVendorSpecificHeaderOptions; i++)
        {
            if (((OCHeaderOption)rcvdOptions[i]).protocolID == OC_COAP_ID)
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

OCStackApplicationResult obsReqCB(void* ctx, OCDoHandle /*handle*/,
                                  OCClientResponse * clientResponse)
{
    if (ctx == (void*)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for OBS query recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        OC_LOG_V(INFO, TAG, "Callback Context for OBSERVE notification recvd successfully %d",
                gNumObserveNotifies);
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Obs Response"));
        gNumObserveNotifies++;
        if (gNumObserveNotifies == 15) //large number to test observing in DELETE case.
        {
            if (TestCase == TEST_OBS_REQ_NON || TestCase == TEST_OBS_REQ_CON)
            {
                if (OCCancel (gObserveDoHandle, OC_LOW_QOS, NULL, 0) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Observe cancel error");
                }
                return OC_STACK_DELETE_TRANSACTION;
            }
            else if (TestCase == TEST_OBS_REQ_NON_CANCEL_IMM)
            {
                if (OCCancel (gObserveDoHandle, OC_HIGH_QOS, NULL, 0) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Observe cancel error");
                }
            }
        }
        if (clientResponse->sequenceNumber == OC_OBSERVE_REGISTER)
        {
            OC_LOG(INFO, TAG, "This also serves as a registration confirmation");
        }
        else if (clientResponse->sequenceNumber == OC_OBSERVE_DEREGISTER)
        {
            OC_LOG(INFO, TAG, "This also serves as a deregistration confirmation");
            return OC_STACK_DELETE_TRANSACTION;
        }
        else if (clientResponse->sequenceNumber == OC_OBSERVE_NO_OPTION)
        {
            OC_LOG(INFO, TAG, "This also tells you that registration/deregistration failed");
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    else
    {
        OC_LOG_V(INFO, TAG, "obsReqCB received Null clientResponse");
    }
    return OC_STACK_KEEP_TRANSACTION;
}
#ifdef WITH_PRESENCE
OCStackApplicationResult presenceCB(void* ctx, OCDoHandle /*handle*/,
                                    OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for Presence recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "Callback Context for Presence notification recvd successfully %d",
                gNumPresenceNotifies);
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        OC_LOG(INFO, TAG, ("=============> Presence Response"));
        gNumPresenceNotifies++;
        if (gNumPresenceNotifies == 20)
        {
            if (OCCancel(gPresenceHandle, OC_LOW_QOS, NULL, 0) != OC_STACK_OK)
            {
                OC_LOG(ERROR, TAG, "Presence cancel error");
            }
            return OC_STACK_DELETE_TRANSACTION;
        }
    }
    else
    {
        OC_LOG_V(INFO, TAG, "presenceCB received Null clientResponse");
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
        OC_LOG(INFO, TAG, "Callback Context for DISCOVER query recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));

        std::string connectionType = getConnectivityType (clientResponse->connType);
        OC_LOG_V(INFO, TAG, "Discovered on %s", connectionType.c_str());
        OC_LOG_V(INFO, TAG,
                "Device =============> Discovered @ %s:%d",
                clientResponse->devAddr.addr,
                clientResponse->devAddr.port);
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);

        ConnType = clientResponse->connType;
        serverAddr = clientResponse->devAddr;
        parseClientResponse(clientResponse);

        switch(TestCase)
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
        OC_LOG_V(INFO, TAG, "discoveryReqCB received Null clientResponse");
    }
    return OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult PlatformDiscoveryReqCB(void* ctx,
                                                OCDoHandle /*handle*/,
                                                OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for Platform DISCOVER query recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG(INFO, TAG, ("Discovery Response:"));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    }
    else
    {
        OC_LOG_V(INFO, TAG, "PlatformDiscoveryReqCB received Null clientResponse");
    }

    return (UnicastDiscovery) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION;
}

OCStackApplicationResult DeviceDiscoveryReqCB(void* ctx, OCDoHandle /*handle*/,
                                              OCClientResponse * clientResponse)
{
    if (ctx == (void*) DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for Device DISCOVER query recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG(INFO, TAG, ("Discovery Response:"));
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    }
    else
    {
        OC_LOG_V(INFO, TAG, "PlatformDiscoveryReqCB received Null clientResponse");
    }

    return (UnicastDiscovery) ? OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION;
}

#ifdef WITH_PRESENCE
int InitPresence()
{
    OCStackResult result = OC_STACK_OK;
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    std::ostringstream querySuffix;
    query << OC_RSRVD_PRESENCE_URI;
    if (TestCase == TEST_OBS_PRESENCE)
    {
        result = InvokeOCDoResource(query, &serverAddr, OC_REST_PRESENCE,
                OC_LOW_QOS, presenceCB, NULL, 0);
    }
    if (TestCase == TEST_OBS_PRESENCE_WITH_FILTER || TestCase == TEST_OBS_PRESENCE_WITH_FILTERS)
    {
        querySuffix.str("");
        querySuffix << query.str() << "?rt=core.led";
        result = InvokeOCDoResource(querySuffix, &serverAddr, OC_REST_PRESENCE,
                OC_LOW_QOS, presenceCB, NULL, 0);
    }
    if (TestCase == TEST_OBS_PRESENCE_WITH_FILTERS)
    {
        if (result == OC_STACK_OK)
        {
            querySuffix.str("");
            querySuffix << query.str() << "?rt=core.fan";
            result = InvokeOCDoResource(querySuffix, &serverAddr, OC_REST_PRESENCE, OC_LOW_QOS,
                    presenceCB, NULL, 0);
        }
    }
    if (TestCase == TEST_OBS_MULTICAST_PRESENCE)
    {
        if (result == OC_STACK_OK)
        {
            std::ostringstream multicastPresenceQuery;
            multicastPresenceQuery.str("");
            multicastPresenceQuery << "coap://" << OC_MULTICAST_PREFIX << OC_RSRVD_PRESENCE_URI;
            result = InvokeOCDoResource(multicastPresenceQuery, &serverAddr, OC_REST_PRESENCE, OC_LOW_QOS,
                    presenceCB, NULL, 0);
        }
    }
    return result;
}
#endif

int InitGetRequestToUnavailableResource(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << "/SomeUnknownResource";
    return (InvokeOCDoResource(query, &serverAddr, OC_REST_GET, (qos == OC_HIGH_QOS)? OC_HIGH_QOS:OC_LOW_QOS,
            getReqCB, NULL, 0));
}

int InitObserveRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    return (InvokeOCDoResource(query, &serverAddr, OC_REST_OBSERVE,
              (qos == OC_HIGH_QOS)? OC_HIGH_QOS:OC_LOW_QOS, obsReqCB, NULL, 0));
}

int InitPutRequest(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;
    return (InvokeOCDoResource(query, &serverAddr, OC_REST_PUT, (qos == OC_HIGH_QOS)? OC_HIGH_QOS:OC_LOW_QOS,
            putReqCB, NULL, 0));
}

int InitPostRequest(OCQualityOfService qos)
{
    OCStackResult result;
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;

    // First POST operation (to create an Light instance)
    result = InvokeOCDoResource(query, &serverAddr, OC_REST_POST,
                               ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                               postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        // Error can happen if for example, network connectivity is down
        OC_LOG(INFO, TAG, "First POST call did not succeed");
    }

    // Second POST operation (to create an Light instance)
    result = InvokeOCDoResource(query, &serverAddr, OC_REST_POST,
                               ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                               postReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        OC_LOG(INFO, TAG, "Second POST call did not succeed");
    }

    // This POST operation will update the original resourced /a/light
    return (InvokeOCDoResource(query, &serverAddr, OC_REST_POST,
                               ((qos == OC_HIGH_QOS) ? OC_HIGH_QOS: OC_LOW_QOS),
                               postReqCB, NULL, 0));
}

void* RequestDeleteDeathResourceTask(void* myqos)
{
    Sleep (30000);//long enough to give the server time to finish deleting the resource.
    std::ostringstream query;
    query << coapServerResource;

    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);

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

    OCStackResult result = InvokeOCDoResource(query, &serverAddr, OC_REST_DELETE,
                               qos,
                               deleteReqCB, NULL, 0);

    if (OC_STACK_OK != result)
    {
        OC_LOG(INFO, TAG, "Second DELETE call did not succeed");
    }

    return NULL;
}

int InitDeleteRequest(OCQualityOfService qos)
{
    OCStackResult result;
    std::ostringstream query;
    query << coapServerResource;

    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);

    // First DELETE operation
    result = InvokeOCDoResource(query, &serverAddr, OC_REST_DELETE,
                               qos,
                               deleteReqCB, NULL, 0);
    if (OC_STACK_OK != result)
    {
        // Error can happen if for example, network connectivity is down
        OC_LOG(INFO, TAG, "First DELETE call did not succeed");
    }
    else
    {
        //Create a thread to delete this resource again
        HANDLE threadId;
        threadId = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RequestDeleteDeathResourceTask, (void*)qos, 0, NULL);
    }

    OC_LOG_V(INFO, TAG, "\n\nExit  %s", __func__);
    return result;
}

int InitGetRequest(OCQualityOfService qos, uint8_t withVendorSpecificHeaderOptions, bool getWithQuery)
{

    OCHeaderOption options[MAX_HEADER_OPTIONS];

    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);
    std::ostringstream query;
    query << coapServerResource;

    // ocserver is written to only process "power<X" query.
    if (getWithQuery)
    {
        OC_LOG(INFO, TAG, "Using query power<50");
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
        return (InvokeOCDoResource(query, &serverAddr, OC_REST_GET,
                (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS, getReqCB, options, 2));
    }
    else
    {
        return (InvokeOCDoResource(query, &serverAddr, OC_REST_GET,
                (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS, getReqCB, NULL, 0));
    }
}

int InitPlatformDiscovery(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);

    OCStackResult ret;
    OCCallbackData cbData;
    char szQueryUri[64] = { 0 };

    snprintf(szQueryUri, sizeof (szQueryUri) - 1, PLATFORM_DISCOVERY_QUERY, discoveryAddr);

    cbData.cb = PlatformDiscoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, CT_DEFAULT,
                       (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack device error");
    }

    return ret;
}

int InitDeviceDiscovery(OCQualityOfService qos)
{
    OC_LOG_V(INFO, TAG, "\n\nExecuting %s", __func__);

    OCStackResult ret;
    OCCallbackData cbData;
    char szQueryUri[100] = { 0 };

    snprintf(szQueryUri, sizeof (szQueryUri) - 1, DEVICE_DISCOVERY_QUERY, discoveryAddr);

    cbData.cb = DeviceDiscoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, CT_DEFAULT,
                       (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack device error");
    }

    return ret;
}

int InitDiscovery(OCQualityOfService qos)
{
    OCStackResult ret;
    OCCallbackData cbData;
    char szQueryUri[100] = { 0 };

    snprintf(szQueryUri, sizeof (szQueryUri) - 1, RESOURCE_DISCOVERY_QUERY, discoveryAddr);

    cbData.cb = discoveryReqCB;
    cbData.context = (void*)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    ret = OCDoResource(NULL, OC_REST_DISCOVER, szQueryUri, NULL, 0, CT_DEFAULT,
                       (qos == OC_HIGH_QOS) ? OC_HIGH_QOS : OC_LOW_QOS,
                       &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
    return ret;
}

int main(int argc, char* argv[])
{
    if (argc != 7)
    {
        PrintUsage();
        return -1;
    }

    for (int i = 0; i < 3; ++i)
    {
        char *opt = argv[i * 2 + 1];
        if (_stricmp(opt, "-u") == 0)
        {
            UnicastDiscovery = atoi(argv[i * 2 + 2]);
        }
        else if (_stricmp(opt, "-t") == 0)
        {
            TestCase = atoi(argv[i * 2 + 2]);
        }
        else if (_stricmp(opt, "-c") == 0)
        {
            Connectivity = atoi(argv[i * 2 + 2]);
        }
        else
        {
            PrintUsage();
            return -1;
        }
    }
    
    if ((UnicastDiscovery != 0 && UnicastDiscovery != 1) ||
            (TestCase < TEST_DISCOVER_REQ || TestCase >= MAX_TESTS) ||
            (Connectivity < CT_ADAPTER_DEFAULT || Connectivity >= MAX_CT))
    {
        PrintUsage();
        return -1;
    }

    if (OCInit1(OC_CLIENT, OC_DEFAULT_FLAGS, OC_DEFAULT_FLAGS) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

#ifdef ROUTING_GATEWAY
    /*
     * Before invoking Discover resource, we process the gateway requests
     * and form the routing table.
     */
    for (int index = 0; index < MAX_NUM_GATEWAY_REQUEST; index++)
    {
        if (OC_STACK_OK != OCProcess())
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }
        usleep(SLEEP_DURATION);
    }
#endif
    if (Connectivity == CT_ADAPTER_DEFAULT || Connectivity == CT_IP)
    {
        ConnType = CT_ADAPTER_IP;
    }
    else
    {
        OC_LOG(INFO, TAG, "Default Connectivity type selected...");
        PrintUsage();
    }

    discoveryAddr[0] = '\0';

    if (UnicastDiscovery)
    {
        OC_LOG(INFO, TAG, "Enter IP address of server with optional port number");
        OC_LOG(INFO, TAG, "IPv4: 192.168.0.15:45454\n");
        OC_LOG(INFO, TAG, "IPv6: [fe80::20c:29ff:fe1b:9c5]:45454\n");

        if (fgets(discoveryAddr, sizeof (discoveryAddr), stdin))
        {
            //Strip newline char from ipv4addr
            StripNewLineChar(discoveryAddr);
        }
        else
        {
            OC_LOG(ERROR, TAG, "!! Bad input for IP address. !!");
            return OC_STACK_INVALID_PARAM;
        }
    }

    if (UnicastDiscovery == 0 && TestCase == TEST_DISCOVER_DEV_REQ)
    {
        InitDeviceDiscovery(OC_LOW_QOS);
    }
    else if (UnicastDiscovery == 0 && TestCase == TEST_DISCOVER_PLATFORM_REQ)
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
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }
#ifndef ROUTING_GATEAWAY
        Sleep(1000);
#endif
    }
    OC_LOG(INFO, TAG, "Exiting occlient main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack stop error");
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
