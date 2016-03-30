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


#include <stdlib.h>
#include <string.h>
#include "ocstack.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "cJSON.h"
#include "base64.h"
#include "resourcemanager.h"
#include "psinterface.h"
#include "utlist.h"
#include "srmresourcestrings.h"
#include "amaclresource.h"
#include "srmutility.h"
#include <stdlib.h>
#include <string.h>

#define TAG  "SRM-AMACL"

OicSecAmacl_t *gAmacl = NULL;
static OCResourceHandle gAmaclHandle = NULL;

void DeleteAmaclList(OicSecAmacl_t* amacl)
{
    if (amacl)
    {
        OicSecAmacl_t *amaclTmp1 = NULL, *amaclTmp2 = NULL;
        LL_FOREACH_SAFE(amacl, amaclTmp1, amaclTmp2)
        {
            unsigned int i = 0;

            LL_DELETE(amacl, amaclTmp1);

            // Clean Resources
            for (i = 0; i < amaclTmp1->resourcesLen; i++)
            {
                OICFree(amaclTmp1->resources[i]);
            }
            OICFree(amaclTmp1->resources);

            // Clean Amss
            OICFree(amaclTmp1->amss);

            // Clean Owners
            OICFree(amaclTmp1->owners);

            // Clean Amacl node itself
            OICFree(amaclTmp1);
        }
    }
}

/*
 * This internal method converts AMACL data into JSON format.
 *
 * Note: Caller needs to invoke 'free' when finished using the return string.
 */
char * BinToAmaclJSON(const OicSecAmacl_t * amacl)
{
    cJSON *jsonRoot = NULL;
    char *jsonStr = NULL;

    if (amacl)
    {
        jsonRoot = cJSON_CreateObject();
        VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

        cJSON *jsonAmaclArray = NULL;
        cJSON_AddItemToObject (jsonRoot, OIC_JSON_AMACL_NAME, jsonAmaclArray = cJSON_CreateArray());
        VERIFY_NON_NULL(TAG, jsonAmaclArray, ERROR);

        while(amacl)
        {
            char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0};
            uint32_t outLen = 0;
            B64Result b64Ret = B64_OK;

            cJSON *jsonAmacl = cJSON_CreateObject();

            // Resources -- Mandatory
            cJSON *jsonRsrcArray = NULL;
            cJSON_AddItemToObject(jsonAmacl, OIC_JSON_RESOURCES_NAME, jsonRsrcArray =
                    cJSON_CreateArray());
            VERIFY_NON_NULL(TAG, jsonRsrcArray, ERROR);
            for (unsigned int i = 0; i < amacl->resourcesLen; i++)
            {
                cJSON_AddItemToArray(jsonRsrcArray, cJSON_CreateString(amacl->resources[i]));
            }

            // Amss -- Mandatory
            cJSON *jsonAmsArray = NULL;
            cJSON_AddItemToObject(jsonAmacl, OIC_JSON_AMSS_NAME, jsonAmsArray =
                    cJSON_CreateArray());
            VERIFY_NON_NULL(TAG, jsonAmsArray, ERROR);
            for (unsigned int i = 0; i < amacl->amssLen; i++)
            {
                outLen = 0;

                b64Ret = b64Encode(amacl->amss[i].id, sizeof(((OicUuid_t*) 0)->id), base64Buff,
                        sizeof(base64Buff), &outLen);
                VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);

                cJSON_AddItemToArray(jsonAmsArray, cJSON_CreateString(base64Buff));
            }

            // Owners -- Mandatory
            cJSON *jsonOwnrArray = NULL;
            cJSON_AddItemToObject(jsonAmacl, OIC_JSON_OWNERS_NAME, jsonOwnrArray =
                    cJSON_CreateArray());
            VERIFY_NON_NULL(TAG, jsonOwnrArray, ERROR);
            for (unsigned int i = 0; i < amacl->ownersLen; i++)
            {
                outLen = 0;

                b64Ret = b64Encode(amacl->owners[i].id, sizeof(((OicUuid_t*) 0)->id), base64Buff,
                        sizeof(base64Buff), &outLen);
                VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);

                cJSON_AddItemToArray(jsonOwnrArray, cJSON_CreateString(base64Buff));
            }

            // Attach current amacl node to Amacl Array
            cJSON_AddItemToArray(jsonAmaclArray, jsonAmacl);
            amacl = amacl->next;
        }

        jsonStr = cJSON_PrintUnformatted(jsonRoot);
    }

exit:
    if (jsonRoot)
    {
        cJSON_Delete(jsonRoot);
    }
    return jsonStr;
}




/*
 * This internal method converts JSON AMACL into binary AMACL.
 */
OicSecAmacl_t * JSONToAmaclBin(const char * jsonStr)
{
    OCStackResult ret = OC_STACK_ERROR;
    OicSecAmacl_t * headAmacl = NULL;
    OicSecAmacl_t * prevAmacl = NULL;
    cJSON *jsonRoot = NULL;
    cJSON *jsonAmaclArray = NULL;

    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    jsonRoot = cJSON_Parse(jsonStr);
    VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

    jsonAmaclArray = cJSON_GetObjectItem(jsonRoot, OIC_JSON_AMACL_NAME);
    VERIFY_NON_NULL(TAG, jsonAmaclArray, INFO);

    if (cJSON_Array == jsonAmaclArray->type)
    {
        int numAmacl = cJSON_GetArraySize(jsonAmaclArray);
        int idx = 0;

        VERIFY_SUCCESS(TAG, numAmacl > 0, INFO);
        do
        {
            cJSON *jsonAmacl = cJSON_GetArrayItem(jsonAmaclArray, idx);
            VERIFY_NON_NULL(TAG, jsonAmacl, ERROR);

            OicSecAmacl_t *amacl = (OicSecAmacl_t*)OICCalloc(1, sizeof(OicSecAmacl_t));
            VERIFY_NON_NULL(TAG, amacl, ERROR);

            headAmacl = (headAmacl) ? headAmacl : amacl;
            if (prevAmacl)
            {
                prevAmacl->next = amacl;
            }

            size_t jsonObjLen = 0;
            cJSON *jsonObj = NULL;

            // Resources -- Mandatory
            jsonObj = cJSON_GetObjectItem(jsonAmacl, OIC_JSON_RESOURCES_NAME);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_Array == jsonObj->type, ERROR);

            amacl->resourcesLen = cJSON_GetArraySize(jsonObj);
            VERIFY_SUCCESS(TAG, amacl->resourcesLen > 0, ERROR);
            amacl->resources = (char**)OICCalloc(amacl->resourcesLen, sizeof(char*));
            VERIFY_NON_NULL(TAG, (amacl->resources), ERROR);

            size_t idxx = 0;
            do
            {
                cJSON *jsonRsrc = cJSON_GetArrayItem(jsonObj, idxx);
                VERIFY_NON_NULL(TAG, jsonRsrc, ERROR);

                jsonObjLen = strlen(jsonRsrc->valuestring) + 1;
                amacl->resources[idxx] = (char*)OICMalloc(jsonObjLen);
                VERIFY_NON_NULL(TAG, (amacl->resources[idxx]), ERROR);
                OICStrcpy(amacl->resources[idxx], jsonObjLen, jsonRsrc->valuestring);
            } while ( ++idxx < amacl->resourcesLen);

            // Amss -- Mandatory
            VERIFY_SUCCESS( TAG, OC_STACK_OK == AddUuidArray(jsonAmacl, OIC_JSON_AMSS_NAME,
                               &(amacl->amssLen), &(amacl->amss)), ERROR);

            // Owners -- Mandatory
            VERIFY_SUCCESS( TAG, OC_STACK_OK == AddUuidArray(jsonAmacl, OIC_JSON_OWNERS_NAME,
                               &(amacl->ownersLen), &(amacl->owners)), ERROR);

            prevAmacl = amacl;
        } while( ++idx < numAmacl);
    }

    ret = OC_STACK_OK;

exit:
    cJSON_Delete(jsonRoot);
    if (OC_STACK_OK != ret)
    {
        DeleteAmaclList(headAmacl);
        headAmacl = NULL;
    }
    return headAmacl;
}

static OCEntityHandlerResult HandleAmaclGetRequest (const OCEntityHandlerRequest * ehRequest)
{
    // Convert Amacl data into JSON for transmission
    char* jsonStr = BinToAmaclJSON(gAmacl);

    OCEntityHandlerResult ehRet = (jsonStr ? OC_EH_OK : OC_EH_ERROR);

    // Send response payload to request originator
    SendSRMResponse(ehRequest, ehRet, jsonStr);

    OICFree(jsonStr);

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ehRet);
    return ehRet;
}

static OCEntityHandlerResult HandleAmaclPostRequest (const OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;

    // Convert JSON Amacl data into binary. This will also validate the Amacl data received.
    OicSecAmacl_t* newAmacl = JSONToAmaclBin(((OCSecurityPayload*)ehRequest->payload)->securityData);

    if (newAmacl)
    {
        // Append the new Amacl to existing Amacl
        LL_APPEND(gAmacl, newAmacl);

        // Convert Amacl data into JSON for update to persistent storage
        char *jsonStr = BinToAmaclJSON(gAmacl);
        if (jsonStr)
        {
            cJSON *jsonAmacl = cJSON_Parse(jsonStr);
            OICFree(jsonStr);

            if ((jsonAmacl) &&
                (OC_STACK_OK == UpdateSVRDatabase(OIC_JSON_AMACL_NAME, jsonAmacl)))
            {
                ehRet = OC_EH_RESOURCE_CREATED;
            }
            cJSON_Delete(jsonAmacl);
        }
    }

    // Send payload to request originator
    SendSRMResponse(ehRequest, ehRet, NULL);

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ehRet);
    return ehRet;
}

/*
 * This internal method is the entity handler for Amacl resources and
 * will handle REST request (GET/PUT/POST/DEL) for them.
 */
OCEntityHandlerResult AmaclEntityHandler (OCEntityHandlerFlag flag,
                                          OCEntityHandlerRequest * ehRequest,
                                          void* callbackParameter)
{
    (void) callbackParameter;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;

    if (!ehRequest)
    {
        return ehRet;
    }

    if (flag & OC_REQUEST_FLAG)
    {
        OC_LOG (DEBUG, TAG, "Flag includes OC_REQUEST_FLAG");
        switch (ehRequest->method)
        {
            case OC_REST_GET:
                ehRet = HandleAmaclGetRequest(ehRequest);
                break;

            case OC_REST_POST:
                ehRet = HandleAmaclPostRequest(ehRequest);
                break;

            default:
                ehRet = OC_EH_ERROR;
                SendSRMResponse(ehRequest, ehRet, NULL);
        }
    }

    return ehRet;
}

/*
 * This internal method is used to create '/oic/sec/amacl' resource.
 */
OCStackResult CreateAmaclResource()
{
    OCStackResult ret;

    ret = OCCreateResource(&gAmaclHandle,
                           OIC_RSRC_TYPE_SEC_AMACL,
                           OIC_MI_DEF,
                           OIC_RSRC_AMACL_URI,
                           AmaclEntityHandler,
                           NULL,
                           OC_OBSERVABLE);

    if (OC_STACK_OK != ret)
    {
        OC_LOG (FATAL, TAG, "Unable to instantiate Amacl resource");
        DeInitAmaclResource();
    }
    return ret;
}

/**
 * Initialize Amacl resource by loading data from persistent storage.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult InitAmaclResource()
{
    OCStackResult ret = OC_STACK_ERROR;

    // Read Amacl resource from PS
    char* jsonSVRDatabase = GetSVRDatabase();

    if (jsonSVRDatabase)
    {
        // Convert JSON Amacl into binary format
        gAmacl = JSONToAmaclBin(jsonSVRDatabase);
        OICFree(jsonSVRDatabase);
    }

    // Instantiate 'oic/sec/amacl' resource
    ret = CreateAmaclResource();

    if (OC_STACK_OK != ret)
    {
        DeInitAmaclResource();
    }
    return ret;
}

/**
 * Perform cleanup for Amacl resources.
 *
 * @retval  none
 */
void DeInitAmaclResource()
{
    OCDeleteResource(gAmaclHandle);
    gAmaclHandle = NULL;

    DeleteAmaclList(gAmacl);
    gAmacl = NULL;
}


OCStackResult AmaclGetAmsDeviceId(const char *resource, OicUuid_t *amsDeviceId)
{
    OicSecAmacl_t *amacl = NULL;

    VERIFY_NON_NULL(TAG, resource, ERROR);
    VERIFY_NON_NULL(TAG, amsDeviceId, ERROR);

    LL_FOREACH(gAmacl, amacl)
    {
        for(size_t i = 0; i < amacl->resourcesLen; i++)
        {
            if (strncmp((amacl->resources[i]), resource, strlen(amacl->resources[i])) == 0)
            {
                //Returning the ID of the first AMS service for the resource
                memcpy(amsDeviceId, &amacl->amss[0], sizeof(*amsDeviceId));
                return OC_STACK_OK;
            }
        }
    }

exit:
    return OC_STACK_ERROR;
}
