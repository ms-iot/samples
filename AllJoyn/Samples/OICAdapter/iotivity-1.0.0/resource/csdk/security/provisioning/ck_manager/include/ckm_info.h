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


#ifndef INCLUDE_CA_STORAGE_H_
#define INCLUDE_CA_STORAGE_H_

#include "byte_array.h"
#include <stdio.h>
#include "pki_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CA_STORAGE_FILE         "ckminfo.dat"
#define CA_STORAGE_CRT_FILE     "crt.der"
#define ISSUER_MAX_NAME_SIZE    (100)
#define PRIVATE_KEY_SIZE        (32)
#define PUBLIC_KEY_SIZE         (64)
#define ISSUER_MAX_CERT_SIZE    (1000)
/**
 *  Certificate Authority storage
 */
typedef struct /*CA private key, CA certificate, other CA-related info*/
{
    uint8_t     CKMInfoIsLoaded;

    uint8_t     CAPrivateKeyIsSet;
    uint8_t     CAPrivateKey[PRIVATE_KEY_SIZE];

    uint8_t     CAPublicKeyIsSet;
    uint8_t     CAPublicKey[PUBLIC_KEY_SIZE];

    uint8_t     CAChainLength;

    ByteArray  *CACertificateChain;

    uint32_t    CANameSize;
    uint8_t     CAName[ISSUER_MAX_NAME_SIZE];

    long        nextSerialNumber;

    long        CRLSerialNumber;

    long        numberOfRevoked;
} CKMInfo_t;

typedef struct /*CA private key, CA certificate, certificate revocation/white list*/
{
    uint32_t    CRLsize;
    uint8_t     *certificateRevocationList;//should be allocated dynamically
} CRLInfo_t;

//General functions

/**
 * Initializes CA storage from CA_STORAGE_FILE.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError InitCKMInfo(void);

/**
 * Saves CA storage into CA_STORAGE_FILE.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SaveCKMInfo(void);

/**
 * Frees CA storage memory.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError CloseCKMInfo(void);

/**
 * Sets CKM Info.
 *
 * @param[in] nextSN pointer to the next serial number to set
 *                   or 0 to skip this parameter
 * @param[in] CRLSerialNumber pointer to the next CRL serial number to set
 *                   or 0 to skip this parameter
 * @param[in] CAPrivateKey pointer to the CA's private key to set
 *                   or 0 to skip this parameter
 * @param[in] CAPublicKey pointer to the CA's public key to set
 *                   or 0 to skip this parameter
 * @param[in] CAName pointer to the CA's common name to set
 *                   or 0 to skip this parameter
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCKMInfo (const long *nextSN, const long *CRLSerialNumber,
                     const ByteArray *CAPrivateKey, const ByteArray *CAPublicKey,
                     const ByteArray *CAName);

/**
 * Gets CKM Info.
 *
 * @param[out] nextSN pointer to the next serial number to get
 *                   (Memory should be allocated before call)
 *                   or 0 to skip this parameter
 * @param[out] CRLSerialNumber pointer to the next CRL serial number to get
 *                   (Memory should be allocated before call)
 *                   or 0 to skip this parameter
 * @param[out] CAPrivateKey pointer to the CA's private key to get
 *                   (PRIVATE_KEY_SIZE bytes should be allocated before call)
 *                   or 0 to skip this parameter
 * @param[out] CAPublicKey pointer to the CA's public key to get
 *                   (PUBLIC_KEY_SIZE bytes should be allocated before call)
 *                   or 0 to skip this parameter
 * @param[out] CAName pointer to the CA's common name to get
 *                   (ISSUER_MAX_NAME_SIZE bytes should be allocated before call)
 *                   or 0 to skip this parameter
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCKMInfo (long *nextSN, long *CRLSerialNumber,
                     ByteArray *CAPrivateKey, ByteArray *CAPublicKey,
                     ByteArray *CAName);

/**
 * Sets CA's private key.
 *
 * @param[in] CAPrivateKey pointer to the CA's private key to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCAPrivateKey (const ByteArray *CAPrivateKey);

/**
 * Gets CA's private key.
 *
 * @param[out] CAPrivateKey pointer to the CA's private key to get
 *             (PRIVATE_KEY_SIZE bytes should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCAPrivateKey (ByteArray *CAPrivateKey);

/**
 * Sets CA's public key.
 *
 * @param[in] CAPublicKey pointer to the CA's public key to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCAPublicKey (const ByteArray *CAPublicKey);

/**
 * Gets CA's public key.
 *
 * @param[out] CAPublicKey pointer to the CA's public key to get
 *            (PUBLIC_KEY_SIZE bytes should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCAPublicKey (ByteArray *CAPublicKey);

/**
 * Sets CA's common name.
 *
 * @param[in] CAName pointer to the CA's common name to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCAName (const ByteArray *CAName);

/**
 * Gets CA's common name.
 *
 * @param[out] CAName pointer to the CA's common name to get
 *            (ISSUER_MAX_NAME_SIZE bytes should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCAName (ByteArray *CAName);

//Certificate-related functions

/**
 * Initializes CA Certificate from CA_STORAGE_CRT_FILE.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError InitCRT(void);

/**
 * Saves CA Certificate into CA_STORAGE_CRT_FILE.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SaveCRT(void);

/**
 * Sets next serial number for certificate issuer.
 *
 * @param[in] nextSN pointer to the next serial number to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetNextSerialNumber (const long *nextSN);

/**
 * Gets next serial number for certificate issuer.
 *
 * @param[out] nextSN pointer to the next serial number to get
 *            (Memory should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetNextSerialNumber (long *nextSN);

/**
 * Sets DER encoded CA's certificate chain.
 *
 * @param[in] CAChain pointer to the CA's certificate to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCAChain (const uint8_t CAChainLength, const ByteArray *CAChain);

/**
 * Gets DER encoded CA's certificate chain.
 *
 * @param[out] CAChain pointer to allocated memory to get the CA's certificate chain
 *            (ISSUER_MAX_CHAIN_SIZE bytes should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCAChain (uint8_t* CAChainLength, ByteArray *CAChain);

/**
 * Sets DER encoded CA's certificate.
 *
 * @param[in] CACertificate pointer to the CA's certificate to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCACertificate (const ByteArray *CACertificate);

/**
 * Gets DER encoded CA's certificate.
 *
 * @param[out] CACertificate pointer to the CA's certificate to get
 *            (ISSUER_MAX_CERT_SIZE bytes should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCACertificate (ByteArray *CACertificate);

//CRL-related functions

/**
 * Initializes CRL from CA_STORAGE_CRL_FILE.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError InitCRL(void);

/**
 * Saves CRL into CA_STORAGE_CRL_FILE.
 *
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SaveCRL(void);

/**
 * Sets next CRL serial number for certificate issuer.
 *
 * @param[in] CRLSerialNumber pointer to the next CRL serial number to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCRLSerialNumber (const long *CRLSerialNumber);

/**
 * Gets next CRL serial number for certificate issuer.
 *
 * @param[out] CRLSerialNumber pointer to the next CRL serial number to get
 *            (Memory should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCRLSerialNumber (long *CRLSerialNumber);

/**
 * Sets current certificate revocation list.
 *
 * @param[in] certificateRevocationList pointer to the certificate revocation list to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetCertificateRevocationList (const ByteArray *certificateRevocationList);

/**
 * Gets current certificate revocation list.
 *
 * @param[out] certificateRevocationList pointer to the certificate revocation list to get
 *            (Memory should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetCertificateRevocationList (ByteArray *certificateRevocationList);

/**
 * Sets number of revoked certificates.
 *
 * @param[in] numberOfRevoked pointer to number of revoked certificates to set
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError SetNumberOfRevoked (const long *numberOfRevoked);

/**
 * Gets number of revoked certificates.
 *
 * @param[out] numberOfRevoked pointer to number of revoked certificates to get
 *            (Memory should be allocated before call)
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError GetNumberOfRevoked (long *numberOfRevoked);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_CA_STORAGE_H_ */
