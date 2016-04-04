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
#include <pthread.h>
#include <iostream>
#include <sstream>
#include "ocstack.h"
#include "logger.h"
#include "cJSON.h"
#include "global.h"
#include "cainterface.h"
#include "cacommon.h"
#include "payload_logging.h"
#include "ocpayload.h"


#define TAG "DEMO"
#define DEFAULT_CONTEXT_VALUE 0x99
#define STATE "state"
#define OPEN_DURATION "openDuration"
#define OPEN_ALARM "openAlarm"

static const char MULTICAST_DISCOVERY_QUERY[] = "/oic/res";

volatile sig_atomic_t gQuitFlag = 0;
OCPersistentStorage ps = {0, 0, 0, 0, 0};
static const char *gResourceUri = "/a/door";
uint8_t lightIpAddr[4] = {};
uint16_t lightPortNu;
static bool isUpdated = false;
static std::string coapServerIP;
static std::string coapServerPort;
static std::string coapServerResource;
static OCConnectivityType ocConnType;

static std::string address;

static int coapSecureResource;

static const char CRED_FILE[] = "oic_svr_db_door.json";

CAEndpoint_t endpoint = {CA_DEFAULT_ADAPTER, CA_DEFAULT_FLAGS, 0, {0}, 0};

// myDoorState_t variable to store resource's state .
typedef enum
{
    STATE_OPEN,    /**< State is opened */
    STATE_CLOSED        /**< State is closed*/
} myDoorState_t;

//Structure to represent a door resource  and its attributes
typedef struct DOORRESOURCE
{
    OCResourceHandle handle;
    myDoorState_t state; //ReadOnly, The state of the door (open or closed)"
    char *openDuration;  //ReadOnly, The time duration the door has been open
    bool openAlarm ; //The state of the door open alarm

} DoorResource;

static DoorResource Door;

int parseClientResponse(OCClientResponse * clientResponse)
{
    if(!clientResponse)
    {
        return 0;
    }

    OCResourcePayload* res = ((OCDiscoveryPayload*)clientResponse->payload)->resources;

    // Initialize all global variables
    coapServerResource.clear();
    coapSecureResource = 0;

    while (res)
    {
        coapServerResource.assign(res->uri);
        OC_LOG_V(INFO, TAG, "Uri -- %s", coapServerResource.c_str());

        if (res->secure)
        {
            endpoint.port = res->port;
            coapSecureResource = 1;
        }

        OC_LOG_V(INFO, TAG, "Secure -- %s", coapSecureResource == 1 ? "YES" : "NO");

        // If we discovered a secure resource, exit from here
        if (coapSecureResource)
        {
            break;
        }

        res = res->next;
    }

    return 0;
}

OCRepPayload* getPayload(const char* uri, int64_t state, char* openDuration, bool openAlarm)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if (!payload)
    {
        OC_LOG(ERROR, TAG, PCF("Failed to allocate Payload"));
        return nullptr;
    }

    OCRepPayloadSetUri(payload, uri);
    OCRepPayloadSetPropInt(payload, STATE, state);
    OCRepPayloadSetPropString(payload, OPEN_DURATION, openDuration);
    OCRepPayloadSetPropBool(payload, OPEN_ALARM, openAlarm);

    return payload;
}

//This function takes the request as an input and returns the response
OCRepPayload* constructResponse (OCEntityHandlerRequest *ehRequest)
{
    if(ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        OC_LOG(ERROR, TAG, PCF("Incoming payload not a representation"));
        return nullptr;
    }

    DoorResource *currdoorResource = &Door;

    return getPayload(gResourceUri, currdoorResource->state, currdoorResource->openDuration, currdoorResource->openAlarm);
}

OCEntityHandlerResult ProcessGetRequest(OCEntityHandlerRequest *ehRequest,
        OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;

    OCRepPayload *getResp = constructResponse(ehRequest);

    if(getResp)
    {
        *payload = getResp;
        ehResult = OC_EH_OK;
    }

    return ehResult;
}

OCEntityHandlerResult OCEntityHandlerCb(OCEntityHandlerFlag flag,
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

FILE *server_fopen(const char * /*path*/, const char *mode)
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
    case OC_STACK_UNAUTHORIZED_REQ:
        return "OC_STACK_UNAUTHORIZED_REQ";
    #ifdef WITH_PRESENCE
    case OC_STACK_PRESENCE_STOPPED:
        return "OC_STACK_PRESENCE_STOPPED";
    #endif
        case OC_STACK_ERROR:
            return "OC_STACK_ERROR";
        default:
            return "UNKNOWN";
    }
}


/**
 * CreateDoorResource creates a new door resource by calling the OCCreateResource() method.
 * @param   uri                   [IN] uri
 * @param   doorResource          [IN] info of resource
 * @return ::OC_STACK_OK on success, some other value upon failure.
 */
int  createDoorResource (const char *uri, DoorResource *doorResource)
{
    if (!uri)
    {
        OC_LOG(ERROR, TAG, "Resource URI cannot be NULL");

    }

    doorResource->state = STATE_CLOSED; //1:closed , 0: open
    char str[] = "10min";
    doorResource->openDuration = str;
    doorResource->openAlarm = false;
    OCStackResult res = OCCreateResource(&(doorResource->handle),
                                         "core.door",
                                         OC_RSRVD_INTERFACE_DEFAULT,
                                         uri,
                                         OCEntityHandlerCb,
                                         NULL,
                                         OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE);

    OC_LOG_V(INFO, TAG, "Created Door resource with result: %s", getResult(res));
    return 0;
}

OCStackApplicationResult putReqCB(void * ctx, OCDoHandle /*handle*/, OCClientResponse *clientResponse)
{
    if (ctx == (void *)DEFAULT_CONTEXT_VALUE)
    {
        OC_LOG(INFO, TAG, "Callback Context for PUT recvd successfully");
    }

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        if ((OCSecurityPayload*)clientResponse->payload)
        {
            OC_LOG_V(INFO, TAG, "=============> Put Response",
                    ((OCSecurityPayload*)clientResponse->payload)->securityData);
        }
    }
    return OC_STACK_DELETE_TRANSACTION;
}

OCStackApplicationResult getReqCB(void * /*ctx*/, OCDoHandle /*handle*/, OCClientResponse *clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for GET query recvd successfully");

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s",  getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG, "SEQUENCE NUMBER: %d", clientResponse->sequenceNumber);
        OC_LOG_PAYLOAD(INFO, clientResponse->payload);
        if ((OCSecurityPayload*)clientResponse->payload)
        {
            OC_LOG(INFO, TAG, PCF("=============> Get Response"));
        }
    }
    return OC_STACK_DELETE_TRANSACTION;
}

// This is a function called back when a device is discovered
OCStackApplicationResult discoveryReqCB(void* /*ctx*/, OCDoHandle /*handle*/,
        OCClientResponse * clientResponse)
{
    OC_LOG(INFO, TAG, "Callback Context for DISCOVER query recvd successfully");

    if (clientResponse)
    {
        OC_LOG_V(INFO, TAG, "StackResult: %s", getResult(clientResponse->result));
        OC_LOG_V(INFO, TAG,
                "Device =============> Discovered @ %s:%d",
                clientResponse->devAddr.addr,
                clientResponse->devAddr.port);

        if (clientResponse->result == OC_STACK_OK)
        {
            OC_LOG_PAYLOAD(INFO, clientResponse->payload);
            ocConnType = clientResponse->connType;
            parseClientResponse(clientResponse);
        }
    }

    return OC_STACK_KEEP_TRANSACTION;

}



void initAddress()
{
    static bool initFlag = false;
    if (!initFlag)
    {
        OC_LOG(INFO, TAG, "Enter IP address (with optional port) of the Server hosting resource\n");
        OC_LOG(INFO, TAG, "IPv4: 192.168.0.15:45454\n");
        OC_LOG(INFO, TAG, "IPv6: [fe80::20c:29ff:fe1b:9c5]:45454\n");

        std::cin >> address;
    }
    initFlag = true;
}

// Local function to send get request of light resource
void SendGetRequest()
{
    OCStackResult ret;
    OC_LOG(INFO, TAG, "Send Get REQ to Light server");

    initAddress();

    char szQueryUri[64] = { '\0'};
    OCDoHandle handle;
    OCCallbackData cbData;
    cbData.cb = getReqCB;
    cbData.context = (void *)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;
    OC_LOG_V(INFO, TAG, "Get payload from Door sample = /a/light ");
    snprintf(szQueryUri,  sizeof(szQueryUri), "coaps://%s/a/light", const_cast<char*> (address.c_str())); // lightPortNu);
    ret = OCDoResource(&handle, OC_REST_GET, szQueryUri, 0, NULL, ocConnType, OC_LOW_QOS,
                 &cbData, NULL, 0);
    if (ret != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack resource error");
    }
}


void *input_function(void * /*data*/)
{
    char input;
    char szQueryUri[64] = { 0 };
    OCDoHandle handle;
    OCCallbackData cbData;
    cbData.cb = discoveryReqCB;
    cbData.context = (void *)DEFAULT_CONTEXT_VALUE;
    cbData.cd = NULL;

    strcpy(szQueryUri, MULTICAST_DISCOVERY_QUERY);

    while (1)
    {
        std::cin >> input;
        switch (input)
        {
            case 'D':
            case 'd':
                if (isUpdated == false)
                {
                    OC_LOG(INFO, TAG, "isUpdated is false...");
                    OCDoResource(&handle, OC_REST_DISCOVER, szQueryUri, 0, 0, CT_DEFAULT, OC_LOW_QOS, &cbData, NULL, 0);

                }
                break;
            case 'G':
            case 'g':
                isUpdated = true;
                if (isUpdated == true)
                {
                    OC_LOG(INFO, TAG, "isUpdated is true...");
                    SendGetRequest();
                }
                break;
            case 'Q':
            case 'q':
                gQuitFlag = 1;
                   return 0;
            default: break;
        }
    }
    return 0;
}

static void PrintUsage()
{
    OC_LOG(INFO, TAG, "*******************************************");
    OC_LOG(INFO, TAG, "Input D or d to discover Resources");
    OC_LOG(INFO, TAG, "Input G or g to initiate Get Request");
    OC_LOG(INFO, TAG, "Input Q or q to exit");
    OC_LOG(INFO, TAG, "*******************************************");
}

int main()
{

    OC_LOG(INFO, TAG, "OCServer is starting...");
    SetPersistentHandler(&ps);
    //PrintUsage();
    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack init error");
        return 0;
    }

    /*
     * Declare and create the example resource: Door
     */
    createDoorResource(gResourceUri, &Door);
    PrintUsage();

    //select ciphersuite for certificates
    CASelectCipherSuite(TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8);

    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L;

    // Break from loop with Ctrl-C
    OC_LOG(INFO, TAG, "Entering ocserver main loop...");
    signal(SIGINT, handleSigInt);
    int thr_id;
    pthread_t p_thread;
    thr_id = pthread_create(&p_thread, NULL, input_function, (void *)NULL);
    if (thr_id < 0)
    {
        OC_LOG(ERROR, TAG, "create thread error");
        return 0;
    }

    while (!gQuitFlag)
    {

        if (OCProcess() != OC_STACK_OK)
        {
            OC_LOG(ERROR, TAG, "OCStack process error");
            return 0;
        }


        nanosleep(&timeout, NULL);
    }

    pthread_join(p_thread, NULL);

    OC_LOG(INFO, TAG, "Exiting ocserver main loop...");

    if (OCStop() != OC_STACK_OK)
    {
        OC_LOG(ERROR, TAG, "OCStack process error");
    }

    return 0;
}
