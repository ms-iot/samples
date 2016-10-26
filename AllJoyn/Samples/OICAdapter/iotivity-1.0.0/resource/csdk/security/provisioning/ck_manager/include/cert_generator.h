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


#ifndef INCLUDE_ISSUER_ISSUER_H_
#define INCLUDE_ISSUER_ISSUER_H_

#include "Certificate.h" // Certificate ASN.1 type
#include "pki.h"
#include "pki_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define X509_V1 0
#define X509_V2 1
#define X509_V3 2

/**
 * Encodes X.509 certificate with specified parameters.
 *
 * @param[in] subjectName pointer to serial subject's common name
 * @param[in] issuerName pointer to issuer's common name
 * @param[in] notBefore pointer to certificate validity limit
 * @param[in] notAfter pointer to certificate validity limit
 * @param[in] subjectPublicKey pointer to subject's public key to be signed
 * @param[in] issuerPrivateKey pointer to issuer's private key to sign certificate
 * @param[out] encodedCertificate pointer to allocated memory for DER encoded certificate
 *          (ISSUER_MAX_CERT_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateCertificate (const UTF8String_t *subjectName, const UTF8String_t *issuerName,
                        const UTCTime_t *notBefore, const UTCTime_t *notAfter,
                        const BIT_STRING_t *subjectPublicKey, const BIT_STRING_t *issuerPrivateKey,
                        ByteArray *encodedCertificate);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ISSUER_ISSUER_H_ */
