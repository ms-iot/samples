/* 
Copyright (c) 2015 Atmel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3.  Neither the name of Atmel nor the names of its contributors may be used to 
endorse or promote products derived from this software without specific 
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file dtls_hal.h
 * @brief DTLS Hardware Abstraction Layer API and visible structures. 
 */

#ifndef _DTLS_DTLS_HAL_H_
#define _DTLS_DTLS_HAL_H_

#include <stdint.h>
#include <stdbool.h>

// Define return values for HAL
#define HAL_SUCCESS (1)
#define HAL_FAILURE (0)
#define ENC_KEY_SIZE (32)

/**
 * This structure contains callback functions used by tinydtls to
 * provide a hardware abstraction layer for secure storage . 
 */
typedef struct {

  /**
   * Call this function during application initialization to create HAL related resources.
   *
   * @param[in] ctx  The current DTLS context.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_init)(struct dtls_context_t *ctx);

  /**
   * Call this function during application shut down to clean up any HAL related resources.
   *
   * @param[in] ctx  The current DTLS context.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_finish)(struct dtls_context_t *ctx);

  /**
   * Call this function to store unsecured data on hardware.
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_location_handle  The handle to the hardware location to store p_data.
   * @param[in] p_data  The data to store.
   * @param[in] data_size  The size of the data to store.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_write_mem)(struct dtls_context_t *ctx,
                       const uint8_t* p_location,
                       const uint8_t* p_data,
                       size_t data_size);

  /**
   * Call this function to read data from unsecured storage from hardware.
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_location_handle  The handle to the hardware location to read p_data.
   * @param[in] p_data  The data to read.
   * @param[inout] data_size  IN: The size of the buffer to receive data.  OUT: The actual number of bytes read.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_read_mem)(struct dtls_context_t *ctx,
                      const uint8_t* p_location,
                      uint8_t* p_data,
                      size_t* dataSize);

  /**
   * Call this function to store sensitive data in secure hardware.
   * Data will be securely stored and encrypted on the wire
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_location_handle  The handle to the hardware location to store p_data.
   * @param[in] p_data  The data to store securely.
   * @param[in] data_size  The size of the data to store securely.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_write_mem_enc)(struct dtls_context_t *ctx,
                           const uint8_t* p_location,
                           const uint8_t* p_data,
                           size_t data_size);

  /**
   * Call this function to read sensitive data from secure hardware.
   * Data will be read from secure storage and encrypted on the wire
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_location_handle  The handle to the hardware location to read p_data.
   * @param[in] p_data  The data to read securely.
   * @param[inout] data_size  IN: The size of the buffer to receive data.  OUT: The actual number of bytes read.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_read_mem_enc)(struct dtls_context_t *ctx,
                          const uint8_t* p_location,
                          uint8_t* p_data,
                          size_t* dataSize);

  /**
   * Call this function to generate a unique encryption key that will be used for secure storage.
   * This encryption key should be stored as a platform resource
   *
   * @param[in] ctx  The current DTLS context.
   * @param[out] p_enc_key_out  The internally generated unique encryption key that will be used for secure storage.
   *    This key needs to be stored on the platform.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_init_enckey)(struct dtls_context_t *ctx,
                         uint8_t p_enc_key_out[ENC_KEY_SIZE]);

  /**
   * Call this function to set a unique encryption key that will be used for secure storage.
   * This encryption key should be stored as a platform resource
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_enc_key_in  The encryption key that will be used for secure storage.
   *    This key needs to be stored on the platform.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_set_enckey)(struct dtls_context_t *ctx,
                        const uint8_t p_enc_key_in[ENC_KEY_SIZE]);

  /**
   * This function is called internally from HAL_read_mem_enc and HAL_write_mem_enc.
   * The platform must implement this function to return the encryption key that was
   *   returned by HAL_init_enckey or HAL_set_enckey.
   *
   * @param[in] ctx  The current DTLS context.
   * @param[out] p_enc_key_out  The encryption key that is used for secure storage.
   *    Retrieve from platform in this function.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_get_enckey)(struct dtls_context_t *ctx,
                        uint8_t p_enc_key_out[ENC_KEY_SIZE]);

  /**
   * Call this function to use the hardware to calculate a SHA256
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_msg  The message to calculate the SHA256.
   * @param[in] msg_size  The size of the message to hash.
   * @param[out] p_hash  The hash of p_msg.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_sha2_calc)(struct dtls_context_t *ctx,
                       uint8_t *p_msg,
                       size_t msg_size,
                       uint8_t p_hash[SHA256_DIGEST_LENGTH]);

  /**
   * Call this function to use the hardware key derivation function (KDF)
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] p_seed  The seed used to produce the key.
   * @param[in] seed_size  The size of the message to hash.
   * @param[out] p_keyout  The key produced by the KDF.
   * @param[in] key_size  The size of the key to be returned.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_kdf_calc)(struct dtls_context_t *ctx,
                      const uint8_t* p_seed,
                      const size_t seed_size,
                      uint8_t* p_keyout,
                      const size_t key_size);

  /**
   * Call this function to retrieve the specified certificate from hardware
   *
   * @param[in] ctx  The current DTLS context.
   * @param[in] cert_id  An index into the certificate to returned.
   * @param[inout] cert  The certificate to be returned.
   * @param[inout] cert_size  The size of the certificate.
   * @return HAL_SUCCESS or HAL_FAILURE
   */
  int (*HAL_get_x509_cert)(struct dtls_context_t *ctx,
                           uint32_t cert_ref,
                           uint8_t **cert,
                           size_t *cert_size);

} dtls_hal_handler_t;


#endif /* _DTLS_DTLS_HAL_H_ */

