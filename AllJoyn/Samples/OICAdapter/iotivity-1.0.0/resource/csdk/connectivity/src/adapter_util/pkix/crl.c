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


#include "crl.h"
#include "byte_array.h"
#include "der_dec.h"
#include "sn_store.h"
#include "der_dec.h"
#include "crypto_adapter.h"


extern const uint8_t g_ECDSA_WITH_SHA256_OID[ECDSA_WITH_SHA256_OID_LEN];
extern const uint8_t g_EC_PUBLIC_KEY_OID[EC_PUBLIC_KEY_OID_LEN];
extern const uint8_t g_PRIME_256_V1_OID[PRIME_256_V1_OID_LEN];

/*
 *   TBSCertList  ::=  SEQUENCE  {
 *       version                 Version OPTIONAL,
 *                                     -- if present, MUST be v2
 *        signature               AlgorithmIdentifier,
 *        issuer                  Name,
 *        thisUpdate              Time,
 *        revokedCertificates     SEQUENCE OF SEQUENCE  {
 *             userCertificate         CertificateSerialNumber,
 *             revocationDate          Time
 *                                  }  OPTIONAL,
 *                                  }
*/


/**
 * Decodes TBS of CRL.
 */
static PKIError DecodeTbs(CertificateList *const crl)
{
    FUNCTION_INIT(
        size_t length;
        ByteArray tbs = crl->tbs, temp;
        CHECK_NULL(crl, PKI_NULL_PASSED);
        ByteArray sn = BYTE_ARRAY_INITIALIZER;
        FreeSNStore();
    );

    CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &tbs, &length);

    INC_BYTE_ARRAY(tbs, length); // skip algorithm identifier
    //1.2.840.10045.4.3.2
    //copy issuer X.500 name
    COPY_DER_FIELD(tbs, crl, issuer, DER_SEQUENCE, length);
    //copy date
    COPY_DER_FIELD(tbs, crl, date, DER_UTC_TIME, length);
    //COPY_DER_FIELD(tbs, crl, date, DER_UTC_TIME, length); // optional
    // copy serial numbers
    CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &tbs, &length);
    temp.data = tbs.data;
    temp.len = length;
    while (tbs.data < temp.data + temp.len)
    {
        CHECK_EQUAL(*(tbs.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
        CHECK_CALL(DecodeLength , &tbs, &length);
        //serial number
        CHECK_EQUAL(*(tbs.data), DER_INTEGER, PKI_INVALID_FORMAT);
        CHECK_CALL(DecodeLength , &tbs, &length);
        sn.data = tbs.data;
        sn.len = length;
        CHECK_CALL(StoreSerialNumber, sn);
        INC_BYTE_ARRAY(tbs, length);
        SKIP_DER_FIELD(tbs, DER_UTC_TIME, length);
    }
    FUNCTION_CLEAR();
}

/*
 * CertificateList  ::=  SEQUENCE  {
 *      tbsCertList          TBSCertList,
 *      signatureAlgorithm   AlgorithmIdentifier,
 *      signatureValue       BIT STRING  }
*/

/**
 * Decodes certificate in DER format.
 */
PKIError DecodeCertificateList(ByteArray code, CertificateList *crl, ByteArray caPubKey)
{
    FUNCTION_INIT(
        size_t length, tempLen;
        ByteArray temp;
        CHECK_NULL(crl, PKI_NULL_PASSED);
        CHECK_NULL(code.data, PKI_NULL_PASSED);
    );
    CHECK_EQUAL(*(code.data), DER_SEQUENCE, PKI_INVALID_FORMAT);
    CHECK_CALL(DecodeLength , &code, &length);
    //store sequence position
    temp = code;
    //TODO check length of TBS
    //copy tbs
    COPY_DER_FIELD(code, crl, tbs, DER_SEQUENCE, length);
    //decode tbs
    CHECK_CALL(DecodeTbs, crl);  //TODO
    //include sequense and len to tbs
    crl->tbs.len +=  crl->tbs.data - temp.data;
    crl->tbs.data = temp.data;

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
    COPY_DER_FIELD(code, crl, signR, DER_INTEGER, length);
    //copy sign s value
    COPY_DER_FIELD(code, crl, signS, DER_INTEGER, length);
    if (caPubKey.data != NULL)
    {
        PARSE_SIGNATURE(crl);
        CHECK_SIGN(*crl, caPubKey);
    }
    FUNCTION_CLEAR();
}

#ifdef X509_DEBUG
/**
 * Prints CRL to console.
 */
PKIError PrintCRL(const CertificateList *const crl)
{
    FUNCTION_INIT(
        CHECK_NULL(crl, PKI_NULL_PASSED);
    );
    printf("\n-----BEGIN CRL-----\n");
    PRINT_BYTE_ARRAY("ISSUER:\n", crl->issuer);
    PRINT_BYTE_ARRAY("DATE:\n", crl->date);
    PRINT_BYTE_ARRAY("TBS:\n", crl->tbs);
    printf("-----END CRL-----\n");
    FUNCTION_CLEAR(
    );
}
#endif
