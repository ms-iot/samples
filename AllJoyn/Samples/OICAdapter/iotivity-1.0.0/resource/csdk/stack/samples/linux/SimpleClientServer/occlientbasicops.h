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

#ifndef OCCLIENT_BASICOPS_H_
#define OCCLIENT_BASICOPS_H_

#include "ocstack.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TAG "occlientbasicops"
#define DEFAULT_CONTEXT_VALUE 0x99

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

/**
 * List of methods that can be inititated from the client
 */
typedef enum {
    TEST_DISCOVER_REQ = 1,
    TEST_NON_CON_OP,
    TEST_CON_OP,
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

//-----------------------------------------------------------------------------
//ResourceNode
//-----------------------------------------------------------------------------
struct ResourceNode
{
    const char * sid;
    const char * uri;
    OCDevAddr endpoint;
    ResourceNode * next;
};

//-----------------------------------------------------------------------------
// Function prototype
//-----------------------------------------------------------------------------

/* call getResult in common.cpp to get the result in string format. */
const char *getResult(OCStackResult result);

/* Performs GET/PUT/POST query on most recently discovered resource*/
void queryResource();

/* Parses JSON payload received in the clientResponse to extract sid and resource uri information.
 * Populates uri_c array with uris of the resources discovered and assigns sid_c with the server
 * id received in the clientResponse.
 */
int parseJSON(unsigned  const char * resJSONPayload, char ** sid_c,
              char *** uri_c, int * totalRes);

/*
 * Collect unique resource(sid:uri), regardless of the transport it arrives on.
 */
void collectUniqueResource(const OCClientResponse * clientResponse);

/*
 * Insert the newly discovered unique resource(sid:uri) in the front of the resourceList
 *
 */

int insertResource(const char * sid, char const * uri,
        const OCClientResponse * clientResponse);

/*
 * Returns most recently discovered resource
 */
const ResourceNode * getResource();

/*
 * Frees the ResourceList
 */
void freeResourceList();

/* Following are initialization functions for GET, PUT
 * POST & Discovery operations
 */
int InitPutRequest(OCQualityOfService qos);
int InitGetRequest(OCQualityOfService qos);
int InitPostRequest(OCQualityOfService qos);
int InitDiscovery();

/* Function to retrieve ip address, port no. of the server
 *  and query for the operations to be performed.
 */
void parseClientResponse(OCClientResponse * clientResponse);

/* This method calls OCDoResource() which in turn makes calls
 * to the lower layers
 */
OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCDevAddr *dest, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions);

/*
 * SIGINT handler: set gQuitFlag to 1 for graceful termination
 */
void handleSigInt(int signum);

/*
 * Printing helper functions
 */
static void PrintUsage();
void printResourceList();



//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------

/* Following are callback functions for the  GET, PUT
 * POST & Discovery operations
 */

OCStackApplicationResult putReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult postReqCB(void *ctx, OCDoHandle handle, OCClientResponse *clientResponse);

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
        OCClientResponse * clientResponse);
void StripNewLineChar(char* str);
#endif

