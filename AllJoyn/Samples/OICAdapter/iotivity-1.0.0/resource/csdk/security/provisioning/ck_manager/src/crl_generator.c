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

#include "crl_generator.h"
#include "pki.h"
#include "oic_malloc.h"
#include "ckm_info.h"

//ecdsa-with-SHA256 1.2.840.10045.4.3.2 [RFC5759]
static const uint8_t g_ECDSA_WITH_SHA256_OID[] = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02};

//commonName 2.5.4.3 [RFC2256]
static const uint8_t g_COMMON_NAME_OID[] = {0x55, 0x04, 0x03};

PKIError GenerateCRL (const UTF8String_t *issuerName,
                       const UTCTime_t *thisUpdateTime, const uint32_t nuberOfRevoked,
                       const CertificateRevocationInfo_t *certificateRevocationInfo,
                       const BIT_STRING_t *issuerPrivateKey, ByteArray *encodedCRL)
{
    FUNCTION_INIT();

    CertificateRevocationList_t *certificateRevocationList = NULL; /* Type to encode */
    AttributeTypeAndValue_t *issuerTypeAndValue     = NULL;
    RelativeDistinguishedName_t *issuerRDN          = NULL;
    CertificateRevocationInfo_t *cri                = NULL;

    uint32_t crlMaxSize = (CRL_MIN_SIZE +
            nuberOfRevoked * (sizeof(CertificateRevocationInfo_t) + 4));

    uint32_t i;
    long serialNumber = 0;

    CHECK_NULL(issuerName, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(thisUpdateTime, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(certificateRevocationInfo, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(issuerPrivateKey, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(encodedCRL, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(encodedCRL->data, ISSUER_CRL_NULL_PASSED);
    CHECK_LESS_EQUAL(crlMaxSize, encodedCRL->len, ISSUER_CRL_WRONG_BYTE_ARRAY_LEN);

    /* Allocate the memory */
    certificateRevocationList      = OICCalloc(1, sizeof(CertificateRevocationList_t));//not malloc!
    CHECK_NULL(certificateRevocationList, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    issuerTypeAndValue = OICCalloc(1, sizeof(AttributeTypeAndValue_t));
    CHECK_NULL(issuerTypeAndValue, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    issuerRDN          = OICCalloc(1, sizeof(RelativeDistinguishedName_t));
    CHECK_NULL(issuerRDN, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    //set subject name
    issuerTypeAndValue->value = *issuerName;
    issuerTypeAndValue->type.buf = (uint8_t *)g_COMMON_NAME_OID;  //2.5.4.3
    issuerTypeAndValue->type.size = sizeof(g_COMMON_NAME_OID) / sizeof(g_COMMON_NAME_OID[0]);
    ASN_SET_ADD(issuerRDN, issuerTypeAndValue);
    ASN_SEQUENCE_ADD(&(certificateRevocationList->tbsCertList.issuer), issuerRDN);

    //set signature algorithm
    certificateRevocationList->signatureAlgorithm.algorithm.buf =
        (uint8_t *)g_ECDSA_WITH_SHA256_OID;    //1.2.840.10045.4.3.2
    certificateRevocationList->signatureAlgorithm.algorithm.size =
        sizeof(g_ECDSA_WITH_SHA256_OID) / sizeof(g_ECDSA_WITH_SHA256_OID[0]);
    certificateRevocationList->signatureAlgorithm.nul = OICCalloc(1, sizeof(NULL_t));
    CHECK_NULL(certificateRevocationList->signatureAlgorithm.nul,
               ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    //set signature algorithm in TBS part
    certificateRevocationList->tbsCertList.signature.algorithm.buf =
        (uint8_t *)g_ECDSA_WITH_SHA256_OID;    //1.2.840.10045.4.3.2
    certificateRevocationList->tbsCertList.signature.algorithm.size =
        sizeof(g_ECDSA_WITH_SHA256_OID) / sizeof(g_ECDSA_WITH_SHA256_OID[0]);
    certificateRevocationList->tbsCertList.signature.nul = OICCalloc(1, sizeof(NULL_t));
    CHECK_NULL(certificateRevocationList->tbsCertList.signature.nul,
               ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    //set thisUpdateTime
    certificateRevocationList->tbsCertList.thisUpdate = *thisUpdateTime;

    //add revoked info
    for ( i = 0; i < nuberOfRevoked; i++)
    {
        cri = OICCalloc(1, sizeof(CertificateRevocationInfo_t));
        CHECK_NULL(cri, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

        cri->revocationDate.size = (certificateRevocationInfo + i)->revocationDate.size;
        cri->revocationDate.buf = OICCalloc((cri->revocationDate.size) + 1, sizeof(char));
        CHECK_NULL(cri->revocationDate.buf, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

        memcpy(cri->revocationDate.buf, (certificateRevocationInfo + i)->revocationDate.buf,
               cri->revocationDate.size + 1);
        cri->userCertificate = (certificateRevocationInfo + i)->userCertificate;
        ASN_SEQUENCE_ADD((void *)(&(certificateRevocationList->
                tbsCertList.revokedCertificates.list)), (void *)(cri));
    }

    CHECK_CALL(SignCRL, certificateRevocationList, crlMaxSize, issuerPrivateKey, encodedCRL);

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCRLSerialNumber, &serialNumber);
    serialNumber++;
    CHECK_CALL(SetCRLSerialNumber, &serialNumber);
    CHECK_CALL(SetNumberOfRevoked, (const long *)&nuberOfRevoked);
    CHECK_CALL(SaveCKMInfo);

    FUNCTION_CLEAR(
        if (issuerTypeAndValue)
        {
            issuerTypeAndValue->value.buf                                                    = NULL;
            issuerTypeAndValue->type.buf                                                     = NULL;
        }
        if (certificateRevocationList)
        {
            certificateRevocationList->tbsCertList.signature.algorithm.buf                = NULL;
            certificateRevocationList->signatureAlgorithm.algorithm.buf                   = NULL;
            certificateRevocationList->tbsCertList.thisUpdate.buf                         = NULL;
        }

        ASN_STRUCT_FREE(asn_DEF_CertificateRevocationList, certificateRevocationList);
        certificateRevocationList = NULL;
    );
}

PKIError SignCRL(CertificateRevocationList_t *certificateRevocationList,
                   const uint32_t crlMaxSize, const BIT_STRING_t *issuerPrivateKey,
                   ByteArray *encodedCRL)
{
    FUNCTION_INIT();
    uint8_t *crlInfoInDER                        = NULL;
    asn_enc_rval_t ec; /* Encoder return value */
    uint8_t *uint8Pointer                        = NULL;
    ByteArray tbs                                = BYTE_ARRAY_INITIALIZER;
    uint8_t signature[SIGN_FULL_SIZE];
    uint8_t sha256[SHA_256_HASH_LEN];

    CHECK_NULL(certificateRevocationList, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(crlMaxSize, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(issuerPrivateKey, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(encodedCRL, ISSUER_CRL_NULL_PASSED);
    CHECK_NULL(encodedCRL->data, ISSUER_CRL_NULL_PASSED);
    CHECK_LESS_EQUAL(crlMaxSize, encodedCRL->len, ISSUER_CRL_WRONG_BYTE_ARRAY_LEN);

    //encode TBS to DER
    crlInfoInDER = OICCalloc(1, crlMaxSize);
    CHECK_NULL(crlInfoInDER, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    ec = der_encode_to_buffer(&asn_DEF_TBSCertList, &(certificateRevocationList->tbsCertList),
                              crlInfoInDER, crlMaxSize);

    //sign CRL
    CHECK_COND(ec.encoded > 0, ISSUER_CRL_ENCODER_DER_ENCODE_FAIL);
    tbs.len = ec.encoded;
    tbs.data = crlInfoInDER;
    GET_SHA_256(tbs, sha256);
    CHECK_COND(uECC_sign((issuerPrivateKey->buf) + 1, sha256, signature),
               ISSUER_CRL_ENCODER_SIGNATURE_FAIL);
    //additional byte for ASN1_UNCOMPRESSED_KEY_ID

    // ECDSA-Sig-Value ::= SEQUENCE { r INTEGER, s INTEGER } (RFC 5480)
    certificateRevocationList->signatureValue.size = SIGN_FULL_SIZE + 6;
    // size for SEQUENCE ID + 2 * INTEGER ID

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[0] > 127)
    {
        certificateRevocationList->signatureValue.size ++;
    }

    // if first byte of positive INTEGER exceed 127 add 0 byte before
    if (signature[SIGN_R_LEN] > 127)
    {
        certificateRevocationList->signatureValue.size ++;
    }
    OICFree(certificateRevocationList->signatureValue.buf);
    certificateRevocationList->signatureValue.buf = (uint8_t *)OICCalloc(
                certificateRevocationList->signatureValue.size, sizeof(uint8_t));
    CHECK_NULL(certificateRevocationList->signatureValue.buf,
               ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);
    *(certificateRevocationList->signatureValue.buf) = (12 << 2); //ASN.1 SEQUENCE ID
    *(certificateRevocationList->signatureValue.buf + 1) =
        certificateRevocationList->signatureValue.size - 2; //ASN.1 SEQUENCE size

    uint8Pointer = certificateRevocationList->signatureValue.buf + 2; //skip SEQUENCE ID and size
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
        *(uint8Pointer + 1) = SIGN_S_LEN + 1; //ASN.1 INTEGER size
        uint8Pointer += 3; //skip INTEGER ID and size
    }
    else
    {
        *(uint8Pointer + 1) = SIGN_S_LEN; //ASN.1 INTEGER size
        uint8Pointer += 2; //skip INTEGER ID and size
    }
    memcpy(uint8Pointer, signature + SIGN_R_LEN, SIGN_S_LEN);

    ec = der_encode_to_buffer(&asn_DEF_CertificateRevocationList, certificateRevocationList,
                              encodedCRL->data, crlMaxSize);
    CHECK_COND(ec.encoded > 0, ISSUER_CRL_ENCODER_DER_ENCODE_FAIL);
    encodedCRL->len = ec.encoded;

    FUNCTION_CLEAR(
        OICFree(crlInfoInDER);
        crlInfoInDER = NULL;
    );
}
