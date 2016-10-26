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

#ifndef IOTVT_SRM_PSI_H
#define IOTVT_SRM_PSI_H

/**
 * Reads the Secure Virtual Database from PS into dynamically allocated
 * memory buffer.
 *
 * @note Caller of this method MUST use OCFree() method to release memory
 *       referenced by return value.
 *
 * @retval  reference to memory buffer containing SVR database.
 */
char * GetSVRDatabase();

/**
 * This method is used by a entity handlers of SVR's to update
 * SVR database.
 *
 * @param rsrcName string denoting the SVR name ("acl", "cred", "pstat" etc).
 * @param jsonObj JSON object containing the SVR contents.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult UpdateSVRDatabase(const char* rsrcName, cJSON* jsonObj);

#endif //IOTVT_SRM_PSI_H
