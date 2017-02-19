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

#ifndef INCLUDE_MASTER_ISSUER_H_
#define INCLUDE_MASTER_ISSUER_H_

#include "byte_array.h"
#include "cert_generator.h"
#include "ckm_info.h"
#include "csr_generator.h"
#include "ecc.h"
#include "crl_generator.h"
#include "pki_errors.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISSUER_DEFAULT_CA_NAME          "CA_name"
#define ISSUER_DEFAULT_SUBJECT_NAME     "Subj_name"
#define ISSUER_DEFAULT_NOT_BEFORE       "130101000000Z"
#define ISSUER_DEFAULT_NOT_AFTER        "490101000000Z"
#define ISSUER_DEFAULT_THIS_UPDATE      "150601000000Z"

/**
 * Set serial number for next certificate and save it in the CA storage.
 *
 * Used by CKMSetCAInfo()
 *
 * @param[in] serNum certificate serial number to be set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetSerialNumber (const long serNum);

/**
 * Set CA common name and save it in the CA storage.
 *
 * Used by CKMSetCAInfo()
 *
 * @param[in] rootName CA common name to be set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetRootName (const ByteArray rootName);

/**
 * Set CA info and save it in the CA storage.
 *
 * Should be called before certificate generation
 *
 * @param[in] serNum certificate serial number to be set
 * @param[in] rootName CA common name to be set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CKMSetCAInfo (const long serNum, const ByteArray rootName);

/**
 * Generate key pair for CA.
 *
 * Should be called before certificate generation
 *
 * @param[out] caPrivateKey pointer to allocated ByteArray for CA private key
 * @param[out] caPublicKey pointer to allocated ByteArray for CA public key
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateCAKeyPair (ByteArray *caPrivateKey, ByteArray *caPublicKey);

/**
 * Issues X.509 certificate with specified parameters.
 *
 * SetSerialNumber, SetRootName and GenerateCAKeyPair should be called before.
 *
 * @param[in] uint8NotBefore pointer to string with certificate validity limit
 *                             or 0 to use default value
 * @param[in] uint8NotAfter pointer to string with certificate validity limit
 *                            or 0 to use default value
 * @param[out] issuedCertificate pointer to allocated memory for DER encoded certificate
 *          (ISSUER_MAX_CERT_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CKMIssueRootCertificate (const uint8_t *uint8NotBefore, const uint8_t *uint8NotAfter,
                                     ByteArray *issuedRootCertificate);

/**
 * Generate key pair for ordinary device.
 *
 * @param[out] privateKey pointer to allocated ByteArray for private key
 * @param[out] publicKey pointer to allocated ByteArray for public key
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateKeyPair (ByteArray *privateKey, ByteArray *publicKey);

/**
 * Issues X.509 certificate with specified parameters.
 *
 * SetSerialNumber, SetRootName and GenerateCAKeyPair should be called before.
 *
 * @param[in] uint8SubjectName pointer to string with serial subject's common name
 *                               or 0 to use default value
 * @param[in] uint8NotBefore pointer to string with certificate validity limit
 *                             or 0 to use default value
 * @param[in] uint8NotAfter pointer to string with certificate validity limit
 *                            or 0 to use default value
 * @param[in] uint8SubjectPublicKey pointer to array with subject's public key to be signed
 * @param[out] issuedCertificate pointer to allocated memory for DER encoded certificate
 *          (ISSUER_MAX_CERT_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CKMIssueDeviceCertificate (const uint8_t *uint8SubjectName,
                                    const uint8_t *uint8NotBefore, const uint8_t *uint8NotAfter,
                                    const uint8_t *uint8SubjectPublicKey,
                                    ByteArray *issuedCertificate);

/**
 * Write certificate into specified file.
 *
 * @param[in] certificate pointer to DER-encoded certificate that should be written into the file
 * @param[in] certFileName pointer to null-terminated string with file name
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateDERCertificateFile (const ByteArray *certificate, const char *certFileName);

/**
 * Issues certificate signing request with specified parameters.
 *
 * @param[in] uint8SubjectName pointer to string with serial subject's common name
 *                               or 0 to use default value
 * @param[in] uint8SubjectPublicKey pointer to array with subject's public key to be signed
 * @param[in] uint8SubjectPrivateKey pointer to array with subject's private key to be signed
 * @param[out] encodedCSR pointer to allocated memory for DER encoded certificate signing request
 *          (CSR_MAX_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateCSR (const uint8_t *uint8SubjectName,
                         const uint8_t *uint8SubjectPublicKey,
                         const uint8_t *uint8SubjectPrivateKey,
                         ByteArray *encodedCSR);

/**
 * Issues X.509 certificate based on certificate signing request.
 *
 * @param[in] encodedCSR pointer to array with DER encoded certificate signing request
 * @param[out] issuedCertificate pointer to allocated memory for DER encoded certificate
 *          (MAX_CERT_SIZE bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GenerateCertificateByCSR (const ByteArray *encodedCSR, ByteArray *issuedCertificate);

/**
 * Generate certificate revocation list with specified parameters.
 *
 * @param[in] thisUpdateTime pointer to time of issuing CRL or 0 to use default
 * @param[in] nuberOfRevoked number of revoked certificates
 * @param[in] revokedNumbers array with numbers of revoked certificates
 * @param[in] revocationDates
 * @param[out] encodedCRL pointer to allocated memory for DER encoded certificate revocation list
 *          ( (CRL_MIN_SIZE + nuberOfRevoked * (sizeof(CertificateRevocationInfo_t) + 4)) bytes
 *          should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CKMIssueCRL (const uint8_t *uint8ThisUpdateTime, const uint32_t nuberOfRevoked,
                      const uint32_t *revokedNumbers, const uint8_t **revocationDates,
                      ByteArray *encodedCRL);

/**
 * Gets current certificate revocation list.
 *
 * @param[out] certificateRevocationList pointer to the certificate revocation list to get
 *            (Memory should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CKMGetCRL (ByteArray *certificateRevocationList);

/**
 * Add specified certificate into certificate revocation list.
 *
 * @param[in] thisUpdateTime pointer to time of issuing CRL or 0 to use default
 * @param[in] revokedNumber number of certificate to revoke
 * @param[out] encodedCRL pointer to allocated memory for DER encoded certificate revocation list
 *          ( (CRL_MIN_SIZE + number of ALL revoked certificates *
 *           (sizeof(CertificateRevocationInfo_t) + 4)) bytes should be allocated)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CKMRevocateCertificate (const uint8_t *uint8ThisUpdateTime, const long revokedNumber,
                                 ByteArray *encodedCRL);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_MASTER_ISSUER_H_ */
