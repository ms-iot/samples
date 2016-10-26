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


#ifndef _CRYPTO_ADAPTER_H_
#define _CRYPTO_ADAPTER_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "ecc.h"
#include "sha2.h"

/// Sizes for ECDSA prime256v1 elliptic curve
#define PUBLIC_KEY_SIZE     (64)
#define SIGN_R_LEN          (32)
#define SIGN_S_LEN          (32)
#define SIGN_FULL_SIZE      (64)
#define PRIVATE_KEY_SIZE    (32)

/// Length of SHA 256 hash
#define SHA_256_HASH_LEN    (32)

#define uECC_SIGN_VERIFICATION_SUCCESS (1)

/**
 * @def GET_SHA_256(tbs, sha256)
 *
 * A macro that compute sha-256 hash of tbs part.
 *
 * @param[in] tbs "to be signed" part
 * @param[out] sha256 hash of tbs
 */
#undef GET_SHA_256
#define GET_SHA_256(tbs, sha256) do{                     \
        SHA256_CTX ctx256;                               \
        SHA256_Init(&ctx256);                            \
        SHA256_Update(&ctx256, tbs.data, tbs.len);       \
        SHA256_Final(sha256, &ctx256);                   \
    }while(0)

/**@def CHECK_SIGN(structure, caPubKey)
 * Checks the sign of ASN.1 structure.
 *
 * @param structure ASN.1 stucture
 * @param caPubKey public key of CA
 */
#undef CHECK_SIGN
#define CHECK_SIGN(structure, caPubKey) do{                                  \
    int err;                                                                 \
    uint8_t sha256[SHA_256_HASH_LEN];                                        \
    uint8_t fullSignature[SIGN_FULL_SIZE];                                   \
    GET_SHA_256((structure).tbs, sha256);                                    \
    memcpy(fullSignature, (structure).signR.data, SIGN_R_LEN);               \
    memcpy((fullSignature + SIGN_R_LEN), (structure).signS.data, SIGN_S_LEN);\
    err = uECC_verify(caPubKey.data, sha256, fullSignature);                 \
    CHECK_EQUAL(err, uECC_SIGN_VERIFICATION_SUCCESS, PKI_SIG_MISMATCH);      \
    }while(0)


#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_CRYPTO_ADAPTER_H_
