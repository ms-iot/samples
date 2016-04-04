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
#include "aclresource.h"
#include "psinterface.h"
#include "utlist.h"
#include "srmresourcestrings.h"
#include "doxmresource.h"
#include "srmutility.h"
#include "ocserverrequest.h"
#include <stdlib.h>
#if defined(WITH_ARDUINO) || defined(WIN32)
#include <string.h>
#else
#include <strings.h>
#endif

#define TAG  "SRM-ACL"

OicSecAcl_t               *gAcl = NULL;
static OCResourceHandle    gAclHandle = NULL;

/**
 * This function frees OicSecAcl_t object's fields and object itself.
 */
static void FreeACE(OicSecAcl_t *ace)
{
    size_t i;
    if(NULL == ace)
    {
        OC_LOG (ERROR, TAG, "Invalid Parameter");
        return;
    }

    // Clean Resources
    for (i = 0; i < ace->resourcesLen; i++)
    {
        OICFree(ace->resources[i]);
    }
    OICFree(ace->resources);

    //Clean Period
    if(ace->periods)
    {
        for(i = 0; i < ace->prdRecrLen; i++)
        {
            OICFree(ace->periods[i]);
        }
        OICFree(ace->periods);
    }

    //Clean Recurrence
    if(ace->recurrences)
    {
        for(i = 0; i < ace->prdRecrLen; i++)
        {
            OICFree(ace->recurrences[i]);
        }
        OICFree(ace->recurrences);
    }

    // Clean Owners
    OICFree(ace->owners);

    // Clean ACL node itself
    OICFree(ace);
}

void DeleteACLList(OicSecAcl_t* acl)
{
    if (acl)
    {
        OicSecAcl_t *aclTmp1 = NULL;
        OicSecAcl_t *aclTmp2 = NULL;
        LL_FOREACH_SAFE(acl, aclTmp1, aclTmp2)
        {
            LL_DELETE(acl, aclTmp1);
            FreeACE(aclTmp1);
        }
    }
}

/*
 * This internal method converts ACL data into JSON format.
 *
 * Note: Caller needs to invoke 'free' when finished done using
 * return string.
 */
char * BinToAclJSON(const OicSecAcl_t * acl)
{
    cJSON *jsonRoot = NULL;
    char *jsonStr = NULL;

    if (acl)
    {
        jsonRoot = cJSON_CreateObject();
        VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

        cJSON *jsonAclArray = NULL;
        cJSON_AddItemToObject (jsonRoot, OIC_JSON_ACL_NAME, jsonAclArray = cJSON_CreateArray());
        VERIFY_NON_NULL(TAG, jsonAclArray, ERROR);

        while(acl)
        {
            char base64Buff[B64ENCODE_OUT_SAFESIZE(sizeof(((OicUuid_t*)0)->id)) + 1] = {0};
            uint32_t outLen = 0;
            size_t inLen = 0;
            B64Result b64Ret = B64_OK;

            cJSON *jsonAcl = cJSON_CreateObject();

            // Subject -- Mandatory
            outLen = 0;
            if (memcmp(&(acl->subject), &WILDCARD_SUBJECT_ID, sizeof(OicUuid_t)) == 0)
            {
                inLen = WILDCARD_SUBJECT_ID_LEN;
            }
            else
            {
                inLen =  sizeof(OicUuid_t);
            }
            b64Ret = b64Encode(acl->subject.id, inLen, base64Buff,
                sizeof(base64Buff), &outLen);
            VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);
            cJSON_AddStringToObject(jsonAcl, OIC_JSON_SUBJECT_NAME, base64Buff );

            // Resources -- Mandatory
            cJSON *jsonRsrcArray = NULL;
            cJSON_AddItemToObject (jsonAcl, OIC_JSON_RESOURCES_NAME, jsonRsrcArray = cJSON_CreateArray());
            VERIFY_NON_NULL(TAG, jsonRsrcArray, ERROR);
            for (size_t i = 0; i < acl->resourcesLen; i++)
            {
                cJSON_AddItemToArray (jsonRsrcArray, cJSON_CreateString(acl->resources[i]));
            }

            // Permissions -- Mandatory
            cJSON_AddNumberToObject (jsonAcl, OIC_JSON_PERMISSION_NAME, acl->permission);

            //Period & Recurrence -- Not Mandatory
            if(0 != acl->prdRecrLen)
            {
                cJSON *jsonPeriodArray = NULL;
                cJSON_AddItemToObject (jsonAcl, OIC_JSON_PERIODS_NAME,
                        jsonPeriodArray = cJSON_CreateArray());
                VERIFY_NON_NULL(TAG, jsonPeriodArray, ERROR);
                for (size_t i = 0; i < acl->prdRecrLen; i++)
                {
                    cJSON_AddItemToArray (jsonPeriodArray,
                            cJSON_CreateString(acl->periods[i]));
                }
            }

            //Recurrence -- Not Mandatory
            if(0 != acl->prdRecrLen && acl->recurrences)
            {
                cJSON *jsonRecurArray  = NULL;
                cJSON_AddItemToObject (jsonAcl, OIC_JSON_RECURRENCES_NAME,
                        jsonRecurArray = cJSON_CreateArray());
                VERIFY_NON_NULL(TAG, jsonRecurArray, ERROR);
                for (size_t i = 0; i < acl->prdRecrLen; i++)
                {
                    cJSON_AddItemToArray (jsonRecurArray,
                            cJSON_CreateString(acl->recurrences[i]));
                }
            }

            // Owners -- Mandatory
            cJSON *jsonOwnrArray = NULL;
            cJSON_AddItemToObject (jsonAcl, OIC_JSON_OWNERS_NAME, jsonOwnrArray = cJSON_CreateArray());
            VERIFY_NON_NULL(TAG, jsonOwnrArray, ERROR);
            for (size_t i = 0; i < acl->ownersLen; i++)
            {
                outLen = 0;

                b64Ret = b64Encode(acl->owners[i].id, sizeof(((OicUuid_t*)0)->id), base64Buff,
                    sizeof(base64Buff), &outLen);
                VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);

                cJSON_AddItemToArray (jsonOwnrArray, cJSON_CreateString(base64Buff));
            }

            // Attach current acl node to Acl Array
            cJSON_AddItemToArray(jsonAclArray, jsonAcl);
            acl = acl->next;
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
 * This internal method converts JSON ACL into binary ACL.
 */
OicSecAcl_t * JSONToAclBin(const char * jsonStr)
{
    OCStackResult ret = OC_STACK_ERROR;
    OicSecAcl_t * headAcl = NULL;
    OicSecAcl_t * prevAcl = NULL;
    cJSON *jsonRoot = NULL;
    cJSON *jsonAclArray = NULL;

    VERIFY_NON_NULL(TAG, jsonStr, ERROR);

    jsonRoot = cJSON_Parse(jsonStr);
    VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

    jsonAclArray = cJSON_GetObjectItem(jsonRoot, OIC_JSON_ACL_NAME);
    VERIFY_NON_NULL(TAG, jsonAclArray, ERROR);

    if (cJSON_Array == jsonAclArray->type)
    {
        int numAcl = cJSON_GetArraySize(jsonAclArray);
        int idx = 0;

        VERIFY_SUCCESS(TAG, numAcl > 0, INFO);
        do
        {
            cJSON *jsonAcl = cJSON_GetArrayItem(jsonAclArray, idx);
            VERIFY_NON_NULL(TAG, jsonAcl, ERROR);

            OicSecAcl_t *acl = (OicSecAcl_t*)OICCalloc(1, sizeof(OicSecAcl_t));
            VERIFY_NON_NULL(TAG, acl, ERROR);

            headAcl = (headAcl) ? headAcl : acl;
            if (prevAcl)
            {
                prevAcl->next = acl;
            }

            size_t jsonObjLen = 0;
            cJSON *jsonObj = NULL;

            unsigned char base64Buff[sizeof(((OicUuid_t*)0)->id)] = {0};
            uint32_t outLen = 0;
            B64Result b64Ret = B64_OK;

            // Subject -- Mandatory
            jsonObj = cJSON_GetObjectItem(jsonAcl, OIC_JSON_SUBJECT_NAME);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_String == jsonObj->type, ERROR);
            outLen = 0;
            b64Ret = b64Decode(jsonObj->valuestring, strlen(jsonObj->valuestring), base64Buff,
                        sizeof(base64Buff), &outLen);
            VERIFY_SUCCESS(TAG, (b64Ret == B64_OK && outLen <= sizeof(acl->subject.id)), ERROR);
            memcpy(acl->subject.id, base64Buff, outLen);

            // Resources -- Mandatory
            jsonObj = cJSON_GetObjectItem(jsonAcl, OIC_JSON_RESOURCES_NAME);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_Array == jsonObj->type, ERROR);

            acl->resourcesLen = cJSON_GetArraySize(jsonObj);
            VERIFY_SUCCESS(TAG, acl->resourcesLen > 0, ERROR);
            acl->resources = (char**)OICCalloc(acl->resourcesLen, sizeof(char*));
            VERIFY_NON_NULL(TAG, (acl->resources), ERROR);

            size_t idxx = 0;
            do
            {
                cJSON *jsonRsrc = cJSON_GetArrayItem(jsonObj, idxx);
                VERIFY_NON_NULL(TAG, jsonRsrc, ERROR);

                jsonObjLen = strlen(jsonRsrc->valuestring) + 1;
                acl->resources[idxx] = (char*)OICMalloc(jsonObjLen);
                VERIFY_NON_NULL(TAG, (acl->resources[idxx]), ERROR);
                OICStrcpy(acl->resources[idxx], jsonObjLen, jsonRsrc->valuestring);
            } while ( ++idxx < acl->resourcesLen);

            // Permissions -- Mandatory
            jsonObj = cJSON_GetObjectItem(jsonAcl,
                                OIC_JSON_PERMISSION_NAME);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_Number == jsonObj->type, ERROR);
            acl->permission = jsonObj->valueint;

            //Period -- Not Mandatory
            cJSON *jsonPeriodObj = cJSON_GetObjectItem(jsonAcl,
                    OIC_JSON_PERIODS_NAME);
            if(jsonPeriodObj)
            {
                VERIFY_SUCCESS(TAG, cJSON_Array == jsonPeriodObj->type,
                               ERROR);
                acl->prdRecrLen = cJSON_GetArraySize(jsonPeriodObj);
                if(acl->prdRecrLen > 0)
                {
                    acl->periods = (char**)OICCalloc(acl->prdRecrLen,
                                    sizeof(char*));
                    VERIFY_NON_NULL(TAG, acl->periods, ERROR);

                    cJSON *jsonPeriod = NULL;
                    for(size_t i = 0; i < acl->prdRecrLen; i++)
                    {
                        jsonPeriod = cJSON_GetArrayItem(jsonPeriodObj, i);
                        VERIFY_NON_NULL(TAG, jsonPeriod, ERROR);

                        jsonObjLen = strlen(jsonPeriod->valuestring) + 1;
                        acl->periods[i] = (char*)OICMalloc(jsonObjLen);
                        VERIFY_NON_NULL(TAG, acl->periods[i], ERROR);
                        OICStrcpy(acl->periods[i], jsonObjLen,
                                  jsonPeriod->valuestring);
                    }
                }
            }

            //Recurrence -- Not mandatory
            cJSON *jsonRecurObj = cJSON_GetObjectItem(jsonAcl,
                                        OIC_JSON_RECURRENCES_NAME);
            if(jsonRecurObj)
            {
                VERIFY_SUCCESS(TAG, cJSON_Array == jsonRecurObj->type,
                               ERROR);

                if(acl->prdRecrLen > 0)
                {
                    acl->recurrences = (char**)OICCalloc(acl->prdRecrLen,
                                             sizeof(char*));
                    VERIFY_NON_NULL(TAG, acl->recurrences, ERROR);

                    cJSON *jsonRecur = NULL;
                    for(size_t i = 0; i < acl->prdRecrLen; i++)
                    {
                        jsonRecur = cJSON_GetArrayItem(jsonRecurObj, i);
                        VERIFY_NON_NULL(TAG, jsonRecur, ERROR);
                        jsonObjLen = strlen(jsonRecur->valuestring) + 1;
                        acl->recurrences[i] = (char*)OICMalloc(jsonObjLen);
                        VERIFY_NON_NULL(TAG, acl->recurrences[i], ERROR);
                        OICStrcpy(acl->recurrences[i], jsonObjLen,
                              jsonRecur->valuestring);
                    }
                }
            }

            // Owners -- Mandatory
            jsonObj = cJSON_GetObjectItem(jsonAcl, OIC_JSON_OWNERS_NAME);
            VERIFY_NON_NULL(TAG, jsonObj, ERROR);
            VERIFY_SUCCESS(TAG, cJSON_Array == jsonObj->type, ERROR);

            acl->ownersLen = cJSON_GetArraySize(jsonObj);
            VERIFY_SUCCESS(TAG, acl->ownersLen > 0, ERROR);
            acl->owners = (OicUuid_t*)OICCalloc(acl->ownersLen, sizeof(OicUuid_t));
            VERIFY_NON_NULL(TAG, (acl->owners), ERROR);

            idxx = 0;
            do
            {
                cJSON *jsonOwnr = cJSON_GetArrayItem(jsonObj, idxx);
                VERIFY_NON_NULL(TAG, jsonOwnr, ERROR);
                VERIFY_SUCCESS(TAG, cJSON_String == jsonOwnr->type, ERROR);

                outLen = 0;
                b64Ret = b64Decode(jsonOwnr->valuestring, strlen(jsonOwnr->valuestring), base64Buff,
                            sizeof(base64Buff), &outLen);

                VERIFY_SUCCESS(TAG, (b64Ret == B64_OK && outLen <= sizeof(acl->owners[idxx].id)),
                                    ERROR);
                memcpy(acl->owners[idxx].id, base64Buff, outLen);
            } while ( ++idxx < acl->ownersLen);

            prevAcl = acl;
        } while( ++idx < numAcl);
    }

    ret = OC_STACK_OK;

exit:
    cJSON_Delete(jsonRoot);
    if (OC_STACK_OK != ret)
    {
        DeleteACLList(headAcl);
        headAcl = NULL;
    }
    return headAcl;
}

static bool UpdatePersistentStorage(const OicSecAcl_t *acl)
{
    // Convert ACL data into JSON for update to persistent storage
    char *jsonStr = BinToAclJSON(acl);
    if (jsonStr)
    {
        cJSON *jsonAcl = cJSON_Parse(jsonStr);
        OICFree(jsonStr);

        if ((jsonAcl) && (OC_STACK_OK == UpdateSVRDatabase(OIC_JSON_ACL_NAME, jsonAcl)))
        {
            return true;
        }
        cJSON_Delete(jsonAcl);
    }
    return false;
}

/*
 * This method removes ACE for the subject and resource from the ACL
 *
 * @param subject  - subject of the ACE
 * @param resource - resource of the ACE
 *
 * @return
 *     OC_STACK_RESOURCE_DELETED on success
 *     OC_STACK_NO_RESOURC on failure to find the appropriate ACE
 *     OC_STACK_INVALID_PARAM on invalid parameter
 */
static OCStackResult RemoveACE(const OicUuid_t * subject,
                               const char * resource)
{
    OC_LOG(DEBUG, TAG, "IN RemoveACE");

    OicSecAcl_t *acl = NULL;
    OicSecAcl_t *tempAcl = NULL;
    bool deleteFlag = false;
    OCStackResult ret = OC_STACK_NO_RESOURCE;

    if(memcmp(subject->id, &WILDCARD_SUBJECT_ID, sizeof(subject->id)) == 0)
    {
        OC_LOG_V (ERROR, TAG, "%s received invalid parameter", __func__ );
        return  OC_STACK_INVALID_PARAM;
    }

    //If resource is NULL then delete all the ACE for the subject.
    if(NULL == resource || resource[0] == '\0')
    {
        LL_FOREACH_SAFE(gAcl, acl, tempAcl)
        {
            if(memcmp(acl->subject.id, subject->id, sizeof(subject->id)) == 0)
            {
                LL_DELETE(gAcl, acl);
                FreeACE(acl);
                deleteFlag = true;
            }
        }
    }
    else
    {
        //Looping through ACL to find the right ACE to delete. If the required resource is the only
        //resource in the ACE for the subject then delete the whole ACE. If there are more resources
        //than the required resource in the ACE, for the subject then just delete the resource from
        //the resource array
        LL_FOREACH_SAFE(gAcl, acl, tempAcl)
        {
            if(memcmp(acl->subject.id, subject->id, sizeof(subject->id)) == 0)
            {
                if(1 == acl->resourcesLen && strcmp(acl->resources[0],  resource) == 0)
                {
                    LL_DELETE(gAcl, acl);
                    FreeACE(acl);
                    deleteFlag = true;
                    break;
                }
                else
                {
                    int resPos = -1;
                    size_t i;
                    for(i = 0; i < acl->resourcesLen; i++)
                    {
                        if(strcmp(acl->resources[i],  resource) == 0)
                        {
                            resPos = i;
                            break;
                        }
                    }
                    if((0 <= resPos))
                    {
                        OICFree(acl->resources[resPos]);
                        acl->resources[resPos] = NULL;
                        acl->resourcesLen -= 1;
                        for(i = resPos; i < acl->resourcesLen; i++)
                        {
                            acl->resources[i] = acl->resources[i+1];
                        }
                        deleteFlag = true;
                        break;
                    }
                }
            }
        }
    }

    if(deleteFlag)
    {
        if(UpdatePersistentStorage(gAcl))
        {
            ret = OC_STACK_RESOURCE_DELETED;
        }
    }
    return ret;
}

/*
 * This method parses the query string received for REST requests and
 * retrieves the 'subject' field.
 *
 * @param query querystring passed in REST request
 * @param subject subject UUID parsed from query string
 *
 * @return true if query parsed successfully and found 'subject', else false.
 */
static bool GetSubjectFromQueryString(const char *query, OicUuid_t *subject)
{
    OicParseQueryIter_t parseIter = {.attrPos=NULL};

    ParseQueryIterInit((unsigned char *)query, &parseIter);


    while(GetNextQuery(&parseIter))
    {
        if(strncasecmp((char *)parseIter.attrPos, OIC_JSON_SUBJECT_NAME, parseIter.attrLen) == 0)
        {
            VERIFY_SUCCESS(TAG, 0 != parseIter.valLen, ERROR);
            unsigned char base64Buff[sizeof(((OicUuid_t*)0)->id)] = {0};
            uint32_t outLen = 0;
            B64Result b64Ret = B64_OK;
            b64Ret = b64Decode((char *)parseIter.valPos, parseIter.valLen, base64Buff,
                    sizeof(base64Buff), &outLen);
            VERIFY_SUCCESS(TAG, (B64_OK == b64Ret && outLen <= sizeof(subject->id)), ERROR);
            memcpy(subject->id, base64Buff, outLen);

            return true;
        }
    }

exit:
   return false;
}

/*
 * This method parses the query string received for REST requests and
 * retrieves the 'resource' field.
 *
 * @param query querystring passed in REST request
 * @param resource resource parsed from query string
 * @param resourceSize size of the memory pointed to resource
 *
 * @return true if query parsed successfully and found 'resource', else false.
 */
static bool GetResourceFromQueryString(const char *query, char *resource, size_t resourceSize)
{
    OicParseQueryIter_t parseIter = {.attrPos=NULL};

    ParseQueryIterInit((unsigned char *)query, &parseIter);

    while(GetNextQuery(&parseIter))
    {
        if(strncasecmp((char *)parseIter.attrPos, OIC_JSON_RESOURCES_NAME, parseIter.attrLen) == 0)
        {
            VERIFY_SUCCESS(TAG, 0 != parseIter.valLen, ERROR);
            OICStrcpy(resource, resourceSize, (char *)parseIter.valPos);

            return true;
        }
    }

exit:
   return false;
}



static OCEntityHandlerResult HandleACLGetRequest (const OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    char* jsonStr = NULL;

    // Process the REST querystring parameters
    if(ehRequest->query)
    {
        OC_LOG (DEBUG, TAG, "HandleACLGetRequest processing query");

        OicUuid_t subject = {.id={0}};
        char resource[MAX_URI_LENGTH] = {0};

        OicSecAcl_t *savePtr = NULL;
        const OicSecAcl_t *currentAce = NULL;

        // 'Subject' field is MUST for processing a querystring in REST request.
        VERIFY_SUCCESS(TAG,
                       true == GetSubjectFromQueryString(ehRequest->query, &subject),
                       ERROR);

        GetResourceFromQueryString(ehRequest->query, resource, sizeof(resource));

        /*
         * TODO : Currently, this code only provides one ACE for a Subject.
         * Below code needs to be updated for scenarios when Subject have
         * multiple ACE's in ACL resource.
         */
        while((currentAce = GetACLResourceData(&subject, &savePtr)))
        {
            /*
             * If REST querystring contains a specific resource, we need
             * to search for that resource in ACE.
             */
            if (resource[0] != '\0')
            {
                for(size_t n = 0; n < currentAce->resourcesLen; n++)
                {
                    if((currentAce->resources[n]) &&
                            (0 == strcmp(resource, currentAce->resources[n]) ||
                             0 == strcmp(WILDCARD_RESOURCE_URI, currentAce->resources[n])))
                    {
                        // Convert ACL data into JSON for transmission
                        jsonStr = BinToAclJSON(currentAce);
                        goto exit;
                    }
                }
            }
            else
            {
                // Convert ACL data into JSON for transmission
                jsonStr = BinToAclJSON(currentAce);
                goto exit;
            }
        }
    }
    else
    {
        // Convert ACL data into JSON for transmission
        jsonStr = BinToAclJSON(gAcl);
    }

exit:
    ehRet = (jsonStr ? OC_EH_OK : OC_EH_ERROR);

    // Send response payload to request originator
    SendSRMResponse(ehRequest, ehRet, jsonStr);

    OICFree(jsonStr);

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ehRet);
    return ehRet;
}

static OCEntityHandlerResult HandleACLPostRequest (const OCEntityHandlerRequest * ehRequest)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;

    // Convert JSON ACL data into binary. This will also validate the ACL data received.
    OicSecAcl_t* newAcl = JSONToAclBin(((OCSecurityPayload*)ehRequest->payload)->securityData);

    if (newAcl)
    {
        // Append the new ACL to existing ACL
        LL_APPEND(gAcl, newAcl);

        if(UpdatePersistentStorage(gAcl))
        {
            ehRet = OC_EH_RESOURCE_CREATED;
        }
    }

    // Send payload to request originator
    SendSRMResponse(ehRequest, ehRet, NULL);

    OC_LOG_V (DEBUG, TAG, "%s RetVal %d", __func__ , ehRet);
    return ehRet;
}

static OCEntityHandlerResult HandleACLDeleteRequest(const OCEntityHandlerRequest *ehRequest)
{
    OC_LOG (DEBUG, TAG, "Processing ACLDeleteRequest");
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    OicUuid_t subject = {.id={0}};
    char resource[MAX_URI_LENGTH] = {0};

    VERIFY_NON_NULL(TAG, ehRequest->query, ERROR);

    // 'Subject' field is MUST for processing a querystring in REST request.
    VERIFY_SUCCESS(TAG,
            true == GetSubjectFromQueryString(ehRequest->query, &subject),
            ERROR);

    GetResourceFromQueryString(ehRequest->query, resource, sizeof(resource));

    if(OC_STACK_RESOURCE_DELETED == RemoveACE(&subject, resource))
    {
        ehRet = OC_EH_RESOURCE_DELETED;
    }

exit:
    // Send payload to request originator
    SendSRMResponse(ehRequest, ehRet, NULL);

    return ehRet;
}

/*
 * This internal method is the entity handler for ACL resources and
 * will handle REST request (GET/PUT/POST/DEL) for them.
 */
OCEntityHandlerResult ACLEntityHandler (OCEntityHandlerFlag flag,
                                        OCEntityHandlerRequest * ehRequest,
                                        void* callbackParameter)
{
    OC_LOG(DEBUG, TAG, "Received request ACLEntityHandler");
    (void)callbackParameter;
    OCEntityHandlerResult ehRet = OC_EH_ERROR;

    if (!ehRequest)
    {
        return ehRet;
    }

    if (flag & OC_REQUEST_FLAG)
    {
        // TODO :  Handle PUT method
        OC_LOG (DEBUG, TAG, "Flag includes OC_REQUEST_FLAG");
        switch (ehRequest->method)
        {
            case OC_REST_GET:
                ehRet = HandleACLGetRequest(ehRequest);
                break;

            case OC_REST_POST:
                ehRet = HandleACLPostRequest(ehRequest);
                break;

            case OC_REST_DELETE:
                ehRet = HandleACLDeleteRequest(ehRequest);
                break;

            default:
                ehRet = OC_EH_ERROR;
                SendSRMResponse(ehRequest, ehRet, NULL);
        }
    }

    return ehRet;
}

/*
 * This internal method is used to create '/oic/sec/acl' resource.
 */
OCStackResult CreateACLResource()
{
    OCStackResult ret;

    ret = OCCreateResource(&gAclHandle,
                           OIC_RSRC_TYPE_SEC_ACL,
                           OIC_MI_DEF,
                           OIC_RSRC_ACL_URI,
                           ACLEntityHandler,
                           NULL,
                           OC_OBSERVABLE | OC_SECURE | OC_EXPLICIT_DISCOVERABLE);

    if (OC_STACK_OK != ret)
    {
        OC_LOG (FATAL, TAG, "Unable to instantiate ACL resource");
        DeInitACLResource();
    }
    return ret;
}

/*
 * This internal method is to retrieve the default ACL.
 * If SVR database in persistent storage got corrupted or
 * is not available for some reason, a default ACL is created
 * which allows user to initiate ACL provisioning again.
 */
OCStackResult  GetDefaultACL(OicSecAcl_t** defaultAcl)
{
    OCStackResult ret = OC_STACK_ERROR;

    OicUuid_t ownerId = {.id = {0}};

    /*
     * TODO In future, when new virtual resources will be added in OIC
     * specification, Iotivity stack should be able to add them in
     * existing SVR database. To support this, we need to add 'versioning'
     * mechanism in SVR database.
     */

    const char *rsrcs[] = {
        OC_RSRVD_WELL_KNOWN_URI,
        OC_RSRVD_DEVICE_URI,
        OC_RSRVD_PLATFORM_URI,
        OC_RSRVD_RESOURCE_TYPES_URI,
#ifdef WITH_PRESENCE
        OC_RSRVD_PRESENCE_URI,
#endif //WITH_PRESENCE
        OIC_RSRC_ACL_URI,
        OIC_RSRC_DOXM_URI,
        OIC_RSRC_PSTAT_URI,
    };

    if (!defaultAcl)
    {
        return OC_STACK_INVALID_PARAM;
    }

    OicSecAcl_t *acl = (OicSecAcl_t *)OICCalloc(1, sizeof(OicSecAcl_t));
    VERIFY_NON_NULL(TAG, acl, ERROR);

    // Subject -- Mandatory
    memcpy(&(acl->subject), &WILDCARD_SUBJECT_ID, sizeof(acl->subject));

    // Resources -- Mandatory
    acl->resourcesLen = sizeof(rsrcs)/sizeof(rsrcs[0]);

    acl->resources = (char**)OICCalloc(acl->resourcesLen, sizeof(char*));
    VERIFY_NON_NULL(TAG, (acl->resources), ERROR);

    for (size_t i = 0; i <  acl->resourcesLen; i++)
    {
        size_t len = strlen(rsrcs[i]) + 1;
        acl->resources[i] = (char*)OICMalloc(len * sizeof(char));
        VERIFY_NON_NULL(TAG, (acl->resources[i]), ERROR);
        OICStrcpy(acl->resources[i], len, rsrcs[i]);
    }

    acl->permission = PERMISSION_READ;
    acl->prdRecrLen = 0;
    acl->periods = NULL;
    acl->recurrences = NULL;

    // Device ID is the owner of this default ACL
    ret = GetDoxmDeviceID( &ownerId);
    VERIFY_SUCCESS(TAG, OC_STACK_OK == ret, FATAL);

    acl->ownersLen = 1;
    acl->owners = (OicUuid_t*)OICMalloc(sizeof(OicUuid_t));
    VERIFY_NON_NULL(TAG, (acl->owners), ERROR);
    memcpy(acl->owners, &ownerId, sizeof(OicUuid_t));

    acl->next = NULL;

    *defaultAcl = acl;
    ret = OC_STACK_OK;

exit:

    if (ret != OC_STACK_OK)
    {
        DeleteACLList(acl);
        acl = NULL;
    }

    return ret;
}

/**
 * Initialize ACL resource by loading data from persistent storage.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult InitACLResource()
{
    OCStackResult ret = OC_STACK_ERROR;

    // Read ACL resource from PS
    char* jsonSVRDatabase = GetSVRDatabase();

    if (jsonSVRDatabase)
    {
        // Convert JSON ACL into binary format
        gAcl = JSONToAclBin(jsonSVRDatabase);
        OICFree(jsonSVRDatabase);
    }
    /*
     * If SVR database in persistent storage got corrupted or
     * is not available for some reason, a default ACL is created
     * which allows user to initiate ACL provisioning again.
     */
    if (!jsonSVRDatabase || !gAcl)
    {
        GetDefaultACL(&gAcl);
        // TODO Needs to update persistent storage
    }
    VERIFY_NON_NULL(TAG, gAcl, FATAL);

    // Instantiate 'oic.sec.acl'
    ret = CreateACLResource();

exit:
    if (OC_STACK_OK != ret)
    {
        DeInitACLResource();
    }
    return ret;
}

/**
 * Perform cleanup for ACL resources.
 *
 * @retval  none
 */
void DeInitACLResource()
{
    OCDeleteResource(gAclHandle);
    gAclHandle = NULL;

    DeleteACLList(gAcl);
    gAcl = NULL;
}

/**
 * This method is used by PolicyEngine to retrieve ACL for a Subject.
 *
 * @param subjectId ID of the subject for which ACL is required.
 * @param savePtr is used internally by @ref GetACLResourceData to maintain index between
 *                successive calls for same subjectId.
 *
 * @retval  reference to @ref OicSecAcl_t if ACL is found, else NULL
 *
 * @note On the first call to @ref GetACLResourceData, savePtr should point to NULL
 */
const OicSecAcl_t* GetACLResourceData(const OicUuid_t* subjectId, OicSecAcl_t **savePtr)
{
    OicSecAcl_t *acl = NULL;
    OicSecAcl_t *begin = NULL;

    if ( NULL == subjectId)
    {
        return NULL;
    }

    /*
     * savePtr MUST point to NULL if this is the 'first' call to retrieve ACL for
     * subjectID.
     */
    if (NULL == *savePtr)
    {
        begin = gAcl;
    }
    else
    {
        /*
         * If this is a 'successive' call, search for location pointed by
         * savePtr and assign 'begin' to the next ACL after it in the linked
         * list and start searching from there.
         */
        LL_FOREACH(gAcl, acl)
        {
            if (acl == *savePtr)
            {
                begin = acl->next;
            }
        }
    }

    // Find the next ACL corresponding to the 'subjectID' and return it.
    LL_FOREACH(begin, acl)
    {
        if (memcmp(&(acl->subject), subjectId, sizeof(OicUuid_t)) == 0)
        {
            *savePtr = acl;
            return acl;
        }
    }

    // Cleanup in case no ACL is found
    *savePtr = NULL;
    return NULL;
}


OCStackResult InstallNewACL(const char* newJsonStr)
{
    OCStackResult ret = OC_STACK_ERROR;

    // Convert JSON ACL data into binary. This will also validate the ACL data received.
    OicSecAcl_t* newAcl = JSONToAclBin(newJsonStr);

    if (newAcl)
    {
        // Append the new ACL to existing ACL
        LL_APPEND(gAcl, newAcl);

        // Convert ACL data into JSON for update to persistent storage
        char *jsonStr = BinToAclJSON(gAcl);
        if (jsonStr)
        {
            cJSON *jsonAcl = cJSON_Parse(jsonStr);
            OICFree(jsonStr);

            if (jsonAcl)
            {
                ret = UpdateSVRDatabase(OIC_JSON_ACL_NAME, jsonAcl);
            }
            cJSON_Delete(jsonAcl);
        }
    }

    return ret;
}
