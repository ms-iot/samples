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

#ifndef IOTVT_SRM_PSTATR_H
#define IOTVT_SRM_PSTATR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize Pstat resource by loading data from persistent storage.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult InitPstatResource();

/**
 * Perform cleanup for Pstat resources.
 *
 * @retval  OC_STACK_OK for Success, otherwise some error value
 */
OCStackResult DeInitPstatResource();

/**
 * This method converts JSON PSTAT into binary PSTAT.
 *
 * @param[in] jsonStr  pstat data in json string.
 * @return pointer to OicSecPstat_t.
 */
OicSecPstat_t * JSONToPstatBin(const char * jsonStr);

/**
 * This method converts pstat data into JSON format.
 *
 * @param[in] pstat  pstat data in binary format.
 * @return pointer to pstat json string.
 */
char * BinToPstatJSON(const OicSecPstat_t * pstat);

/** This function deallocates the memory for OicSecPstat_t.
 *
 * @param[in] pstat  Pointer to OicSecPstat_t.
 */
void DeletePstatBinData(OicSecPstat_t* pstat);


#ifdef __cplusplus
}
#endif

#endif //IOTVT_SRM_PSTATR_H


