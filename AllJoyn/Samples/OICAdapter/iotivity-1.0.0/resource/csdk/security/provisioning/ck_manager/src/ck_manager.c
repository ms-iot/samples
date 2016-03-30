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

#include "ck_manager.h"
#include "crlresource.h"
#include "oic_malloc.h"

/* The first octet of the OCTET STRING indicates whether the key is
compressed or uncompressed.  The uncompressed form is indicated by 0x04
and the compressed form is indicated by either 0x02 or 0x03 (RFC 5480)*/
#define ASN1_UNCOMPRESSED_KEY_ID   (0x04)

PKIError GenerateCAKeyPair (ByteArray *caPrivateKey, ByteArray *caPublicKey)
{
    FUNCTION_INIT();

    CHECK_NULL(caPrivateKey, ISSUER_NULL_PASSED);
    CHECK_NULL(caPrivateKey->data, ISSUER_NULL_PASSED);
    CHECK_NULL(caPublicKey, ISSUER_NULL_PASSED);
    CHECK_NULL(caPublicKey->data, ISSUER_NULL_PASSED);

    CHECK_COND(uECC_make_key(caPublicKey->data, caPrivateKey->data), ISSUER_MAKE_KEY_ERROR);
    caPublicKey->len = PUBLIC_KEY_SIZE;
    caPrivateKey->len = PRIVATE_KEY_SIZE;

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(SetCAPrivateKey, caPrivateKey);
    CHECK_CALL(SetCAPublicKey, caPublicKey);
    CHECK_CALL(SaveCKMInfo);
    FUNCTION_CLEAR();
}

PKIError CKMIssueRootCertificate (const uint8_t *uint8NotBefore, const uint8_t *uint8NotAfter,
                                  ByteArray *issuedRootCertificate)
{
    FUNCTION_INIT();

    UTF8String_t *rootName          = NULL;
    UTCTime_t *notBefore            = NULL;
    UTCTime_t *notAfter             = NULL;
    BIT_STRING_t *subjectPublicKey  = NULL;
    BIT_STRING_t *issuerPrivateKey  = NULL;

    ByteArray pubKeyIss =  BYTE_ARRAY_INITIALIZER;
    ByteArray privKeyIss = BYTE_ARRAY_INITIALIZER;
    ByteArray caName = BYTE_ARRAY_INITIALIZER;

    uint8_t caPublicKey[PUBLIC_KEY_SIZE];
    uint8_t caPrivateKey[PRIVATE_KEY_SIZE];
    uint8_t uint8caName[ISSUER_MAX_NAME_SIZE];

    CHECK_NULL(issuedRootCertificate, ISSUER_NULL_PASSED);
    CHECK_NULL(issuedRootCertificate->data, ISSUER_NULL_PASSED);
    CHECK_LESS_EQUAL(ISSUER_MAX_CERT_SIZE, issuedRootCertificate->len, ISSUER_WRONG_BYTE_ARRAY_LEN);

    pubKeyIss.data = caPublicKey;
    pubKeyIss.len = PUBLIC_KEY_SIZE;
    privKeyIss.data = caPrivateKey;
    privKeyIss.len = PRIVATE_KEY_SIZE;
    caName.data = uint8caName;
    caName.len = ISSUER_MAX_NAME_SIZE;

    rootName = (UTF8String_t *)OICCalloc(1, sizeof(UTF8String_t));
    CHECK_NULL(rootName, ISSUER_MEMORY_ALLOC_FAILED);

    notBefore  = (UTCTime_t *)OICCalloc(1, sizeof(UTCTime_t));
    CHECK_NULL(notBefore, ISSUER_MEMORY_ALLOC_FAILED);

    notAfter = (UTCTime_t *)OICCalloc(1, sizeof(UTCTime_t));
    CHECK_NULL(notAfter, ISSUER_MEMORY_ALLOC_FAILED);

    subjectPublicKey = (BIT_STRING_t *)OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(subjectPublicKey, ISSUER_MEMORY_ALLOC_FAILED);

    issuerPrivateKey = (BIT_STRING_t *)OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(issuerPrivateKey, ISSUER_MEMORY_ALLOC_FAILED);

    //RootName
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAName, &caName);
    rootName->buf  = caName.data;
    rootName->size = caName.len;

    //notBefore
    if (uint8NotBefore)
    {
        notBefore->buf = (uint8_t *)uint8NotBefore;
    }
    else
    {
        notBefore->buf    = (uint8_t *)ISSUER_DEFAULT_NOT_BEFORE;
    }
    notBefore->size   = strlen((const char *)notBefore->buf);

    //notAfter
    if (uint8NotAfter)
    {
        notAfter->buf = (uint8_t *)uint8NotAfter;
    }
    else
    {
        notAfter->buf     = (uint8_t *)ISSUER_DEFAULT_NOT_AFTER;
    }
    notAfter->size    = strlen((const char *)notAfter->buf);

    //common keys
    issuerPrivateKey->size = PRIVATE_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    issuerPrivateKey->buf = (uint8_t *)OICCalloc((issuerPrivateKey->size), sizeof(uint8_t));
    CHECK_NULL(issuerPrivateKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(issuerPrivateKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;

    subjectPublicKey->size = PUBLIC_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    subjectPublicKey->buf = (uint8_t *)OICCalloc(subjectPublicKey->size, sizeof(uint8_t));
    CHECK_NULL(subjectPublicKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(subjectPublicKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;
    //common keys

    //read CA key pair from the CA storage
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAPrivateKey, &privKeyIss);

    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((issuerPrivateKey->buf) + 1, privKeyIss.data, PRIVATE_KEY_SIZE);
    CHECK_CALL(GetCAPublicKey, &pubKeyIss);

    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((subjectPublicKey->buf) + 1, pubKeyIss.data, PUBLIC_KEY_SIZE);

    CHECK_CALL(GenerateCertificate, rootName, rootName, notBefore, notAfter,
                             subjectPublicKey, issuerPrivateKey, issuedRootCertificate);

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(SetCACertificate, issuedRootCertificate);
    CHECK_CALL(SaveCKMInfo);

    FUNCTION_CLEAR(
        OICFree(rootName);
        OICFree(notBefore);
        OICFree(notAfter);
        ASN_STRUCT_FREE(asn_DEF_BIT_STRING, subjectPublicKey);
        ASN_STRUCT_FREE(asn_DEF_BIT_STRING, issuerPrivateKey);
    );
}

PKIError GenerateKeyPair (ByteArray *privateKey, ByteArray *publicKey)
{
    FUNCTION_INIT();
    CHECK_NULL(privateKey, ISSUER_NULL_PASSED);
    CHECK_NULL(privateKey->data, ISSUER_NULL_PASSED);
    CHECK_NULL(publicKey, ISSUER_NULL_PASSED);
    CHECK_NULL(publicKey->data, ISSUER_NULL_PASSED);
    CHECK_COND(uECC_make_key(publicKey->data, privateKey->data), ISSUER_MAKE_KEY_ERROR);
    publicKey->len = PUBLIC_KEY_SIZE;
    privateKey->len = PRIVATE_KEY_SIZE;
    FUNCTION_CLEAR();
}

PKIError CKMIssueDeviceCertificate (const uint8_t *uint8SubjectName,
                                    const uint8_t *uint8NotBefore, const uint8_t *uint8NotAfter,
                                    const uint8_t *uint8SubjectPublicKey,
                                    ByteArray *issuedCertificate)
{
    FUNCTION_INIT();

    UTF8String_t *subjectName       = NULL;
    UTF8String_t *issuerName        = NULL;
    UTCTime_t *notBefore            = NULL;
    UTCTime_t *notAfter             = NULL;
    BIT_STRING_t *subjectPublicKey  = NULL;
    BIT_STRING_t *issuerPrivateKey  = NULL;

    ByteArray privKeyIss  = BYTE_ARRAY_INITIALIZER;
    ByteArray pubKeySubj  = BYTE_ARRAY_INITIALIZER;
    ByteArray privKeySubj = BYTE_ARRAY_INITIALIZER;
    ByteArray caName      = BYTE_ARRAY_INITIALIZER;

    uint8_t subjPubKey[PUBLIC_KEY_SIZE];
    uint8_t subjPrivKey[PRIVATE_KEY_SIZE];
    uint8_t caPrivateKey[PRIVATE_KEY_SIZE];
    uint8_t uint8caName[ISSUER_MAX_NAME_SIZE];

    CHECK_NULL(uint8SubjectPublicKey, ISSUER_NULL_PASSED);
    CHECK_NULL(issuedCertificate, ISSUER_NULL_PASSED);
    CHECK_NULL(issuedCertificate->data, ISSUER_NULL_PASSED);
    CHECK_LESS_EQUAL(ISSUER_MAX_CERT_SIZE, issuedCertificate->len, ISSUER_WRONG_BYTE_ARRAY_LEN);

    privKeyIss.data = caPrivateKey;
    privKeyIss.len = PRIVATE_KEY_SIZE;
    pubKeySubj.data = subjPubKey;
    pubKeySubj.len = PUBLIC_KEY_SIZE;
    privKeySubj.data = subjPrivKey;
    privKeySubj.len = PRIVATE_KEY_SIZE;
    caName.data = uint8caName;
    caName.len = ISSUER_MAX_NAME_SIZE;

    subjectName = (UTF8String_t *)OICCalloc(1, sizeof(UTF8String_t));
    CHECK_NULL(subjectName, ISSUER_MEMORY_ALLOC_FAILED);

    issuerName = (UTF8String_t *)OICCalloc(1, sizeof(UTF8String_t));
    CHECK_NULL(issuerName, ISSUER_MEMORY_ALLOC_FAILED);

    notBefore = (UTCTime_t *)OICCalloc(1, sizeof(UTCTime_t));
    CHECK_NULL(notBefore, ISSUER_MEMORY_ALLOC_FAILED);

    notAfter = (UTCTime_t *)OICCalloc(1, sizeof(UTCTime_t));
    CHECK_NULL(notAfter, ISSUER_MEMORY_ALLOC_FAILED);

    subjectPublicKey = (BIT_STRING_t *)OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(subjectPublicKey, ISSUER_MEMORY_ALLOC_FAILED);

    issuerPrivateKey = (BIT_STRING_t *)OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(issuerPrivateKey, ISSUER_MEMORY_ALLOC_FAILED);

    //SubjectName
    if (uint8SubjectName)
    {
        subjectName->buf = (uint8_t *)uint8SubjectName;
    }
    else
    {
        subjectName->buf  = (uint8_t *)ISSUER_DEFAULT_SUBJECT_NAME;
    }
    subjectName->size = strlen((const char *)subjectName->buf);

    //IssuerName
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAName, &caName);
    issuerName->buf  = caName.data;
    issuerName->size = caName.len;

    //notBefore
    if (uint8NotBefore)
    {
        notBefore->buf = (uint8_t *)uint8NotBefore;
    }
    else
    {
        notBefore->buf    = (uint8_t *)ISSUER_DEFAULT_NOT_BEFORE;
    }
    notBefore->size   = strlen((const char *)notBefore->buf);

    //notAfter
    if (uint8NotAfter)
    {
        notAfter->buf = (uint8_t *)uint8NotAfter;
    }
    else
    {
        notAfter->buf     = (uint8_t *)ISSUER_DEFAULT_NOT_AFTER;
    }
    notAfter->size    = strlen((const char *)notAfter->buf);

    //common keys
    issuerPrivateKey->size = PRIVATE_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    issuerPrivateKey->buf = (uint8_t *)OICCalloc((issuerPrivateKey->size), sizeof(uint8_t));
    CHECK_NULL(issuerPrivateKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(issuerPrivateKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;

    subjectPublicKey->size = PUBLIC_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    subjectPublicKey->buf = (uint8_t *)OICCalloc(subjectPublicKey->size, sizeof(uint8_t));
    CHECK_NULL(subjectPublicKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(subjectPublicKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;
    //common keys

    //read CA private key from the CA storage
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAPrivateKey, &privKeyIss);

    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((issuerPrivateKey->buf) + 1, privKeyIss.data, PRIVATE_KEY_SIZE);

    if (!uint8SubjectPublicKey)
    {
        //GenerateKeyPair
        GenerateKeyPair(&privKeySubj, &pubKeySubj);
    }
    else
    {
        //additional byte for ASN1_UNCOMPRESSED_KEY_ID
        memcpy((subjectPublicKey->buf) + 1, uint8SubjectPublicKey, PUBLIC_KEY_SIZE);
    }

    CHECK_CALL(GenerateCertificate, subjectName, issuerName, notBefore, notAfter,
                             subjectPublicKey, issuerPrivateKey, issuedCertificate);

    FUNCTION_CLEAR(
        OICFree(subjectName);
        OICFree(issuerName);
        OICFree(notBefore);
        OICFree(notAfter);
        ASN_STRUCT_FREE(asn_DEF_BIT_STRING, subjectPublicKey);
        ASN_STRUCT_FREE(asn_DEF_BIT_STRING, issuerPrivateKey);
    );
}

PKIError GenerateDERCertificateFile (const ByteArray *certificate, const char *certFileName)
{
    FUNCTION_INIT();
    FILE *filePointer = NULL;

    CHECK_NULL(certFileName, ISSUER_NULL_PASSED);
    CHECK_NULL(certificate, ISSUER_NULL_PASSED);
    CHECK_NULL(certificate->data, ISSUER_NULL_PASSED);
    filePointer = fopen(certFileName, "wb");
    CHECK_NULL(filePointer, ISSUER_FILE_WRITE_ERROR);
    CHECK_EQUAL(fwrite(certificate->data, 1, certificate->len, filePointer), certificate->len,
            ISSUER_FILE_WRITE_ERROR);

    FUNCTION_CLEAR(
        if(filePointer)
            {
                fclose(filePointer);
            }
        filePointer = NULL;
    );
}

PKIError SetSerialNumber (const long serNum)
{
    FUNCTION_INIT();

    CHECK_LESS_EQUAL(0, serNum, ISSUER_WRONG_SERIAL_NUMBER);
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(SetNextSerialNumber, &serNum);
    CHECK_CALL(SaveCKMInfo);

    FUNCTION_CLEAR();
}

PKIError SetRootName (const ByteArray rootName)
{
    FUNCTION_INIT();

    CHECK_NULL(rootName.data, ISSUER_NULL_PASSED);
    CHECK_LESS(0, rootName.len, ISSUER_WRONG_ROOT_NAME_LEN);
    CHECK_LESS(rootName.len, ISSUER_MAX_NAME_SIZE, ISSUER_WRONG_ROOT_NAME_LEN);
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(SetCAName, &rootName);
    CHECK_CALL(SaveCKMInfo);

    FUNCTION_CLEAR();
}

PKIError CKMSetCAInfo (const long serNum, const ByteArray rootName)
{
    FUNCTION_INIT();
    CHECK_CALL(SetSerialNumber, serNum);
    CHECK_CALL(SetRootName, rootName);

    FUNCTION_CLEAR();
}

PKIError GenerateCSR (const uint8_t *uint8SubjectName,
                      const uint8_t *uint8SubjectPublicKey,
                      const uint8_t *uint8SubjectPrivateKey,
                      ByteArray *encodedCSR)
{
    FUNCTION_INIT();
    UTF8String_t *subjectName       = NULL;
    BIT_STRING_t *subjectPublicKey  = NULL;
    BIT_STRING_t *subjectPrivateKey  = NULL;

    CHECK_NULL(uint8SubjectPublicKey, ISSUER_NULL_PASSED);
    CHECK_NULL(uint8SubjectPrivateKey, ISSUER_NULL_PASSED);
    CHECK_NULL(encodedCSR, ISSUER_NULL_PASSED);
    CHECK_NULL(encodedCSR->data, ISSUER_NULL_PASSED);
    CHECK_LESS_EQUAL(CSR_MAX_SIZE, encodedCSR->len, ISSUER_WRONG_BYTE_ARRAY_LEN);

    subjectName = OICCalloc(1, sizeof(UTF8String_t));
    CHECK_NULL(subjectName, ISSUER_MEMORY_ALLOC_FAILED);

    subjectPublicKey = OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(subjectPublicKey, ISSUER_MEMORY_ALLOC_FAILED);

    subjectPrivateKey = OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(subjectPrivateKey, ISSUER_MEMORY_ALLOC_FAILED);

    //SubjectName
    if (uint8SubjectName)
    {
        subjectName->buf = (uint8_t *)uint8SubjectName;
    }
    else
    {
        subjectName->buf  = (uint8_t *)ISSUER_DEFAULT_SUBJECT_NAME;
    }
    subjectName->size = strlen((const char *)subjectName->buf);

    //common keys
    subjectPrivateKey->size = PRIVATE_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    subjectPrivateKey->buf = (uint8_t *)OICCalloc((subjectPrivateKey->size), sizeof(uint8_t));
    CHECK_NULL(subjectPrivateKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(subjectPrivateKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;

    subjectPublicKey->size = PUBLIC_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    subjectPublicKey->buf = (uint8_t *)OICCalloc(subjectPublicKey->size, sizeof(uint8_t));
    CHECK_NULL(subjectPublicKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(subjectPublicKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;
    //common keys

    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((subjectPrivateKey->buf) + 1, uint8SubjectPrivateKey, PRIVATE_KEY_SIZE);
    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((subjectPublicKey->buf) + 1, uint8SubjectPublicKey, PUBLIC_KEY_SIZE);

    CHECK_CALL(EncodeCSR, subjectName, subjectPublicKey, subjectPrivateKey, encodedCSR);

    FUNCTION_CLEAR(
        OICFree(subjectName);
        OICFree(subjectPublicKey);
        OICFree(subjectPrivateKey->buf);
        OICFree(subjectPrivateKey);
    );
}

PKIError GenerateCertificateByCSR (const ByteArray *encodedCSR, ByteArray *issuedCertificate)
{
    FUNCTION_INIT();
    UTF8String_t *subjectName = NULL;
    BIT_STRING_t *subjectPublicKey = NULL;
    uint8_t uint8SubjectName[ISSUER_MAX_NAME_SIZE];
    uint8_t uint8SubjectPublicKey[PUBLIC_KEY_SIZE + 1];

    CHECK_NULL(encodedCSR, ISSUER_NULL_PASSED);
    CHECK_NULL(encodedCSR->data, ISSUER_NULL_PASSED);
    CHECK_NULL(issuedCertificate, ISSUER_NULL_PASSED);
    CHECK_NULL(issuedCertificate->data, ISSUER_NULL_PASSED);
    CHECK_LESS_EQUAL(ISSUER_MAX_CERT_SIZE, issuedCertificate->len, ISSUER_WRONG_BYTE_ARRAY_LEN);

    subjectName = OICCalloc(1, sizeof(UTF8String_t));
    CHECK_NULL(subjectName, ISSUER_MEMORY_ALLOC_FAILED);

    subjectPublicKey = OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(subjectPublicKey, ISSUER_MEMORY_ALLOC_FAILED);

    subjectName->buf = uint8SubjectName;
    subjectPublicKey->buf = uint8SubjectPublicKey;

    CHECK_CALL(DecodeCSR, encodedCSR, subjectName, subjectPublicKey);

    uint8SubjectName[subjectName->size] = '\0';
    CHECK_CALL(CKMIssueDeviceCertificate, uint8SubjectName, 0, 0, uint8SubjectPublicKey + 1,
            //additional byte for ASN1_UNCOMPRESSED_KEY_ID
            issuedCertificate);

    FUNCTION_CLEAR(
        OICFree(subjectPublicKey);
        OICFree(subjectName);
    );
}

PKIError CKMIssueCRL (const uint8_t *uint8ThisUpdateTime, const uint32_t numberOfRevoked,
                      const uint32_t *revokedNumbers, const uint8_t **revocationDates,
                      ByteArray *encodedCRL)
{
    FUNCTION_INIT();
    BIT_STRING_t *issuerPrivateKey                          = NULL;
    UTCTime_t *thisUpdateTime                               = NULL;
    CertificateRevocationInfo_t *certificateRevocationInfo  = NULL;
    UTF8String_t *issuerName                                = NULL;
    uint32_t i;

    uint8_t caPrivateKey[PRIVATE_KEY_SIZE];
    uint8_t uint8caName[ISSUER_MAX_NAME_SIZE];

    ByteArray privKeyIss     = BYTE_ARRAY_INITIALIZER;
    ByteArray caName         = BYTE_ARRAY_INITIALIZER;

    CHECK_NULL(numberOfRevoked, ISSUER_NULL_PASSED);
    CHECK_NULL(revokedNumbers, ISSUER_NULL_PASSED);
    CHECK_NULL(revocationDates, ISSUER_NULL_PASSED);
    CHECK_NULL(encodedCRL, ISSUER_NULL_PASSED);
    CHECK_NULL(encodedCRL->data, ISSUER_NULL_PASSED);
    CHECK_LESS_EQUAL((CRL_MIN_SIZE + numberOfRevoked * (sizeof(CertificateRevocationInfo_t) + 4)),
                      encodedCRL->len, ISSUER_WRONG_BYTE_ARRAY_LEN);

    issuerPrivateKey          = (BIT_STRING_t *)OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(issuerPrivateKey, ISSUER_MEMORY_ALLOC_FAILED);

    thisUpdateTime            = (UTCTime_t *)OICCalloc(1, sizeof(UTCTime_t));
    CHECK_NULL(thisUpdateTime, ISSUER_MEMORY_ALLOC_FAILED);

    issuerName                  = (UTF8String_t *)OICCalloc(1, sizeof(UTF8String_t));
    CHECK_NULL(issuerName, ISSUER_MEMORY_ALLOC_FAILED);

    certificateRevocationInfo = (CertificateRevocationInfo_t *)OICCalloc(numberOfRevoked,
                                sizeof(CertificateRevocationInfo_t));
    CHECK_NULL(certificateRevocationInfo, ISSUER_MEMORY_ALLOC_FAILED);

    privKeyIss.data = caPrivateKey;
    privKeyIss.len  = PRIVATE_KEY_SIZE;
    caName.data     = uint8caName;
    caName.len      = ISSUER_MAX_NAME_SIZE;

    //allocate issuerPrivateKey
    issuerPrivateKey->size = PRIVATE_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    issuerPrivateKey->buf = (uint8_t *)OICCalloc((issuerPrivateKey->size), sizeof(uint8_t));
    CHECK_NULL(issuerPrivateKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(issuerPrivateKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;

    //read CA private key from the CA storage
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAPrivateKey, &privKeyIss);
    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((issuerPrivateKey->buf) + 1, privKeyIss.data, PRIVATE_KEY_SIZE);

    //thisUpdateTime
    if (uint8ThisUpdateTime)
    {
        thisUpdateTime->buf = (uint8_t *)uint8ThisUpdateTime;
    }
    else
    {
        thisUpdateTime->buf    = (uint8_t *)ISSUER_DEFAULT_THIS_UPDATE;
    }
    thisUpdateTime->size   = strlen((const char *)thisUpdateTime->buf);

    //RootName
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAName, &caName);
    issuerName->buf  = caName.data;
    issuerName->size = caName.len;

    // CRI
    for ( i = 0; i < numberOfRevoked; i++ )
    {
        certificateRevocationInfo[i].userCertificate = revokedNumbers[i];
        certificateRevocationInfo[i].revocationDate.buf = (uint8_t *)revocationDates[i];
        certificateRevocationInfo[i].revocationDate.size =
                strlen((const char *)revocationDates[i]);
    }

    CHECK_CALL(GenerateCRL, issuerName, thisUpdateTime, numberOfRevoked, certificateRevocationInfo,
                    issuerPrivateKey, encodedCRL);

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(SetCertificateRevocationList, encodedCRL);
    CHECK_CALL(SaveCKMInfo);

    FUNCTION_CLEAR(
        OICFree(issuerName);
        OICFree(thisUpdateTime);
        OICFree(certificateRevocationInfo);
        ASN_STRUCT_FREE(asn_DEF_BIT_STRING, issuerPrivateKey);
    );
}

PKIError CKMRevocateCertificate (const uint8_t *uint8ThisUpdateTime, const long revokedNumber,
                                 ByteArray *encodedCRL)
{
    FUNCTION_INIT();
    ByteArray oldCRL = BYTE_ARRAY_INITIALIZER;
    asn_dec_rval_t rval; /* Decoder return value */
    CertificateRevocationList_t *certificateRevocationList = NULL; // Type to decode
    CertificateRevocationInfo_t *CRI             = NULL;
    long serialNumber = 0;
    long numberOfRevoked = 0;
    uint32_t crlMaxSize = 0;

    BIT_STRING_t *issuerPrivateKey                          = NULL;
    uint8_t caPrivateKey[PRIVATE_KEY_SIZE];
    ByteArray privKeyIss     = BYTE_ARRAY_INITIALIZER;

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetNumberOfRevoked, &numberOfRevoked);

    crlMaxSize = (CRL_MIN_SIZE +
            (numberOfRevoked + 1) * (sizeof(CertificateRevocationInfo_t) + 4));

    CHECK_NULL(encodedCRL, ISSUER_NULL_PASSED);
    CHECK_NULL(encodedCRL->data, ISSUER_NULL_PASSED);
    CHECK_LESS_EQUAL(crlMaxSize, encodedCRL->len, ISSUER_WRONG_BYTE_ARRAY_LEN);

    //obtain CRL
    oldCRL.data = (uint8_t *)OICMalloc(crlMaxSize);
    CHECK_NULL(oldCRL.data, ISSUER_MEMORY_ALLOC_FAILED);
    oldCRL.len = crlMaxSize;

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCertificateRevocationList, &oldCRL);
    CHECK_CALL(CloseCKMInfo);

    //decode CRL
    rval = ber_decode(0, &asn_DEF_CertificateRevocationList, (void **)&certificateRevocationList,
                      oldCRL.data, oldCRL.len);
    CHECK_EQUAL(rval.code, RC_OK, ISSUER_CSR_DER_DECODE_FAIL);

    //add one certificate into CRL
    CRI = (CertificateRevocationInfo_t *)OICCalloc(1, sizeof(CertificateRevocationInfo_t));
    CHECK_NULL(CRI, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    CRI->revocationDate.size = (int)strlen((const char *)uint8ThisUpdateTime);
    CRI->revocationDate.buf = OICCalloc((CRI->revocationDate.size) + 1, sizeof(char));
    //additional byte for \0 at the end
    CHECK_NULL(CRI->revocationDate.buf, ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED);

    memcpy(CRI->revocationDate.buf, uint8ThisUpdateTime, CRI->revocationDate.size + 1);
    //additional byte for \0 at the end

    CRI->userCertificate = revokedNumber;
    ASN_SEQUENCE_ADD((void *)(&(certificateRevocationList->
            tbsCertList.revokedCertificates.list)), (void *)(CRI));

    //prepare memory for issuerPrivateKey
    issuerPrivateKey          = (BIT_STRING_t *)OICCalloc(1, sizeof(BIT_STRING_t));
    CHECK_NULL(issuerPrivateKey, ISSUER_MEMORY_ALLOC_FAILED);
    privKeyIss.data = caPrivateKey;
    privKeyIss.len  = PRIVATE_KEY_SIZE;
    //allocate issuerPrivateKey
    issuerPrivateKey->size = PRIVATE_KEY_SIZE + 1; //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    issuerPrivateKey->buf = (uint8_t *)OICCalloc((issuerPrivateKey->size), sizeof(uint8_t));
    CHECK_NULL(issuerPrivateKey->buf, ISSUER_MEMORY_ALLOC_FAILED);
    *(issuerPrivateKey->buf) = (uint8_t)ASN1_UNCOMPRESSED_KEY_ID;

    //read CA private key from the CA storage
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCAPrivateKey, &privKeyIss);

    //additional byte for ASN1_UNCOMPRESSED_KEY_ID
    memcpy((issuerPrivateKey->buf) + 1, privKeyIss.data, PRIVATE_KEY_SIZE);

    //SignCRL
    CHECK_CALL(SignCRL, certificateRevocationList, crlMaxSize, issuerPrivateKey, encodedCRL);

    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCRLSerialNumber, &serialNumber);
    serialNumber++;
    CHECK_CALL(SetCRLSerialNumber, &serialNumber);
    numberOfRevoked++;
    CHECK_CALL(SetNumberOfRevoked, &numberOfRevoked);
    CHECK_CALL(SetCertificateRevocationList, encodedCRL);
    CHECK_CALL(SaveCKMInfo);

    FUNCTION_CLEAR(
        ASN_STRUCT_FREE(asn_DEF_CertificateRevocationList, certificateRevocationList);
        certificateRevocationList = NULL;

    );
}

PKIError CKMGetCRL (ByteArray *certificateRevocationList)
{
    FUNCTION_INIT();
    CHECK_NULL(certificateRevocationList, ISSUER_NULL_PASSED);
    CHECK_NULL(certificateRevocationList->data, ISSUER_NULL_PASSED);
    CHECK_CALL(InitCKMInfo);
    CHECK_CALL(GetCertificateRevocationList, certificateRevocationList);
    CHECK_CALL(CloseCKMInfo);

    FUNCTION_CLEAR();
}
