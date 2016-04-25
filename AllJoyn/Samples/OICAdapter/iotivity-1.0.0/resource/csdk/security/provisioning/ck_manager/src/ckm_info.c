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


#include "ckm_info.h"
#include "ocstack.h"
#include "oic_malloc.h"
#include "cJSON.h"
#include "base64.h"
#include "psinterface.h"
#include "srmresourcestrings.h"
#include "crlresource.h"
#include "crl_generator.h"

//constants used in ckmInfo
#define CKM_INFO_IS_NOT_LOADED                       (0)
#define CKM_INFO_IS_LOADED                           (1)
#define CA_PRIVATE_KEY_IS_NOT_SET                    (0)
#define CA_PRIVATE_KEY_IS_SET                        (1)
#define CA_PRIVATE_KEY_DEFAULT_VALUE                 (0)
#define CA_PUBLIC_KEY_IS_NOT_SET                     (0)
#define CA_PUBLIC_KEY_IS_SET                         (1)
#define CA_PUBLIC_KEY_DEFAULT_VALUE                  (0)
#define CA_CERTIFICATE_CHAIN_IS_NOT_SET              (0)
#define CA_CERTIFICATE_CHAIN_MEMORY_IS_NOT_ALLOCATED (0)
#define CA_NAME_IS_NOT_SET                           (0)
#define CA_NAME_DEFAULT_VALUE                        (0)
#define CERTIFICATE_SN_INITIAL_VALUE                 (1)
#define CRL_SN_INITIAL_VALUE                         (1)
#define NUMBER_OF_REVOKED_CERTIFICATES_INITIAL_VALUE (0)

//constants used in crlInfo
#define CRL_IS_NOT_SET                               (0)
#define CRL_MEMORY_IS_NOT_ALLOCATED                  (0)

static CKMInfo_t g_ckmInfo = {CKM_INFO_IS_NOT_LOADED,
                             CA_PRIVATE_KEY_IS_NOT_SET, {CA_PRIVATE_KEY_DEFAULT_VALUE},
                             CA_PUBLIC_KEY_IS_NOT_SET, {CA_PUBLIC_KEY_DEFAULT_VALUE},
                             CA_CERTIFICATE_CHAIN_IS_NOT_SET,
                             CA_CERTIFICATE_CHAIN_MEMORY_IS_NOT_ALLOCATED,
                             CA_NAME_IS_NOT_SET, {CA_NAME_DEFAULT_VALUE},
                             CERTIFICATE_SN_INITIAL_VALUE, CRL_SN_INITIAL_VALUE,
                             NUMBER_OF_REVOKED_CERTIFICATES_INITIAL_VALUE};

static OicSecCrl_t g_crlInfo = {CRL_IS_NOT_SET,
                                  BYTE_ARRAY_INITIALIZER, BYTE_ARRAY_INITIALIZER};

//General functions

PKIError InitCKMInfo(void)
{
    FUNCTION_INIT();
    FILE *filePointer = NULL;
    int count = 1;
    int objectsRead = 0;
    int objectsWrote = 0;

    if (!g_ckmInfo.CKMInfoIsLoaded)
    {
        filePointer = fopen(CA_STORAGE_FILE, "rb");
        if (filePointer) //read existing storage
        {
            objectsRead = fread(&g_ckmInfo, sizeof(CKMInfo_t), count, filePointer);
            g_ckmInfo.CACertificateChain = CA_CERTIFICATE_CHAIN_MEMORY_IS_NOT_ALLOCATED;
            CHECK_EQUAL(objectsRead, count, ISSUER_CA_STORAGE_FILE_READ_ERROR);
        }
        else ////create new storage
        {
            filePointer = fopen(CA_STORAGE_FILE, "wb");
            CHECK_NULL(filePointer, ISSUER_CA_STORAGE_FILE_WRITE_ERROR);
            objectsWrote = fwrite(&g_ckmInfo, sizeof(CKMInfo_t), count, filePointer);
            CHECK_EQUAL(objectsWrote, count, ISSUER_CA_STORAGE_FILE_WRITE_ERROR);
        }
        CHECK_CALL(InitCRL);
        CHECK_CALL(InitCRT);
        g_ckmInfo.CKMInfoIsLoaded = CKM_INFO_IS_LOADED;
    }
    FUNCTION_CLEAR(
        if (filePointer)
        {
            fclose(filePointer);
            filePointer = NULL;
        }
    );
}

PKIError SaveCKMInfo(void)
{
    FUNCTION_INIT();
    FILE *filePointer = NULL;
    int count = 1;
    int objectsWrote = 0;

    CHECK_COND(g_ckmInfo.CKMInfoIsLoaded, CKM_INFO_IS_NOT_INIT);
    filePointer = fopen(CA_STORAGE_FILE, "wb");
    CHECK_NULL(filePointer, ISSUER_CA_STORAGE_FILE_WRITE_ERROR);
    objectsWrote = fwrite(&g_ckmInfo, sizeof(CKMInfo_t), count, filePointer);
    CHECK_EQUAL(objectsWrote, count, ISSUER_CA_STORAGE_FILE_WRITE_ERROR);
    if ((g_crlInfo.CrlData.data)&&(g_crlInfo.CrlData.len))
    {
        CHECK_CALL(SaveCRL);
    }
    if (g_ckmInfo.CAChainLength)
    {
        CHECK_CALL(SaveCRT);
    }
    FUNCTION_CLEAR(
        if (filePointer)
        {
            fclose(filePointer);
            filePointer = NULL;
        }
    );
}

PKIError CloseCKMInfo(void)
{
    FUNCTION_INIT();
    CHECK_CALL(SaveCKMInfo);
    OICFree(g_crlInfo.CrlData.data);
    g_crlInfo.CrlData.data = CRL_MEMORY_IS_NOT_ALLOCATED;
    OICFree(g_crlInfo.ThisUpdate.data);
    g_crlInfo.ThisUpdate.data = CRL_MEMORY_IS_NOT_ALLOCATED;
    OICFree(g_ckmInfo.CACertificateChain);
    g_ckmInfo.CACertificateChain = CA_CERTIFICATE_CHAIN_MEMORY_IS_NOT_ALLOCATED;
    g_ckmInfo.CKMInfoIsLoaded = CKM_INFO_IS_NOT_LOADED;
    g_crlInfo.CrlId = CRL_IS_NOT_SET;
    FUNCTION_CLEAR();
}

PKIError SetCKMInfo (const long *nextSN, const long *CRLSerialNumber,
                     const ByteArray *CAPrivateKey, const ByteArray *CAPublicKey,
                     const ByteArray *CAName)
{
    FUNCTION_INIT();
    if (nextSN)
        CHECK_CALL(SetNextSerialNumber, nextSN);
    if (CRLSerialNumber)
        CHECK_CALL(SetCRLSerialNumber, CRLSerialNumber);
    if (CAPrivateKey)
        CHECK_CALL(SetCAPrivateKey, CAPrivateKey);
    if (CAPublicKey)
        CHECK_CALL(SetCAPublicKey, CAPublicKey);
    if (CAName)
        CHECK_CALL(SetCAName, CAName);

    FUNCTION_CLEAR();
}

PKIError GetCKMInfo (long *nextSN, long *CRLSerialNumber,
                     ByteArray *CAPrivateKey, ByteArray *CAPublicKey,
                     ByteArray *CAName)
{
    FUNCTION_INIT();
    if (nextSN)
        CHECK_CALL(GetNextSerialNumber, nextSN);
    if (CRLSerialNumber)
        CHECK_CALL(GetCRLSerialNumber, CRLSerialNumber);
    if (CAPrivateKey)
        CHECK_CALL(GetCAPrivateKey, CAPrivateKey);
    if (CAPublicKey)
        CHECK_CALL(GetCAPublicKey, CAPublicKey);
    if (CAName)
        CHECK_CALL(GetCAName, CAName);

    FUNCTION_CLEAR();
}

/*Private Key*/
PKIError SetCAPrivateKey (const ByteArray *CAPrivateKey)
{
    FUNCTION_INIT();
    CHECK_NULL_BYTE_ARRAY_PTR(CAPrivateKey, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_EQUAL(CAPrivateKey->len, PRIVATE_KEY_SIZE, ISSUER_CA_STORAGE_WRONG_PRIVATE_KEY_LEN);
    memcpy(g_ckmInfo.CAPrivateKey, CAPrivateKey->data, PRIVATE_KEY_SIZE);
    g_ckmInfo.CAPrivateKeyIsSet = CA_PRIVATE_KEY_IS_SET;

    FUNCTION_CLEAR();
}

PKIError GetCAPrivateKey (ByteArray *CAPrivateKey)
{
    FUNCTION_INIT();
    CHECK_COND(g_ckmInfo.CAPrivateKeyIsSet, ISSUER_CA_STORAGE_PRIVATE_KEY_UNDEFINED);
    CHECK_NULL_BYTE_ARRAY_PTR(CAPrivateKey, ISSUER_CA_STORAGE_NULL_PASSED);
    memcpy(CAPrivateKey->data, g_ckmInfo.CAPrivateKey, PRIVATE_KEY_SIZE);
    CAPrivateKey->len = PRIVATE_KEY_SIZE;

    FUNCTION_CLEAR();
}

/*Public Key*/
PKIError SetCAPublicKey (const ByteArray *CAPublicKey)
{
    FUNCTION_INIT();
    CHECK_NULL_BYTE_ARRAY_PTR(CAPublicKey, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_EQUAL(CAPublicKey->len, PUBLIC_KEY_SIZE, ISSUER_CA_STORAGE_WRONG_PUBLIC_KEY_LEN);
    memcpy(g_ckmInfo.CAPublicKey, CAPublicKey->data, PUBLIC_KEY_SIZE);
    g_ckmInfo.CAPublicKeyIsSet = CA_PUBLIC_KEY_IS_SET;

    FUNCTION_CLEAR();
}

PKIError GetCAPublicKey (ByteArray *CAPublicKey)
{
    FUNCTION_INIT();
    CHECK_COND(g_ckmInfo.CAPublicKeyIsSet, ISSUER_CA_STORAGE_PUBLIC_KEY_UNDEFINED);
    CHECK_NULL_BYTE_ARRAY_PTR(CAPublicKey, ISSUER_CA_STORAGE_NULL_PASSED);
    memcpy(CAPublicKey->data, g_ckmInfo.CAPublicKey, PUBLIC_KEY_SIZE);
    CAPublicKey->len = PUBLIC_KEY_SIZE;

    FUNCTION_CLEAR();
}

/*CAName*/
PKIError SetCAName (const ByteArray *CAName)
{
    FUNCTION_INIT();
    CHECK_NULL_BYTE_ARRAY_PTR(CAName, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_LESS_EQUAL(CAName->len, ISSUER_MAX_NAME_SIZE, ISSUER_CA_STORAGE_WRONG_CA_NAME_LEN);
    memcpy(g_ckmInfo.CAName, CAName->data, CAName->len);
    g_ckmInfo.CANameSize = CAName->len;

    FUNCTION_CLEAR();
}

PKIError GetCAName (ByteArray *CAName)
{
    FUNCTION_INIT();
    CHECK_COND(g_ckmInfo.CANameSize, ISSUER_CA_STORAGE_CA_NAME_UNDEFINED);
    CHECK_NULL_BYTE_ARRAY_PTR(CAName, ISSUER_CA_STORAGE_NULL_PASSED);
    memcpy(CAName->data, g_ckmInfo.CAName, g_ckmInfo.CANameSize);
    CAName->len = g_ckmInfo.CANameSize;

    FUNCTION_CLEAR();
}

//Certificate-related functions

#define CERT_LEN_PREFIX (3)
#define BYTE_SIZE (8) //bits

static void WriteCertPrefix(uint8_t *prefix, uint32_t certLen)
{
    for (size_t i = 0; i < CERT_LEN_PREFIX; ++i)
    {
        prefix[i] = (certLen >> (BYTE_SIZE * (CERT_LEN_PREFIX - 1 - i))) & 0xFF;
    }
}

static uint32_t ParseCertPrefix(uint8_t *prefix)
{
    uint32_t res = 0;
    if(NULL != prefix)
    {
        for(int i=0; i < CERT_LEN_PREFIX; ++i)
        {
            res |= (((uint32_t) prefix[i]) << ((CERT_LEN_PREFIX - 1 -i) * BYTE_SIZE));
        }
    }
    return res;
}

PKIError InitCRT(void)
{
    FUNCTION_INIT();
    FILE *filePointer = NULL;
    uint32_t objectsRead = 0;
    uint8_t prefix[CERT_LEN_PREFIX] = {0};

    if (g_ckmInfo.CAChainLength)
    {
        filePointer = fopen(CA_STORAGE_CRT_FILE, "rb");
        CHECK_NULL(filePointer, ISSUER_CA_STORAGE_CRT_READ_ERROR);

        g_ckmInfo.CACertificateChain =
                    (ByteArray *)OICMalloc(sizeof(ByteArray) * g_ckmInfo.CAChainLength);
        CHECK_NULL(g_ckmInfo.CACertificateChain, ISSUER_CA_STORAGE_MEMORY_ALLOC_FAILED);

        for (int i = 0; i < g_ckmInfo.CAChainLength; i++)
        {
            objectsRead = fread(prefix, sizeof(uint8_t), CERT_LEN_PREFIX, filePointer);
            CHECK_EQUAL(objectsRead, CERT_LEN_PREFIX, ISSUER_CA_STORAGE_CRT_READ_ERROR);
            g_ckmInfo.CACertificateChain[i].len = ParseCertPrefix(prefix);

            g_ckmInfo.CACertificateChain[i].data =
                            (uint8_t *)OICMalloc(g_ckmInfo.CACertificateChain[i].len);
            CHECK_NULL(g_ckmInfo.CACertificateChain[i].data,
                       ISSUER_CA_STORAGE_MEMORY_ALLOC_FAILED);
            objectsRead = fread(g_ckmInfo.CACertificateChain[i].data, sizeof(uint8_t),
                                g_ckmInfo.CACertificateChain[i].len, filePointer);
            CHECK_EQUAL(objectsRead, g_ckmInfo.CACertificateChain[i].len,
                        ISSUER_CA_STORAGE_CRT_READ_ERROR);
        }
    }
    FUNCTION_CLEAR(
        if (filePointer)
        {
            fclose(filePointer);
            filePointer = NULL;
        }
    );
}

PKIError SaveCRT(void)
{
    FUNCTION_INIT();
    FILE *filePointer = NULL;
    uint32_t objectsWrote = 0;
    uint8_t prefix[CERT_LEN_PREFIX] = {0};

    filePointer = fopen(CA_STORAGE_CRT_FILE, "wb");
    CHECK_NULL(filePointer, ISSUER_CA_STORAGE_CRT_WRITE_ERROR);

    for (int i = 0; i < g_ckmInfo.CAChainLength; i++)
    {
        WriteCertPrefix(prefix, g_ckmInfo.CACertificateChain[i].len);
        objectsWrote = fwrite(prefix, sizeof(uint8_t), CERT_LEN_PREFIX, filePointer);
        CHECK_EQUAL(objectsWrote, CERT_LEN_PREFIX, ISSUER_CA_STORAGE_CRT_WRITE_ERROR);
        objectsWrote = fwrite(g_ckmInfo.CACertificateChain[i].data, sizeof(uint8_t),
                              g_ckmInfo.CACertificateChain[i].len, filePointer);
        CHECK_EQUAL(objectsWrote, g_ckmInfo.CACertificateChain[i].len,
                    ISSUER_CA_STORAGE_CRT_WRITE_ERROR);
    }

    FUNCTION_CLEAR(
        if (filePointer)
        {
            fclose(filePointer);
            filePointer = NULL;
        }
    );
}

/*Serial Number*/
PKIError SetNextSerialNumber (const long *nextSN)
{
    FUNCTION_INIT();
    CHECK_NULL(nextSN, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_LESS_EQUAL(0, *nextSN, ISSUER_CA_STORAGE_WRONG_SERIAL_NUMBER);
    g_ckmInfo.nextSerialNumber = *nextSN;

    FUNCTION_CLEAR();
}

PKIError GetNextSerialNumber (long *nextSN)
{
    FUNCTION_INIT();
    CHECK_NULL(nextSN, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_NULL(g_ckmInfo.nextSerialNumber, ISSUER_CA_STORAGE_SN_UNDEFINED);
    *nextSN = g_ckmInfo.nextSerialNumber;

    FUNCTION_CLEAR();
}

/*CA Certificate Chain*/
PKIError SetCAChain (const uint8_t CAChainLength, const ByteArray *CAChain)
{
    FUNCTION_INIT();
    CHECK_NULL_BYTE_ARRAY_PTR(CAChain, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_NULL(CAChainLength, ISSUER_CA_STORAGE_NULL_PASSED);

    OICFree(g_ckmInfo.CACertificateChain);
    g_ckmInfo.CACertificateChain = NULL;
    g_ckmInfo.CACertificateChain = (ByteArray *)OICMalloc(sizeof(ByteArray) * CAChainLength);
    CHECK_NULL(g_ckmInfo.CACertificateChain, ISSUER_CA_STORAGE_MEMORY_ALLOC_FAILED);

    for (int i = 0; i < CAChainLength; i++)
    {
        g_ckmInfo.CACertificateChain[i].data = (uint8_t *)OICMalloc(CAChain[i].len);
        CHECK_NULL(g_ckmInfo.CACertificateChain[i].data, ISSUER_CA_STORAGE_MEMORY_ALLOC_FAILED);
        memcpy(g_ckmInfo.CACertificateChain[i].data, CAChain[i].data, CAChain[i].len);
        g_ckmInfo.CACertificateChain[i].len = CAChain[i].len;
    }
    g_ckmInfo.CAChainLength = CAChainLength;

    FUNCTION_CLEAR();
}

PKIError GetCAChain (uint8_t* CAChainLength, ByteArray *CAChain)
{
    FUNCTION_INIT();
    CHECK_COND(g_ckmInfo.CAChainLength, ISSUER_CA_STORAGE_CA_CHAIN_LENGTH_UNDEFINED);
    CHECK_NULL_BYTE_ARRAY_PTR(CAChain, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_NULL(CAChainLength, PKI_NULL_PASSED);

    for (int i = 0; i < g_ckmInfo.CAChainLength; i++)
    {
        CHECK_LESS_EQUAL(g_ckmInfo.CACertificateChain[i].len, CAChain[i].len,
                         ISSUER_CA_STORAGE_WRONG_BYTE_ARRAY_LEN);
        memcpy(CAChain[i].data, g_ckmInfo.CACertificateChain[i].data,
               g_ckmInfo.CACertificateChain[i].len);
        CAChain[i].len = g_ckmInfo.CACertificateChain[i].len;
    }

    *CAChainLength = g_ckmInfo.CAChainLength;

    FUNCTION_CLEAR();
}

/*Certificate*/
PKIError SetCACertificate (const ByteArray *CACertificate)
{
    FUNCTION_INIT();
    CHECK_NULL_BYTE_ARRAY_PTR(CACertificate, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_CALL(SetCAChain, 1, CACertificate);

    FUNCTION_CLEAR();
}

PKIError GetCACertificate (ByteArray *CACertificate)
{
    FUNCTION_INIT();
    uint8_t i;
    CHECK_NULL_BYTE_ARRAY_PTR(CACertificate, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_CALL(GetCAChain, &i, CACertificate);

    FUNCTION_CLEAR();
}
//CRL-related functions

PKIError InitCRL(void)
{
    FUNCTION_INIT();
    g_crlInfo = *(OicSecCrl_t *)GetCRLResource();
    CHECK_NULL(g_crlInfo.CrlData.data, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_NULL(g_crlInfo.ThisUpdate.data, ISSUER_CA_STORAGE_NULL_PASSED);

    FUNCTION_CLEAR();
}

PKIError SaveCRL(void)
{
    FUNCTION_INIT();

    CHECK_EQUAL(UpdateCRLResource(&g_crlInfo),
                OC_STACK_OK, ISSUER_CA_STORAGE_CRL_WRITE_ERROR);
    FUNCTION_CLEAR();
}

/*CRL Serial Number*/
PKIError SetCRLSerialNumber (const long *CRLSerialNumber)
{
    FUNCTION_INIT();
    CHECK_NULL(CRLSerialNumber, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_LESS_EQUAL(0, *CRLSerialNumber, ISSUER_CA_STORAGE_WRONG_CRL_SERIAL_NUMBER);
    g_ckmInfo.CRLSerialNumber = *CRLSerialNumber;

    FUNCTION_CLEAR();
}

PKIError GetCRLSerialNumber (long *CRLSerialNumber)
{
    FUNCTION_INIT();
    CHECK_NULL(CRLSerialNumber, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_NULL(g_ckmInfo.CRLSerialNumber, ISSUER_CA_STORAGE_CRL_SN_UNDEFINED);
    *CRLSerialNumber = g_ckmInfo.CRLSerialNumber;

    FUNCTION_CLEAR();
}

/*Revocation List*/
PKIError SetCertificateRevocationList (const ByteArray *certificateRevocationList)
{
    FUNCTION_INIT();
    CHECK_NULL_BYTE_ARRAY_PTR(certificateRevocationList, ISSUER_CA_STORAGE_NULL_PASSED);

    OICFree(g_crlInfo.CrlData.data);
    g_crlInfo.CrlData.data = CRL_MEMORY_IS_NOT_ALLOCATED;
    g_crlInfo.CrlData.data = (uint8_t *)OICMalloc(certificateRevocationList->len + 1);
    CHECK_NULL(g_crlInfo.CrlData.data, ISSUER_CA_STORAGE_MEMORY_ALLOC_FAILED);
    memcpy(g_crlInfo.CrlData.data, certificateRevocationList->data, certificateRevocationList->len);
    g_crlInfo.CrlData.len = certificateRevocationList->len;

    g_crlInfo.CrlId ++;
    CHECK_CALL(SaveCRL);

    FUNCTION_CLEAR();
}

PKIError GetCertificateRevocationList (ByteArray *certificateRevocationList)
{
    FUNCTION_INIT();
    CHECK_COND(g_crlInfo.CrlData.data, ISSUER_CA_STORAGE_CRL_UNDEFINED);
    CHECK_NULL_BYTE_ARRAY_PTR(certificateRevocationList, ISSUER_CA_STORAGE_NULL_PASSED);
    OicSecCrl_t *tmpCRL;
    tmpCRL = (OicSecCrl_t *)GetCRLResource();
    g_crlInfo.CrlId = tmpCRL->CrlId;
    g_crlInfo.CrlData = tmpCRL->CrlData;
    g_crlInfo.ThisUpdate = tmpCRL->ThisUpdate;

    CHECK_LESS_EQUAL(g_crlInfo.CrlData.len, certificateRevocationList->len,
                     ISSUER_WRONG_BYTE_ARRAY_LEN);
    memcpy(certificateRevocationList->data, g_crlInfo.CrlData.data, g_crlInfo.CrlData.len);
    certificateRevocationList->len = g_crlInfo.CrlData.len;

    FUNCTION_CLEAR(
            OICFree(tmpCRL);
    );
}

PKIError SetNumberOfRevoked (const long *numberOfRevoked)
{
    FUNCTION_INIT();
    CHECK_NULL(numberOfRevoked, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_LESS_EQUAL(0, *numberOfRevoked, ISSUER_CA_STORAGE_WRONG_CRL_SERIAL_NUMBER);
    g_ckmInfo.numberOfRevoked = *numberOfRevoked;

    FUNCTION_CLEAR();
}
PKIError GetNumberOfRevoked (long *numberOfRevoked)
{
    FUNCTION_INIT();
    CHECK_NULL(numberOfRevoked, ISSUER_CA_STORAGE_NULL_PASSED);
    CHECK_NULL(g_ckmInfo.numberOfRevoked, ISSUER_CA_STORAGE_CRL_SN_UNDEFINED);
    *numberOfRevoked = g_ckmInfo.numberOfRevoked;

    FUNCTION_CLEAR();
}
