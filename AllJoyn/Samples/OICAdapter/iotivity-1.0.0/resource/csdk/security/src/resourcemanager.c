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

#include "resourcemanager.h"
#include "securevirtualresourcetypes.h"
#include "aclresource.h"
#include "pstatresource.h"
#include "doxmresource.h"
#include "credresource.h"
#include "svcresource.h"
#include "amaclresource.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "logger.h"
#include "utlist.h"
#include <string.h>

#define TAG "SRM-RM"

#ifdef __WITH_X509__
#include "crlresource.h"
#endif // __WITH_X509__

/**
 * This method is used by all secure resource modules to send responses to REST queries.
 *
 * @param ehRequest pointer to entity handler request data structure.
 * @param ehRet result code from entity handler.
 * @param rspPayload response payload in JSON.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult SendSRMResponse(const OCEntityHandlerRequest *ehRequest,
        OCEntityHandlerResult ehRet, const char *rspPayload)
{
    OC_LOG (DEBUG, TAG, "SRM sending SRM response");
    OCEntityHandlerResponse response = {.requestHandle = NULL};
    if (ehRequest)
    {
        OCSecurityPayload ocPayload = {.base = {.type = PAYLOAD_TYPE_INVALID}};

        response.requestHandle = ehRequest->requestHandle;
        response.resourceHandle = ehRequest->resource;
        response.ehResult = ehRet;
        response.payload = (OCPayload*)(&ocPayload);
        response.payload->type = PAYLOAD_TYPE_SECURITY;
        ((OCSecurityPayload*)response.payload)->securityData = (char *)rspPayload;
        response.persistentBufferFlag = 0;

        return OCDoResponse(&response);
    }
    return OC_STACK_ERROR;
}

/**
 * Initialize all secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult InitSecureResources( )
{
    OCStackResult ret;

    /*
     * doxm resource should be initialized first as it contains the DeviceID
     * which MAY be used during initialization of other resources.
     */

    ret = InitDoxmResource();

    if(OC_STACK_OK == ret)
    {
        ret = InitPstatResource();
    }
    if(OC_STACK_OK == ret)
    {
        ret = InitACLResource();
    }
    if(OC_STACK_OK == ret)
    {
        ret = InitCredResource();
    }
#ifdef __WITH_X509__
    if(OC_STACK_OK == ret)
    {
        ret = InitCRLResource();
    }
#endif // __WITH_X509__
    if(OC_STACK_OK == ret)
    {
        ret = InitSVCResource();
	}
	if(OC_STACK_OK == ret)
    {
        ret = InitAmaclResource();
    }
    if(OC_STACK_OK != ret)
    {
        //TODO: Update the default behavior if one of the SVR fails
        DestroySecureResources();
    }
    return ret;
}

/**
 * Perform cleanup for secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult DestroySecureResources( )
{
    DeInitACLResource();
    DeInitCredResource();
    DeInitDoxmResource();
    DeInitPstatResource();
#ifdef __WITH_X509__
    DeInitCRLResource();
#endif // __WITH_X509__
    DeInitSVCResource();
    DeInitAmaclResource();

    return OC_STACK_OK;
}
