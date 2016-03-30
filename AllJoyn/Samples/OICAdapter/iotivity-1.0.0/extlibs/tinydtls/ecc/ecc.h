/* Copyright 2014, Kenneth MacKay. Licensed under the BSD 2-clause license. */

#ifndef _MICRO_ECC_H_
#define _MICRO_ECC_H_

#include <stdint.h>

/* Platform selection options.
If uECC_PLATFORM is not defined, the code will try to guess it based on compiler macros.
Possible values for uECC_PLATFORM are defined below: */
#define uECC_arch_other 0
#define uECC_x86        1
#define uECC_x86_64     2
#define uECC_arm        3
#define uECC_arm_thumb  4
#define uECC_avr        5

/* If desired, you can define uECC_WORD_SIZE as appropriate for your platform (1, 4, or 8 bytes).
If uECC_WORD_SIZE is not explicitly defined then it will be automatically set based on your platform. */

/* Inline assembly options.
uECC_asm_none  - Use standard C99 only.
uECC_asm_small - Use GCC inline assembly for the target platform (if available), optimized for minimum size.
uECC_asm_fast  - Use GCC inline assembly optimized for maximum speed. */
#define uECC_asm_none  0
#define uECC_asm_small 1
#define uECC_asm_fast  2
#ifndef uECC_ASM
    #define uECC_ASM uECC_asm_none//uECC_asm_fast
#endif

/* Curve selection options. */
#define uECC_secp160r1 1
#define uECC_secp192r1 2
#define uECC_secp256r1 3
#define uECC_secp256k1 4
#ifndef uECC_CURVE
    #define uECC_CURVE uECC_secp256r1
#endif

/* uECC_SQUARE_FUNC - If enabled (defined as nonzero), this will cause a specific function to be used for (scalar) squaring
    instead of the generic multiplication function. This will make things faster by about 8% but increases the code size. */
#define uECC_SQUARE_FUNC 1

#define uECC_CONCAT1(a, b) a##b
#define uECC_CONCAT(a, b) uECC_CONCAT1(a, b)

#define uECC_size_1 20 /* secp160r1 */
#define uECC_size_2 24 /* secp192r1 */
#define uECC_size_3 32 /* secp256r1 */
#define uECC_size_4 32 /* secp256k1 */

#define uECC_BYTES uECC_CONCAT(uECC_size_, uECC_CURVE)

#ifdef __cplusplus
extern "C"
{
#endif

/* uECC_RNG_Function type
The RNG function should fill p_size random bytes into p_dest. It should return 1 if
p_dest was filled with random data, or 0 if the random data could not be generated.
The filled-in values should be either truly random, or from a cryptographically-secure PRNG.

A correctly functioning RNG function must be set (using uECC_set_rng()) before calling
uECC_make_key() or uECC_sign().

A correct RNG function is set by default when building for Windows, Linux, or OS X.
If you are building on another POSIX-compliant system that supports /dev/random or /dev/urandom,
you can define uECC_POSIX to use the predefined RNG. For embedded platforms there is no predefined
RNG function; you must provide your own.
*/
typedef int (*uECC_RNG_Function)(uint8_t *p_dest, unsigned p_size);

/* uECC_set_rng() function.
Set the function that will be used to generate random bytes. The RNG function should
return 1 if the random data was generated, or 0 if the random data could not be generated.

On platforms where there is no predefined RNG function (eg embedded platforms), this must
be called before uECC_make_key() or uECC_sign() are used.

Inputs:
    p_rng  - The function that will be used to generate random bytes.
*/
void uECC_set_rng(uECC_RNG_Function p_rng);

//////////////////////////////////////////
// DTLS_CRYPTO_HAL
/**
* Call this function to create a unique public-private key pair in secure hardware
*
* @param[out] p_publicKey  The public key that is associated with the private key that was just created.
* @param[out] p_privateKeyHandle  A handle that is used to point to the private key stored in hardware.
* @return 1 upon success, 0 if an error occurred.
*/
typedef int (*uECC_make_key_Function)(uint8_t p_publicKey[uECC_BYTES*2], uint8_t p_privateKeyHandle[uECC_BYTES]);

/**
* Set the callback function that will be used to generate a public-private key pair.
* This function will replace uECC_make_key.
*
* @param[in] p_make_key_cb  The function that will be used to generate a public-private key pair.
*/
void uECC_set_make_key_cb(uECC_make_key_Function p_make_key_cb);

/**
* Call this function to sign a hash using a hardware protected private key.
*
* @param[in] p_privateKeyHandle  A handle that is used to point to the private key stored in hardware.
* @param[in] p_hash  The hash to sign.
* @param[out] p_signature  The signature that is produced in hardware by the private key..
* @return 1 upon success, 0 if an error occurred.
*/
typedef int (*uECC_sign_Function)(uint8_t p_privateKeyHandle[uECC_BYTES], const uint8_t p_hash[uECC_BYTES], uint8_t p_signature[uECC_BYTES*2]);

/**
* Set the callback function that will be used to sign.
* This function will replace uECC_sign.
*
* @param[in] p_sign_cb  The function that will be used to sign.
*/
void uECC_set_sign_cb(uECC_sign_Function p_sign_cb);

/**
* Call this function to verify a signature using the public key and hash that was signed. 
*
* @param[in] p_publicKey  The public key that is associated with the private key that produced the signature.
* @param[in] p_hash  The hash that was signed.
* @param[in] p_signature  The signature that was produced the private key that is associated with p_public_key
* @return 1 upon success, 0 if an error occurred.
*/
typedef int (*uECC_verify_Function)(const uint8_t p_publicKey[uECC_BYTES*2], const uint8_t p_hash[uECC_BYTES], const uint8_t p_signature[uECC_BYTES*2]);

/**
* Set the callback function that will be used to verify.
* This function will replace uECC_verify.
*
* @param[in] p_verify_cb  The function that will be used to verify.
*/
void uECC_set_verify_cb(uECC_verify_Function p_verify_cb);

/**
* Call this function to produce an ECDH shared key using the public key of the other node.
* A hardware protected private key will be used for the point multiply
*
* @param[in] p_publicKey  The public key from the other node used for communication.
* @param[in] p_privateKeyHandle  A handle that is used to point to the private key stored in hardware.
* @param[out] p_secret  The pre-master key that is produced by the point multiply with p_public_key and our private key
* @return 1 upon success, 0 if an error occurred.
*/
typedef int (*uECC_shared_secret_Function)(const uint8_t p_publicKey[uECC_BYTES*2], const uint8_t p_privateKeyHandle[uECC_BYTES], uint8_t p_secret[uECC_BYTES]);

/**
* Set the callback function that will be used to produce a shared secret.
* This function will replace uECC_shared_secret.
*
* @param[in] p_make_key_cb  The function that will be used to generate the shared secret.
*/
void uECC_set_shared_secret_cb(uECC_shared_secret_Function p_shared_secret_cb);

/**
* Call this function to produce a shared key using the public key of the other node.
* An ephemeral private key will be created in secure hardware that will be used for the point multiply
*
* @param[in] p_public_key  The public key from the other node used for communication.
* @param[out] p_public_key_out  The ephemeral public key that will be used in the point multiply.
* @param[out] p_secret  The pre-master key that is produced by the point multiply with p_public_key and our private key
* @return 1 upon success, 0 if an error occurred.
*/
typedef int (*uECC_ecdhe_Function)(const uint8_t p_public_key_in[uECC_BYTES*2],
                                   uint8_t p_public_key_out[uECC_BYTES*2],
                                   uint8_t p_secret[uECC_BYTES]);

/**
* Set the callback function that will be used to produce a ECDHE shared secret.
*
* @param[in] p_ecdhe_cb  The function that will be used to generate the ECDHE shared secret.
*/
void uECC_set_ecdhe_cb(uECC_ecdhe_Function p_ecdhe_cb);

/**
* Call this function to return the public key for an existing private key.
*
* @param[out] p_key_handle  A handle that is used to point to the private key stored in hardware.
*    The public key that is associated with this private key will be returned
* @param[out] p_public_key  The public key that is associated with the private key that was just created.
* @return 1 upon success, 0 if an error occurred.
*/
typedef int (*uECC_get_pubkey_Function)(const uint8_t p_key_handle[uECC_BYTES],
                                        uint8_t p_public_key[uECC_BYTES*2]);

/**
* Set the callback function that will be used to return the public key for an existing private key.
*
* @param[in] p_get_pubkey_cb  The function that will be used to return the public key for an existing private key.
*/
void uECC_set_get_pubkey_cb(uECC_get_pubkey_Function p_get_pubkey_cb);


/**
* Call this function to produce a shared key using the public key of the other node.
* An ephemeral private key will be created that will be used for the point multiply
*
* @param[in] p_public_key  The public key from the other node used for communication.
* @param[out] p_public_key_out  The ephemeral public key that will be used in the point multiply.
* @param[out] p_secret  The pre-master key that is produced by the point multiply with p_public_key and our private key
* @return 1 upon success, 0 if an error occurred.
*/
int uECC_ecdhe(const uint8_t p_public_key_in[uECC_BYTES*2],
               uint8_t p_public_key_out[uECC_BYTES*2],
               uint8_t p_secret[uECC_BYTES]);

/**
* Call this function to return the public key for an existing private key.
*
* @param[out] p_key_handle  A handle that is used to point to the private key stored in hardware.
*    The public key that is associated with this private key will be returned
* @param[out] p_public_key  The public key that is associated with the private key that was just created.
* @return 1 upon success, 0 if an error occurred.
*/
int uECC_get_pubkey(const uint8_t p_key_handle[uECC_BYTES],
                    uint8_t p_public_key[uECC_BYTES*2]);

//////////////////////////////////////////


/* uECC_make_key() function.
Create a public/private key pair.

Outputs:
    p_publicKey  - Will be filled in with the public key.
    p_privateKey - Will be filled in with the private key.

Returns 1 if the key pair was generated successfully, 0 if an error occurred.
*/
int uECC_make_key(uint8_t p_publicKey[uECC_BYTES*2], uint8_t p_privateKey[uECC_BYTES]);

/* uECC_shared_secret() function.
Compute a shared secret given your secret key and someone else's public key.
Note: It is recommended that you hash the result of uECC_shared_secret() before using it for symmetric encryption or HMAC.

Inputs:
    p_publicKey  - The public key of the remote party.
    p_privateKey - Your private key.

Outputs:
    p_secret - Will be filled in with the shared secret value.

Returns 1 if the shared secret was generated successfully, 0 if an error occurred.
*/
int uECC_shared_secret(const uint8_t p_publicKey[uECC_BYTES*2], const uint8_t p_privateKey[uECC_BYTES], uint8_t p_secret[uECC_BYTES]);

/* uECC_compress() function.
Compress a public key.

Inputs:
    p_publicKey - The public key to compress.

Outputs:
    p_compressed - Will be filled in with the compressed public key.
*/
void uECC_compress(const uint8_t p_publicKey[uECC_BYTES*2], uint8_t p_compressed[uECC_BYTES+1]);

/* uECC_decompress() function.
Decompress a compressed public key.

Inputs:
    p_compressed - The compressed public key.

Outputs:
    p_publicKey - Will be filled in with the decompressed public key.
*/
void uECC_decompress(const uint8_t p_compressed[uECC_BYTES+1], uint8_t p_publicKey[uECC_BYTES*2]);

/* uECC_sign() function.
Generate an ECDSA signature for a given hash value.

Usage: Compute a hash of the data you wish to sign (SHA-2 is recommended) and pass it in to
this function along with your private key.

Inputs:
    p_privateKey - Your private key.
    p_hash       - The message hash to sign.

Outputs:
    p_signature  - Will be filled in with the signature value.

Returns 1 if the signature generated successfully, 0 if an error occurred.
*/
int uECC_sign(const uint8_t p_privateKey[uECC_BYTES], const uint8_t p_hash[uECC_BYTES], uint8_t p_signature[uECC_BYTES*2]);

/* uECC_verify() function.
Verify an ECDSA signature.

Usage: Compute the hash of the signed data using the same hash as the signer and
pass it to this function along with the signer's public key and the signature values (r and s).

Inputs:
    p_publicKey - The signer's public key
    p_hash      - The hash of the signed data.
    p_signature - The signature value.

Returns 1 if the signature is valid, 0 if it is invalid.
*/
int uECC_verify(const uint8_t p_publicKey[uECC_BYTES*2], const uint8_t p_hash[uECC_BYTES], const uint8_t p_signature[uECC_BYTES*2]);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _MICRO_ECC_H_ */
