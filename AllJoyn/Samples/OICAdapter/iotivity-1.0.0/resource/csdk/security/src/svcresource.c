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

#include "ocstack.h"
#include "logger.h"
#include "oic_malloc.h"
#include "cJSON.h"
#include "base64.h"
#include "resourcemanager.h"
#include "psinterface.h"
#include "svcresource.h"
#include "utlist.h"
#include "srmresourcestrings.h"
#include "srmutility.h"
#include <stdlib.h>
#include <string.h>

#define TAG  "SRM-SVC"

OicSecSvc_t        *gSvc = NULL;
static OCResourceHandle    gSvcHandle = NULL;

void DeleteSVCList(OicSecSvc_t* svc)
{
    if (svc)
    {
        OicSecSvc_t *svcTmp1 = NULL, *svcTmp2 = NULL;
        LL_FOREACH_SAFE(svc, svcTmp1, svcTmp2)
        {
            LL_DELETE(svc, svcTmp1);

            // Clean Owners
            OICFree(svcTmp1->owners);

            // Clean SVC node itself
            OICFree(svcTmp1);
        }
    }
}

/*
 * This internal method converts SVC data into JSON format.
 *
 * Note: Caller needs to invoke 'free' when finished done using
 * return string.
 */
char * BinToSvcJSON(const OicSecSvc_t * svc)
{
    cJSON *jsonRoot = NULL;
    char *jsonStr = NULL;

    if (svc)
    {
        jsonRoot = cJSON_CreateObject();
        VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

        cJSON *jsonSvcArray = NULL;
        cJSON_AddItemToObject (jsonRoot, OIC_JSON_SVC_NAME, jsonSvcArray = cJSON_CreateArray());
        VERIFY_NON_NULL(TAG, jsonSvcArray, ERROR);

        while(svc)
        {
            char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0};
            uint32_t outLen = 0;
            B64Result b64Ret = B64_OK;

            cJSON *jsonSvc = cJSON_CreateObject();

            // Service Device Identity
            outLen = 0;
            b64Ret = b64Encode(svc->svcdid.id, sizeof(OicUuid_t), base64Buff,
                    sizeof(base64Buff), &outLen);
            VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);
            cJSON_AddStringToObject(jsonSvc, OIC_JSON_SERVICE_DEVICE_ID, base64Buff );

            // Service Type
            cJSON_AddNumberToObject (jsonSvc, OIC_JSON_SERVICE_TYPE, svc->svct);

            // Owners
            cJSON *jsonOwnrArray = NULL;
            cJSON_AddItemToObject (jsonSvc, OIC_JSON_OWNERS_NAME, jsonOwnrArray = cJSON_CreateArray());
            VERIFY_NON_NULL(TAG, jsonOwnrArray, ERROR);
            for (unsigned int i = 0; i < svc->ownersLen; i++)
            {
                outLen = 0;

                b64Ret = b64Encode(svc->owners[i].id, sizeof(((OicUuid_t*)0)->id), base64Buff,
                        sizeof(base64Buff), &outLen);
                VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);

                cJSON_AddItemToArray (jsonOwnrArray, cJSON_CreateString(base64Buff));
            }

            // Attach current svc node to Svc Array
            cJSON_AddItemToArray(jsonSvcArray, jsonSvc);
            svc = svc->next;
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
 * This internal method converts JSON SVC into binary SVC.
 */
OicSecSvc_t * JSONToSvcBin(const char * jsonStr)
{
    OCStackResult ret = OC_STACK_ERROR;
    OicSecSvc_t * headSvc = NULL;
    OicSecSvc_t * prevSvc = NULL;
    cJSON *jsonRoot = NULL;
    cJSON *jsonSvcArray = NULL;

    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    jsonRoot = cJSON_Parse(jsonStr);
    VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

    jsonSvcArray = cJSON_GetObjectItem(jsonRoot, OIC_JSON_SVC_NAME);
    VERIFY_NON_NULL(TAG, jsonSvcArray, INFO);

    if (cJSON_Array == jsonSvcArray->type)
    {
        int numSvc = cJSON_GetArraySize(jsonSvcArray);
        int idx = 0;

        VERIFY_SUCCESS(TAG, numSvc > 0, INFO);
        do
        {
            cJSON *jsonSvc = cJSON_GetArrayItem(jsonSvcArray, idx);
            VERIFY_NON_NULL(TAG, jsonSvc, ERROR);

            OicSecSvc_t *svc = (OicSecSvc_t*)OICCalloc(1, sizeof(OicSecSvc_t));
            VERIFY_NON_NULL(TAG, svc, ERROR);

            headSvc = (headSvc) ? headSvc : svc;
            if (prevSvc)
            {
                prevSvc->next = svc;
            }

            cJSON *jsonObj = NULL;

            unsigned char base64Buff[sizeof(((OicUuid_t*)0)->id)] = {0};
            uint32_t outLen = 0;
            B64Result b64Ret = B64_OK;

            // Service Device Identity
            jsonObj = cJSON_GetObjectItem(jsonSvc, OIC_JSON_SERVICE_DEVICE_ID);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_String == jsonObj->type, ERROR);
            outLen = 0;
            b64Ret = b64Decode(jsonObj->valuestring, strlen(jsonObj->valuestring), base64Buff,
                        sizeof(base64Buff), &outLen);
            VERIFY_SUCCESS(TAG, (b64Ret == B64_OK && outLen <= sizeof(svc->svcdid.id)), ERROR);
            memcpy(svc->svcdid.id, base64Buff, outLen);

            // Service Type
            jsonObj = cJSON_GetObjectItem(jsonSvc, OIC_JSON_SERVICE_TYPE);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_Number == jsonObj->type, ERROR);
            svc->svct = (OicSecSvcType_t)jsonObj->valueint;

            // Resource Owners
            jsonObj = cJSON_GetObjectItem(jsonSvc, OIC_JSON_OWNERS_NAME);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_Array == jsonObj->type, ERROR);

            svc->ownersLen = cJSON_GetArraySize(jsonObj);
            VERIFY_SUCCESS(TAG, svc->ownersLen > 0, ERROR);
            svc->owners = (OicUuid_t*)OICCalloc(svc->ownersLen, sizeof(OicUuid_t));
            VERIFY_NON_NULL(TAG, (svc->owners), ERROR);

            size_t idxx = 0;
            do
            {
                cJSON *jsonOwnr = cJSON_GetArrayItem(jsonObj, idxx);
                VERIFY_NON_NULL(TAG, jsonOwnr, ERROR);
                VERIFY_SUCCESS(TAG, cJSON_String == jsonOwnr->type, ERROR);

                outLen = 0;
                b64Ret = b64Decode(jsonOwnr->valuestring, strlen(jsonOwnr->valuestring), base64Buff,
                            sizeof(base64Buff), &outLen);

                VERIFY_SUCCESS(TAG, (b64Ret == B64_OK && outLen <= sizeof(svc->owners[idxx].id)),
                                    ERROR);
                memcpy(svc->owners[idxx].id, base64Buff, outLen);
            } while ( ++idxx < svc->ownersLen);

            prevSvc = svc;
        } while( ++idx < numSvc);
    }

    ret = OC_STACK_OK;

exit:
    cJSON_Delete(jsonRoot);
    if (OC_STACK_OK != ret)
    {
        DeleteSVCList(headSvc);
        headSvc = NULL;
    }
    return headSvc;
}

static OCEntityHandlerResult HandleSVCGetRequest (const OCEntityHandlerRequest * ehRequest)
{
    // Convert SVC data into JSON for transmission
    char* jsonStr = BinToSvcJSON(gSvc);

    OCEntityHandlerResult ehRet = (jsonStr ? OC_EH_OK : OC_EH_ERROR);

    // Send response payload to request originator
    SendSRMResponse(ehRequest, ehRet, jsonStr);

    OICFree(jsonStr);

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ehRet);
    return ehRet;
}

static OCEntityHandlerResult HandleSVCPostRequest (const OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;

    // Convert JSON SVC data into binary. This will also validate the SVC data received.
    OicSecSvc_t* newSvc = JSONToSvcBin(((OCSecurityPayload*)ehRequest->payload)->securityData);

    if (newSvc)
    {
        // Append the new SVC to existing SVC
        LL_APPEND(gSvc, newSvc);

        // Convert SVC data into JSON for update to persistent storage
        char *jsonStr = BinToSvcJSON(gSvc);
        if (jsonStr)
        {
            cJSON *jsonSvc = cJSON_Parse(jsonStr);
            OICFree(jsonStr);

            if ((jsonSvc) &&
                (OC_STACK_OK == UpdateSVRDatabase(OIC_JSON_SVC_NAME, jsonSvc)))
            {
                ehRet = OC_EH_RESOURCE_CREATED;
            }
            cJSON_Delete(jsonSvc);
        }
    }

    // Send payload to request originator
    SendSRMResponse(ehRequest, ehRet, NULL);

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ehRet);
    return ehRet;
}

/*
 * This internal method is the entity handler for SVC resources and
 * will handle REST request (GET/PUT/POST/DEL) for them.
 */
OCEntityHandlerResult SVCEntityHandler (OCEntityHandlerFlag flag,
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
        switch (ehRequest->method)
        {
            case OC_REST_GET:
                ehRet = HandleSVCGetRequest(ehRequest);
                break;

            case OC_REST_POST:
                ehRet = HandleSVCPostRequest(ehRequest);
                break;

            default:
                ehRet = OC_EH_ERROR;
                SendSRMResponse(ehRequest, ehRet, NULL);
        }
    }

    return ehRet;
}

/*
 * This internal method is used to create '/oic/sec/svc' resource.
 */
OCStackResult CreateSVCResource()
{
    OCStackResult ret;

    ret = OCCreateResource(&gSvcHandle,
                           OIC_RSRC_TYPE_SEC_SVC,
                           OIC_MI_DEF,
                           OIC_RSRC_SVC_URI,
                           SVCEntityHandler,
                           NULL,
                           OC_OBSERVABLE);

    if (OC_STACK_OK != ret)
    {
        OC_LOG (FATAL, TAG, "Unable to instantiate SVC resource");
        DeInitSVCResource();
    }
    return ret;
}


OCStackResult InitSVCResource()
{
    OCStackResult ret = OC_STACK_ERROR;

    OC_LOG_V (DEBUG, TAG, "Begin %s ", __func__ );

    // Read SVC resource from PS
    char* jsonSVRDatabase = GetSVRDatabase();

    if (jsonSVRDatabase)
    {
        // Convert JSON SVC into binary format
        gSvc = JSONToSvcBin(jsonSVRDatabase);
        OICFree(jsonSVRDatabase);
    }

    // Instantiate 'oic.sec.svc'
    ret = CreateSVCResource();

    if (OC_STACK_OK != ret)
    {
        DeInitSVCResource();
    }

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ret);
    return ret;
}

/**
 * Perform cleanup for SVC resources.
 *
 * @retval  none
 */
void DeInitSVCResource()
{
    OCDeleteResource(gSvcHandle);
    gSvcHandle = NULL;

    DeleteSVCList(gSvc);
    gSvc = NULL;
}

