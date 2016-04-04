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


#ifndef INCLUDE_MASTER_CSR_H_
#define INCLUDE_MASTER_CSR_H_

#include "CertificationRequest.h" /* CertificationRequest ASN.1 type */
#include "pki.h"
#include "pki_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory allocated for DER encoded CSR
#define CSR_MAX_SIZE   1000

#define CSR_DEFAULT_VERSION 0
#define CSR_MAX_NAME_SIZE 100

/**
 * Encode certificate signing request with specified parameters.
 *
 * @param[in] subjectName pointer to subject's common name
 * @param[in] subjectPublicKey pointer to subject's public key to be signed
 * @param[in] subjectPrivateKey pointer to subject's private key to be signed
 * @param[out] encodedCSR pointer to allocated memory for DER encoded certificate signing request
 *          (CSR_MAX_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError EncodeCSR (const UTF8String_t *subjectName,
                    const BIT_STRING_t *subjectPublicKey,
                    const BIT_STRING_t *subjectPrivateKey,
                    ByteArray *encodedCSR);

/**
 * Decode certificate signing request, extracts its common name and public key.
 *
 * Return error if signature is not valid.
 *
 * @param[in] encodedCSR  pointer to array with DER encoded certificate signing request
 * @param[out] subjectName pointer to allocated memory for subject's common name extraction
 *          (CSR_MAX_NAME_SIZE bytes should be allocated)
 * @param[out] subjectPublicKey pointer to allocated memory for subject's public key extraction
 *          (PUBLIC_KEY_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError DecodeCSR (const ByteArray *encodedCSR,
                       UTF8String_t *subjectName, BIT_STRING_t *subjectPublicKey);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_MASTER_CSR_H_ */
