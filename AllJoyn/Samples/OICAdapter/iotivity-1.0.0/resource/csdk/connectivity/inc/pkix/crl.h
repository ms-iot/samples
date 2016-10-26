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


#ifndef _CRL_H_
#define _CRL_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "byte_array.h"
#include "pki_errors.h"

/// Maximal number of revoked certificates in list
#ifdef WITH_ARDUINO
#define CRL_MAX_LEN              (256)
#else
#define CRL_MAX_LEN              (1024)
#endif // WITH_ARDUINO
/**
 * @struct CertificateList
 *
 * CRL structure.
 *
 * Structure fields contain byte arrays  pointed to relative DER certificate positions.
 */
typedef struct
{
    ByteArray    tbs;       /**< TBS sequence of CRL.*/
    ByteArray    issuer;    /**< Issuer name.*/
    ByteArray    date;      /**< The issue-date for CRL.*/
    ByteArray    signR;     /**< Signature  r value.*/
    ByteArray    signS;     /**< Signature  s value.*/
} CertificateList;

/**@def CRL_INITIALIZER
 *
 * Initializes of existing CRL fields to {NULL, 0}.
 */
#undef CRL_INITIALIZER
#define CRL_INITIALIZER {BYTE_ARRAY_INITIALIZER,\
                         BYTE_ARRAY_INITIALIZER,\
                         BYTE_ARRAY_INITIALIZER,\
                         BYTE_ARRAY_INITIALIZER,\
                         BYTE_ARRAY_INITIALIZER}

#ifdef X509_DEBUG
/**
 * Prints Certificate List to console.
 *
 * @param crl - pointer to certificate list structure
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError PrintCRL(const CertificateList *const crl);
#endif

/**
 * Decodes and checks Certificate List.
 *
 * @param code - certificate list structure in DER format
 * @param crl - pointer to certificate list structure
 * @param caPubKey - ByteArray structure contains CA public key
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError DecodeCertificateList(ByteArray code, CertificateList *crl,  ByteArray caPubKey);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_CRL_H_
