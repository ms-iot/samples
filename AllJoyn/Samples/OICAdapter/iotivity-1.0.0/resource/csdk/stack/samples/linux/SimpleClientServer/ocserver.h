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

#ifndef OCSERVER_H_
#define OCSERVER_H_

#include "ocstack.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define TAG "ocserver"
#define SAMPLE_MAX_NUM_OBSERVATIONS     8
#define SAMPLE_MAX_NUM_POST_INSTANCE  2

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

/* Structure to represent a Light resource */
typedef struct LIGHTRESOURCE
{
    OCResourceHandle handle;
    bool state;
    int power;
} LightResource;

/* Structure to represent the observers */
typedef struct
{
    OCObservationId observationId;
    bool valid;
    OCResourceHandle resourceHandle;
} Observers;

//-----------------------------------------------------------------------------
// Function prototype
//-----------------------------------------------------------------------------

/* call getResult in common.cpp to get the result in string format. */
const char *getResult(OCStackResult result);

/* Function that creates a new Light resource by calling the
 * OCCreateResource() method.
 */
int createLightResource (char *uri, LightResource *lightResource);

/* This method constructs a response from the request */
OCRepPayload* constructResponse (OCEntityHandlerRequest *ehRequest);

/* This method changes the Light power using an independent thread
 * and notifies the observers of new state of the resource.
 */
void *ChangeLightRepresentation (void *param);

/* This method check the validity of resourceTypeName and resource interfaces
 * Entity Handler has to parse the query string in order to process it
 */
OCEntityHandlerResult ValidateQueryParams (OCEntityHandlerRequest *entityHandlerRequest);

/* Following methods process the PUT, GET, POST, Delete,
 * & Observe requests */
OCEntityHandlerResult ProcessGetRequest (OCEntityHandlerRequest *ehRequest,
                                         OCRepPayload **payload);
OCEntityHandlerResult ProcessPutRequest (OCEntityHandlerRequest *ehRequest,
                                         OCRepPayload **payload);
OCEntityHandlerResult ProcessPostRequest (OCEntityHandlerRequest *ehRequest,
                                          OCEntityHandlerResponse *response,
                                         OCRepPayload **payload);
OCEntityHandlerResult ProcessDeleteRequest (OCEntityHandlerRequest *ehRequest);

OCEntityHandlerResult ProcessNonExistingResourceRequest (OCEntityHandlerRequest *ehRequest);

void ProcessObserveRegister (OCEntityHandlerRequest *ehRequest);
void ProcessObserveDeregister (OCEntityHandlerRequest *ehRequest);

void DeleteDeviceInfo();

OCStackResult SetDeviceInfo(const char *contentType, const char *dateOfManufacture,
                const char *deviceName, const char *deviceUUID, const char *firmwareVersion,
                const char *hostName, const char *manufacturerName, const char *manufacturerUrl,
                const char *modelNumber, const char *platformVersion, const char *supportUrl,
                const char *version);


//-----------------------------------------------------------------------------
// Callback functions
//-----------------------------------------------------------------------------

/* Entity Handler callback functions */
OCEntityHandlerResult
OCDeviceEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest, char* uri);

OCEntityHandlerResult
OCEntityHandlerCb (OCEntityHandlerFlag flag,
        OCEntityHandlerRequest *entityHandlerRequest);


#endif

