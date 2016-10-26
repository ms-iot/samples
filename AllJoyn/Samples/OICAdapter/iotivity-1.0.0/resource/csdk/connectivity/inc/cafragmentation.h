/* ****************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/**
 * @file
 *
 * This file contains common utility functions to manage the CA msg
 * fragmentation and reassemebly.
 */

#ifndef CA_FRAGMENTATION_H_
#define CA_FRAGMENTATION_H_

#include "cacommon.h"
#include "logger.h"

/**
 * From the adapter level, this is the maximum data length is supported
 * for the data transmission.
 */
#define MAX_DATA_LENGTH_SUPPORTED 4095

/**
 * The number of bits allocated to represent data length in header.
 */
#define NUMBER_OF_BITS_TO_IDENTIFY_DATA 12

/**
 * The length of the header in bits.
 */
#define NUMBER_OF_BITS_IN_CA_HEADER 15

/**
 * The length of the header in bytes.
 */
#define CA_HEADER_LENGTH 2

/**
 * The MTU supported for BLE adapter
 */
#define CA_SUPPORTED_BLE_MTU_SIZE  20

#ifdef __TIZEN__
/**
 * Reserved bit to differentiating the platform. Currently not in use.
 */
#define PLATFORM_IDENTIFIER_BIT 1

/**
 * The MTU supported from Tizen platform for EDR adapter.
 */
#define CA_SUPPORTED_EDR_MTU_SIZE  512

#elif __ANDROID__
/**
 * Reserved bit to differentiating the platform. Currently not in use.
 */
#define PLATFORM_IDENTIFIER_BIT 0

/**
 * The MTU supported from Android platform for EDR adapter.
 */
#define CA_SUPPORTED_EDR_MTU_SIZE  200

#elif __ARDUINO__
/**
 * Reserved bit to differentiating the platform. Currently not in use.
 */
#define PLATFORM_IDENTIFIER_BIT 0

/**
 * The MTU supported from Arduino platform for EDR adapter.
 */
#define CA_SUPPORTED_EDR_MTU_SIZE  200

#else //Other Platforms
/**
 * Reserved bit to differentiating the platform. Currently not in use.
 */
#define PLATFORM_IDENTIFIER_BIT 0

/**
 * The MTU supported for EDR adapter
 */
#define CA_SUPPORTED_EDR_MTU_SIZE  200

#endif

/**
 * Current Header version.
 */
#define HEADER_VERSION 1

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************
 * @file The CA Header format
 * CA Header will be defined by 2 bytes of Header.
 * First two bits : Header version(Currently Its not being used)
 * Third bit and fourth bit: Reserved bits for future use.
 * 5th to 16th bit : 12 bits to provide the length of the data.
 *****************************************************************/

/**
 * This function is used to generate the CA specific header to
 * maintain the fragmentation logic. The header structure explained
 * above will be formed and returned to the caller.
 *
 * @param[in,out] header       Pointer to the octet array that will
 *                             contain the generated header.
 * @param[in]     headerLength Length of the @a header octet array.
 * @param[in]     dataLength   The total length of the data.  The
 *                             length will be embedded in bits 5-16 of
 *                             the header, meaning the maximum overall
 *                             length of the data to be fragmented can
 *                             be no more than 4096 (2^12).
 *
 * @return @c CA_STATUS_OK on success. One of the @c CA_STATUS_FAILED
 *         or other error values on error.
 * @retval @c CA_STATUS_OK             Successful
 * @retval @c CA_STATUS_INVALID_PARAM  Invalid input arguments
 * @retval @c CA_STATUS_FAILED         Operation failed
 */
CAResult_t CAGenerateHeader(uint8_t *header,
                            size_t headerLength,
                            size_t datalength);

/**
 * This function is used to parse the header in the receiver end. This
 * function will provide the information of the total length of the
 * data which has been fragmented.
 *
 * @param[in] header Pointer to the octet array data which contains
 *                   the header information.  Note that pointer should
 *                   point to two bytes of data header which needs to
 *                   be parsed.
 * @param[in] length Length of the @a octet array containing the
 *                   header.
 *
 * @return Overall length of the data to be reassembled, or 0 on
 *         failure.
 */
uint32_t CAParseHeader(const uint8_t *header, size_t length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* CA_FRAGMENTATION_H_ */
