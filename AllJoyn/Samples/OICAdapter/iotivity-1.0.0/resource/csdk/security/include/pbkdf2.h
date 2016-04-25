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

#ifndef _PBKDF2_H
#define _PBKDF2_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * The number of iterations desired to derived key.
 * (Recommened by RFC 2898)
 */
#define PBKDF_ITERATIONS 1000

/**
 * Function to derive cryptographic key from the password. (RFC 2898)
 * In this implementation, HMAC with SHA2 is considered as a pseudorandom function
 *
 * @param[in] passwd is the master password from which a derived key is generated.
 * @param[in] pLen is the byte size of the passwd.
 * @param[in] salt is a cryptographic salt.
 * @param[in] saltlen is the byte size of the salt.
 * @param[in] iteration is the number of iterations desired.
 * @param[in] keyLen is the desired byte size of the derived key. (should be the same as
 *       derivedKey size)
 * @param[out] derivedKey is the generated derived key
 *
 * @return  0 on success
 */
int DeriveCryptoKeyFromPassword(const unsigned char* passwd, size_t pLen,
                                const uint8_t* salt, const size_t saltLen,
                                const size_t iterations,
                                const size_t keyLen, uint8_t* derivedKey);

#ifdef __cplusplus
}
#endif
#endif // _PBKDF2_H

