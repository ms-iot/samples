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

#include <glib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <array>
#include "ocstack.h"
#include "logger.h"
#include "ocpayload.h"
#include "ocserver.h"
using namespace std;

//string length of "/a/light/" + std::numeric_limits<int>::digits10 + '\0'"
// 9 + 9 + 1 = 19
const int URI_MAXSIZE = 19;

static int gObserveNotifyType = 3;

int gQuitFlag = 0;
int gLightUnderObservation = 0;

static LightResource Light;
// This variable determines instance number of the Light resource.
// Used by POST method to create a new instance of Light resource.
static int gCurrLightInstance = 0;

static LightResource gLightInstance[SAMPLE_MAX_NUM_POST_INSTANCE];

Observers interestedObservers[SAMPLE_MAX_NUM_OBSERVATIONS];

#ifdef WITH_PRESENCE
static int stopPresenceCount = 10;
#define numPresenceResources (2)
#endif

char *gResourceUri= (char *)"/a/light";
const char *dateOfManufacture = "myDateOfManufacture";
const char *deviceName = "myDeviceName";
const char *deviceUUID = "myDeviceUUID";
const char *firmwareVersion = "myFirmwareVersion";
const char *manufacturerName = "myName";
const char *operatingSystemVersion = "myOS";
const char *hardwareVersion = "myHardwareVersion";
const char* platformID = "myPlatformID";
const char *manufacturerUrl = "myManufacturerUrl";
const char *modelNumber = "myModelNumber";
const char *platformVersion = "myPlatformVersion";
const char *supportUrl = "mySupportUrl";
const char *version = "myVersion";
const char *systemTime = "2015-05-15T11.04";

// Entity handler should check for resourceTypeName and ResourceInterface in order to GET
// the existence of a known resource
const char *resourceTypeName = "core.light";
const char *resourceInterface = OC_RSRVD_INTERFACE_DEFAULT;

OCPlatformInfo platformInfo;
OCDeviceInfo deviceInfo;

OCRepPayload* getPayload(const char* uri, int64_t power, bool state)
{
    OCRepPayload* payload = OCRepPayloadCreate();
    if(!payload)
    {
        cout << "Failed to allocate Payload";
        return nullptr;
    }

    OCRepPayloadSetUri(payload, uri);
    OCRepPayloadSetPropBool(payload, "state", state);
    OCRepPayloadSetPropInt(payload, "power", power);

    return payload;
}

//This function takes the request as an input and returns the response
OCRepPayload* constructResponse(OCEntityHandlerRequest *ehRequest)
{
    if(ehRequest->payload && ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        cout << "Incoming payload not a representation";
        return nullptr;
    }

    OCRepPayload* input = reinterpret_cast<OCRepPayload*>(ehRequest->payload);

    LightResource *currLightResource = &Light;

    if (ehRequest->resource == gLightInstance[0].handle)
    {
        currLightResource = &gLightInstance[0];
        gResourceUri = (char *) "a/light/0";
    }
    else if (ehRequest->resource == gLightInstance[1].handle)
    {
        currLightResource = &gLightInstance[1];
        gResourceUri = (char *) "a/light/1";
    }

    if(OC_REST_PUT == ehRequest->method)
    {
        // Get pointer to query
        int64_t pow;
        if(OCRepPayloadGetPropInt(input, "power", &pow))
        {
            currLightResource->power =pow;
        }

        bool state;
        if(OCRepPayloadGetPropBool(input, "state", &state))
        {
            currLightResource->state = state;
        }
    }

    return getPayload(gResourceUri, currLightResource->power, currLightResource->state);
}

/*
 * Very simple example of query parsing.
 * The query may have multiple filters separated by ';'.
 * It is upto the entity handler to parse the query for the individual filters,
 * VALIDATE them and respond as it sees fit.

 * This function only returns false if the query is exactly "power<X" and
 * current power is greater than X. If X cannot be parsed for an int,
 * true is returned.
 */
bool checkIfQueryForPowerPassed(char * query)
{
    if (query && strncmp(query, "power<", strlen("power<")) == 0)
    {
        char * pointerToOperator = strstr(query, "<");

        if (pointerToOperator)
        {
            int powerRequested = atoi(pointerToOperator + 1);
            if (Light.power > powerRequested)
            {
                cout << "\nCurrent power: "<< Light.power << "Requested: " << powerRequested;
                return false;
            }
        }
    }
    return true;
}

/*
 * Application should validate and process these as desired.
 */
OCEntityHandlerResult ValidateQueryParams (OCEntityHandlerRequest *entityHandlerRequest)
{
    cout << "\nReceived query %s" << entityHandlerRequest->query;
    cout << "\nNot processing query";
    return OC_EH_OK;
}

OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest,
        OCRepPayload **payload)
{
    OCEntityHandlerResult ehResult;
    bool queryPassed = checkIfQueryForPowerPassed(ehRequest->query);

    // Empty payload if the query has no match.
    if (queryPassed)
    {
        OCRepPayload *getResp = constructResponse(ehRequest);
        if(!getResp)
        {
            cout << "\nconstructResponse failed";
            return OC_EH_ERROR;
        }

        *payload = getResp;
        ehResult = OC_EH_OK;
    }
    else
    {
        ehResult = OC_EH_OK;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessPutRequest (OCEntityHandlerRequest *ehRequest,
        OCRepPayload** payload)
{
    OCEntityHandlerResult ehResult;
    OCRepPayload *putResp = constructResponse(ehRequest);

    if(!putResp)
    {
        cout << "\nFailed to construct response";
        return OC_EH_ERROR;
    }

    *payload = putResp;
    ehResult = OC_EH_OK;

    return ehResult;
}

OCEntityHandlerResult ProcessPostRequest (OCEntityHandlerRequest *ehRequest,
        OCEntityHandlerResponse *response, OCRepPayload** payload)
{
    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCRepPayload *respPLPost_light = nullptr;

    /*
     * The entity handler determines how to process a POST request.
     * Per the REST paradigm, POST can also be used to update representation of existing
     * resource or create a new resource.
     * In the sample below, if the POST is for /a/light then a new instance of the Light
     * resource is created with default representation (if representation is included in
     * POST payload it can be used as initial values) as long as the instance is
     * lesser than max new instance count. Once max instance count is reached, POST on
     * /a/light updated the representation of /a/light (just like PUT).
     */

    if (ehRequest->resource == Light.handle)
    {
        if (gCurrLightInstance < SAMPLE_MAX_NUM_POST_INSTANCE)
        {
            // Create new Light instance
            char newLightUri[URI_MAXSIZE];
            snprintf(newLightUri, URI_MAXSIZE, "/a/light/%d", gCurrLightInstance);

            respPLPost_light = OCRepPayloadCreate();
            OCRepPayloadSetUri(respPLPost_light, gResourceUri);
            OCRepPayloadSetPropString(respPLPost_light, "createduri", newLightUri);

            if (0 == createLightResource (newLightUri, &gLightInstance[gCurrLightInstance]))
            {
                cout << "\nCreated new Light instance";
                gLightInstance[gCurrLightInstance].state = 0;
                gLightInstance[gCurrLightInstance].power = 0;
                gCurrLightInstance++;
                strncpy ((char *)response->resourceUri, newLightUri, MAX_URI_LENGTH);
                ehResult = OC_EH_RESOURCE_CREATED;
            }
        }
        else
        {
            // Update repesentation of /a/light
            Light.state = true;
            Light.power = 11;
            respPLPost_light = constructResponse(ehRequest);
        }
    }
    else
    {
        for (int i = 0; i < SAMPLE_MAX_NUM_POST_INSTANCE; i++)
        {
            if (ehRequest->resource == gLightInstance[i].handle)
            {
                gLightInstance[i].state = true;
                gLightInstance[i].power = 22;
                if (i == 0)
                {
                    respPLPost_light = constructResponse(ehRequest);
                    break;
                }
                else if (i == 1)
                {
                    respPLPost_light = constructResponse(ehRequest);
                }
            }
        }
    }

    if ((respPLPost_light != NULL))
    {
        *payload = respPLPost_light;
    }
    else
    {
        cout << "\n Payload was NULL";
        ehResult = OC_EH_ERROR;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessDeleteRequest (OCEntityHandlerRequest *ehRequest)
{
    if(ehRequest == NULL)
    {
        cout << "\nThe ehRequest is NULL";
        return OC_EH_ERROR;
    }
    OCEntityHandlerResult ehResult = OC_EH_OK;

    cout << "\nExecuting " << __func__ << " for resource " << ehRequest->resource;

    /*
     * In the sample below, the application will:
     * 1a. pass the delete request to the c stack
     * 1b. internally, the c stack figures out what needs to be done and does it accordingly
     *    (e.g. send observers notification, remove observers...)
     * 1c. the c stack returns with the result whether the request is fullfilled.
     * 2. optionally, app removes observers out of its array 'interestedObservers'
     */

    if ((ehRequest != NULL) && (ehRequest->resource == Light.handle))
    {
        //Step 1: Ask stack to do the work.
        OCStackResult result = OCDeleteResource(ehRequest->resource);

        if (result == OC_STACK_OK)
        {
            cout << "\nDelete Resource operation succeeded.";
            ehResult = OC_EH_OK;

            //Step 2: clear observers who wanted to observe this resource at the app level.
            for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
            {
                if (interestedObservers[i].resourceHandle == ehRequest->resource)
                {
                    interestedObservers[i].valid = false;
                    interestedObservers[i].observationId = 0;
                    interestedObservers[i].resourceHandle = NULL;
                }
            }
        }
        else if (result == OC_STACK_NO_RESOURCE)
        {
            cout << "\nThe resource doesn't exist or it might have been deleted.";
            ehResult = OC_EH_RESOURCE_DELETED;
        }
        else
        {
            cout << "\nEncountered error from OCDeleteResource().";
            ehResult = OC_EH_ERROR;
        }
    }
    else if (ehRequest->resource != Light.handle)
    {
        //Let's this app not supporting DELETE on some resources so
        //consider the DELETE request is received for a non-support resource.
        cout << "\nThe request is received for a non-support resource.";
        ehResult = OC_EH_FORBIDDEN;
    }

    return ehResult;
}

OCEntityHandlerResult ProcessNonExistingResourceRequest(OCEntityHandlerRequest *ehRequest)
{
    cout << "\nExecuting " << __func__;

    return OC_EH_RESOURCE_NOT_FOUND;
}

void ProcessObserveRegister (OCEntityHandlerRequest *ehRequest)
{
    cout << "\nReceived observation registration request with observation Id "
         << ehRequest->obsInfo.obsId;
    for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
    {
        if (interestedObservers[i].valid == false)
        {
            interestedObservers[i].observationId = ehRequest->obsInfo.obsId;
            interestedObservers[i].valid = true;
            gLightUnderObservation = 1;
            break;
        }
    }
}

void ProcessObserveDeregister (OCEntityHandlerRequest *ehRequest)
{
    bool clientStillObserving = false;

    cout << "\nReceived observation deregistration request for observation Id "
         << ehRequest->obsInfo.obsId;
    for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
    {
        if (interestedObservers[i].observationId == ehRequest->obsInfo.obsId)
        {
            interestedObservers[i].valid = false;
        }
        if (interestedObservers[i].valid == true)
        {
            // Even if there is one single client observing we continue notifying entity handler
            clientStillObserving = true;
        }
    }
    if (clientStillObserving == false)
        gLightUnderObservation = 0;
}

OCEntityHandlerResult
OCDeviceEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, char* uri, void* callbackParam)
{
    cout << "\nInside device default entity handler - flags: " << flag << ", uri: %s" << uri;

    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCEntityHandlerResponse response;

    // Validate pointer
    if (!entityHandlerRequest)
    {
        cout << "\nInvalid request pointer";
        return OC_EH_ERROR;
    }
    // Initialize certain response fields
    response.numSendVendorSpecificHeaderOptions = 0;
    memset(response.sendVendorSpecificHeaderOptions, 0,
            sizeof response.sendVendorSpecificHeaderOptions);
    memset(response.resourceUri, 0, sizeof response.resourceUri);
    OCRepPayload* payload = nullptr;


    if (flag & OC_REQUEST_FLAG)
    {
        cout << "\nFlag includes OC_REQUEST_FLAG";

        if (entityHandlerRequest->resource == NULL)
        {
            cout << "\nReceived request from client to a non-existing resource";
            ehResult = ProcessNonExistingResourceRequest(entityHandlerRequest);
        }
        else if (OC_REST_GET == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_GET from client";
            ehResult = ProcessGetRequest (entityHandlerRequest, &payload);
        }
        else if (OC_REST_PUT == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_PUT from client";
            ehResult = ProcessPutRequest (entityHandlerRequest, &payload);
        }
        else if (OC_REST_DELETE == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_DELETE from client";
            ehResult = ProcessDeleteRequest (entityHandlerRequest);
        }
        else
        {
            cout << "\nReceived unsupported method " << entityHandlerRequest->method
                << " from client";
            ehResult = OC_EH_ERROR;
        }
               // If the result isn't an error or forbidden, send response
        if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = entityHandlerRequest->requestHandle;
            response.resourceHandle = entityHandlerRequest->resource;
            response.ehResult = ehResult;
            response.payload = reinterpret_cast<OCPayload*>(payload);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                cout << "\nError sending response";
                ehResult = OC_EH_ERROR;
            }
        }
    }
    if (flag & OC_OBSERVE_FLAG)
    {
        cout << "\nFlag includes OC_OBSERVE_FLAG";
        if (OC_OBSERVE_REGISTER == entityHandlerRequest->obsInfo.action)
        {
            cout << "\nReceived OC_OBSERVE_REGISTER from client";
        }
        else if (OC_OBSERVE_DEREGISTER == entityHandlerRequest->obsInfo.action)
        {
            cout << "\nReceived OC_OBSERVE_DEREGISTER from client";
        }
    }

    return ehResult;
}

OCEntityHandlerResult
OCNOPEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, void* callbackParam)
{
    // This is callback is associated with the 2 presence notification
    // resources. They are non-operational.
    return OC_EH_OK;
}

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, void* callback)
{
    cout << "\nInside entity handler - flags: " << flag;

    OCEntityHandlerResult ehResult = OC_EH_OK;
    OCEntityHandlerResponse response;

    // Validate pointer
    if (!entityHandlerRequest)
    {
        cout << "\nInvalid request pointer";
        return OC_EH_ERROR;
    }

    // Initialize certain response fields
    response.numSendVendorSpecificHeaderOptions = 0;
    memset(response.sendVendorSpecificHeaderOptions,
            0, sizeof response.sendVendorSpecificHeaderOptions);
    memset(response.resourceUri, 0, sizeof response.resourceUri);
    OCRepPayload* payload = nullptr;

    if (flag & OC_REQUEST_FLAG)
    {
        cout << "\n=================================\n";
        cout << "\nFlag includes OC_REQUEST_FLAG\n";

        if (OC_REST_GET == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_GET from client";
            ehResult = ProcessGetRequest (entityHandlerRequest, &payload);
            cout << "\n=================================\n";
        }
        else if (OC_REST_PUT == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_PUT from client";
            ehResult = ProcessPutRequest (entityHandlerRequest, &payload);
            cout << "\n=================================\n";
        }
        else if (OC_REST_POST == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_POST from client";
            ehResult = ProcessPostRequest (entityHandlerRequest, &response, &payload);
            cout << "\n=================================\n";
        }
        else if (OC_REST_DELETE == entityHandlerRequest->method)
        {
            cout << "\nReceived OC_REST_DELETE from client";
            ehResult = ProcessDeleteRequest (entityHandlerRequest);
            cout << "\n=================================\n";
        }
        else
        {
            cout << "\nReceived unsupported method " << entityHandlerRequest->method << " from client";
            ehResult = OC_EH_ERROR;
        }
        // If the result isn't an error or forbidden, send response
        if (!((ehResult == OC_EH_ERROR) || (ehResult == OC_EH_FORBIDDEN)))
        {
            // Format the response.  Note this requires some info about the request
            response.requestHandle = entityHandlerRequest->requestHandle;
            response.resourceHandle = entityHandlerRequest->resource;
            response.ehResult = ehResult;
            response.payload = reinterpret_cast<OCPayload*>(payload);
            // Indicate that response is NOT in a persistent buffer
            response.persistentBufferFlag = 0;

            // Handle vendor specific options
            if(entityHandlerRequest->rcvdVendorSpecificHeaderOptions &&
                    entityHandlerRequest->numRcvdVendorSpecificHeaderOptions)
            {
                cout << "\nReceived vendor specific options";
                uint8_t i = 0;
                OCHeaderOption * rcvdOptions =
                        entityHandlerRequest->rcvdVendorSpecificHeaderOptions;
                for( i = 0; i < entityHandlerRequest->numRcvdVendorSpecificHeaderOptions; i++)
                {
                    if(((OCHeaderOption)rcvdOptions[i]).protocolID == OC_COAP_ID)
                    {
                        cout << "\nReceived option with ID " << ((OCHeaderOption)rcvdOptions[i]).optionID;
                    }
                }
                OCHeaderOption * sendOptions = response.sendVendorSpecificHeaderOptions;
                uint8_t option2[] = {21,22,23,24,25,26,27,28,29,30};
                uint8_t option3[] = {31,32,33,34,35,36,37,38,39,40};
                sendOptions[0].protocolID = OC_COAP_ID;
                sendOptions[0].optionID = 2248;
                memcpy(sendOptions[0].optionData, option2, sizeof(option2));
                sendOptions[0].optionLength = 10;
                sendOptions[1].protocolID = OC_COAP_ID;
                sendOptions[1].optionID = 2600;
                memcpy(sendOptions[1].optionData, option3, sizeof(option3));
                sendOptions[1].optionLength = 10;
                response.numSendVendorSpecificHeaderOptions = 2;
            }

            // Send the response
            if (OCDoResponse(&response) != OC_STACK_OK)
            {
                cout << "\nError sending response";
                ehResult = OC_EH_ERROR;
            }
        }
    }
    if (flag & OC_OBSERVE_FLAG)
    {
        cout << "\nFlag includes OC_OBSERVE_FLAG";

        if (OC_OBSERVE_REGISTER == entityHandlerRequest->obsInfo.action)
        {
            cout << "\nReceived OC_OBSERVE_REGISTER from client";
            ProcessObserveRegister (entityHandlerRequest);
        }
        else if (OC_OBSERVE_DEREGISTER == entityHandlerRequest->obsInfo.action)
        {
            cout << "\nReceived OC_OBSERVE_DEREGISTER from client";
            ProcessObserveDeregister (entityHandlerRequest);
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

void *ChangeLightRepresentation (void *param)
{
    (void)param;
    OCStackResult result = OC_STACK_ERROR;

    uint8_t j = 0;
    uint8_t numNotifies = (SAMPLE_MAX_NUM_OBSERVATIONS)/2;
    OCObservationId obsNotify[numNotifies];

    while (!gQuitFlag)
    {
        sleep(3);
        Light.power += 5;
        if (gLightUnderObservation)
        {
            cout << "\n=====> Notifying stack of new power level" << Light.power;
            if (gObserveNotifyType == 1)
            {
                // Notify list of observers. Alternate observers on the list will be notified.
                j = 0;
                for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; (i=i+2))
                {
                    if (interestedObservers[i].valid == true)
                    {
                        obsNotify[j] = interestedObservers[i].observationId;
                        j++;
                    }
                }

                OCRepPayload* payload = getPayload(gResourceUri, Light.power, Light.state);
                result = OCNotifyListOfObservers (Light.handle, obsNotify, j,
                        payload, OC_NA_QOS);
                OCRepPayloadDestroy(payload);
            }
            else if (gObserveNotifyType == 0)
            {
                // Notifying all observers
                result = OCNotifyAllObservers (Light.handle, OC_NA_QOS);
                if (OC_STACK_NO_OBSERVERS == result)
                {
                    cout << "\n=====> No more observers exist, stop sending observations";
                    gLightUnderObservation = 0;
                }
            }
            else
            {
                cout << "\nIncorrect notification type selected";
            }
        }
#ifdef WITH_PRESENCE
        if(stopPresenceCount > 0)
        {
            cout << "\n=====> Counting down to stop presence " << stopPresenceCount;
        }
        if(!stopPresenceCount--)
        {
            cout << "\n=====> stopping presence";
            OCStopPresence();
        }
#endif
    }
    return NULL;
}

#ifdef WITH_PRESENCE
void *presenceNotificationGenerator(void *param)
{
    sleep(10);
    (void)param;
    OCDoHandle presenceNotificationHandles[numPresenceResources];
    OCStackResult res = OC_STACK_OK;

    std::array<std::string, numPresenceResources> presenceNotificationResources { {
        std::string("core.fan"),
        std::string("core.led") } };
    std::array<std::string, numPresenceResources> presenceNotificationUris { {
        std::string("/a/fan"),
        std::string("/a/led") } };

    for(int i=0; i<numPresenceResources; i++)
    {
        if(res == OC_STACK_OK)
        {
            sleep(1);
            res = OCCreateResource(&presenceNotificationHandles[i],
                    presenceNotificationResources.at(i).c_str(),
                    OC_RSRVD_INTERFACE_DEFAULT,
                    presenceNotificationUris.at(i).c_str(),
                    OCNOPEntityHandlerCb,
                    NULL,
                    OC_DISCOVERABLE|OC_OBSERVABLE);
        }
        if(res != OC_STACK_OK)
        {
            cout << "\nPresence Notification Generator failed[" << getResult(res)
                << "] to create resource " << presenceNotificationResources.at(i).c_str();
            break;
        }
        cout << "\nCreated " << presenceNotificationUris[i].c_str() << " for presence notification";
    }
    sleep(5);
    for(int i=0; i<numPresenceResources; i++)
    {
        if(res == OC_STACK_OK)
        {
            res = OCDeleteResource(presenceNotificationHandles[i]);
        }
        if(res != OC_STACK_OK)
        {
            cout << "\nPresence Notification Generator failed to delete resource"
                << presenceNotificationResources.at(i).c_str();
            break;
        }
        cout << "\nDeleted " << presenceNotificationUris[i].c_str() << " for presence notification";
    }
    return NULL;
}
#endif

int createLightResource (char *uri, LightResource *lightResource)
{
    if (!uri)
    {
        cout << "\nResource URI cannot be NULL";
        return -1;
    }

    lightResource->state = false;
    lightResource->power= 0;
    OCStackResult res = OCCreateResource(&(lightResource->handle),
            "core.light",
            "oc.mi.def",
            uri,
            OCEntityHandlerCb,
            NULL,
            OC_DISCOVERABLE|OC_OBSERVABLE);
    cout << "\nCreated Light resource with result " << getResult(res);

    return 0;
}

void DeletePlatformInfo()
{
    free (platformInfo.platformID);
    free (platformInfo.manufacturerName);
    free (platformInfo.manufacturerUrl);
    free (platformInfo.modelNumber);
    free (platformInfo.dateOfManufacture);
    free (platformInfo.platformVersion);
    free (platformInfo.operatingSystemVersion);
    free (platformInfo.hardwareVersion);
    free (platformInfo.firmwareVersion);
    free (platformInfo.supportUrl);
    free (platformInfo.systemTime);
}

void DeleteDeviceInfo()
{
    free (deviceInfo.deviceName);
}

bool DuplicateString(char** targetString, const char* sourceString)
{
    if(!sourceString)
    {
        return false;
    }
    else
    {
        *targetString = (char *) malloc(strlen(sourceString) + 1);

        if(*targetString)
        {
            strncpy(*targetString, sourceString, (strlen(sourceString) + 1));
            return true;
        }
    }
    return false;
}

OCStackResult SetPlatformInfo(const char* platformID, const char *manufacturerName,
    const char *manufacturerUrl, const char *modelNumber, const char *dateOfManufacture,
    const char *platformVersion, const char* operatingSystemVersion, const char* hardwareVersion,
    const char *firmwareVersion, const char* supportUrl, const char* systemTime)
{

    bool success = true;

    if(manufacturerName != NULL && (strlen(manufacturerName) > MAX_MANUFACTURER_NAME_LENGTH))
    {
        return OC_STACK_INVALID_PARAM;
    }

    if(manufacturerUrl != NULL && (strlen(manufacturerUrl) > MAX_MANUFACTURER_URL_LENGTH))
    {
        return OC_STACK_INVALID_PARAM;
    }

    if(!DuplicateString(&platformInfo.platformID, platformID))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.manufacturerName, manufacturerName))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.manufacturerUrl, manufacturerUrl))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.modelNumber, modelNumber))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.dateOfManufacture, dateOfManufacture))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.platformVersion, platformVersion))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.operatingSystemVersion, operatingSystemVersion))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.hardwareVersion, hardwareVersion))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.firmwareVersion, firmwareVersion))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.supportUrl, supportUrl))
    {
        success = false;
    }

    if(!DuplicateString(&platformInfo.systemTime, systemTime))
    {
        success = false;
    }

    if(success)
    {
        return OC_STACK_OK;
    }

    DeletePlatformInfo();
    return OC_STACK_ERROR;
}

OCStackResult SetDeviceInfo(const char* deviceName)
{
    if(!DuplicateString(&deviceInfo.deviceName, deviceName))
    {
        return OC_STACK_ERROR;
    }
    return OC_STACK_OK;
}

static void PrintUsage()
{
    cout << "\nUsage : ocserver -o <0|1>";
    cout << "\n-o 0 : Notify all observers";
    cout << "\n-o 1 : Notify list of observers";
}

int main(int argc, char* argv[])
{
    pthread_t threadId;
    pthread_t threadId_presence;
    int opt;

    while ((opt = getopt(argc, argv, "o:")) != -1)
    {
        switch(opt)
        {
            case 'o':
                gObserveNotifyType = atoi(optarg);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    if ((gObserveNotifyType != 0) && (gObserveNotifyType != 1))
    {
        PrintUsage();
        return -1;
    }

    cout << "\nOCServer is starting...";

    if (OCInit(NULL, 0, OC_SERVER) != OC_STACK_OK)
    {
        cout << "\nOCStack init error";
        return 0;
    }
#ifdef WITH_PRESENCE
    if (OCStartPresence(0) != OC_STACK_OK)
    {
        cout << "\nOCStack presence/discovery error";
        return 0;
    }
#endif

    OCSetDefaultDeviceEntityHandler(OCDeviceEntityHandlerCb, NULL);

    OCStackResult registrationResult =
        SetPlatformInfo(platformID, manufacturerName, manufacturerUrl, modelNumber,
            dateOfManufacture, platformVersion,  operatingSystemVersion,  hardwareVersion,
            firmwareVersion,  supportUrl, systemTime);

    if (registrationResult != OC_STACK_OK)
    {
        cout << "\nPlatform info setting failed locally!";
        exit (EXIT_FAILURE);
    }

    registrationResult = OCSetPlatformInfo(platformInfo);

    if (registrationResult != OC_STACK_OK)
    {
        cout << "\nPlatform Registration failed!";
        exit (EXIT_FAILURE);
    }

    registrationResult = SetDeviceInfo(deviceName);

    if (registrationResult != OC_STACK_OK)
    {
        cout << "\nDevice info setting failed locally!";
        exit (EXIT_FAILURE);
    }

    registrationResult = OCSetDeviceInfo(deviceInfo);

    if (registrationResult != OC_STACK_OK)
    {
        cout << "\nDevice Registration failed!";
        exit (EXIT_FAILURE);
    }

    /*
     * Declare and create the example resource: Light
     */
    createLightResource(gResourceUri, &Light);

    // Initialize observations data structure for the resource
    for (uint8_t i = 0; i < SAMPLE_MAX_NUM_OBSERVATIONS; i++)
    {
        interestedObservers[i].valid = false;
    }

    /*
     * Create a thread for changing the representation of the Light
     */
    pthread_create (&threadId, NULL, ChangeLightRepresentation, (void *)NULL);

    /*
     * Create a thread for generating changes that cause presence notifications
     * to be sent to clients
     */

    #ifdef WITH_PRESENCE
    pthread_create(&threadId_presence, NULL, presenceNotificationGenerator, (void *)NULL);
    #endif

    // Break from loop with Ctrl-C
    cout << "\nEntering ocserver main loop...";

    DeletePlatformInfo();
    DeleteDeviceInfo();

    signal(SIGINT, handleSigInt);

    while (!gQuitFlag)
    {
        if (OCProcess() != OC_STACK_OK)
        {
            cout << "\nOCStack process error";
            return 0;
        }
#ifndef ROUTING_GATEWAY
        sleep(1);
#endif
    }

    /*
     * Cancel the Light thread and wait for it to terminate
     */
    pthread_cancel(threadId);
    pthread_join(threadId, NULL);
    pthread_cancel(threadId_presence);
    pthread_join(threadId_presence, NULL);

    cout << "\nExiting ocserver main loop...\n";

    if (OCStop() != OC_STACK_OK)
    {
        cout << "\nOCStack process error";
    }

    return 0;
}
