//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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
#include "cJSON.h"
#include "base64.h"
#include "resourcemanager.h"
#include "psinterface.h"
#include "utlist.h"
#include "srmresourcestrings.h"
#include "doxmresource.h"
#include "srmutility.h"
#ifdef __WITH_X509__
#include "crlresource.h"
#include "crl.h"
#endif /* __WITH_X509__ */

#define TAG  "SRM-CRL"

#define SEPARATOR                   ":"
#define SEPARATOR_LEN               (1)
#define JSON_CRL_NAME               "\"CRL\""
#define JSON_CRL_NAME_LEN           (5)
#define OIC_JSON_CRL_NAME           "crl"
#define OIC_JSON_CRL_ID             "CRLId"
#define OIC_JSON_CRL_THIS_UPDATE    "ThisUpdate"
#define OIC_JSON_CRL_DATA           "CRLData"
#define CRL_DEFAULT_CRL_ID           1
#define CRL_DEFAULT_THIS_UPDATE     "150101000000Z"
#define CRL_DEFAULT_CRL_DATA        "-"

static OCResourceHandle     gCrlHandle  = NULL;
static OicSecCrl_t         *gCrl        = NULL;


void DeleteCrlBinData(OicSecCrl_t *crl)
{
    if (crl)
    {
        //Clean ThisUpdate
        OICFree(crl->ThisUpdate.data);

        //clean CrlData
        OICFree(crl->CrlData.data);

        //Clean crl itself
        OICFree(crl);
    }
}

char *BinToCrlJSON(const OicSecCrl_t *crl)
{
    if (NULL == crl)
    {
        return NULL;
    }

    char *base64Buff = NULL;
    uint32_t outLen = 0;
    uint32_t base64CRLLen = 0;
    B64Result b64Ret = B64_OK;
    char *jsonStr = NULL;
    cJSON *jsonRoot = cJSON_CreateObject();
    VERIFY_NON_NULL(TAG, jsonRoot, ERROR);
    cJSON *jsonCrl = cJSON_CreateObject();
    VERIFY_NON_NULL(TAG, jsonCrl, ERROR);

    cJSON_AddItemToObject(jsonRoot, OIC_JSON_CRL_NAME, jsonCrl);

    //CRLId -- Mandatory
    cJSON_AddNumberToObject(jsonCrl, OIC_JSON_CRL_ID, (int)crl->CrlId);

    //ThisUpdate -- Mandatory
    outLen = 0;
    base64CRLLen = B64ENCODE_OUT_SAFESIZE(crl->ThisUpdate.len);
    base64Buff = OICMalloc(base64CRLLen);
    b64Ret = b64Encode(crl->ThisUpdate.data, crl->ThisUpdate.len, base64Buff,
             base64CRLLen, &outLen);
    VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);
    cJSON_AddStringToObject(jsonCrl, OIC_JSON_CRL_THIS_UPDATE, base64Buff);
    OICFree(base64Buff);

    //CRLData -- Mandatory
    outLen = 0;
    base64CRLLen = B64ENCODE_OUT_SAFESIZE(crl->CrlData.len);
    base64Buff = OICMalloc(base64CRLLen);
    b64Ret = b64Encode(crl->CrlData.data, crl->CrlData.len, base64Buff,
             base64CRLLen, &outLen);
    VERIFY_SUCCESS(TAG, b64Ret == B64_OK, ERROR);
    cJSON_AddStringToObject(jsonCrl, OIC_JSON_CRL_DATA, base64Buff);

    jsonStr = cJSON_PrintUnformatted(jsonRoot);

exit:
    OICFree(base64Buff);
    if (jsonRoot)
    {
        cJSON_Delete(jsonRoot);
    }
    return jsonStr;
}

OicSecCrl_t *JSONToCrlBin(const char * jsonStr)
{
    if (NULL == jsonStr)
    {
        return NULL;
    }

    OCStackResult ret = OC_STACK_ERROR;
    OicSecCrl_t *crl =  NULL;
    cJSON *jsonCrl = NULL;
    cJSON *jsonObj = NULL;

    unsigned char *base64Buff = NULL;
    uint32_t base64CRLLen = 0;
    uint32_t outLen = 0;
    B64Result b64Ret = B64_OK;

    cJSON *jsonRoot = cJSON_Parse(jsonStr);
    VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

    jsonCrl = cJSON_GetObjectItem(jsonRoot, OIC_JSON_CRL_NAME);
    VERIFY_NON_NULL(TAG, jsonCrl, ERROR);
    crl = (OicSecCrl_t *)OICCalloc(1, sizeof(OicSecCrl_t));
    VERIFY_NON_NULL(TAG, crl, ERROR);

    //CRLId -- Mandatory
    jsonObj = cJSON_GetObjectItem(jsonCrl, OIC_JSON_CRL_ID);
    if(jsonObj)
    {
        VERIFY_SUCCESS(TAG, cJSON_Number == jsonObj->type, ERROR);
        crl->CrlId = (uint16_t)jsonObj->valueint;
    }
    else // PUT/POST JSON may not have CRLId so set it to the gCRList->CRLId
    {
        VERIFY_NON_NULL(TAG, gCrl, ERROR);
        crl->CrlId = gCrl->CrlId;
    }

    //ThisUpdate -- Mandatory
    jsonObj = cJSON_GetObjectItem(jsonCrl, OIC_JSON_CRL_THIS_UPDATE);
    if(jsonObj)
    {
        VERIFY_SUCCESS(TAG, cJSON_String == jsonObj->type, ERROR);
        if(cJSON_String == jsonObj->type)
        {
            //Check for empty string, in case ThisUpdate field has not been set yet
            if (jsonObj->valuestring[0])
            {
                base64CRLLen = B64ENCODE_OUT_SAFESIZE(strlen(jsonObj->valuestring));
                base64Buff = OICMalloc(base64CRLLen);
                b64Ret = b64Decode(jsonObj->valuestring, strlen(jsonObj->valuestring), base64Buff,
                        base64CRLLen, &outLen);
                VERIFY_SUCCESS(TAG, (b64Ret == B64_OK && outLen <= base64CRLLen),
                                ERROR);
                crl->ThisUpdate.data = OICMalloc(outLen + 1);
                memcpy(crl->ThisUpdate.data, base64Buff, outLen);
                crl->ThisUpdate.len = outLen;
                OICFree(base64Buff);
                base64Buff = NULL;
            }
        }
    }
    else // PUT/POST JSON will not have ThisUpdate so set it to the gCRList->ThisUpdate
    {
        VERIFY_NON_NULL(TAG, gCrl, ERROR);
        outLen = gCrl->ThisUpdate.len;
        crl->ThisUpdate.data = OICMalloc(outLen + 1);
        memcpy(crl->ThisUpdate.data, gCrl->ThisUpdate.data, outLen);
        crl->ThisUpdate.len = outLen;
    }

    //CRLData -- Mandatory
    jsonObj = cJSON_GetObjectItem(jsonCrl, OIC_JSON_CRL_DATA);
    if(jsonObj)
    {
        VERIFY_SUCCESS(TAG, cJSON_String == jsonObj->type, ERROR);
        if(cJSON_String == jsonObj->type)
        {
            //Check for empty string, in case CRLData field has not been set yet
            if (jsonObj->valuestring[0])
            {
                outLen = 0;
                base64CRLLen = B64ENCODE_OUT_SAFESIZE(strlen(jsonObj->valuestring));
                base64Buff = OICMalloc(base64CRLLen);
                b64Ret = b64Decode(jsonObj->valuestring, strlen(jsonObj->valuestring), base64Buff,
                        base64CRLLen, &outLen);
                VERIFY_SUCCESS(TAG, (b64Ret == B64_OK && outLen <= base64CRLLen),
                                ERROR);
                crl->CrlData.data = OICMalloc(outLen + 1);
                memcpy(crl->CrlData.data, base64Buff, outLen);
                crl->CrlData.len = outLen;
                OICFree(base64Buff);
                base64Buff = NULL;
            }
        }
    }
    else // PUT/POST JSON will not have CRLData so set it to the gCRList->CRLData
    {
        VERIFY_NON_NULL(TAG, gCrl, ERROR);
        outLen = gCrl->CrlData.len;
        crl->CrlData.data = OICMalloc(outLen + 1);
        memcpy(crl->CrlData.data, gCrl->CrlData.data, outLen);
        crl->CrlData.len = outLen;
    }

    ret = OC_STACK_OK;
exit:
    cJSON_Delete(jsonRoot);
    OICFree(base64Buff);
    base64Buff = NULL;
    if (OC_STACK_OK != ret)
    {
        DeleteCrlBinData(crl);
        crl = NULL;
    }
    return crl;
}

OCStackResult UpdateCRLResource(const OicSecCrl_t *crl)
{
    char *jsonStr = NULL;
    OCStackResult res = OC_STACK_ERROR;

    jsonStr = BinToCrlJSON((OicSecCrl_t *) crl);
    if (!jsonStr)
    {
        return OC_STACK_ERROR;
    }

    cJSON *jsonObj = cJSON_Parse(jsonStr);
    OICFree(jsonStr);

    if (jsonObj == NULL)
    {
        return OC_STACK_ERROR;
    }

    res = UpdateSVRDatabase(OIC_JSON_CRL_NAME, jsonObj);
    cJSON_Delete(jsonObj);

    return res;
}

static OCEntityHandlerResult HandleCRLPostRequest(const OCEntityHandlerRequest *ehRequest)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;

    char *jsonCRL = (char *)(((OCSecurityPayload *)ehRequest->payload)->securityData);

    if (jsonCRL)
    {
        OC_LOG(INFO, TAG, "UpdateSVRDB...");
        OC_LOG_V(INFO, TAG, "crl: \"%s\"", jsonCRL);

        cJSON *jsonObj = cJSON_Parse(jsonCRL);
        OicSecCrl_t *crl = NULL;
        crl = JSONToCrlBin(jsonCRL);
        if (!crl)
        {
            OC_LOG(ERROR, TAG, "Error JSONToCrlBin");
        }

        gCrl->CrlId = crl->CrlId;

        OICFree(gCrl->ThisUpdate.data);
        gCrl->ThisUpdate.data = NULL;
        gCrl->ThisUpdate.data = OICMalloc(crl->ThisUpdate.len);
        memcpy(gCrl->ThisUpdate.data, crl->ThisUpdate.data, crl->ThisUpdate.len);
        gCrl->ThisUpdate.len = crl->ThisUpdate.len;

        OICFree(gCrl->CrlData.data);
        gCrl->CrlData.data = NULL;
        gCrl->CrlData.data = OICMalloc(crl->CrlData.len);
        memcpy(gCrl->CrlData.data, crl->CrlData.data, crl->CrlData.len);
        gCrl->CrlData.len = crl->CrlData.len;

        if (OC_STACK_OK == UpdateSVRDatabase(OIC_JSON_CRL_NAME, jsonObj))
        {
            OC_LOG(INFO, TAG, "UpdateSVRDB == OK");
            ehRet = OC_EH_RESOURCE_CREATED;
        }

        DeleteCrlBinData(crl);
        cJSON_Delete(jsonObj);

    }

    // Send payload to request originator
    SendSRMResponse(ehRequest, ehRet, NULL);

    OC_LOG_V(INFO, TAG, "%s RetVal %d", __func__, ehRet);
    return ehRet;
}


/*
 * This internal method is the entity handler for CRL resource and
 * will handle REST request (GET/PUT/POST/DEL) for them.
 */
OCEntityHandlerResult CRLEntityHandler(OCEntityHandlerFlag flag,
                                       OCEntityHandlerRequest *ehRequest,
                                       void *callbackParameter)
{
    OCEntityHandlerResult ehRet = OC_EH_ERROR;
    (void)callbackParameter;

    if (!ehRequest)
    {
        return ehRet;
    }

    OC_LOG(INFO, TAG, "Handle CRL resource");

    if (flag & OC_REQUEST_FLAG)
    {
        // TODO :  Handle PUT and DEL methods
        OC_LOG (INFO, TAG, "Flag includes OC_REQUEST_FLAG");
        switch (ehRequest->method)
        {
            case OC_REST_GET:
                OC_LOG (INFO, TAG, "Not implemented request method.");
                //ehRet = HandleCRLGetRequest(ehRequest);
                break;

            case OC_REST_POST:
                ehRet = HandleCRLPostRequest(ehRequest);
                break;

            default:
                ehRet = OC_EH_ERROR;
                SendSRMResponse(ehRequest, ehRet, NULL);
        }
    }

    return ehRet;
}

/*
 * This internal method is used to create '/oic/sec/crl' resource.
 */
OCStackResult CreateCRLResource()
{
    OCStackResult ret;
    ret = OCCreateResource(&gCrlHandle,
                           OIC_RSRC_TYPE_SEC_CRL,
                           OIC_MI_DEF,
                           OIC_RSRC_CRL_URI,
                           CRLEntityHandler,
                           NULL,
                           OC_OBSERVABLE);

    if (OC_STACK_OK != ret)
    {
        OC_LOG(FATAL, TAG, "Unable to instantiate CRL resource");
        DeInitCRLResource();
    }
    return ret;
}

/**
 * Get the default value
 * @retval  NULL for now. Update it when we finalize the default info.
 */
static OicSecCrl_t *GetCrlDefault()
{
    OicSecCrl_t *defaultCrl = NULL;
    defaultCrl = (OicSecCrl_t *)OICCalloc(1, sizeof(OicSecCrl_t));

    defaultCrl->CrlId = CRL_DEFAULT_CRL_ID;

    defaultCrl->CrlData.len = strlen(CRL_DEFAULT_CRL_DATA);
    defaultCrl->CrlData.data = OICMalloc(defaultCrl->CrlData.len);
    memcpy(defaultCrl->CrlData.data, CRL_DEFAULT_CRL_DATA, defaultCrl->CrlData.len);

    defaultCrl->ThisUpdate.len = strlen(CRL_DEFAULT_THIS_UPDATE);
    defaultCrl->ThisUpdate.data = OICMalloc(defaultCrl->ThisUpdate.len);
    memcpy(defaultCrl->ThisUpdate.data, CRL_DEFAULT_THIS_UPDATE, defaultCrl->ThisUpdate.len);

    return defaultCrl;
}

/**
 * Initialize CRL resource by loading data from persistent storage.
 *
 * @retval
 *     OC_STACK_OK    - no errors
 *     OC_STACK_ERROR - stack process error
 */
OCStackResult InitCRLResource()
{
    OCStackResult ret = OC_STACK_ERROR;
    char* jsonSVRDatabase;

    //Read CRL resource from PS
    jsonSVRDatabase = GetSVRDatabase();

    if (jsonSVRDatabase)
    {
        //Convert JSON CRL into binary format
        gCrl = JSONToCrlBin(jsonSVRDatabase);
    }
    /*
     * If SVR database in persistent storage got corrupted or
     * is not available for some reason, a default CrlResource is created
     * which allows user to initiate CrlResource provisioning again.
     */
    if (!jsonSVRDatabase || !gCrl)
    {
        gCrl = GetCrlDefault();
    }

    ret = CreateCRLResource();
    OICFree(jsonSVRDatabase);
    return ret;
}

/**
 * Perform cleanup for ACL resources.
 */
void DeInitCRLResource()
{
    OCDeleteResource(gCrlHandle);
    gCrlHandle = NULL;
    DeleteCrlBinData(gCrl);
    gCrl = NULL;
}

OicSecCrl_t *GetCRLResource()
{
    OicSecCrl_t *crl =  NULL;

    //Read CRL resource from PS
    char* jsonSVRDatabase = GetSVRDatabase();

    if (jsonSVRDatabase)
    {
        //Convert JSON CRL into binary format
        crl = JSONToCrlBin(jsonSVRDatabase);
    }
    /*
     * If SVR database in persistent storage got corrupted or
     * is not available for some reason, a default CrlResource is created
     * which allows user to initiate CrlResource provisioning again.
     */
    if (!jsonSVRDatabase || !crl)
    {
        crl = GetCrlDefault();
    }
    OICFree(jsonSVRDatabase);

    return crl;
}

char *GetBase64CRL()
{
    cJSON *jsonCrl = NULL;
    cJSON *jsonObj = NULL;
    char *jsonSVRDatabase = GetSVRDatabase();
    char* ret = NULL;

    cJSON *jsonRoot = cJSON_Parse(jsonSVRDatabase);
    VERIFY_NON_NULL(TAG, jsonRoot, ERROR);

    jsonCrl = cJSON_GetObjectItem(jsonRoot, OIC_JSON_CRL_NAME);
    VERIFY_NON_NULL(TAG, jsonCrl, ERROR);

    //CRLData -- Mandatory
    jsonObj = cJSON_GetObjectItem(jsonCrl, OIC_JSON_CRL_DATA);
    if(jsonObj)
    {
        VERIFY_SUCCESS(TAG, cJSON_String == jsonObj->type, ERROR);
        if(cJSON_String == jsonObj->type)
        {
            //Check for empty string, in case CRLData field has not been set yet
            if (jsonObj->valuestring[0])
            {
                ret = jsonObj->valuestring;
            }
        }
    }
exit:
    OICFree(jsonSVRDatabase);
    cJSON_Delete(jsonRoot);
    return ret;
}

void  GetDerCrl(ByteArray crlArray)
{
    OicSecCrl_t * crlRes = GetCRLResource();
    if (crlRes && crlRes->CrlData.len <= crlArray.len)
    {
        memcpy(crlArray.data, crlRes->CrlData.data, crlRes->CrlData.len);
        crlArray.len = crlRes->CrlData.len;
    }
    else
    {
        crlArray.len = 0;
    }
    DeleteCrlBinData(crlRes);
}
