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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <pthread.h>
#include "ocstack.h"
#include "logger.h"
#include "cJSON.h"
#include "global.h"
#include "cainterface.h"
#include "cacommon.h"
#include "ocpayload.h"

#define TAG "DEMO"

volatile sig_atomic_t gQuitFlag = 0;
OCPersistentStorage ps = {0, 0, 0, 0, 0};
const char *gResourceUri = (char *)"/a/light";

//Secure Virtual Resource database for Iotivity Server
//It contains Server's Identity and the PSK credentials
//of other devices which the server trusts
static char CRED_FILE[] = "oic_svr_db_light.json";


//Structure to represent a light resource  and its attributes
typedef struct LIGHTRESOURCE
{
    OCResourceHandle handle;
    //Attributes
    int brightness;   // 0-100
} LightResource;

// Structure to represent a light resource  and its attributes
static LightResource Light;

OCRepPayload* getPayload(const char* uri, int64_t brightness)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if(!payload)
    {
        OC_LOG(ERROR, TAG, PCF("Failed to allocate Payload"));
        return nullptr;
    }

    OCRepPayloadSetUri(payload, uri);
    OCRepPayloadSetPropInt(payload, "brightness", brightness);

    return payload;
}

//This function takes the request as an input and returns the response
OCRepPayload* constructResponse(OCEntityHandlerRequest *ehRequest)
{
    if(!ehRequest)
    {
        return nullptr;
    }

    if(ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        OC_LOG(ERROR, TAG, PCF("Incoming payload not a representation"));
        return nullptr;
    }

    return getPayload(gResourceUri, Light.brightness);
}

OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest,
        OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    OCRepPayload *getResp = constructResponse(ehRequest);

    if(getResp && payload)
    {
        *payload = getResp;
        ehResult = OC_EH_OK;
    }

    return ehResult;
}


OCEntityHandlerResult OCEntityHandlerCb (OCEntityHandlerFlag flag,
                                         OCEntityHandlerRequest *entityHandlerRequest,
                                         void* /*callbackParam*/)
{
    OC_LOG_V (INFO, TAG, "Inside entity handler - flags: 0x%x", flag);

    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCEntityHandlerResponse response;

    // Validate pointer
    if (!entityHandlerRequest)
    {
        OC_LOG (ERROR, TAG, "Invalid request pointer");
        return OC_EH_ERROR;
    }

    OCRepPayload* payload = nullptr;

    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        if (entityHandlerRequest)
        {
            switch(entityHandlerRequest->method)
            {
            case OC_REST_GET:
            {
                OC_LOG (INFO, TAG, "Received OC_REST_GET from client");
                ehResult = ProcessGetRequest (entityHandlerRequest, &payload);
            }
            break;
            default:
            {
                OC_LOG_V (INFO, TAG, "Received unsupported method %d from client",
                        entityHandlerRequest->method);
                ehResult = OC_EH_ERROR;
            }
            break;
            }

            if (ehResult == OC_EH_OK && ehResult != OC_EH_FORBIDDEN)
            {
                // Format the response.  Note this requires some info about the request
                response.requestHandle = entityHandlerRequest->requestHandle;
                response.resourceHandle = entityHandlerRequest->resource;
                response.ehResult = ehResult;
                response.payload = reinterpret_cast<OCPayload*>(payload);
                response.numSendVendorSpecificHeaderOptions = 0;
                memset(response.sendVendorSpecificHeaderOptions, 0, sizeof response.sendVendorSpecificHeaderOptions);
                memset(response.resourceUri, 0, sizeof(response.resourceUri));
                // Indicate that response is NOT in a persistent buffer
                response.persistentBufferFlag = 0;

                // Send the response
                if (OCDoResponse(&response) != OC_STACK_OK)
                {
                    OC_LOG(ERROR, TAG, "Error sending response");
                    ehResult = OC_EH_ERROR;
                }
            }
        }
    }

    OCPayloadDestroy(response.payload);
    return ehResult;
}

/* SIGINT handler: set gQuitFlag to 1 for graceful termination */
void handleSigInt(int signum)
{
    if (signum == SIGINT)
    {
        gQuitFlag = 1;
    }
}

FILE* server_fopen(const char * /*path*/, const char *mode)
{
    return fopen(CRED_FILE, mode);
}

void SetPersistentHandler(OCPersistentStorage *ps)

{

    if (ps)

    {

        ps->open =  server_fopen;

        ps->read = fread;

        ps->write = fwrite;

        ps->close = fclose;

        ps->unlink = unlink;


        OCRegisterPersistentStorageHandler(ps);

    }

}
/**
 * GetResult is returned result to string.
 * @param   result             [IN] stack result
 * @return  converted OCStackResult as string for debugging
 */
static const char *getResult(OCStackResult result)
{
    switch (result)
    {
        case OC_STACK_OK:
            return "OC_STACK_OK";
        case OC_STACK_RESOURCE_CREATED:
            return "OC_STACK_RESOURCE_CREATED";
        case OC_STACK_RESOURCE_DELETED:
            return "OC_STACK_RESOURCE_DELETED";
        case OC_STACK_INVALID_URI:
            return "OC_STACK_INVALID_URI";
        case OC_STACK_INVALID_QUERY:
            return "OC_STACK_INVALID_QUERY";
        case OC_STACK_INVALID_IP:
            return "OC_STACK_INVALID_IP";
        case OC_STACK_INVALID_PORT:
            return "OC_STACK_INVALID_PORT";
        case OC_STACK_INVALID_CALLBACK:
            return "OC_STACK_INVALID_CALLBACK";
        case OC_STACK_INVALID_METHOD:
            return "OC_STACK_INVALID_METHOD";
        case OC_STACK_NO_MEMORY:
            return "OC_STACK_NO_MEMORY";
        case OC_STACK_COMM_ERROR:
            return "OC_STACK_COMM_ERROR";
        case OC_STACK_INVALID_PARAM:
            return "OC_STACK_INVALID_PARAM";
        case OC_STACK_NOTIMPL:
            return "OC_STACK_NOTIMPL";
        case OC_STACK_NO_RESOURCE:
            return "OC_STACK_NO_RESOURCE";
        case OC_STACK_RESOURCE_ERROR:
            return "OC_STACK_RESOURCE_ERROR";
        case OC_STACK_SLOW_RESOURCE:
            return "OC_STACK_SLOW_RESOURCE";
        case OC_STACK_NO_OBSERVERS:
            return "OC_STACK_NO_OBSERVERS";
        case OC_STACK_ERROR:
            return "OC_STACK_ERROR";
        default:
            return "UNKNOWN";
    }
}

/**
 * CreateLightResource creates a new light resource by calling the OCCreateResource() method.
 * @param   uri                    [IN] uri
 * @param   lightResource          [IN] info of resource
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
int createLightResource (const char *uri, LightResource *lightResource)
{
    if (!uri)
    {
        OC_LOG(ERROR, TAG, "Resource URI cannot be NULL");

    }

    lightResource->brightness = 0;
    OCStackResult res = OCCreateResource(&(lightResource->handle),
                                         "core.light",
                                         OC_RSRVD_INTERFACE_DEFAULT,
                                         uri,
                                         OCEntityHandlerCb,
                                         NULL,
                                         OC_DISCOVERABLE|OC_OBSERVABLE | OC_SECURE);

    OC_LOG_V(INFO, TAG, "Created Light resource with result: %s", getResult(res));
    return 0;
}


int main()
{
    OC_LOG(DEBUG, TAG, "OCServer is starting...");
    SetPersistentHandler(&ps);
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: Light
     */
    createLightResource(gResourceUri, &Light);

    CASelectCipherSuite(TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8);

    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L;

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

        nanosleep(&timeout, NULL);
    }

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}
