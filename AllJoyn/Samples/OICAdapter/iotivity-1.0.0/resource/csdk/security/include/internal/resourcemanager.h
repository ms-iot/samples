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

#ifndef IOTVT_SRM_RM_H
#define IOTVT_SRM_RM_H

#include <stdlib.h>
#include "ocstack.h"
#include "securevirtualresourcetypes.h"

/**
 * Initialize all secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult InitSecureResources();

/**
 * Perform cleanup for secure resources ( /oic/sec/cred, /oic/sec/acl, /oic/sec/pstat etc).
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult DestroySecureResources();

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
        OCEntityHandlerResult ehRet, const char *rspPayload);

#endif //IOTVT_SRM_RM_H


