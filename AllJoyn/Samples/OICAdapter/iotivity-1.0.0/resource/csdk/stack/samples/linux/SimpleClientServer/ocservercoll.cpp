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
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <ocstack.h>
#include <logger.h>
#include "ocpayload.h"

const char *getResult(OCStackResult result);

#define TAG PCF("ocservercontainer")

volatile sig_atomic_t gQuitFlag = 0;
int gLightUnderObservation = 0;
void createResources();
typedef struct LIGHTRESOURCE{
    OCResourceHandle handle;
    bool state;
    int power;
} LightResource;

static LightResource light;

char *gLightResourceUri= (char *)"/a/light";
char *gRoomResourceUri= (char *)"/a/room";
char *gFanResourceUri= (char *)"/a/fan";

typedef enum
{
    TEST_INVALID = 0,
    TEST_DEFAULT_COLL_EH,
    TEST_APP_COLL_EH,
    MAX_TESTS
} SERVER_TEST;

void PrintUsage()
{
    OC_LOG(INFO, TAG, "Usage : ocservercoll -t <Test Case>");
    OC_LOG(INFO, TAG,
            "Test Case 1 : Create room resource with default collection entity handler.");
    OC_LOG(INFO, TAG,
            "Test Case 2 : Create room resource with application collection entity handler.");
}

unsigned static int TEST = TEST_INVALID;

static void
PrintReceivedMsgInfo(OCEntityHandlerFlag flag, OCEntityHandlerRequest * ehRequest)
{
    const char* typeOfMessage;
    const char* typeOfMethod;

    switch (flag)
    {
        case OC_REQUEST_FLAG:
            typeOfMessage = "OC_REQUEST_FLAG";
            break;
        case OC_OBSERVE_FLAG:
            typeOfMessage = "OC_OBSERVE_FLAG";
            break;
        default:
            typeOfMessage = "UNKNOWN";
    }

    if (ehRequest == NULL)
    {
        typeOfMethod = "UNKNOWN";
    }
    else if (ehRequest->method == OC_REST_GET)
    {
        typeOfMethod = "OC_REST_GET";
    }
    else
    {
        typeOfMethod = "OC_REST_PUT";
    }

    OC_LOG_V(INFO, TAG, "Receiving message type: %s, method %s", typeOfMessage,
            typeOfMethod);
}

//The only case when this entity handler is for a non-existing resource.
OCEntityHandlerResult
OCDeviceEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, char* uri, void* /*callbackParam*/)
{
    OC_LOG_V(INFO, TAG, "Inside device default entity handler - flags: 0x%x, uri: %s", flag, uri);

    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCEntityHandlerResponse response;

    if (!entityHandlerRequest)
    {
        OC_LOG(ERROR, TAG, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    if (entityHandlerRequest->resource == NULL)
    {
        OC_LOG(INFO, TAG, "Received request from client to a non-existing resource");
        ehResult = OC_EH_RESOURCE_NOT_FOUND;
    }
    else
    {
        OC_LOG_V(INFO, TAG, "Device Handler: Received unsupported request from client %d",
                        entityHandlerRequest->method);
        ehResult = OC_EH_ERROR;
    }

    if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
    {
        // Format the response.  Note this requires some info about the request
        response.requestHandle = entityHandlerRequest->requestHandle;
        response.resourceHandle = entityHandlerRequest->resource;
        response.ehResult = ehResult;
        response.payload = nullptr;
        response.numSendVendorSpecificHeaderOptions = 0;
        memset(response.sendVendorSpecificHeaderOptions,
                0, sizeof response.sendVendorSpecificHeaderOptions);
        // Indicate that response is NOT in a persistent buffer
        response.persistentBufferFlag = 0;

        // Send the response
        if (OCDoResponse(&response) != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "Error sending response");
            ehResult = OC_EH_ERROR;
        }
    }
    return ehResult;
}

OCEntityHandlerResult OCEntityHandlerRoomCb(OCEntityHandlerFlag flag,
                                            OCEntityHandlerRequest * ehRequest,
                                            void* /*callback*/)
{
    OCEntityHandlerResult ret = OC_EH_OK;
    OCEntityHandlerResponse response;

    OC_LOG_V(INFO, TAG, "Callback for Room");
    PrintReceivedMsgInfo(flag, ehRequest );

    if(ehRequest && flag == OC_REQUEST_FLAG )
    {
        std::string query = (const char*)ehRequest->query;
        OCRepPayload* payload = OCRepPayloadCreate();

        if(OC_REST_GET == ehRequest->method)
        {
            if(query.find(OC_RSRVD_INTERFACE_DEFAULT) != std::string::npos)
            {
                OCRepPayloadSetUri(payload, gRoomResourceUri);
                OCRepPayloadSetPropString(payload, "name", "John's Room");

                OCRepPayload *tempPayload = OCRepPayloadCreate();
                OCRepPayloadSetUri(tempPayload, gLightResourceUri);
                OCRepPayloadAppend(payload, tempPayload);

                OCRepPayload *tempPayload2 = OCRepPayloadCreate();
                OCRepPayloadSetUri(tempPayload2, gFanResourceUri);
                OCRepPayloadAppend(payload, tempPayload2);
            }
            else if(query.find(OC_RSRVD_INTERFACE_LL) != std::string::npos)
            {
                OCRepPayloadSetUri(payload, gRoomResourceUri);

                OCRepPayload *tempPayload = OCRepPayloadCreate();
                OCRepPayloadSetUri(tempPayload, gLightResourceUri);
                OCRepPayloadAppend(payload, tempPayload);

                OCRepPayload *tempPayload2 = OCRepPayloadCreate();
                OCRepPayloadSetUri(tempPayload2, gFanResourceUri);
                OCRepPayloadAppend(payload, tempPayload2);
            }
            else if(query.find(OC_RSRVD_INTERFACE_BATCH) != std::string::npos)
            {

                OCRepPayloadSetUri(payload, gRoomResourceUri);

                OCRepPayload *tempPayload = OCRepPayloadCreate();
                OCRepPayloadSetUri(tempPayload, gLightResourceUri);
                OCRepPayloadSetPropBool(tempPayload, "state", false);
                OCRepPayloadSetPropInt(tempPayload, "power", 0);
                OCRepPayloadAppend(payload, tempPayload);

                OCRepPayload *tempPayload2 = OCRepPayloadCreate();
                OCRepPayloadSetUri(tempPayload2, gFanResourceUri);
                OCRepPayloadSetPropBool(tempPayload2, "state", true);
                OCRepPayloadSetPropInt(tempPayload2, "speed", 10);
                OCRepPayloadAppend(payload, tempPayload2);
            }
            if (ret == OC_EH_OK)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = ehRequest->requestHandle;
                response.resourceHandle = ehRequest->resource;
                response.ehResult = ret;
                response.payload = reinterpret_cast<OCPayload*>(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions,
                        0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof response.resourceUri);
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;
                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ret = OC_EH_ERROR;
                }
            }
        }
        else if(OC_REST_PUT == ehRequest->method)
        {
            if(query.find(OC_RSRVD_INTERFACE_DEFAULT) != std::string::npos)
            {
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayloadSetUri(payload, gRoomResourceUri);
                    OCRepPayloadSetPropString(payload, "name", "John's Room");
                }
            }
            if(query.find(OC_RSRVD_INTERFACE_LL) != std::string::npos)
            {
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayloadSetUri(payload, gRoomResourceUri);
                }
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayload *tempPayload = OCRepPayloadCreate();
                    OCRepPayloadSetUri(tempPayload, gLightResourceUri);
                    OCRepPayloadAppend(payload, tempPayload);
                }
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayload *tempPayload = OCRepPayloadCreate();
                    OCRepPayloadSetUri(tempPayload, gFanResourceUri);
                    OCRepPayloadAppend(payload, tempPayload);
                }
            }
            if(query.find(OC_RSRVD_INTERFACE_BATCH ) != std::string::npos)
            {
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayloadSetUri(payload, gRoomResourceUri);
                }
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayload *tempPayload = OCRepPayloadCreate();
                    OCRepPayloadSetUri(tempPayload, gLightResourceUri);
                    OCRepPayloadSetPropBool(tempPayload, "state", true);
                    OCRepPayloadSetPropInt(tempPayload, "power", 0);
                    OCRepPayloadAppend(payload, tempPayload);
                }
                if(ret != OC_EH_ERROR)
                {
                    OCRepPayload *tempPayload = OCRepPayloadCreate();
                    OCRepPayloadSetUri(tempPayload, gFanResourceUri);
                    OCRepPayloadSetPropBool(tempPayload, "state", false);
                    OCRepPayloadSetPropInt(tempPayload, "speed", 0);
                    OCRepPayloadAppend(payload, tempPayload);
                }
            }
            if (ret == OC_EH_OK)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = ehRequest->requestHandle;
                response.resourceHandle = ehRequest->resource;
                response.ehResult = ret;
                response.payload = reinterpret_cast<OCPayload*>(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions,
                        0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof response.resourceUri);
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;
                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ret = OC_EH_ERROR;
                }
            }
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    ehRequest->method);
            OCRepPayloadDestroy(payload);
            ret = OC_EH_ERROR;
        }
    }
    else if (ehRequest && flag == OC_OBSERVE_FLAG)
    {
        gLightUnderObservation = 1;
    }
    return ret;
}

OCEntityHandlerResult OCEntityHandlerLightCb(OCEntityHandlerFlag flag,
        OCEntityHandlerRequest * ehRequest,void* /*callbackParam*/)
{
    OCEntityHandlerResult ret = OC_EH_OK;
    OCEntityHandlerResponse response;

    OC_LOG_V(INFO, TAG, "Callback for Light");
    PrintReceivedMsgInfo(flag, ehRequest );

    if(ehRequest && flag == OC_REQUEST_FLAG)
    {
        OCRepPayload* payload = OCRepPayloadCreate();
        if(OC_REST_GET == ehRequest->method)
        {
            OCRepPayloadSetUri(payload, gLightResourceUri);
            OCRepPayloadSetPropBool(payload, "state", false);
            OCRepPayloadSetPropInt(payload, "power", 0);
        }
        else if(OC_REST_PUT == ehRequest->method)
        {
            OCRepPayloadSetUri(payload, gLightResourceUri);
            OCRepPayloadSetPropBool(payload, "state", true);
            OCRepPayloadSetPropInt(payload, "power", 0);
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    ehRequest->method);
            ret = OC_EH_ERROR;
        }

        if (ret == OC_EH_OK)
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = ehRequest->requestHandle;
            response.resourceHandle = ehRequest->resource;
            response.ehResult = ret;
            response.payload = reinterpret_cast<OCPayload*>(payload);
            response.numSendVendorSpecificHeaderOptions = 0;
            memset(response.sendVendorSpecificHeaderOptions,
                    0, sizeof response.sendVendorSpecificHeaderOptions);
            memset(response.resourceUri, 0, sizeof response.resourceUri);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                OC_LOG(ERROR, TAG, "Error sending response");
                ret = OC_EH_ERROR;
            }
        }
        else
        {
            OCRepPayloadDestroy(payload);
        }
    }
    else if (ehRequest && flag == OC_OBSERVE_FLAG)
    {
        gLightUnderObservation = 1;
    }

    return ret;
}

OCEntityHandlerResult OCEntityHandlerFanCb(OCEntityHandlerFlag flag,
        OCEntityHandlerRequest * ehRequest, void* /*callback*/)
{
    OCEntityHandlerResult ret = OC_EH_OK;
    OCEntityHandlerResponse response;

    OC_LOG_V(INFO, TAG, "Callback for Fan");
    PrintReceivedMsgInfo(flag, ehRequest );

    if(ehRequest && flag == OC_REQUEST_FLAG)
    {
        OCRepPayload* payload = OCRepPayloadCreate();

        if(OC_REST_GET == ehRequest->method)
        {
            OCRepPayloadSetUri(payload, gFanResourceUri);
            OCRepPayloadSetPropBool(payload, "state", true);
            OCRepPayloadSetPropInt(payload, "speed", 10);
        }
        else if(OC_REST_PUT == ehRequest->method)
        {
            OCRepPayloadSetUri(payload, gFanResourceUri);
            OCRepPayloadSetPropBool(payload, "state", false);
            OCRepPayloadSetPropInt(payload, "speed", 0);
        }
        else
        {
            OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                    ehRequest->method);
            ret = OC_EH_ERROR;
        }

        if (ret == OC_EH_OK)
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = ehRequest->requestHandle;
            response.resourceHandle = ehRequest->resource;
            response.ehResult = ret;
            response.payload = reinterpret_cast<OCPayload*>(payload);
            response.numSendVendorSpecificHeaderOptions = 0;
            memset(response.sendVendorSpecificHeaderOptions,
                    0, sizeof response.sendVendorSpecificHeaderOptions);
            memset(response.resourceUri, 0, sizeof response.resourceUri);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                OC_LOG(ERROR, TAG, "Error sending response");
                ret = OC_EH_ERROR;
            }
        }
        OCRepPayloadDestroy(payload);

    }
    else if (ehRequest && flag == OC_OBSERVE_FLAG)
    {
        gLightUnderObservation = 1;
    }

    return ret;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

void *ChangeLightRepresentation (void *param)
{
    (void)param;
    OCStackResult result = OC_STACK_ERROR;

    while (!gQuitFlag)
    {
        sleep(10);
        light.power += 5;
        if (gLightUnderObservation)
        {
            OC_LOG_V(INFO, TAG,
                " =====> Notifying stack of new power level %d\n", light.power);
            result = OCNotifyAllObservers (light.handle, OC_NA_QOS);
            if (OC_STACK_NO_OBSERVERS == result)
            {
                gLightUnderObservation = 0;
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t threadId;
    int opt;

    while ((opt = getopt(argc, argv, "t:")) != -1)
    {
        switch(opt)
        {
        case 't':
            TEST = atoi(optarg);
            break;
        default:
            PrintUsage();
            return -1;
        }
    }
    if(TEST <= TEST_INVALID || TEST >= MAX_TESTS)
    {
        PrintUsage();
        return -1;
    }

    OC_LOG(DEBUG, TAG, "OCServer is starting...");

    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    OCSetDefaultDeviceEntityHandler(OCDeviceEntityHandlerCb, NULL);

    /*
     * Declare and create the example resource: light
     */
    createResources();

    /*
     * Create a thread for changing the representation of the light
     */
    pthread_create (&threadId, NULL, ChangeLightRepresentation, (void *)NULL);

    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocserver main loop...");
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

    /*
     * Cancel the light thread and wait for it to terminate
     */
    pthread_cancel(threadId);
    pthread_join(threadId, NULL);

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}

void createResources()
{
    light.state = false;

    OCResourceHandle fan;
    OCStackResult res = OCCreateResource(&fan,
            "core.fan",
            OC_RSRVD_INTERFACE_DEFAULT,
            "/a/fan",
            OCEntityHandlerFanCb,
            NULL,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created fan resource with result: %s", getResult(res));

    OCResourceHandle light;
    res = OCCreateResource(&light,
            "core.light",
            OC_RSRVD_INTERFACE_DEFAULT,
            "/a/light",
            OCEntityHandlerLightCb,
            NULL,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    OC_LOG_V(INFO, TAG, "Created light resource with result: %s", getResult(res));

    OCResourceHandle room;

    if(TEST == TEST_APP_COLL_EH)
    {
        res = OCCreateResource(&room,
                "core.room",
                OC_RSRVD_INTERFACE_BATCH,
                "/a/room",
                OCEntityHandlerRoomCb,
                NULL,
                OC_DISCOVERABLE);
    }
    else
    {
        res = OCCreateResource(&room,
                "core.room",
                OC_RSRVD_INTERFACE_BATCH,
                "/a/room",
                NULL,
                NULL,
                OC_DISCOVERABLE);
    }

    OC_LOG_V(INFO, TAG, "Created room resource with result: %s", getResult(res));
    OCBindResourceInterfaceToResource(room, OC_RSRVD_INTERFACE_LL);
    OCBindResourceInterfaceToResource(room, OC_RSRVD_INTERFACE_DEFAULT);

    res = OCBindResource(room, light);
    OC_LOG_V(INFO, TAG, "OC Bind Contained Resource to resource: %s", getResult(res));

    res = OCBindResource(room, fan);
    OC_LOG_V(INFO, TAG, "OC Bind Contained Resource to resource: %s", getResult(res));
}

