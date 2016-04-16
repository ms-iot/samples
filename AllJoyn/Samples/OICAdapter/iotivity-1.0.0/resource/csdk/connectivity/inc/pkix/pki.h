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


#ifndef _PKI_H_
#define _PKI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "byte_array.h"
#include "pki_errors.h"
#include "crypto_adapter.h"
#include "cert.h"

/**
 * Maximal number of certificates in trust chain.
 */
#define MAX_CHAIN_LEN    (3)

/**
 * Maximal length of the TLS certificate message.
 */
#define MAX_CERT_MESSAGE_LEN (2048)

#ifdef X509_DEBUG
/**
 * Prints Certificate to console.
 *
 * @param crt - pointer to Certificate structure
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError PrintCertificate(const CertificateX509 *const crt);
#endif

/**
 * Checks certificate validity period.
 *
 * @param dateFrom - array with not before field
 * @param dateTo - array with not after field
 * @return PKI_SUCCESS if valid, error code otherwise
 */
PKIError CheckValidity(ByteArray dateFrom, ByteArray dateTo);

/**
 * Checks certificate date and sign.
 *
 * @param[in] certDerCode - Byte array with DER encoded certificate
 * @param[in] caPublicKey - CA public key
 * @return  0 if successful
 */
PKIError CheckCertificate(ByteArray certDerCode, ByteArray caPublicKey);

/**
 * Parses each certificates from list.
 *
 * @param[in] chainDerCode Array of DER encoded certificates
 * @param[out] chainCrt Array of parsed certificates
 * @param[in] chainLen Lengths of array
 * @returns  PKI_SUCCESS if no error is occurred
 */
PKIError ParseCertificateChain (ByteArray *chainDerCode, CertificateX509 *chainCrt,
                                uint8_t chainLen);

/**
 * Loads certificates in DER format from TLS message to array.
 *
 * @param[in] msg TLS message with certificate's chain
 * @param[out] chain Array of DER encoded certificates
 * @param[out] chainLen Lengths of array
 * @returns  PKI_SUCCESS if no error is occurred
 */
PKIError LoadCertificateChain (ByteArray msg, ByteArray *chain, uint8_t *chainLength);

/**
 * Checks the signature of each certificate in chain.
 *
 * @param[in] chainCrt Chain of certificates structures
 * @param[in] chainLen Number of certificates in the chain
 * @param[in] caPubKey Public key which sign the last certificate from chain
 * @returns PKI_SUCCESS if no error is occurred
 */
PKIError CheckCertificateChain (CertificateX509 *chainCrt, uint8_t chainLen, ByteArray caPubKey);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif // _PKI_H_
