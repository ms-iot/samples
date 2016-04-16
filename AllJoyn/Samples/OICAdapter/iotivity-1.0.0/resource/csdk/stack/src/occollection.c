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

// Defining _POSIX_C_SOURCE macro with 200112L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1-2001 base
// specification (excluding the XSI extension).
// For POSIX.1-2001 base specification,
// Refer http://pubs.opengroup.org/onlinepubs/009695399/
#define _POSIX_C_SOURCE 200112L
#include "occollection.h"
#include <string.h>
#include "ocstack.h"
#include "ocstackinternal.h"
#include "ocresourcehandler.h"
#include "logger.h"
#include "cJSON.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "ocpayload.h"
#include "payload_logging.h"

/// Module Name
#include <stdio.h>

#define WITH_GROUPACTION 1

#include "oicgroup.h"

#define TAG "occollection"

#define NUM_PARAM_IN_QUERY   2 // The expected number of parameters in a query
#define NUM_FIELDS_IN_QUERY  2 // The expected number of fields in a query

#ifdef WIN32
#define strtok_r strtok_s
#endif

static OCStackResult CheckRTParamSupport(const OCResource* resource, const char* rtPtr)
{
    if(!resource || !rtPtr)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCResourceType* rTPointer = resource->rsrcType;
    while (rTPointer)
    {
        if( strcmp (rTPointer->resourcetypename, rtPtr) == 0)
        {
            return OC_STACK_OK;
        }

        rTPointer = rTPointer->next;
    }
    return OC_STACK_ERROR;
}

static OCStackResult CheckIFParamSupport(const OCResource* resource, const char* ifPtr)
{
    if(!resource || !ifPtr)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCResourceInterface* iFPointer = resource->rsrcInterface;
    while (iFPointer)
    {
        if( strcmp (iFPointer->name, ifPtr) == 0)
        {
            return OC_STACK_OK;
        }

        iFPointer = iFPointer->next;
    }
    return OC_STACK_ERROR;
}

static OCStackResult
ValidateQuery (const char *query, OCResourceHandle resource,
                             OCStackIfTypes *ifParam, char **rtParam)
{
    uint8_t numFields = 0;
    uint8_t numParam;

    //TODO: Query and URL validation is being done for virtual resource case
    // using ValidateUrlQuery function. We should be able to merge it with this
    // function.
    OC_LOG(INFO, TAG, "Entering ValidateQuery");

    if (!query)
    {
        return OC_STACK_ERROR;
    }

    if(!ifParam || !rtParam)
    {
        return OC_STACK_INVALID_PARAM;
    }

    if (!(*query))
    {
        // Query string is empty
        OC_LOG(INFO, TAG, "Empty query string, use default IF and RT");
        *ifParam = STACK_IF_DEFAULT;
        *rtParam = (char *) OCGetResourceTypeName (resource, 0);
        return OC_STACK_OK;
    }

    // Break the query string to validate it and determine IF and RT parameters
    // Validate there are atmost 2 parameters in string and that one is 'if' and other 'rt'
    // separated by token '&' or ';'. Stack will accept both the versions.

    char *endStr, *ifPtr = NULL, *rtPtr = NULL;
    char *token = strtok_r ((char *)query, OC_QUERY_SEPARATOR , &endStr);

    // External loop breaks query string into fields using the & separator
    while (token != NULL)
    {
        numFields++;
        char *endToken;
        char *innerToken = strtok_r (token, "=", &endToken);
        numParam = 0;

        // Internal loop parses the field to extract values (parameters) assigned to each field
        while (innerToken != NULL)
        {
            numParam++;
            if (strncmp (innerToken, OC_RSRVD_INTERFACE, sizeof(OC_RSRVD_INTERFACE)) == 0)
            {
                // Determine the value of IF parameter
                innerToken = strtok_r (NULL, "=", &endToken);
                ifPtr = innerToken;
            }
            else if (strcmp (innerToken, OC_RSRVD_RESOURCE_TYPE) == 0)
            {
                // Determine the value of RT parameter
                innerToken = strtok_r (NULL, "=", &endToken);
                rtPtr = innerToken;
            }
            else
            {
                innerToken = strtok_r (NULL, "=", &endToken);
            }
        }
        if (numParam != NUM_PARAM_IN_QUERY)
        {
            // Query parameter should be of the form if=<string>. String should not have & or =
            return OC_STACK_INVALID_QUERY;
        }
        token = strtok_r (NULL, OC_QUERY_SEPARATOR, &endStr);
    }

    if (numFields > NUM_FIELDS_IN_QUERY)
    {
        // current release supports one IF value, one RT value and no other params
        return OC_STACK_INVALID_QUERY;
    }

    if (ifPtr)
    {
        if(CheckIFParamSupport((OCResource *)resource, ifPtr) != OC_STACK_OK)
        {
            return OC_STACK_INVALID_QUERY;
        }
        if (strcmp (ifPtr, OC_RSRVD_INTERFACE_DEFAULT) == 0)
        {
            *ifParam = STACK_IF_DEFAULT;
        }
        else if (strcmp (ifPtr, OC_RSRVD_INTERFACE_LL) == 0)
        {
            *ifParam = STACK_IF_LL;
        }
        else if (strcmp (ifPtr, OC_RSRVD_INTERFACE_BATCH) == 0)
        {
            *ifParam = STACK_IF_BATCH;
        }
        else if(strcmp (ifPtr, OC_RSRVD_INTERFACE_GROUP) == 0)
        {
            *ifParam = STACK_IF_GROUP;
        }
        else
        {
            return OC_STACK_ERROR;
        }
    }
    else
    {
        // IF not specified in query, use default IF
        *ifParam = STACK_IF_DEFAULT;
    }

    if (rtPtr)
    {
        if (CheckRTParamSupport((OCResource *)resource, rtPtr) == OC_STACK_OK)
        {
            *rtParam = rtPtr;
        }
        else
        {
            return OC_STACK_INVALID_QUERY;
        }
    }
    else
    {
        // RT not specified in query. Use the first resource type for the resource as default.
        *rtParam = (char *) OCGetResourceTypeName (resource, 0);
    }
    OC_LOG_V(INFO, TAG, "Query params: IF = %d, RT = %s", *ifParam, *rtParam);

    return OC_STACK_OK;
}

static OCStackResult
HandleLinkedListInterface(OCEntityHandlerRequest *ehRequest,
                          uint8_t filterOn,
                          char *filterValue)
{
    (void)filterOn;
    (void)filterValue;
    if(!ehRequest)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult ret = OC_STACK_OK;
    OCResource *collResource = (OCResource *)ehRequest->resource;

    OCRepPayload* payload = NULL;

    if(ret == OC_STACK_OK)
    {
        ret = BuildResponseRepresentation(collResource, &payload);
    }

    if (ret == OC_STACK_OK)
    {
        for  (int i = 0; i < MAX_CONTAINED_RESOURCES && ret == OC_STACK_OK; i++)
        {
            OCResource* temp = collResource->rsrcResources[i];
            if (temp)
            {
                //TODO : Add resource type filtering once collections
                // start supporting queries.
                ret = BuildResponseRepresentation(temp, &payload);
            }
        }
    }

    if(ret == OC_STACK_OK)
    {
        OCEntityHandlerResponse response = {0};
        response.ehResult = OC_EH_OK;
        response.payload = (OCPayload*)payload;
        response.persistentBufferFlag = 0;
        response.requestHandle = (OCRequestHandle) ehRequest->requestHandle;
        response.resourceHandle = (OCResourceHandle) collResource;
        ret = OCDoResponse(&response);
    }
    OCRepPayloadDestroy(payload);
    return ret;
}

static OCStackResult
HandleBatchInterface(OCEntityHandlerRequest *ehRequest)
{
    if (!ehRequest)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult stackRet = OC_STACK_OK;
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCResource * collResource = (OCResource *) ehRequest->resource;

    OCRepPayload* payload = OCRepPayloadCreate();
    if(!payload)
    {
        stackRet = OC_STACK_NO_MEMORY;
    }

    if(stackRet == OC_STACK_OK)
    {
        if (collResource)
        {
            OCRepPayloadSetUri(payload, collResource->uri);
        }
    }

    if(stackRet == OC_STACK_OK)
    {
        OCEntityHandlerResponse response = {0};
        response.ehResult = OC_EH_OK;
        response.payload = (OCPayload*)payload;
        response.persistentBufferFlag = 0;
        response.requestHandle = (OCRequestHandle) ehRequest->requestHandle;
        response.resourceHandle = (OCResourceHandle) collResource;
        stackRet = OCDoResponse(&response);
    }

    if (stackRet == OC_STACK_OK)
    {
        for  (uint8_t i = 0; i < MAX_CONTAINED_RESOURCES; i++)
        {
            OCResource* temp = collResource->rsrcResources[i];
            if (temp)
            {
                // Note that all entity handlers called through a collection
                // will get the same pointer to ehRequest, the only difference
                // is ehRequest->resource
                ehRequest->resource = (OCResourceHandle) temp;

                ehResult = temp->entityHandler(OC_REQUEST_FLAG, ehRequest,
                                        temp->entityHandlerCallbackParam);

                // The default collection handler is returning as OK
                if(stackRet != OC_STACK_SLOW_RESOURCE)
                {
                    stackRet = OC_STACK_OK;
                }
                // if a single resource is slow, then entire response will be treated
                // as slow response
                if(ehResult == OC_EH_SLOW)
                {
                    OC_LOG(INFO, TAG, "This is a slow resource");
                    ((OCServerRequest *)ehRequest->requestHandle)->slowFlag = 1;
                    stackRet = EntityHandlerCodeToOCStackCode(ehResult);
                }
            }
            else
            {
                break;
            }
        }
        ehRequest->resource = (OCResourceHandle) collResource;
    }
    return stackRet;
}

uint8_t GetNumOfResourcesInCollection (OCResource *resource)
{
    if(resource)
    {
        uint8_t num = 0;
        for (uint8_t i = 0; i < MAX_CONTAINED_RESOURCES; i++)
        {
            if (resource->rsrcResources[i])
            {
                num++;
            }
        }
        return num;
    }
    else
    {
        return -1;
    }
}


OCStackResult DefaultCollectionEntityHandler (OCEntityHandlerFlag flag,
                                              OCEntityHandlerRequest *ehRequest)
{
    if(!ehRequest || !ehRequest->query)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OCStackResult result = OC_STACK_ERROR;
    OCStackIfTypes ifQueryParam = STACK_IF_INVALID;
    char *rtQueryParam = NULL;

    OC_LOG_V(INFO, TAG, "DefaultCollectionEntityHandler with query %s", ehRequest->query);

    if (flag != OC_REQUEST_FLAG)
    {
        return OC_STACK_ERROR;
    }

    result = ValidateQuery (ehRequest->query,
                            ehRequest->resource, &ifQueryParam, &rtQueryParam);

    if (result != OC_STACK_OK)
    {
        return result;
    }

    switch (ehRequest->method)
    {
        case OC_REST_GET:
            switch (ifQueryParam)
            {
                case STACK_IF_DEFAULT:
                    // Get attributes of collection resource and properties of contained resources
                    // M1 release does not support attributes for collection resource, so the GET
                    // operation is same as the GET on LL interface.
                    OC_LOG(INFO, TAG, "STACK_IF_DEFAULT");
                    return HandleLinkedListInterface(ehRequest, STACK_RES_DISCOVERY_NOFILTER, NULL);

                case STACK_IF_LL:
                    OC_LOG(INFO, TAG, "STACK_IF_LL");
                    return HandleLinkedListInterface(ehRequest, STACK_RES_DISCOVERY_NOFILTER, NULL);

                case STACK_IF_BATCH:
                    OC_LOG(INFO, TAG, "STACK_IF_BATCH");
                    ((OCServerRequest *)ehRequest->requestHandle)->ehResponseHandler =
                                                                            HandleAggregateResponse;

                    ((OCServerRequest *)ehRequest->requestHandle)->numResponses =
                            GetNumOfResourcesInCollection((OCResource *)ehRequest->resource) + 1;

                    return HandleBatchInterface(ehRequest);

                case STACK_IF_GROUP:
                    return BuildCollectionGroupActionCBORResponse(OC_REST_GET/*flag*/,
                            (OCResource *) ehRequest->resource, ehRequest);

                default:
                    return OC_STACK_ERROR;
            }

        case OC_REST_PUT:
            switch (ifQueryParam)
            {
                case STACK_IF_DEFAULT:
                    // M1 release does not support PUT on default interface
                    return OC_STACK_ERROR;

                case STACK_IF_LL:
                    // LL interface only supports GET
                    return OC_STACK_ERROR;

                case STACK_IF_BATCH:
                    ((OCServerRequest *)ehRequest->requestHandle)->ehResponseHandler =
                                                                            HandleAggregateResponse;
                    ((OCServerRequest *)ehRequest->requestHandle)->numResponses =
                            GetNumOfResourcesInCollection((OCResource *)ehRequest->resource) + 1;
                    return HandleBatchInterface(ehRequest);

                case STACK_IF_GROUP:
                    OC_LOG(INFO, TAG, "IF_COLLECTION PUT with request ::\n");
                    OC_LOG_PAYLOAD(INFO, ehRequest->payload);
                    return BuildCollectionGroupActionCBORResponse(OC_REST_PUT/*flag*/,
                            (OCResource *) ehRequest->resource, ehRequest);

                default:
                    return OC_STACK_ERROR;
            }

        case OC_REST_POST:
            switch (ifQueryParam)
            {
                case STACK_IF_DEFAULT:
                    // M1 release does not support POST on default interface
                    return OC_STACK_ERROR;

                case STACK_IF_LL:
                    // LL interface only supports GET
                    return OC_STACK_ERROR;

                case STACK_IF_BATCH:
                    ((OCServerRequest *)ehRequest->requestHandle)->ehResponseHandler =
                                                                            HandleAggregateResponse;
                    ((OCServerRequest *)ehRequest->requestHandle)->numResponses =
                            GetNumOfResourcesInCollection((OCResource *)ehRequest->resource) + 1;
                    return HandleBatchInterface(ehRequest);

                case STACK_IF_GROUP:
                    OC_LOG(INFO, TAG, "IF_COLLECTION POST with request ::\n");
                    OC_LOG_PAYLOAD(INFO, ehRequest->payload);
                    return BuildCollectionGroupActionCBORResponse(OC_REST_POST/*flag*/,
                            (OCResource *) ehRequest->resource, ehRequest);

                default:
                    return OC_STACK_ERROR;
            }

        case OC_REST_DELETE:
            // TODO implement DELETE accordingly to the desired behavior
            return OC_STACK_ERROR;

        default:
            return OC_STACK_ERROR;
    }
}
