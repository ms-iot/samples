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

#ifndef IOTVT_SRM_SVCR_H
#define IOTVT_SRM_SVCR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize SVC resource by loading data from persistent storage.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult InitSVCResource();

/**
 * Perform cleanup for SVC resources.
 *
 * @retval  none
 */
void DeInitSVCResource();

/**
 * This function converts SVC data into JSON format.
 * Caller needs to invoke 'free' when done using
 * returned string.
 * @param svc  instance of OicSecSvc_t structure.
 *
 * @retval  pointer to SVC in json format.
 */
char* BinToSvcJSON(const OicSecSvc_t * svc);

#ifdef __cplusplus
}
#endif

#endif //IOTVT_SRM_SVCR_H


