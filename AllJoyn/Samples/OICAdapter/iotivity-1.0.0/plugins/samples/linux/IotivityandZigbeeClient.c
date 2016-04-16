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

// The source file for sample application "IotivityandZigbee".

// This application will utilize our interface (ie. zpluginz.h).
// The application may still be responsible for making any IoTivity API calls,
// except for resource-specific IoTivity APIs (ie. OCCreateResource(),
// OCDeleteResource(), EntityHandler()..etc.)

#include "IotivityandZigbeeClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "ocstack.h"
#include "logger.h"
#include "ocpayload.h"
#include "payload_logging.h"
#include "oic_string.h"

#define DEFAULT_CONTEXT_VALUE       (0x99)
#define MAX_QUERY_SIZE              (1024)
#define MAX_URI_SIZE                (256)
#define MAX_RESOURCE_TYPE_SIZE      (32)
#define MAX_RESOURCE_TYPE_LENGTH    (MAX_RESOURCE_TYPE_SIZE - 1)
#define MAX_RESOURCES_REMEMBERED    (10)

#define TAG "oc_zb_client"

static uint32_t countDiscoveredResources = 0;
static bool promptUser = false;

static const char*      coapServerIP    = "255.255.255.255";
static       uint32_t   coapServerPort  = 5683;

typedef struct
{
    char uri[MAX_URI_SIZE];
    char resourceType[MAX_RESOURCE_TYPE_SIZE];

} DiscoveredResourceInfo;

static DiscoveredResourceInfo g_discoveredResources[MAX_RESOURCES_REMEMBERED];

static void PrintTestCases()
{
    printf("\nTest Cases:\n");
    printf ("\n\t%d : Quit    %d: GET    %d: Build PUT payload %d: OBSERVE\n\n",
            TEST_QUIT, TEST_GET, TEST_CUSTOM_PUT, TEST_OBSERVE);
    printf ("\t%d : Turn binary switch for light ON\n", TEST_TURN_SWITCH_ON);
    printf ("\t%d : Turn binary switch for light OFF\n", TEST_TURN_SWITCH_OFF);
    printf ("\t%d : Change light brightness\n", TEST_SET_LIGHT_BRIGHTNESS);
    printf ("\n\t%d : Check for observation updates.\n", TEST_CYCLE);
}

static void PrintResources ()
{
    printf("\nResources: \n");
    for (int i = 0; i < countDiscoveredResources; ++i)
    {
        printf("\t# : %u \t URI: %s \t Type:%s\n", i, g_discoveredResources[i].uri,
            g_discoveredResources[i].resourceType);
    }
}

void rememberDiscoveredResources (OCClientResponse *clientResponse)
{
    OCResourcePayload* itr = NULL;
    if(!(OCDiscoveryPayload*)clientResponse->payload)
    {
        OC_LOG (INFO, TAG, "No resources discovered.");
        return;
    }

    itr = ((OCDiscoveryPayload*)(clientResponse->payload))->resources;

    while(itr && itr != itr->next)
    {
        if (countDiscoveredResources == MAX_RESOURCES_REMEMBERED)
        {
            OC_LOG_V (INFO, TAG, "Only remembering %u resources. Ignoring rest.",
                MAX_RESOURCES_REMEMBERED);
            break;
        }
        strncpy (g_discoveredResources[countDiscoveredResources].uri,
            itr->uri, MAX_URI_SIZE - 1);
        strncpy (g_discoveredResources[countDiscoveredResources].resourceType,
            itr->types->value, MAX_RESOURCE_TYPE_SIZE - 1);
        ++countDiscoveredResources;
        itr = itr->next;
    }
}

OCStackResult InvokeOCDoResource (char *query,
                                 OCPayload *payload,
                                 OCMethod method,
                                 OCClientResponseHandler cb)
{
    OCCallbackData cbData = {
                                .context = (void*)DEFAULT_CONTEXT_VALUE,
                                .cb = cb
                            };

    OCDoHandle handle = NULL;

    OCStackResult ret = OCDoResource(&handle, method, query, 0, payload,
                        CT_ADAPTER_IP, OC_LOW_QOS, &cbData, NULL, 0);

    if (ret != OC_STACK_OK)
    {
        promptUser = true;
        OC_LOG_V(ERROR, TAG, "OCDoResource returns error %d with method %d", ret, method);
    }
    return ret;
}

OCStackApplicationResult responseCallbacks(void* ctx,
                OCDoHandle handle,
                OCClientResponse * clientResponse)
{
    (void)handle;
    if(clientResponse == NULL)
    {
        OC_LOG(INFO, TAG, "responseCallbacks received NULL clientResponse");
        return   OC_STACK_DELETE_TRANSACTION;
    }

    OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    promptUser = true;
    return OC_STACK_KEEP_TRANSACTION;
}

int InitGetRequest (const char *resourceUri)
{
    char query[MAX_QUERY_SIZE] = {0};
    snprintf (query, sizeof(query), "coap://%s:%u%s", coapServerIP,
                                            coapServerPort,resourceUri);

    OC_LOG_V(INFO, TAG, "Executing %s queryString is: %s", __func__, query);

    return (InvokeOCDoResource(query, NULL, OC_REST_GET, responseCallbacks));
}

int InitPutRequest (const char *resourceUri, OCPayload* payload)
{
    char query[MAX_QUERY_SIZE] = {0};
    snprintf (query, sizeof(query), "coap://%s:%u%s", coapServerIP, coapServerPort,
                                                            resourceUri);
    OC_LOG_V(INFO, TAG, "Executing %s queryString is: %s", __func__, query);

    return (InvokeOCDoResource(query, payload, OC_REST_PUT, responseCallbacks));
}

int InitObserveRequest(const char *resourceUri)
{
    OC_LOG_V(INFO, TAG, "Executing %s for resource: %s", __func__, resourceUri);
    return (InvokeOCDoResource(resourceUri, NULL, OC_REST_OBSERVE, responseCallbacks));
}

OCPayload * getSwitchStatePayload (bool state)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if(!payload)
    {
       OC_LOG (ERROR, TAG, "Failed to create payload object");
       exit(1);
    }
    OCRepPayloadSetPropBool(payload, "value", state);
    return (OCPayload*) payload;
}

OCPayload* getChangeLevelPayload(uint32_t level)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if(!payload)
    {
        OC_LOG (ERROR, TAG, "Failed to create payload object");
        exit(1);
    }

    OC_LOG_V(INFO, TAG, "Setting level to : %u", level);
    OCRepPayloadSetPropInt(payload, "dimmingSetting", level);
    return (OCPayload*) payload;
}

OCStackApplicationResult discoveryReqCB(void* ctx, OCDoHandle handle,
                            OCClientResponse * clientResponse)
{
    (void)handle;
    if (!clientResponse)
    {
        OC_LOG(INFO, TAG, "Discovery response is NULL");
        return OC_STACK_KEEP_TRANSACTION;
    }

    OC_LOG_PAYLOAD(INFO, clientResponse->payload);
    OC_LOG_V(INFO, TAG, "Discovered @ %s:%d", clientResponse->devAddr.addr,
                                clientResponse->devAddr.port);

    coapServerIP   = OICStrdup (clientResponse->devAddr.addr);
    coapServerPort = clientResponse->devAddr.port;

    rememberDiscoveredResources (clientResponse);

    promptUser = true;

    return OC_STACK_KEEP_TRANSACTION;
}

OCPayload* getCustomPutPayload ()
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if(!payload)
    {
        OC_LOG (ERROR, TAG, "Failed to create payload object");
        exit(1);
    }

    char key[100] = {0};
    char input[100] = {0};
    char valueString[100] = {0};
    int value = 0;
    double valueDouble = 0.0;
    int type = -1;

    printf("\nEnter key value pairs as:\t<type(int)> <key> <value>\n");
    printf("\nType: 0:bool \t 1:int \t 2:double\n");
    while (true)
    {
        printf ("Blank line / press ENTER to finish :");
        char *ret = fgets (input, sizeof(input), stdin);
        (void) ret;
        int inCount = sscanf (input, "%d %s %s", &type, key, valueString);

        if (inCount <= 0)
        {
            break;
        }
        if (inCount != 3)
        {
            printf("Invalid input\n");
            OCRepPayloadDestroy (payload);
            promptUser = true;
            return NULL;
        }

        if (type == 0)  //bool
        {
            if (sscanf(valueString, "%d", &value) == 1)
            {
                OCRepPayloadSetPropBool(payload, key, value);
            }
        }
        else if (type == 1)  //int
        {
            if (sscanf(valueString, "%d", &value) == 1)
            {
                OCRepPayloadSetPropInt(payload, key, value);
            }
        }
        else if (type == 2)  //double
        {
            if (sscanf(valueString, "%lf", &valueDouble) == 1)
            {
                OCRepPayloadSetPropDouble(payload, key, valueDouble);
            }
        }
        else
        {
            OC_LOG(ERROR, TAG, "Invalid entry. Stopping accepting key-values");
            OCRepPayloadDestroy (payload);
            promptUser = true;
            return NULL;
        }
        memset (input, 0, sizeof (input));
        memset (key, 0, sizeof (key));
        memset (valueString, 0, sizeof (valueString));
    }

    if (payload->values)
    {
        return (OCPayload *) payload;
    }
    else
    {
        OCRepPayloadDestroy (payload);
        return NULL;
    }
}

void processUserInput (int resourceNo, int testCase)
{
    switch (testCase)
    {
        case TEST_GET:
            InitGetRequest (g_discoveredResources[resourceNo].uri);
            break;

        case TEST_CUSTOM_PUT:
        {
            OCPayload *payload = getCustomPutPayload ();
            if (payload)
            {
                InitPutRequest (g_discoveredResources[resourceNo].uri, payload);
            }
            else
            {
                OC_LOG(ERROR, TAG, "Error creating payload. Not sending PUT request");
                promptUser = true;
            }
            break;
        }

        case TEST_OBSERVE:
            InitObserveRequest (g_discoveredResources[resourceNo].uri);
            break;

        case TEST_TURN_SWITCH_ON:
            InitPutRequest (g_discoveredResources[resourceNo].uri, getSwitchStatePayload (true));
            break;

        case TEST_TURN_SWITCH_OFF:
            InitPutRequest (g_discoveredResources[resourceNo].uri, getSwitchStatePayload (false));
            break;

        case TEST_SET_LIGHT_BRIGHTNESS:
            printf ("Change bulb level [0-100] to ? :");
            int level = 0;
            if (scanf ("%d", &level) > 0)
            {
                InitPutRequest (g_discoveredResources[resourceNo].uri,
                    getChangeLevelPayload (level));
            }
            else
            {
                printf("Invalid value\n");
                promptUser = true;
            }
            break;

        case TEST_CYCLE:
            OCProcess();
            promptUser = true;
            break;

        case TEST_QUIT:
            raise (SIGINT);
            break;

        default:
            promptUser = true;
            OC_LOG(INFO, TAG, "Invalid test case");
    }
}

void getTestCaseFromUser ()
{
    PrintResources ();
    PrintTestCases ();
    printf("\nUsage:<resource number> <test case> :");

    char input[10] = {0};
    int resourceNo = 0;
    int testCase = 0;

    char * ret = fgets (input, sizeof(input), stdin);
    (void) ret;
    int inCount = sscanf (input, "%d %d", &resourceNo, &testCase);

    if (inCount != 2)
    {
        printf("Invalid input\n");
        promptUser = true;
        return;
    }
    if (resourceNo >= countDiscoveredResources)
    {
        printf("Invalid resource\n");
        promptUser = true;
        return;
    }
    processUserInput (resourceNo, testCase);
}

OCStackResult DiscoverResources ()
{
    OCCallbackData cbData = {
                                .context = (void*)DEFAULT_CONTEXT_VALUE,
                                .cb = discoveryReqCB
                            };

    OCStackResult ret = OCDoResource(NULL, OC_REST_DISCOVER, OC_RSRVD_WELL_KNOWN_URI,
                        0, 0, CT_ADAPTER_IP,OC_LOW_QOS, &cbData, NULL, 0);

    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCDoResource error");
    }
    return ret;
}

bool processSignal(bool set)
{
    static sig_atomic_t signal = 0;
    if (set)
    {
        signal = 1;
    }
    return signal == 1;
}

void processCancel(int signal)
{
    if(signal == SIGINT)
    {
        processSignal(true);
    }
}

int main(int argc, char* argv[])
{
    OCStackResult result;
    OC_LOG(INFO, TAG, "Initializing IoTivity...");

    result = OCInit(NULL, 0, OC_CLIENT);
    if (result != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCInit Failed %d", result);
        return -1;
    }

    DiscoverResources ();

    if (signal(SIGINT, processCancel) == SIG_ERR)
    {
        OC_LOG(ERROR, TAG, "Unable to catch SIGINT, terminating...");
    }
    else
    {
        OC_LOG(INFO, TAG, "Press Ctrl-C to terminate");
        // Loop until sigint
        while (!processSignal(false) && result == OC_STACK_OK)
        {
            result = OCProcess();
            if (result != OC_STACK_OK)
            {
                OC_LOG_V(ERROR, TAG, "OCProcess Failed: %d", result);
                break;
            }

            if (promptUser)
            {
                promptUser = false;
                getTestCaseFromUser ();
            }
        }
    }

    OC_LOG(INFO, TAG, "Stopping IoTivity...");
    result = OCStop();
    if (result != OC_STACK_OK)
    {
        OC_LOG_V(ERROR, TAG, "OCStop Failed: %d", result);
    }

    return 0;
}

