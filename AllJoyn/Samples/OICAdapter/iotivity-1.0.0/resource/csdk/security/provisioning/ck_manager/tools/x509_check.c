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
#include "pki_errors.h"
#include "pki.h"
#include "crl.h"
#include "oic_malloc.h"

#define SUCCESS_RES            0
#define FAIL_RES               1

const char COMMAND_CRT[] = "crt";
const char COMMAND_CRL[] = "crl";

/**
 * Shows program usage hint.
 */
void Usage()
{
    printf("Use next command:\n");
    printf("x509_check -c <crt/crl> -f <path to crt/crl file> -s <path to CA certificate>\n");
    printf("\t[-c]\t command name crt | crl\n");
    printf("\t[-f]\t path to crt/crl file\n");
    printf("\t[-s]\t path to CA certificate>\n");
}

/**
 * Converts DER file to byte array.
 *
 * @param[in]  filePath - path to DER file (Certificate or CRL)
 * @param[out] caPublicKey - ByteArray with DER encoded CRT or CRL
 */
void FileToByteArray(const char *filePath, ByteArray *out)
{
    if (!filePath)
    {
        printf("File path is NULL!\n");
        exit(0);
    }

    FILE *inFile = fopen(filePath, "rb");

    if (!inFile)
    {
        printf("Specified file doesn't exist!\n");
        exit(0);
    }

    fseek(inFile, 0, SEEK_END);
    out->len = ftell(inFile);
    rewind(inFile);
    out->data = (uint8_t *)OICMalloc(out->len);

    if (!out->data)
    {
        printf("Impossible to allocate memory!\n");
        exit(0);
    }

    if (!fread(out->data, sizeof(uint8_t), out->len, inFile))
    {
        printf("No info in file!\n");
        exit(0);
    }
    fclose(inFile);
}

/**
 * Main function.
 *
 * Checks certificate and certificate revocation list
 *
 * @param[in]  argc An integer argument count of the command line arguments
 * @param[in]  argv An argument vector of the command line arguments
 *
 * @return[out] an integer 0 upon exit success
 */
int main(int argc, char *argv[])
{
    int isCrt = 0;
    const char *testedFileName = 0;
    const char *caCrtFileName = 0;

    // Parse command line arguments
    int opt;

    while ((opt = getopt(argc, argv, "c:f:s:")) != -1)
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
            case 'f':
                testedFileName = optarg;
                break;
            case 's':
                caCrtFileName = optarg;
                break;
            default:
                Usage();
                exit(0);
        }
    }

    if (testedFileName == NULL || caCrtFileName == NULL)
    {
        printf("Wrong file name\n");
        Usage();
        exit(0);
    }

    // main
    ByteArray testedCrtCrl = BYTE_ARRAY_INITIALIZER; // Could contain either Certificate or CRL
    ByteArray caCrt = BYTE_ARRAY_INITIALIZER;

    // Copy DER files, specified in command line to ByteArray structure
    FileToByteArray(testedFileName, &testedCrtCrl);
    FileToByteArray(caCrtFileName, &caCrt);

    // Decoding CA certificate
    CertificateX509 caCrtStuct = CERTIFICATE_X509_INITIALIZER;
    PKIError errorCode = DecodeCertificate(caCrt, &caCrtStuct);

    if (errorCode != PKI_SUCCESS)
    {
        printf("Unable to decode CA Certificate!\n");
        printf("Error code: %d\n", errorCode);
    }

    ByteArray caPublicKey = BYTE_ARRAY_INITIALIZER;
    // Verifies Certificate or CRL depending on request
    if (isCrt)
    {
        errorCode = CheckCertificate(testedCrtCrl, caCrtStuct.pubKey);
    }
    else
    {
        caPublicKey.data = caCrtStuct.pubKey.data;
        caPublicKey.len = caCrtStuct.pubKey.len;
        ParsePublicKey(&caPublicKey);
        CertificateList crlStruct = CRL_INITIALIZER;
        errorCode = DecodeCertificateList(testedCrtCrl, &crlStruct, caPublicKey);
    }

    if (errorCode == PKI_SUCCESS)
    {
        printf("Verification SUCCESS!\n");
    }
    else
    {
        printf("Verification FAILED!\n");
        printf("Error code: %d\n", errorCode);
    }

    // Free the allocated memory
    OICFree(testedCrtCrl.data);
    OICFree(caCrt.data);

    return 0;
}
