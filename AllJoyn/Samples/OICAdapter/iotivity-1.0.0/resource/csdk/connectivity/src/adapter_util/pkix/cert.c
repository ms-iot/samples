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


#include "byte_array.h"
#include "cert.h"
#include "der_dec.h"


extern const uint8_t g_ECDSA_WITH_SHA256_OID[];
extern const uint8_t g_EC_PUBLIC_KEY_OID[];
extern const uint8_t g_PRIME_256_V1_OID[];

/**
 * Decodes TBSCertificate.
 */
static PKIError DecodeTbs(CertificateX509 *const crt)
{
    FUNCTION_INIT(
        size_t length, temp_len;
        ByteArray tbs = crt->tbs, temp;
        CHECK_NULL(crt, PKI_NULL_PASSED);
    );
    //skip version
    SKIP_DER_FIELD(tbs, DER_VERSION, length);
    //serial number
    COPY_DER_FIELD(tbs, crt, serNum, DER_INTEGER, length);

    CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &tbs, &length);
    //copy to temp
    temp = tbs; // OPTIONAL
    INC_BYTE_ARRAY(tbs, length); // skip algorithm identifier
    //check_signature_algorithm
    //1.2.840.10045.4.3.2
    CHECK_DER_OID(temp, g_ECDSA_WITH_SHA256_OID, ECDSA_WITH_SHA256_OID_LEN, temp_len);
    //copy issuer X.500 name
    COPY_DER_FIELD(tbs, crt, issuer, DER_SEQUENCE, length);
    CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &tbs, &length);

    //copy valid period
    COPY_DER_FIELD(tbs, crt, validFrom, DER_UTC_TIME, length);
    COPY_DER_FIELD(tbs, crt, validTo, DER_UTC_TIME, length);

    //copy subject X.500 name
    COPY_DER_FIELD(tbs, crt, subject, DER_SEQUENCE, length);
    //public key
    CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &tbs, &length);
    CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &tbs, &length);
    //check public key type
    //1.2.840.10045.2.1
    CHECK_DER_OID(tbs, g_EC_PUBLIC_KEY_OID, EC_PUBLIC_KEY_OID_LEN, length);
    INC_BYTE_ARRAY(tbs, length);
    //check curve
    //1.2.840.10045.3.1.7
    CHECK_DER_OID(tbs, g_PRIME_256_V1_OID, PRIME_256_V1_OID_LEN, length);
    INC_BYTE_ARRAY(tbs, length);
    //copy public key
    COPY_DER_FIELD(tbs, crt, pubKey, DER_BIT_STRING, length);
    FUNCTION_CLEAR();
}


/**
 * Decodes certificate in DER format.
 */
PKIError DecodeCertificate(ByteArray code, CertificateX509 *crt)
{
    FUNCTION_INIT(
        size_t length, tempLen;
        ByteArray temp;
        CHECK_NULL(crt, PKI_NULL_PASSED);
        CHECK_NULL(code.data, PKI_NULL_PASSED);
    );
    CHECK_EQUAL(*(code.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &code, &length);
    //store sequence position
    temp = code;
    //TODO check length of TBS
    //copy tbs
    COPY_DER_FIELD(code, crt, tbs, DER_SEQUENCE, length);
    //decode tbs
    CHECK_CALL(DecodeTbs, crt);
    //include sequense and len to tbs
    crt->tbs.len +=  crt->tbs.data - temp.data;
    crt->tbs.data = temp.data;
    //printf("DATA %02X\n", *(code.data));
    CHECK_EQUAL(*(code.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &code, &length);
    //copy to temp
    temp = code;
    INC_BYTE_ARRAY(code, length); // skip algorithm identifier
    //check_signature_algorithm
    //1.2.840.10045.4.3.2
    CHECK_DER_OID(temp, g_ECDSA_WITH_SHA256_OID, ECDSA_WITH_SHA256_OID_LEN, tempLen);
    //decode_signature_value
    CHECK_EQUAL(*(code.data), DER_BIT_STRING, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &code, &length);
    //skip DER_UNIVERSAL
    CHECK_EQUAL(*(code.data), DER_UNIVERSAL, PKI_INVALID_FORMAT);
    CHECK_INC_BYTE_ARRAY(code, 1);
    CHECK_EQUAL(*(code.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &code, &length);
    //copy sign r value
    COPY_DER_FIELD(code, crt, signR, DER_INTEGER, length);
    //copy sign s value
    COPY_DER_FIELD(code, crt, signS, DER_INTEGER, length);

    PARSE_SIGNATURE(crt);

    FUNCTION_CLEAR();
}

#ifdef X509_DEBUG
/**
 * Prints certificate to console.
 */
PKIError PrintCertificate(const CertificateX509 *const crt)
{
    FUNCTION_INIT(
        CHECK_NULL(crt, PKI_NULL_PASSED);
    );
    printf("\n-----BEGIN CERTIFICATE-----\n");
    PRINT_BYTE_ARRAY("SER NUM:\n", crt->serNum);
    PRINT_BYTE_ARRAY("ISSUER:\n", crt->issuer);
    PRINT_BYTE_ARRAY("SUBJECT:\n", crt->subject);
    PRINT_BYTE_ARRAY("PUB KEY:\n", crt->pubKey);
    PRINT_BYTE_ARRAY("SIGN R VALUE:\n", crt->signR);
    PRINT_BYTE_ARRAY("SIGN S VALUE:\n", crt->signS);
    PRINT_BYTE_ARRAY("TBS:\n", crt->tbs);
    printf("-----END CERTIFICATE-----\n");
    FUNCTION_CLEAR(
    );
}
#endif

PKIError ParsePublicKey(ByteArray *caPublicKey)
{
    FUNCTION_INIT(
        CHECK_NULL(caPublicKey, PKI_NULL_PASSED);
    );

    if ((caPublicKey->len == PUBLIC_KEY_SIZE + 2) && (caPublicKey->data[0] == 0)
        && (caPublicKey->data[1] == ASN1_UNCOMPRESSED_KEY))
        INC_BYTE_ARRAY(*caPublicKey, 2);
    else if (caPublicKey->len != PUBLIC_KEY_SIZE)
        CHECK_NULL(NULL, PKI_WRONG_ARRAY_LEN);

    FUNCTION_CLEAR();
}
