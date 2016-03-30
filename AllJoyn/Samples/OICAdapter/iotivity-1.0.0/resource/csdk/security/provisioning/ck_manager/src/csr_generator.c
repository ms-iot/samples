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


#include "csr_generator.h"
#include "pki.h"
#include "oic_malloc.h"

//ecdsa-with-SHA256 1.2.840.10045.4.3.2 [RFC5759]
static const uint8_t g_ECDSA_WITH_SHA256_OID[] = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02};

//Elliptic Curve Digital Signature Algorithm (ECDSA) 1.2.840.10045.2.1 [RFC7250]
static const uint8_t g_EC_PUBLIC_KEY_OID[] = {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01};

//secp256r1 1.2.840.10045.3.1.7 [RFC5656]
static const uint8_t g_PRIME_256_V1_OID[] = {0x2A, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07};

//commonName 2.5.4.3 [RFC2256]
static const uint8_t g_COMMON_NAME_OID[] = {0x55, 0x04, 0x03};

PKIError EncodeCSR (const UTF8String_t *subjectName,
                    const BIT_STRING_t *subjectPublicKey,
                    const BIT_STRING_t *subjectPrivateKey,
                    ByteArray *encodedCSR)
{
    FUNCTION_INIT();
    asn_enc_rval_t ec; /* Encoder return value */
    CertificationRequest_t *certificationRequest = NULL; /* Type to encode */
    AttributeTypeAndValue_t *subjTypeAndValue    = NULL;
    RelativeDistinguishedName_t *subjRDN         = NULL;
    uint8_t *uint8Pointer                        = NULL;
    ByteArray tbs                                = BYTE_ARRAY_INITIALIZER;
    uint8_t signature[SIGN_FULL_SIZE];
    uint8_t sha256[SHA_256_HASH_LEN];
    uint8_t ReqInfoInDER[CSR_MAX_SIZE];

    CHECK_NULL(subjectName, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(subjectPublicKey, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(subjectPrivateKey, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(encodedCSR, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(encodedCSR->data, ISSUER_CSR_NULL_PASSED);
    CHECK_LESS_EQUAL(CSR_MAX_SIZE, encodedCSR->len, ISSUER_CSR_WRONG_BYTE_ARRAY_LEN);

    /* Allocate the memory */
    certificationRequest      = OICCalloc(1, sizeof(CertificationRequest_t)); // not malloc!
    CHECK_NULL(certificationRequest, ISSUER_CSR_MEMORY_ALLOC_FAILED);

    subjTypeAndValue = OICCalloc(1, sizeof(AttributeTypeAndValue_t));
    CHECK_NULL(subjTypeAndValue, ISSUER_CSR_MEMORY_ALLOC_FAILED);

    subjRDN          = OICCalloc(1, sizeof(RelativeDistinguishedName_t));
    CHECK_NULL(subjRDN, ISSUER_CSR_MEMORY_ALLOC_FAILED);

    //set subject name
    subjTypeAndValue->value = *subjectName;
    subjTypeAndValue->type.buf = (uint8_t *)g_COMMON_NAME_OID;  //2.5.4.3
    subjTypeAndValue->type.size = sizeof(g_COMMON_NAME_OID) / sizeof(g_COMMON_NAME_OID[0]);
    ASN_SET_ADD(subjRDN, subjTypeAndValue);
    ASN_SEQUENCE_ADD(&(certificationRequest->certificationRequestInfo.subject), subjRDN);

    //set version 0
    certificationRequest->certificationRequestInfo.version = CSR_DEFAULT_VERSION;

    //set signature algorithm
    certificationRequest->signatureAlgorithm.algorithm.buf =
        (uint8_t *)g_ECDSA_WITH_SHA256_OID;    //1.2.840.10045.4.3.2
    certificationRequest->signatureAlgorithm.algorithm.size =
        sizeof(g_ECDSA_WITH_SHA256_OID) / sizeof(g_ECDSA_WITH_SHA256_OID[0]);
    certificationRequest->signatureAlgorithm.nul = OICCalloc(1, sizeof(NULL_t));
    CHECK_NULL(certificationRequest->signatureAlgorithm.nul, ISSUER_CSR_MEMORY_ALLOC_FAILED);

    //set subject Public Key algorithm
    certificationRequest->certificationRequestInfo.subjectPKInfo.algorithm.algorithm.buf =
        (uint8_t *)g_EC_PUBLIC_KEY_OID;   //1.2.840.10045.2.1
    certificationRequest->certificationRequestInfo.subjectPKInfo.algorithm.algorithm.size =
        sizeof(g_EC_PUBLIC_KEY_OID) / sizeof(g_EC_PUBLIC_KEY_OID[0]);

    //set subject Public Key curve
    certificationRequest->certificationRequestInfo.subjectPKInfo.algorithm.id_ecPublicKey =
        OICCalloc(1, sizeof(OBJECT_IDENTIFIER_t));
    CHECK_NULL(certificationRequest->
               certificationRequestInfo.subjectPKInfo.algorithm.id_ecPublicKey,
               ISSUER_CSR_MEMORY_ALLOC_FAILED);

    certificationRequest->certificationRequestInfo.subjectPKInfo.algorithm.id_ecPublicKey->buf =
        (uint8_t *)g_PRIME_256_V1_OID;  //1.2.840.10045.3.1.7
    certificationRequest->certificationRequestInfo.subjectPKInfo.algorithm.id_ecPublicKey->size =
        sizeof(g_PRIME_256_V1_OID) / sizeof(g_PRIME_256_V1_OID[0]);

    //set subject Public Key
    certificationRequest->certificationRequestInfo.subjectPKInfo.subjectPublicKey =
            *subjectPublicKey;

    //encode TBS to DER
    ec = der_encode_to_buffer(&asn_DEF_CertificationRequestInfo,
                              &(certificationRequest->certificationRequestInfo),
                              ReqInfoInDER, CSR_MAX_SIZE);
    CHECK_COND(ec.encoded > 0, ISSUER_CSR_DER_ENCODE_FAIL);
    tbs.len = ec.encoded;
    tbs.data = ReqInfoInDER;
    GET_SHA_256(tbs, sha256);
    CHECK_COND(uECC_sign((subjectPrivateKey->buf) + 1, sha256, signature),
               ISSUER_CSR_SIGNATURE_FAIL);
    //additional byte for ASN1_UNCOMPRESSED_KEY_ID

    // ECDSA-Sig-Value ::= SEQUENCE { r INTEGER, s INTEGER } (RFC 5480)
    certificationRequest->signature.size = SIGN_FULL_SIZE + 6;
    // size for SEQUENCE ID + 2 * INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[0] > 127)
    {
        certificationRequest->signature.size ++;
    }

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[SIGN_R_LEN] > 127)
    {
        certificationRequest->signature.size ++;
    }
    certificationRequest->signature.buf = OICCalloc(certificationRequest->signature.size,
                                                 sizeof(uint8_t));
    CHECK_NULL(certificationRequest->signature.buf, ISSUER_CSR_MEMORY_ALLOC_FAILED);
    *(certificationRequest->signature.buf) = (12 << 2); //ASN.1 SEQUENCE ID
    *(certificationRequest->signature.buf + 1) = certificationRequest->signature.size - 2;
    //ASN.1 SEQUENCE size

    uint8Pointer = certificationRequest->signature.buf + 2; //skip SEQUENCE ID and size
    *uint8Pointer = (2 << 0); //ASN.1 INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[0] > 127)
    {
        *(uint8Pointer + 1) = SIGN_R_LEN + 1; //ASN.1 INTEGER size
        uint8Pointer += 3; //skip INTEGER ID and size
    }
    else
    {
        *(uint8Pointer + 1) = SIGN_R_LEN; //ASN.1 INTEGER size
        uint8Pointer += 2; //skip INTEGER ID and size
    }
    memcpy(uint8Pointer, signature, SIGN_R_LEN);

    uint8Pointer += SIGN_R_LEN;
    *uint8Pointer = (2 << 0); //ASN.1 INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature [SIGN_R_LEN] > 127)
    {
        *(uint8Pointer + 1) = SIGN_R_LEN + 1; //ASN.1 INTEGER size
        uint8Pointer += 3; //skip INTEGER ID and size
    }
    else
    {
        *(uint8Pointer + 1) = SIGN_S_LEN; //ASN.1 INTEGER size
        uint8Pointer += 2; //skip INTEGER ID and size
    }
    memcpy(uint8Pointer, signature + SIGN_R_LEN, SIGN_S_LEN);

    ec = der_encode_to_buffer(&asn_DEF_CertificationRequest, certificationRequest,
                              encodedCSR->data, CSR_MAX_SIZE);
    CHECK_COND(ec.encoded > 0, ISSUER_CSR_DER_ENCODE_FAIL);
    encodedCSR->len = ec.encoded;

    FUNCTION_CLEAR(
        if (subjTypeAndValue)
        {
            subjTypeAndValue->value.buf = NULL;
            subjTypeAndValue->type.buf  = NULL;
        }
        if (certificationRequest)
        {
            certificationRequest->
                    certificationRequestInfo.subjectPKInfo.algorithm.algorithm.buf = NULL;

            certificationRequest->signatureAlgorithm.algorithm.buf = NULL;

            if (certificationRequest->
                    certificationRequestInfo.subjectPKInfo.algorithm.id_ecPublicKey)
            {
                certificationRequest->
                certificationRequestInfo.subjectPKInfo.algorithm.id_ecPublicKey->buf = NULL;
            }
        }
        ASN_STRUCT_FREE(asn_DEF_CertificationRequest, certificationRequest);
        certificationRequest = NULL;
    );
}

PKIError DecodeCSR (const ByteArray *encodedCSR,
                       UTF8String_t *subjectName, BIT_STRING_t *subjectPublicKey)
{
    FUNCTION_INIT();
    asn_dec_rval_t rval; /* Decoder return value */
    asn_enc_rval_t ec; /* Encoder return value */
    CertificationRequest_t *certificationRequest = NULL; // Type to decode
    ByteArray tbs                                = BYTE_ARRAY_INITIALIZER;
    uint8_t sha256[SHA_256_HASH_LEN];
    uint8_t ReqInfoInDER[CSR_MAX_SIZE];
    uint8_t signature[SIGN_FULL_SIZE];

    CHECK_NULL(encodedCSR, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(encodedCSR->data, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(subjectName, ISSUER_CSR_NULL_PASSED);
    CHECK_NULL(subjectPublicKey, ISSUER_CSR_NULL_PASSED);

    rval = ber_decode(0, &asn_DEF_CertificationRequest, (void **)&certificationRequest,
                      encodedCSR->data, encodedCSR->len);
    CHECK_EQUAL(rval.code, RC_OK, ISSUER_CSR_DER_DECODE_FAIL);

    //encode TBS to DER
    ec = der_encode_to_buffer(&asn_DEF_CertificationRequestInfo,
                              &(certificationRequest->certificationRequestInfo),
                              ReqInfoInDER, CSR_MAX_SIZE);
    CHECK_COND(ec.encoded > 0, ISSUER_CSR_DER_ENCODE_FAIL);
    tbs.len = ec.encoded;
    tbs.data = ReqInfoInDER;
    GET_SHA_256(tbs, sha256);

    // ECDSA-Sig-Value ::= SEQUENCE { r INTEGER, s INTEGER } (RFC 5480)
    if (*(certificationRequest->signature.buf + 3) > SIGN_R_LEN) //check length of first INTEGER (r)
    {
        //move to start of second INTEGER (s)
        memcpy(signature, certificationRequest->signature.buf + 5, SIGN_R_LEN);
    }
    else
    {
        //move to start of second INTEGER (s)
        memcpy(signature, certificationRequest->signature.buf + 4, SIGN_R_LEN);
    }
    memcpy((signature + SIGN_R_LEN),
           (certificationRequest->signature.buf +
            certificationRequest->signature.size - SIGN_S_LEN), SIGN_S_LEN);
    //verify signature
    CHECK_COND(uECC_verify(certificationRequest->
                           certificationRequestInfo.subjectPKInfo.subjectPublicKey.buf + 1,
                           //additional byte for ASN1_UNCOMPRESSED_KEY_ID
                           sha256, signature), ISSUER_CSR_INVALID_SIGNATURE);

    subjectName->size =
        certificationRequest->certificationRequestInfo.subject.list.array[0]->
        list.array[0]->value.size;
    CHECK_LESS_EQUAL(subjectName->size, CSR_MAX_NAME_SIZE, ISSUER_CSR_TOO_LONG_NAME);
    memcpy(subjectName->buf,
           certificationRequest->certificationRequestInfo.subject.list.array[0]->
           list.array[0]->value.buf, subjectName->size);

    subjectPublicKey->size =
        certificationRequest->certificationRequestInfo.subjectPKInfo.subjectPublicKey.size;
    CHECK_EQUAL(subjectPublicKey->size, PUBLIC_KEY_SIZE + 1, ISSUER_CSR_INVALID_KEY_FORMAT);
    //additional byte for ASN1_UNCOMPRESSED_KEY_ID

    memcpy(subjectPublicKey->buf,
               certificationRequest->certificationRequestInfo.subjectPKInfo.subjectPublicKey.buf,
               subjectPublicKey->size);

    FUNCTION_CLEAR(
        ASN_STRUCT_FREE(asn_DEF_CertificationRequest, certificationRequest);
    );
}
