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

#ifndef _CERT_H_
#define _CERT_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "byte_array.h"
#include "pki_errors.h"

/**
 * @struct CertificateX509
 *
 * Certificate structure.
 *
 * Structure fields contain byte arrays  pointed to relative DER certificate positions.
 */
typedef struct
{
    ByteArray    tbs;       /**< TBS certificate.*/
    ByteArray    serNum;    /**< Serial number.*/
    ByteArray    pubKey;    /**< Public key.*/

    ByteArray    signR;     /**< Signature  r value.*/
    ByteArray    signS;     /**< Signature  s value.*/

    ByteArray    issuer;    /**< Issuer name.*/
    ByteArray    subject;   /**< Subject name.*/

    ByteArray   validFrom;   /**< Start time of certificate validity. */
    ByteArray   validTo;     /**< End time of certificate validity. */

} CertificateX509;

/**
 *
 *
 * Initializes of certificate structure.
 *
 */
#define CERTIFICATE_X509_INITIALIZER {\
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER,                                            \
    BYTE_ARRAY_INITIALIZER}

/**
 * Reads certificate from byte array and write it into certificate structure.
 *
 * @param[in] code  Byte array with DER encoded certificate
 * @param[out] crt  Pointer to certificate structure
 * @return  PKI_SUCCESS if successful
 */
PKIError DecodeCertificate(ByteArray code, CertificateX509 *crt);

/**
 * Parse ECDSA public key, remove ASN.1 extra bytes.
 *
 * @param ByteArray structure which contains public key
 * @return PKI_SUCCESS if public key is correct, error code in case of invalid key
 */
PKIError ParsePublicKey(ByteArray *caPublicKey);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif //_CERT_H_
