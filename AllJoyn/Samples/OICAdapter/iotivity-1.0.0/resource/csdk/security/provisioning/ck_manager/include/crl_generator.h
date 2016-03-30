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


#ifndef INCLUDE_MASTER_CRL_ENCODER_H_
#define INCLUDE_MASTER_CRL_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CertificateRevocationList.h" /* CertificateList ASN.1 type */
#include "pki.h"
#include "pki_errors.h"

// Minimal memory allocated for DER encoded CRL
#define CRL_MIN_SIZE   250      //minimal size of CRL (issuer info + signature)

#define CRL_MAX_NAME_SIZE 100   //maximal length of CRL issuer field

/**
 * Encode certificate revocation list with specified parameters.
 *
 * @param[in] issuerName pointer to issuer's common name
 * @param[in] thisUpdateTime pointer to time of issuing CRL
 * @param[in] nuberOfRevoked number of revoked certificates
 * @param[in] certificateRevocationInfo array with certificate revocation info
 * @param[in] issuerPrivateKey pointer to issuer's private key to sign CRL
 * @param[out] encodedCRL pointer to allocated memory for DER encoded certificate revocation list
 *          ( (CRL_MIN_SIZE + nuberOfRevoked * (sizeof(CertificateRevocationInfo_t) + 4)) bytes
 *          should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateCRL (const UTF8String_t *issuerName,
                      const UTCTime_t *thisUpdateTime, const uint32_t nuberOfRevoked,
                      const CertificateRevocationInfo_t *certificateRevocationInfo,
                      const BIT_STRING_t *issuerPrivateKey, ByteArray *encodedCRL);

/**
 * Sign certificate revocation list.
 *
 * @param[in] certificateRevocationList pointer to CRL for signing
 * @param[in] crlMaxSize max size of DER encoded CRL
 * @param[in] issuerPrivateKey pointer to issuer private key
 * @param[out] encodedCRL pointer to allocated memory for DER encoded certificate revocation list
 *          crlMaxSize bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SignCRL(CertificateRevocationList_t *certificateRevocationList,
                 const uint32_t crlMaxSize, const BIT_STRING_t *issuerPrivateKey,
                 ByteArray *encodedCRL);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_MASTER_CRL_ENCODER_H_ */
