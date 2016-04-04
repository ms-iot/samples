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


#include "cert_generator.h"
#include "ckm_info.h"
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


PKIError GenerateCertificate (const UTF8String_t *subjectName, const UTF8String_t *issuerName,
                        const UTCTime_t *notBefore, const UTCTime_t *notAfter,
                        const BIT_STRING_t *subjectPublicKey, const BIT_STRING_t *issuerPrivateKey,
                        ByteArray *encodedCertificate)
{
    FUNCTION_INIT();
    asn_enc_rval_t ec; /* Encoder return value */
    Certificate_t *certificate                  = NULL; /* Type to encode */
    AttributeTypeAndValue_t *issuerTypeAndValue    = NULL;
    AttributeTypeAndValue_t *subjectTypeAndValue   = NULL;
    RelativeDistinguishedName_t *issuerRDN         = NULL;
    RelativeDistinguishedName_t *subjectRDN        = NULL;
    uint8_t *uint8Pointer                       = NULL;
    ByteArray tbs                               = BYTE_ARRAY_INITIALIZER;
    uint8_t signature[SIGN_FULL_SIZE];
    uint8_t sha256[SHA_256_HASH_LEN];
    uint8_t tbsDer[ISSUER_MAX_CERT_SIZE];
    long serialNumber = 0;

    CHECK_NULL(subjectName, ISSUER_X509_NULL_PASSED);
    CHECK_NULL(issuerName, ISSUER_X509_NULL_PASSED);
    CHECK_NULL(notBefore, ISSUER_X509_NULL_PASSED);
    CHECK_NULL(notAfter, ISSUER_X509_NULL_PASSED);
    CHECK_NULL(subjectPublicKey, ISSUER_X509_NULL_PASSED);
    CHECK_NULL(issuerPrivateKey, ISSUER_X509_NULL_PASSED);
    CHECK_NULL_BYTE_ARRAY_PTR(encodedCertificate, ISSUER_X509_NULL_PASSED);
    CHECK_LESS_EQUAL(ISSUER_MAX_CERT_SIZE, encodedCertificate->len,
                     ISSUER_X509_WRONG_BYTE_ARRAY_LEN);

    /* Allocate the memory */
    certificate      = OICCalloc(1, sizeof(Certificate_t)); // not malloc!
    CHECK_NULL(certificate, ISSUER_X509_MEMORY_ALLOC_FAILED);

    issuerTypeAndValue  = OICCalloc(1, sizeof(AttributeTypeAndValue_t));
    CHECK_NULL(issuerTypeAndValue, ISSUER_X509_MEMORY_ALLOC_FAILED);

    issuerRDN           = OICCalloc(1, sizeof(RelativeDistinguishedName_t));
    CHECK_NULL(issuerRDN, ISSUER_X509_MEMORY_ALLOC_FAILED);

    subjectTypeAndValue = OICCalloc(1, sizeof(AttributeTypeAndValue_t));
    CHECK_NULL(subjectTypeAndValue, ISSUER_X509_MEMORY_ALLOC_FAILED);

    subjectRDN          = OICCalloc(1, sizeof(RelativeDistinguishedName_t));
    CHECK_NULL(subjectRDN, ISSUER_X509_MEMORY_ALLOC_FAILED);

    //set issuer name
    issuerTypeAndValue->value = *issuerName;
    issuerTypeAndValue->type.buf = (uint8_t *)g_COMMON_NAME_OID;   //2.5.4.3
    issuerTypeAndValue->type.size = sizeof(g_COMMON_NAME_OID) / sizeof(g_COMMON_NAME_OID[0]);
    ASN_SET_ADD(issuerRDN, issuerTypeAndValue);
    ASN_SEQUENCE_ADD(&(certificate->tbsCertificate.issuer), issuerRDN);

    //set subject name
    subjectTypeAndValue->value = *subjectName;
    subjectTypeAndValue->type.buf = (uint8_t *)g_COMMON_NAME_OID;  //2.5.4.3
    subjectTypeAndValue->type.size = sizeof(g_COMMON_NAME_OID) / sizeof(g_COMMON_NAME_OID[0]);
    ASN_SET_ADD(subjectRDN, subjectTypeAndValue);
    ASN_SEQUENCE_ADD(&(certificate->tbsCertificate.subject), subjectRDN);

    //set validity
    certificate->tbsCertificate.validity.notBefore = *notBefore;
    certificate->tbsCertificate.validity.notAfter  = *notAfter;

    //set X.509 certificate version
    certificate->tbsCertificate.version = X509_V2;

    //set serial number
    certificate->tbsCertificate.serialNumber = 0;

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetNextSerialNumber, &serialNumber);
    certificate->tbsCertificate.serialNumber = serialNumber;
    serialNumber++;
    CHECK_CALL(SetNextSerialNumber, &serialNumber);
    CHECK_CALL(SaveCKMInfo);

    //set signature algorithm in TBS
    certificate->tbsCertificate.signature.algorithm.buf =
        (uint8_t *)g_ECDSA_WITH_SHA256_OID;    //1.2.840.10045.4.3.2
    certificate->tbsCertificate.signature.algorithm.size =
        sizeof(g_ECDSA_WITH_SHA256_OID) / sizeof(g_ECDSA_WITH_SHA256_OID[0]);
    certificate->tbsCertificate.signature.nul = OICCalloc(1, sizeof(NULL_t));
    CHECK_NULL(certificate->tbsCertificate.signature.nul, ISSUER_X509_MEMORY_ALLOC_FAILED);

    //set subject Public Key algorithm
    certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.algorithm.buf =
        (uint8_t *)g_EC_PUBLIC_KEY_OID;   //1.2.840.10045.2.1
    certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.algorithm.size =
        sizeof(g_EC_PUBLIC_KEY_OID) / sizeof(g_EC_PUBLIC_KEY_OID[0]);

    //set subject Public Key curve
    certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.id_ecPublicKey =
        OICCalloc(1, sizeof(OBJECT_IDENTIFIER_t));
    CHECK_NULL(certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.id_ecPublicKey,
               ISSUER_X509_MEMORY_ALLOC_FAILED);
    certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.id_ecPublicKey->buf =
        (uint8_t *)g_PRIME_256_V1_OID;  //1.2.840.10045.3.1.7
    certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.id_ecPublicKey->size =
        sizeof(g_PRIME_256_V1_OID) / sizeof(g_PRIME_256_V1_OID[0]);

    //set subject Public Key
    certificate->tbsCertificate.subjectPublicKeyInfo.subjectPublicKey = *subjectPublicKey;

    //set signature algorithm
    certificate->signatureAlgorithm.algorithm.buf = (uint8_t *)g_ECDSA_WITH_SHA256_OID;
    certificate->signatureAlgorithm.algorithm.size =
        sizeof(g_ECDSA_WITH_SHA256_OID) / sizeof(g_ECDSA_WITH_SHA256_OID[0]);
    certificate->signatureAlgorithm.nul = OICCalloc(1, sizeof(NULL_t));
    CHECK_NULL(certificate->signatureAlgorithm.nul, ISSUER_X509_MEMORY_ALLOC_FAILED);

    //encode TBS to DER
    ec = der_encode_to_buffer(&asn_DEF_TBSCertificate, &(certificate->tbsCertificate),
                              tbsDer, ISSUER_MAX_CERT_SIZE);
    CHECK_COND(ec.encoded > 0, ISSUER_X509_DER_ENCODE_FAIL);
    tbs.len = ec.encoded;
    tbs.data = tbsDer;
    GET_SHA_256(tbs, sha256);
    CHECK_COND(uECC_sign((issuerPrivateKey->buf) + 1, sha256, signature),
               ISSUER_X509_SIGNATURE_FAIL);
            //additional byte for ASN1_UNCOMPRESSED_KEY_ID

    // ECDSA-Sig-Value ::= SEQUENCE { r INTEGER, s INTEGER } (RFC 5480)
    certificate->signatureValue.size = SIGN_FULL_SIZE + 6;// size for SEQUENCE ID + 2 * INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[0] > 127)
    {
        certificate->signatureValue.size ++;
    }

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[SIGN_R_LEN] > 127)
    {
        certificate->signatureValue.size ++;
    }
    certificate->signatureValue.buf = OICCalloc(certificate->signatureValue.size, sizeof(uint8_t));
    CHECK_NULL(certificate->signatureValue.buf, ISSUER_X509_MEMORY_ALLOC_FAILED);
    *(certificate->signatureValue.buf) = (12 << 2); //ASN.1 SEQUENCE ID
    *(certificate->signatureValue.buf + 1) = certificate->signatureValue.size - 2;
    //ASN.1 SEQUENCE size

    uint8Pointer = certificate->signatureValue.buf + 2; //skip SEQUENCE ID and size
    *uint8Pointer = (2 << 0); //ASN.1 INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[0] > 127)
    {
        *(uint8Pointer + 1) = SIGN_R_LEN + 1; //ASN.1 INTEGER size
        uint8Pointer += 3; //skip INTEGER ID and size
    }
    else
    {
        *(uint8Pointer + 1) = SIGN_R_LEN; //ASN.1 INTEGER SIZE
        uint8Pointer += 2; //skip INTEGER ID and size
    }
    memcpy(uint8Pointer, signature, SIGN_R_LEN);

    uint8Pointer += SIGN_R_LEN; //skip first part of signature
    *uint8Pointer = (2 << 0);   //ASN.1 INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature [SIGN_R_LEN] > 127)
    {
        *(uint8Pointer + 1) = SIGN_S_LEN + 1; //ASN.1 INTEGER size
        uint8Pointer += 3; //skip INTEGER ID and size
    }
    else
    {
        *(uint8Pointer + 1) = SIGN_S_LEN; //ASN.1 INTEGER size
        uint8Pointer += 2; //skip INTEGER ID and size
    }
    memcpy(uint8Pointer, signature + SIGN_R_LEN, SIGN_S_LEN);

    ec = der_encode_to_buffer(&asn_DEF_Certificate, certificate,
                              encodedCertificate->data, ISSUER_MAX_CERT_SIZE);
    CHECK_COND(ec.encoded > 0, ISSUER_X509_DER_ENCODE_FAIL);
    encodedCertificate->len = ec.encoded;

    FUNCTION_CLEAR(
        if (issuerTypeAndValue)
        {
            issuerTypeAndValue->value.buf = NULL;
            issuerTypeAndValue->type.buf  = NULL;
        }
        if (subjectTypeAndValue)
        {
            subjectTypeAndValue->value.buf = NULL;
            subjectTypeAndValue->type.buf  = NULL;
        }
        if (certificate)
        {
            certificate->tbsCertificate.validity.notBefore.buf                             = NULL;
            certificate->tbsCertificate.validity.notAfter.buf                              = NULL;
            certificate->tbsCertificate.subjectPublicKeyInfo.subjectPublicKey.buf          = NULL;
            certificate->tbsCertificate.signature.algorithm.buf                            = NULL;
            certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.algorithm.buf       = NULL;
            certificate->tbsCertificate.subjectPublicKeyInfo.algorithm.id_ecPublicKey->buf = NULL;
            certificate->signatureAlgorithm.algorithm.buf                                  = NULL;
        }
        ASN_STRUCT_FREE(asn_DEF_Certificate, certificate);
        certificate = NULL;
    );
}
