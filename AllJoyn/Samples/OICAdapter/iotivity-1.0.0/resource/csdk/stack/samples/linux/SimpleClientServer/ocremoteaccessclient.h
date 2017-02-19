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

#ifndef OCREMOTECLIENT_H_
#define OCREMOTECLIENT_H_

#include "ocstack.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TAG "occlient_remoteaccess"
#define DEFAULT_CONTEXT_VALUE 0x99
#ifndef MAX_LENGTH_IPv4_ADDR
#define MAX_LENGTH_IPv4_ADDR 16
#endif

/**
 * List of methods that can be initiated from the client
 */
typedef enum {
    TEST_DISCOVER_REQ = 1,
    TEST_GET_REQ_NON,
    TEST_PUT_REQ_NON,
    TEST_POST_REQ_NON,
    TEST_DELETE_REQ_NON,
    TEST_OBS_REQ_NON,
    TEST_OBS_REQ_NON_CANCEL_IMM,
    TEST_DISCOVER_PLATFORM_REQ,
    TEST_DISCOVER_DEV_REQ,
    MAX_TESTS
} CLIENT_TEST;

/* call getResult in common.cpp to get the result in string format. */
const char *getResult(OCStackResult result);

/* Following are initialization functions for GET, Observe, PUT
 * POST, Delete & Discovery operations
 */
int InitGetRequestToUnavailableResource(OCQualityOfService qos);
int InitObserveRequest(OCQualityOfService qos);
int InitPutRequest(OCQualityOfService qos);
int InitGetRequest(OCQualityOfService qos);
int InitPostRequest(OCQualityOfService qos);
int InitDeleteRequest(OCQualityOfService qos);
int InitDeviceDiscovery(OCQualityOfService qos);
int InitPlatformDiscovery(OCQualityOfService qos);
int InitDiscovery(OCQualityOfService qos);


/*
 * This method calls OCDoResource() which in turn makes calls
 * to the lower layers
 */
OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions);

/*
 * Following are callback functions for the  GET, Observe, PUT
 * POST, Delete, Presence & Discovery operations
 */
OCStackApplicationResult putReqCB   (void* ctx, OCDoHandle handle, OCClientResponse* clientResponse);
OCStackApplicationResult postReqCB  (void* ctx, OCDoHandle handle, OCClientResponse* clientResponse);
OCStackApplicationResult getReqCB   (void* ctx, OCDoHandle handle, OCClientResponse* clientResponse);
OCStackApplicationResult obsReqCB   (void* ctx, OCDoHandle handle, OCClientResponse* clientResponse);
OCStackApplicationResult presenceCB (void* ctx, OCDoHandle handle, OCClientResponse* clientResponse);
OCStackApplicationResult deleteReqCB(void* ctx, OCDoHandle handle, OCClientResponse* clientResponse);

OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
    OCClientResponse *clientResponse);


#endif

