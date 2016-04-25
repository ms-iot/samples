/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * *****************************************************************/
#include <string.h>
#include "credentialgenerator.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "logger.h"
#include "credresource.h"
#include "ocrandom.h"
#include "base64.h"
#include "stdbool.h"
#include "securevirtualresourcetypes.h"
#ifdef __WITH_X509__
#include "ck_manager.h"

#define CHAIN_LEN (2) //TODO: replace by external define or a runtime value
#endif  //__WITH_X509__

#define TAG "SRPAPI-CG"

/**
 * @def PM_VERIFY_SUCCESS
 * @brief Macro to verify success of operation.
 *        eg: PM_VERIFY_SUCCESS(TAG, OC_STACK_OK == foo(), OC_STACK_ERROR, ERROR);
 * @note Invoking function must define "bail:" label for goto functionality to work correctly and
 *       must define "OCStackResult res" for setting error code.
 * */
#define PM_VERIFY_SUCCESS(tag, op, errCode, logLevel) { if (!(op)) \
                       {OC_LOG((logLevel), tag, #op " failed!!"); res = errCode; goto bail;} }
/**
 * @def PM_VERIFY_NON_NULL
 * @brief Macro to verify argument is not equal to NULL.
 *        eg: PM_VERIFY_NON_NULL(TAG, ptrData, ERROR);
 * @note Invoking function must define "bail:" label for goto functionality to work correctly.
 * */
#define PM_VERIFY_NON_NULL(tag, arg, errCode, logLevel) { if (NULL == (arg)) \
                   { OC_LOG((logLevel), tag, #arg " is NULL"); res = errCode; goto bail;} }

OCStackResult PMGeneratePairWiseCredentials(OicSecCredType_t type, size_t keySize,
                                    const OicUuid_t *ptDeviceId,
                                    const OicUuid_t *firstDeviceId, const OicUuid_t *secondDeviceId,
                                    OicSecCred_t **firstCred, OicSecCred_t **secondCred)
{

    if (NULL == ptDeviceId || NULL == firstDeviceId || NULL != *firstCred || \
        NULL == secondDeviceId || NULL != *secondCred)
    {
        OC_LOG(INFO, TAG, "Invalid params");
        return OC_STACK_INVALID_PARAM;
    }
    if(!(keySize == OWNER_PSK_LENGTH_128 || keySize == OWNER_PSK_LENGTH_256))
    {
        OC_LOG(INFO, TAG, "Invalid key size");
        return OC_STACK_INVALID_PARAM;
    }
    OCStackResult res = OC_STACK_ERROR;
    uint8_t* privData = NULL;
    char* base64Buff = NULL;
    OicSecCred_t *tempFirstCred = NULL;
    OicSecCred_t *tempSecondCred = NULL;

    size_t privDataKeySize = keySize;

    privData = (uint8_t*) OICCalloc(privDataKeySize,sizeof(uint8_t));
    PM_VERIFY_NON_NULL(TAG, privData, OC_STACK_NO_MEMORY, ERROR);

    OCFillRandomMem(privData,privDataKeySize);

    uint32_t outLen = 0;

    base64Buff = (char*) OICCalloc(B64ENCODE_OUT_SAFESIZE(privDataKeySize) + 1, sizeof(char));
    PM_VERIFY_NON_NULL(TAG, base64Buff, OC_STACK_NO_MEMORY, ERROR);
    int memReq = (B64ENCODE_OUT_SAFESIZE(privDataKeySize) + 1) * sizeof(char);
    B64Result b64Ret = b64Encode(privData, privDataKeySize*sizeof(uint8_t), base64Buff,
                                 memReq, &outLen);
    PM_VERIFY_SUCCESS(TAG, B64_OK == b64Ret, OC_STACK_ERROR, ERROR);

    // TODO: currently owner array is 1. only provisioning tool's id.
    tempFirstCred =  GenerateCredential(secondDeviceId, type, NULL, base64Buff, 1, ptDeviceId);
    PM_VERIFY_NON_NULL(TAG, tempFirstCred, OC_STACK_ERROR, ERROR);

    // TODO: currently owner array is 1. only provisioning tool's id.
    tempSecondCred =  GenerateCredential(firstDeviceId, type, NULL, base64Buff, 1, ptDeviceId);
    PM_VERIFY_NON_NULL(TAG, tempSecondCred, OC_STACK_ERROR, ERROR);

    *firstCred = tempFirstCred;
    *secondCred = tempSecondCred;
    res = OC_STACK_OK;

bail:
    OICFree(privData);
    OICFree(base64Buff);

    if(res != OC_STACK_OK)
    {
        OICFree(tempFirstCred);
        OICFree(tempSecondCred);
        *firstCred = NULL;
        *secondCred = NULL;
    }

    return res;
}

#ifdef __WITH_X509__
/**
 * Function to compose JSON Web Key (JWK) string from a certificate and a public key.
 *
 * @param[in]  certificateChain    Array of Base64 encoded certificate strings.
 * @param[in]  chainLength         Number of the certificates in certificateChain.
 * @return     Valid JWK string on success, or NULL on fail.
 */
static char *CreateCertificatePublicJWK(const char *const *certificateChain,
                                        const size_t chainLength)
{
    if (NULL == certificateChain || chainLength == 0)
    {
        OC_LOG(ERROR, TAG, "Error CreateCertificatePublicJWK: Invalid params");
        return NULL;
    }

    size_t certChainSize = 0;
    for (size_t i = 0; i < chainLength; ++i)
    {
        if (NULL != certificateChain[i])
        {
            certChainSize += strlen(certificateChain[i]);
        }
        else
        {
            OC_LOG(ERROR, TAG, "Error CreateCertificatePublicJWK: Invalid params");
            return NULL;
        }

    }
    /* certificates in the json array taken in quotes and separated by a comma
     * so we have to count the number of characters (number of commas and quotes) required
     * for embedding certificates in the array depending on the number of certificates in chain
     * each certificate except last embeded in  "\"%s\"," */
    const int numCommasAndQuotes = chainLength * 3 - 1;
    const char firstPart[] = "{\"kty\":\"EC\",\"crv\":\"P-256\",\"x5c\":[";
    const char secondPart[] = "]}";
    /* to calculate the size of JWK public part we need to add the value of first and  second parts,
     * size of certificate chain, number of additional commas and quotes and 1 for string termination symbol */
    size_t certPubJWKLen = strlen(firstPart) + strlen(secondPart)
                                             + certChainSize + numCommasAndQuotes + 1;
    char *certPubJWK = (char *)OICMalloc(certPubJWKLen);

    if (NULL != certPubJWK)
    {
        OICStrcpy(certPubJWK, certPubJWKLen, firstPart);
        size_t offset = strlen(firstPart);
        for (size_t i = 0; i < chainLength; ++i)
        {
            offset += sprintf(certPubJWK + offset, "\"%s\",", certificateChain[i]);
        }
        sprintf(certPubJWK + offset - 1, secondPart);
    }
    else
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
    }
    return certPubJWK;
}

/**
 * Function to compose JWK string from a private key.
 *
 * @param[in]  privateKey    Base64 encoded private key.
 * @return     Valid JWK string on success, or NULL on fail.
 */
static char *CreateCertificatePrivateJWK(const char *privateKey)
{
    if (NULL == privateKey)
    {
        OC_LOG(ERROR, TAG, "Error privateKey is NULL");
        return NULL;
    }
    const char firstPart[] = "{\"kty\":\"EC\",\"crv\":\"P-256\",\"d\":\"";
    const char secondPart[] = "\"}";
    char *certPrivJWK = (char *)OICMalloc(strlen(firstPart) + strlen(secondPart) + strlen(
            privateKey) + 1);

    if (NULL != certPrivJWK)
    {
        sprintf(certPrivJWK, "%s%s%s", firstPart, privateKey, secondPart);
    }
    else
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
    }
    return certPrivJWK;
}


/**
 * Function to generate Base64 encoded credential data for device.
 *
 * @param[in]   subject             Device id.
 * @param[out]  certificateChain    Pointer to Array of Base64 encoded certificate strings.
 * @param[out]  chainLength         Pointer to number of the certificates in certificateChain.
 * @param[out]  privKey             Pointer to Base64 encoded private key.
 * @return  OC_STACK_OK on success
 */
static OCStackResult GenerateCertificateAndKeys(const OicUuid_t * subject, char *** const certificateChain,
        size_t * const chainLength, char ** const privKey)
{
    if (NULL == subject || NULL == certificateChain || NULL == chainLength || NULL == privKey)
    {
        return  OC_STACK_INVALID_PARAM;
    }
    *certificateChain = NULL;
    *privKey     = NULL;

    ByteArray pubKeyBA  = BYTE_ARRAY_INITIALIZER;
    ByteArray privKeyBA = BYTE_ARRAY_INITIALIZER;
    ByteArray cert[CHAIN_LEN];

    uint8_t pubKeyData[PUBLIC_KEY_SIZE] = {0};
    uint8_t privKeyData[PRIVATE_KEY_SIZE] = {0};
    uint8_t certData[ISSUER_MAX_CERT_SIZE * CHAIN_LEN] = {0};
    uint8_t subjName[UUID_LENGTH + 1] = {0};

    pubKeyBA.data  = pubKeyData;
    pubKeyBA.len   = PUBLIC_KEY_SIZE;
    privKeyBA.data = privKeyData;
    privKeyBA.len  = PRIVATE_KEY_SIZE;
    for (size_t i = 0; i < CHAIN_LEN; ++i)
    {
        cert[i].data      = certData + ISSUER_MAX_CERT_SIZE * i;
        cert[i].len       = ISSUER_MAX_CERT_SIZE;
    }

    memcpy(subjName, subject->id, UUID_LENGTH);
    subjName[UUID_LENGTH] = '\0';

    if (PKI_SUCCESS != GenerateKeyPair(&privKeyBA, &pubKeyBA))
    {
        OC_LOG(ERROR, TAG, "Error generating keys.");
        return OC_STACK_ERROR;
    }
    if (PKI_SUCCESS != CKMIssueDeviceCertificate(subjName, NULL, NULL, pubKeyBA.data, cert))
    {
        OC_LOG(ERROR, TAG, "Error generating certificate.");
        return OC_STACK_ERROR;
    }

    char privB64buf[B64ENCODE_OUT_SAFESIZE(PRIVATE_KEY_SIZE) + 1] = {0};
    uint32_t privB64len = 0;
    if (B64_OK != b64Encode(privKeyBA.data,  privKeyBA.len, privB64buf,
                             B64ENCODE_OUT_SAFESIZE(PRIVATE_KEY_SIZE) + 1, &privB64len))
    {
        OC_LOG(ERROR, TAG, "Error while encoding key");
        return OC_STACK_ERROR;
    }

    if (PKI_SUCCESS != GetCAChain(chainLength , cert + 1))
    {
        OC_LOG(ERROR, TAG, "Error getting CA certificate chain.");
        return OC_STACK_ERROR;
    }

    ++(*chainLength);
    *certificateChain = (char **)OICMalloc(sizeof(char *) * (*chainLength));

    OCStackResult ret = OC_STACK_NO_MEMORY;
    if (NULL == *certificateChain)
    {
        goto memclean;
    }


    for (size_t i = 0; i < *chainLength; ++i)
    {
        (*certificateChain)[i] = NULL;

        char certB64buf[B64ENCODE_OUT_SAFESIZE(ISSUER_MAX_CERT_SIZE) + 1] = {0};
        uint32_t certB64len = 0;
        if (B64_OK != b64Encode(cert[i].data, cert[i].len, certB64buf,
                                B64ENCODE_OUT_SAFESIZE(ISSUER_MAX_CERT_SIZE) + 1, &certB64len))
        {
            OC_LOG(ERROR, TAG, "Error while encoding certificate");
            ret = OC_STACK_ERROR;
            goto memclean;
        }

        (*certificateChain)[i] = (char *) OICMalloc(certB64len + 1);
        if (NULL == (*certificateChain)[i])
        {
            goto memclean;
        }

        memcpy((*certificateChain)[i], certB64buf, certB64len + 1);
    }


    *privKey     = (char *)OICMalloc(privB64len + 1);

    if (NULL == *privKey)
    {
memclean:
        if (NULL != *certificateChain)
        {
            for (size_t i = 0; i < *chainLength; ++i)
            {
                OICFree((*certificateChain)[i]);
            }
        }
        OICFree(*certificateChain);
        *certificateChain = NULL;
        *privKey     = NULL;
        *chainLength = 0;
        if (OC_STACK_NO_MEMORY == ret)
        {
            OC_LOG(ERROR, TAG, "Error while memory allocation");
        }
        return ret;
    }

    memcpy(*privKey, privB64buf, privB64len + 1);

    return OC_STACK_OK;
}


OCStackResult PMGenerateCertificateCredentials(const OicUuid_t *ptDeviceId,
        const OicUuid_t *deviceId, OicSecCred_t **const cred)
{
    if (NULL == ptDeviceId || NULL == deviceId || NULL == cred)
    {
        return OC_STACK_INVALID_PARAM;
    }
    char **certificateChain = NULL;
    char *privKey = NULL;
    size_t certChainLen = 0;
    if (OC_STACK_OK != GenerateCertificateAndKeys(deviceId, &certificateChain,
            &certChainLen, &privKey))
    {
        OC_LOG(ERROR, TAG, "Error while generating credential data.");
        return OC_STACK_ERROR;
    }

    char *publicJWK = CreateCertificatePublicJWK(certificateChain, certChainLen);
    char *privateJWK = CreateCertificatePrivateJWK(privKey);
    for (size_t i = 0; i < certChainLen; ++i)
    {
        OICFree(certificateChain[i]);
    }
    OICFree(certificateChain);
    OICFree(privKey);
    if (NULL == publicJWK || NULL == privateJWK)
    {
        OICFree(publicJWK);
        OICFree(privateJWK);
        OC_LOG(ERROR, TAG, "Error while converting keys to JWK format.");
        return OC_STACK_ERROR;
    }

    OicSecCred_t *tempCred =  GenerateCredential(deviceId, SIGNED_ASYMMETRIC_KEY, publicJWK,
                              privateJWK, 1, ptDeviceId);
    OICFree(publicJWK);
    OICFree(privateJWK);
    if (NULL == tempCred)
    {
        OC_LOG(ERROR, TAG, "Error while generating credential.");
        return OC_STACK_ERROR;
    }
    *cred = tempCred;
    return OC_STACK_OK;
}
#endif // __WITH_X509__
