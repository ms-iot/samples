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
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************/

#ifndef _IOTVT_B64_H_
#define _IOTVT_B64_H_

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Macro to calculate the size of 'output' buffer required for
 * a 'input' buffer of length x during Base64 encoding operation.
 */
#define B64ENCODE_OUT_SAFESIZE(x) ((((x) + 3 - 1)/3) * 4 + 1)

/**
 * Macro to calculate the size of 'output' buffer required for
 * a 'input' buffer of length x during Base64 decoding operation.
 */
#define B64DECODE_OUT_SAFESIZE(x) (((x)*3)/4)

/**
 * Result code of base64 functions
 */
typedef enum {
    B64_OK = 0,
    B64_INVALID_PARAM,
    B64_OUTPUT_BUFFER_TOO_SMALL,
    B64_ERROR
}B64Result;

/**
 * Encode the plain message in base64.
 *
 * @param[in] in  Plain message
 * @param[in] inLen  Byte length of 'in'
 * @param[in,out] outBuf Output buffer
 *                Base64 encoded message will be written into 'outBuf'
 *                NOTE : This method adds a NULL to the string configuration
 * @param[in] outBufSize Size of output buffer
 * @param[out] outLen  Byte length of encoded message
 *
 * @return  B64_OK for Success, otherwise some error value
 */
B64Result b64Encode(const uint8_t* in, const size_t inLen,
               char* outBuf, const size_t outBufSize, uint32_t *outLen);

/**
 * Decode the encoded message in base64.
 *
 * @param[in] in  Base64 encoded message
 * @param[in] inLen  Byte lenth of 'in'
 * @param[in, out] outBuf  Output buffer
 *                 Base64 decoded message will be written into 'outBuf'
 * @param[in] outBufSize Size of output buffer
 * @param[out] outLen  Byte length of decoded message
 *
 * @return  B64_OK for Success, otherwise some error value
 */
B64Result b64Decode(const char* in, const size_t inLen,
               uint8_t* outBuf, size_t outBufSize, uint32_t *outLen);

#ifdef __cplusplus
}
#endif

#endif  //IOTVT_B64_H
