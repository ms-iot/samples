/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      LICENSE-2.0" target="_blank">http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *

 ******************************************************************/
#ifndef _SN_STORE_H_
#define _SN_STORE_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "pki_errors.h"
#include "byte_array.h"


/**
 * Stores serial number to SN storage.
 *
 * @param[in] serNum certificate serial number to be stored
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError StoreSerialNumber(const ByteArray serNum);

/**
 * Check whether there is \a serNum in SN storage.
 *
 * @param[in] serNum certificate serial number to be stored
 * @return PKI_SUCCESS if \a not belongs SN storage, error code otherwise
 */
PKIError CheckSerialNumber(const ByteArray serNum);


#ifdef X509_DEBUG
/**
 * Prints all serial numbers from SN storage.
 */
void PrintSNStore(void);
#endif


/**
 * Frees memory occupied by SN storage.
 */
void FreeSNStore(void);


#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_SN_STORE_H_

