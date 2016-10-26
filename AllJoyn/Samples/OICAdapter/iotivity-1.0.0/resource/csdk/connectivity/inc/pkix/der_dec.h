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
#ifndef _DER_DEC_H_
#define _DER_DEC_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "byte_array.h"
#include "pki_errors.h"
#include "crypto_adapter.h"

/// Maximal octet number in certificate's serial number
#define SERIAL_NUMBER_MAX_LEN    (20)
/**
 * @name DER constants
 * These constants comply with DER encoded the ANS.1 type tags.
 * DER encoding uses hexadecimal representation.
 */
#define DER_UNIVERSAL               (0x00)
#define DER_SEQUENCE                (0x30)
#define DER_OBJECT_IDENTIFIER       (0x06)
#define DER_BIT_STRING              (0x03)
#define DER_INTEGER                 (0x02)
#define DER_UTC_TIME                (0x17)
#define DER_VERSION                 (0xa0)

/* The first octet of the OCTET STRING indicates whether the key is
compressed or uncompressed.  The uncompressed form is indicated by 0x04
and the compressed form is indicated by either 0x02 or 0x03 (RFC 5480)*/
#define ASN1_UNCOMPRESSED_KEY   (0x04)
/// ASN.1 UTC time length
#define UTC_TIME_LEN            (13)
///  Length Octet ASN.1
#define LEN_LONG                (128)
/// Size of byte
#define SIZE_OF_BYTE            (8)

#define ECDSA_WITH_SHA256_OID_LEN    (8)
#define EC_PUBLIC_KEY_OID_LEN        (7)
#define PRIME_256_V1_OID_LEN         (8)

/**@def SKIP_DER_FIELD(array, type, length)
 * Skips the field in the ASN.1 structure.
 *
 * @param array pointer to ASN.1 stucture
 * @param type type of ASN.1 field
 * @param length length of ASN.1 field
 */
#undef SKIP_DER_FIELD
#define SKIP_DER_FIELD(array, type, length) do{                 \
        CHECK_EQUAL(*((array).data), type, PKI_INVALID_FORMAT); \
        CHECK_CALL(DecodeLength , &(array), &(length));         \
        INC_BYTE_ARRAY(array, length);                          \
        }while(0)

/**@def COPY_DER_FIELD(array, str, field, type, length)
 * Copies the field from the ASN.1 structure.
 *
 * @param array pointer to ASN.1 stucture
 * @param str structure in which the array is copied
 * @param field field of the structure in which the array is copied
 * @param type type of ASN.1 field
 * @param length length of ASN.1 field
 */
#undef COPY_DER_FIELD
#define COPY_DER_FIELD(array, crt, field, type, length) do{     \
        CHECK_EQUAL(*((array).data), type, PKI_INVALID_FORMAT); \
        CHECK_CALL(DecodeLength , &(array), &(length));         \
        ((crt)->field).data = (array).data;                     \
        ((crt)->field).len = length;                            \
        INC_BYTE_ARRAY(array, length);                          \
        }while(0)


/**@def CHECK_DER_OID(array, oid, length)
 * Checks the field from the ASN.1 structure.
 *
 * @param array pointer to ASN.1 stucture
 * @param oid type of DER object
 * @param oidLen length of DER array
 * @param length length of ASN.1 field
 */
#undef CHECK_DER_OID
#undef CHECK_DER_OID
#define CHECK_DER_OID(array, oid, oidLen, length) do{                              \
        int ret = 0;                                                               \
        CHECK_EQUAL(*((array).data), DER_OBJECT_IDENTIFIER, PKI_INVALID_FORMAT);   \
        CHECK_CALL(DecodeLength , &(array), &(length));                            \
        CHECK_EQUAL(length, oidLen, PKI_UNKNOWN_OID);                              \
        ret = memcmp ((array).data, oid, oidLen);                                  \
        CHECK_EQUAL(ret, 0, PKI_UNKNOWN_OID);                                      \
        }while(0)

/**@def PARSE_SIGNATURE(structure)
 * Parse signature of ASN.1 structure , remove ASN.1 extra bytes.
 *
 * @param structure Certificate or CertificateList structure
 */
#undef PARSE_SIGNATURE
#define PARSE_SIGNATURE(structure) do{                                                       \
        if (((structure)->signR.len == SIGN_R_LEN + 1) && ((structure)->signR.data[0] == 0)) \
        INC_BYTE_ARRAY((structure)->signR, 1);                                               \
        else if ((structure)->signR.len != SIGN_R_LEN)                                       \
        CHECK_NULL(NULL, PKI_WRONG_ARRAY_LEN);                                               \
        if (((structure)->signS.len == SIGN_S_LEN + 1) && ((structure)->signS.data[0] == 0)) \
        INC_BYTE_ARRAY((structure)->signS, 1);                                               \
        else if ((structure)->signS.len != SIGN_S_LEN)                                       \
        CHECK_NULL(NULL, PKI_WRONG_ARRAY_LEN);                                               \
        }while(0)

/**
 * Computes length of ASN.1 object in DER format.
 *
 * @param[in] code array with DER encoded ASN.1 structure
 * @return PKI_SUCCESS if success, error code otherwise
 */
PKIError DecodeLength(ByteArray *code, size_t *length);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif //_X509_PARSE_H_
