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

#include <stdio.h>
#include <stdint.h>
#include <getopt.h>

#include "byte_array.h"
#include "ck_manager.h"
#include "crl.h"

#define SUCCESS_RES            0
#define FAIL_RES               1

const char COMMAND_CRT[] = "crt";
const char COMMAND_CRL[] = "crl";

#define DEFAULT_SN             1

#define DEFAULT_CA_CRT_NAME           "ca_crt.der"
#define DEFAULT_CA_PUBLIC_KEY_NAME    "ca_public.key"
#define DEFAULT_USER_PRIVATE_KEY_NAME "user_private.key"
#define DEFAULT_USER_CRT_NAME         "user_crt.der"
#define DEFAULT_CRL_NAME              "crl.der"

#define DEFAULT_DER_DATA_SIZE  1024
#define SET_OF_SEQUENCE_SIZE   4
#define NUMBER_OF_REVOKED      1

/**
 * Shows program usage hint.
 */
void Usage()
{
    printf("Use next command:\n");
    printf("x509_issue -c <crt/crl> [-n]\n");
    printf("\t[-c]\t command name crt | crl\n");
    printf("\t[-n]\t generate new CA key pair\n");
}

/**
 * Generates CA Certificate File, writes CA private and public keys to storage
 *
 * @param[in]  updateCAkeys use new or old keys
*/
int GenerateCACertificateFile(const int updateCAkeys)
{
    // Variables definition
    uint8_t derData[DEFAULT_DER_DATA_SIZE] = {0};
    uint8_t caPubKey[PUBLIC_KEY_SIZE] = {0};
    uint8_t caPrivKey[PRIVATE_KEY_SIZE] = {0};
    uint8_t defaultCaName[] = "Default_CA_Name";

    ByteArray certDer = BYTE_ARRAY_CONSTRUCTOR(derData);
    ByteArray pubKeyIss = BYTE_ARRAY_CONSTRUCTOR(caPubKey);
    ByteArray privKeyIss = BYTE_ARRAY_CONSTRUCTOR(caPrivKey);
    ByteArray rootName = BYTE_ARRAY_CONSTRUCTOR(defaultCaName);

    // Generate
    if (updateCAkeys)
    {
        GenerateCAKeyPair(&privKeyIss, &pubKeyIss);
        printf("CA key pair was changed!\n");
        if (GenerateDERCertificateFile(&pubKeyIss, DEFAULT_CA_PUBLIC_KEY_NAME) != PKI_SUCCESS)
        {
            printf("Unable to generate CA public key file!\n");
            exit(0);
        }
        else
        {
            printf("CA public key file generated: %s\n", DEFAULT_CA_PUBLIC_KEY_NAME);
        };
    }

    SetSerialNumber(DEFAULT_SN);
    SetRootName(rootName);
    CKMIssueRootCertificate(0, 0, &certDer);

    // Writes ByteArray to file
    if (GenerateDERCertificateFile(&certDer, DEFAULT_CA_CRT_NAME) != PKI_SUCCESS)
    {
        printf("Unable to generate CA Certificate file!\n");
        exit(0);
    }
    else
    {
        printf("CA Certificate File generated: %s\n", DEFAULT_CA_CRT_NAME);
    };
    return 0;
}

/**
 * Generates User Certificate File
 */
void GenerateUserCertificateFile()
{
    uint8_t subjPubKey[PUBLIC_KEY_SIZE] = {0};
    uint8_t subjPrivKey[PRIVATE_KEY_SIZE] = {0};

    ByteArray pubKeySubj = BYTE_ARRAY_CONSTRUCTOR(subjPubKey);
    ByteArray privKeySubj = BYTE_ARRAY_CONSTRUCTOR(subjPrivKey);

    // TODO: Uncomment GenerateKeyPair
    GenerateKeyPair(&privKeySubj, &pubKeySubj);

    if (GenerateDERCertificateFile(&privKeySubj, DEFAULT_USER_PRIVATE_KEY_NAME) != PKI_SUCCESS)
    {
        printf("Unable to generate user private key file!\n");
        exit(0);
    }
    else
    {
        printf("User private key file generated: %s\n", DEFAULT_USER_PRIVATE_KEY_NAME);
    };

    uint8_t derData[DEFAULT_DER_DATA_SIZE] = {0};
    ByteArray certDer = BYTE_ARRAY_CONSTRUCTOR(derData);

    const uint8_t defaultUserName[]   = "Default_USER_Name";

    CKMIssueDeviceCertificate(defaultUserName, 0, 0, subjPubKey, &certDer);

    if (GenerateDERCertificateFile(&certDer, DEFAULT_USER_CRT_NAME) != PKI_SUCCESS)
    {
        printf("Unable to generate User Certificate file!\n");
        exit(0);
    }
    else
    {
        printf("User Certificate File generated: %s\n", DEFAULT_USER_CRT_NAME);
    };
}

/**
 * Generates Certificate Revocation List File
 */
void GenerateCRLFile()
{
    const uint8_t *uint8ThisUpdateTime = (const uint8_t *)"130101000000Z";
    uint32_t revokedNumbers[NUMBER_OF_REVOKED] = {100};
    const uint8_t *revocationDates[NUMBER_OF_REVOKED] =
    {
        (const uint8_t *)"130101000001Z"
    };

    ByteArray code =
    {
        .len = CRL_MIN_SIZE + NUMBER_OF_REVOKED * (sizeof(CertificateRevocationInfo_t) + SET_OF_SEQUENCE_SIZE),
        .data = (uint8_t *)calloc(1, CRL_MIN_SIZE + NUMBER_OF_REVOKED * (sizeof(CertificateRevocationInfo_t) + SET_OF_SEQUENCE_SIZE))
    };

    if (!code.data)
    {
        printf("calloc error\n");
        exit(0);
    }

    int errorCode = CKMIssueCRL(uint8ThisUpdateTime, NUMBER_OF_REVOKED, revokedNumbers, revocationDates,
                                &code);

    printf("Gen CRL err code: %d\n", errorCode);

    //CRL ByteArray to file
    if (GenerateDERCertificateFile(&code, DEFAULT_CRL_NAME) != PKI_SUCCESS)
    {
        printf("Unable to generate CRL file!\n");
        free(code.data);
        exit(0);
    }
    else
    {
        printf("CRL File generated: %s\n", DEFAULT_CRL_NAME);
    };

    free(code.data);
}

/**
 * Main function.
 *
 * Generates certificate and certificate revocation list
 *
 * @param[in]  argc An integer argument count of the command line arguments
 * @param[in]  argv An argument vector of the command line arguments
 *
 * @return[out] an integer 0 upon exit success
 */
int main(int argc, char *argv[])
{
    int isCrt = 0;
    int updateCAkeys = 0;

    // Parse command line arguments
    int opt;

    while ((opt = getopt(argc, argv, "c:n")) != -1)
    {
        switch (opt)
        {
            case 'c':
                if (!strcmp(COMMAND_CRT, optarg))
                {
                    isCrt = 1;
                }
                else if (!strcmp(COMMAND_CRL, optarg))
                {
                    isCrt = 0;
                }
                else
                {
                    printf("Wrong command(-c)!\n");
                    Usage();
                    exit(0);
                }
                break;
            case 'n':
                updateCAkeys = 1;
                break;
            default:
                Usage();
                exit(0);
        }
    }

    // main
    GenerateCACertificateFile(updateCAkeys);

    if (isCrt)
    {
        GenerateUserCertificateFile();
    }
    else
    {
        GenerateCRLFile();
    }

    return 0;
}
