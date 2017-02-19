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

#ifndef OCCLIENT_H_
#define OCCLIENT_H_

#include "ocstack.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TAG "occlient"
#define DEFAULT_CONTEXT_VALUE 0x99
#ifndef MAX_LENGTH_IPv4_ADDR
#define MAX_LENGTH_IPv4_ADDR 16
#endif

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

/**
 * List of methods that can be initiated from the client
 */
typedef enum {
    TEST_DISCOVER_REQ = 1,
    TEST_GET_REQ_NON,
    TEST_GET_REQ_NON_WITH_FILTERS,
    TEST_PUT_REQ_NON,
    TEST_POST_REQ_NON,
    TEST_DELETE_REQ_NON,
    TEST_OBS_REQ_NON,
    TEST_GET_UNAVAILABLE_RES_REQ_NON,
    TEST_GET_REQ_CON,
    TEST_POST_REQ_CON,
    TEST_DELETE_REQ_CON,
    TEST_OBS_REQ_CON,
#ifdef WITH_PRESENCE
    TEST_OBS_PRESENCE,
    TEST_OBS_PRESENCE_WITH_FILTER,
    TEST_OBS_PRESENCE_WITH_FILTERS,
    TEST_OBS_MULTICAST_PRESENCE,
#endif
    TEST_OBS_REQ_NON_CANCEL_IMM,
    TEST_GET_REQ_NON_WITH_VENDOR_HEADER_OPTIONS,
    TEST_DISCOVER_PLATFORM_REQ,
    TEST_DISCOVER_DEV_REQ,
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
} CLIENT_CONNECTIVITY_TYPE;

#ifdef WITH_PRESENCE
int InitPresence();
#endif

//----------------------------------------------------------------------------
// Function prototype
//----------------------------------------------------------------------------
std::string getConnectivityType (OCConnectivityType connType);

/* call getResult in common.cpp to get the result in string format. */
const char *getResult(OCStackResult result);

/* Get the IP address of the server */
std::string getIPAddrTBServer(OCClientResponse * clientResponse);

/* Get the port number the server is listening on */
std::string getPortTBServer(OCClientResponse * clientResponse);

/* Returns the query string for GET and PUT operations */
std::string getQueryStrForGetPut(OCClientResponse * clientResponse);

/* Following are initialization functions for GET, Observe, PUT
 * POST, Delete & Discovery operations
 */
int InitGetRequestToUnavailableResource(OCQualityOfService qos);
int InitObserveRequest(OCQualityOfService qos);
int InitPutRequest(OCQualityOfService qos);
int InitGetRequest(OCQualityOfService qos, uint8_t withVendorSpecificHeaderOptions, bool getWithQuery);
int InitPostRequest(OCQualityOfService qos);
int InitDeleteRequest(OCQualityOfService qos);
int InitGetRequest(OCQualityOfService qos);
int InitDeviceDiscovery(OCQualityOfService qos);
int InitPlatformDiscovery(OCQualityOfService qos);
int InitDiscovery(OCQualityOfService qos);

/* Function to retrieve ip address, port no. of the server
 *  and query for the operations to be performed.
 */
void parseClientResponse(OCClientResponse * clientResponse);

/* Call delete operation on already deleted resource */
void* RequestDeleteDeathResourceTask(void* myqos);

/* This method calls OCDoResource() which in turn makes calls
 * to the lower layers
 */
OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions);

//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------

/* Following are callback functions for the  GET, Observe, PUT
 * POST, Delete, Presence & Discovery operations
 */
OCStackApplicationResult putReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult postReqCB(void *ctx, OCDoHandle handle, OCClientResponse *clientResponse);

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult obsReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult presenceCB(void* ctx,
            OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult deleteReqCB(void *ctx,
            OCDoHandle handle, OCClientResponse *clientResponse);

OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
        OCClientResponse * clientResponse);


#endif

