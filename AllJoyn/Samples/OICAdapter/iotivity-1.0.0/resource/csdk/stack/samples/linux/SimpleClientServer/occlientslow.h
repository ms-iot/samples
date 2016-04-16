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
#define TAG "occlientslow"
#define DEFAULT_CONTEXT_VALUE 0x99
#ifndef MAX_LENGTH_IPv4_ADDR
#define MAX_LENGTH_IPv4_ADDR 16
#endif

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

/**
 * List of methods that can be inititated from the client
 */
typedef enum
{
    TEST_DISCOVER_REQ = 1,
    TEST_NON_CON_OP,
    TEST_CON_OP,
    TEST_NON_CON_PUT,
    TEST_CON_PUT,
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
// Function prototype
//-----------------------------------------------------------------------------

/* call getResult in common.cpp to get the result in string format. */
const char *getResult(OCStackResult result);

/* Get the IP address of the server */
std::string getIPAddrTBServer(OCClientResponse * clientResponse);

/* Get the port number the server is listening on */
std::string getPortTBServer(OCClientResponse * clientResponse);

/* Returns the query string for GET and PUT operations */
std::string getQueryStrForGetPut(OCClientResponse * clientResponse);

/* Following are initialization functions for GET, PUT
 * POST & Discovery operations
 */
int InitGetRequest(OCQualityOfService qos);
int InitPutRequest(OCQualityOfService qos);
int InitDiscovery();

/* Function to retrieve ip address, port no. of the server
 *  and query for the operations to be performed.
 */
void parseClientResponse(OCClientResponse * clientResponse);

/* This function calls OCDoResource() which in turn makes calls
 * to the lower layers
 */
OCStackResult InvokeOCDoResource(std::ostringstream &query,
        OCMethod method, OCQualityOfService qos,
        OCClientResponseHandler cb, OCHeaderOption * options, uint8_t numOptions);

//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------

/* Following are callback functions for the  GET and Discovery operations
 */

OCStackApplicationResult getReqCB(void* ctx, OCDoHandle handle, OCClientResponse * clientResponse);

OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
        OCClientResponse * clientResponse);

#endif

