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


#ifndef _PKI_ERRORS_H_
#define _PKI_ERRORS_H_


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifdef X509_DEBUG
#warning "DEBUG is enabled"
#include <stdio.h>  // <printf>
#endif

/**
 * @enum PKIError
 *
 * The enumeration of error codes.
 */

typedef enum
{
    PKI_SUCCESS = 0,         /**< No error occurred. */
    PKI_UNKNOWN_ERROR,       /**< Unknown error occurred. */
    PKI_NULL_PASSED,         /**< NULL passed to function. */
    PKI_CERT_DATE_INVALID,   /**< Certificate date expired. */
    PKI_BUFFER_OVERFLOW,     /**< Array out of range. */
    PKI_WRONG_OCTET_LEN,     /**< Wrong length of octet. */
    PKI_UNKNOWN_OID,         /**< Requested OID is unknown. */
    PKI_INVALID_FORMAT,      /**< The CRT/CRL/CSR format is invalid. */
    PKI_INVALID_DATE_FORMAT, /**< The date tag or value is invalid. */
    PKI_INVALID_SIGNATURE,   /**< The signature tag or value invalid. */
    PKI_SIG_MISMATCH,        /**< Signature algorithms do not match.  */
    PKI_CERT_VERIFY_FAILED,  /**< Certificate verification failed*/
    PKI_CERT_REVOKED,        /**< Certificate is revoked. */
    PKI_WRONG_ARRAY_LEN,     /**< Wrong length of input array*/
    PKI_MEMORY_ALLOC_FAILED, /**< Failed to allocate memory */
    PKI_BASE64_ERROR,        /**< Base64 convertion error occurred. */
    PKI_JSON_ERROR,          /**< JSON convertion error occurred. */
    PKI_JSON_NOT_FOUND,       /**< JSON object not found. */

    ISSUER_CA_STORAGE_FILE_READ_ERROR,          /**< File read error in CA storage */
    ISSUER_CA_STORAGE_FILE_WRITE_ERROR,         /**< File write error in CA storage */
    ISSUER_CA_STORAGE_CRL_READ_ERROR,           /**< CRL file read error in CA storage */
    ISSUER_CA_STORAGE_CRL_WRITE_ERROR,          /**< CRL file write error in CA storage */
    ISSUER_CA_STORAGE_CRT_READ_ERROR,           /**< Certificate file read error in CA storage */
    ISSUER_CA_STORAGE_CRT_WRITE_ERROR,          /**< Certificate file write error in CA storage */
    ISSUER_CA_STORAGE_MEMORY_ALLOC_FAILED,      /**< Failed to allocate memory in CA storage */
    ISSUER_CA_STORAGE_WRONG_SERIAL_NUMBER,      /**< Wrong serial number passed to CA storage */
    ISSUER_CA_STORAGE_SN_UNDEFINED,             /**< Serial number is not defined in CA storage */
    ISSUER_CA_STORAGE_WRONG_CRL_SERIAL_NUMBER,  /**< Wrong CRL serial number passed to CA
                                                     storage */
    ISSUER_CA_STORAGE_CRL_SN_UNDEFINED,         /**< CRL serial number is not defined in CA
                                                     storage */
    ISSUER_CA_STORAGE_WRONG_PRIVATE_KEY_LEN,    /**< Passed private key length not equal to
                                                     PRIVATE_KEY_SIZE*/
    ISSUER_CA_STORAGE_PRIVATE_KEY_UNDEFINED,    /**< CA private key is not defined in CA storage */
    ISSUER_CA_STORAGE_WRONG_PUBLIC_KEY_LEN,     /**< Passed public key length not equal to
                                                     PUBLIC_KEY_SIZE*/
    ISSUER_CA_STORAGE_PUBLIC_KEY_UNDEFINED,     /**< CA public key is not defined in CA storage */
    ISSUER_CA_STORAGE_CA_CHAIN_LENGTH_UNDEFINED,/**< CA certificate chain length is not defined in
                                                     CA storage */
    ISSUER_CA_STORAGE_WRONG_CA_NAME_LEN,        /**< CA name length is bigger than
                                                     ISSUER_MAX_NAME_SIZE */
    ISSUER_CA_STORAGE_CA_NAME_UNDEFINED,        /**< CA name is not defined in CA storage */
    ISSUER_CA_STORAGE_CRL_UNDEFINED,            /**< CRL is not defined in CA storage */
    ISSUER_CA_STORAGE_NULL_PASSED,              /**< NULL passed to function in CA storage */
    CKM_INFO_IS_NOT_INIT,                       /**< CKM info storage was not init */
    ISSUER_CA_STORAGE_WRONG_BYTE_ARRAY_LEN,     /**< ByteArray with wrong lenth passed into
                                                     CA storage */

    ISSUER_MAKE_KEY_ERROR,      /**< Error during uECC_make_key() */
    ISSUER_MEMORY_ALLOC_FAILED, /**< Failed to allocate memory in issuer */
    ISSUER_FILE_WRITE_ERROR,    /**< File write error in issuer */
    ISSUER_WRONG_SERIAL_NUMBER, /**< Wrong serial number passed to issuer */
    ISSUER_WRONG_ROOT_NAME_LEN, /**< CA name length is bigger than ISSUER_MAX_NAME_SIZE */
    ISSUER_NULL_PASSED,         /**< NULL passed to function in issuer */
    ISSUER_WRONG_BYTE_ARRAY_LEN,/**< ByteArray with wrong length passed into issuer */

    ISSUER_CRL_ENCODER_MEMORY_ALLOC_FAILED, /**< Failed to allocate memory in CRL encoder */
    ISSUER_CRL_ENCODER_DER_ENCODE_FAIL,     /**< Failed to encode structure into DER
                                                 in CRL encoder */
    ISSUER_CRL_ENCODER_SIGNATURE_FAIL,      /**< Failed to sign TBS in CRL encoder */
    ISSUER_CRL_NULL_PASSED,                 /**< NULL passed to function in CRL encoder */
    ISSUER_CRL_WRONG_BYTE_ARRAY_LEN,        /**< ByteArray with wrong length passed into
                                                 CRL encoder */

    ISSUER_CSR_MEMORY_ALLOC_FAILED, /**< Failed to allocate memory in CSR unit */
    ISSUER_CSR_DER_ENCODE_FAIL,     /**< Failed to encode structure into DER in CSR unit */
    ISSUER_CSR_SIGNATURE_FAIL,      /**< Failed to sign TBS in CSR unit */
    ISSUER_CSR_DER_DECODE_FAIL,     /**< Failed to decode structure from DER in CSR unit */
    ISSUER_CSR_INVALID_SIGNATURE,   /**< Signature check fail in CSR unit. */
    ISSUER_CSR_TOO_LONG_NAME,       /**< CSR subject name length is bigger than CSR_MAX_NAME_SIZE */
    ISSUER_CSR_INVALID_KEY_FORMAT,  /**< Public key format is invalid in CSR unit. */
    ISSUER_CSR_NULL_PASSED,         /**< NULL passed to function in CSR unit */
    ISSUER_CSR_WRONG_BYTE_ARRAY_LEN,/**< ByteArray with wrong length passed into CSR unit */

    ISSUER_X509_MEMORY_ALLOC_FAILED, /**< Failed to allocate memory in X.509 encoder */
    ISSUER_X509_DER_ENCODE_FAIL,     /**< Failed to encode structure into DER in X.509 encoder */
    ISSUER_X509_SIGNATURE_FAIL,      /**< Failed to sign TBS in X.509 encoder */
    ISSUER_X509_NULL_PASSED,         /**< NULL passed to function in X.509 encoder */
    ISSUER_X509_WRONG_BYTE_ARRAY_LEN /**< ByteArray with wrong length passed into X.509 encoder */
} PKIError;


/** @def CHECK_PRINT(err_code, ...)
 *
 * Prints debug information \a err_code and  __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ values.
 *
 * @param[in] err_code Error code
*/
#ifdef X509_DEBUG
#define CHECK_PRINT(err_code) \
fprintf(stderr, "%s() in %s, line %i: %d\n",__func__, __FILE__, __LINE__, err_code);
#else
# define CHECK_PRINT(...)
#endif

/**
 * @def CHECK_NULL(param, error)
 * A macro that checks whether \a param is not NULL.
 *
 * If \a (param = NULL) it goes to error processing with \a error code.
 *
 * @param[in] param  Parameter to check
 * @param[in] error Error code
 */
#define CHECK_NULL(param, error) do {   \
        if  (!(param)) {                \
            error_value = error;        \
            CHECK_PRINT(error);         \
            goto ERROR_PROC;            \
        } } while(0)

/** @def CHECK_EQUAL(param, checker, err_code)
 *
 * A macro that checks whether \a param equal to \a checker.
 *
 * If \a (param != checker) it goes to error processing with \a err_code error code.
 *
 * @param[in] param  Parameter to check
 * @param[in] error Error code
 */
#define CHECK_EQUAL(param, checker, err_code) do { \
        if ((param) != (checker)) {                \
            error_value = err_code;                \
            CHECK_PRINT(err_code);                 \
            goto ERROR_PROC;                       \
        } } while(0)

/** @def CHECK_NOT_EQUAL(param, checker, err_code)
 * A macro that checks whether \a param not equal to \a checker.
 *
 * If \a (param == checker) it goes to error processing with \a err_code error code.
 *
 * @param[in] param  Parameter to check
 * @param[in] error Error code
 */
#define CHECK_NOT_EQUAL(param, checker, err_code) do { \
        if ((param) == (checker)) {                    \
            error_value = err_code;                    \
            CHECK_PRINT(err_code);                     \
            goto ERROR_PROC;                           \
        } } while(0)

/** @def CHECK_LESS(param, checker, err_code)
 * A macro that checks whether \a param less then \a checker.
 *
 * If \a (param > checker) it goes to error processing with \a err_code error code.
 *
 * @param[in] param  Parameter to check
 * @param[in] error error code
 */
#define CHECK_LESS(param, checker, err_code) do { \
        if ((param) >= (checker)) {               \
            error_value = err_code;               \
            CHECK_PRINT(err_code);                \
            goto ERROR_PROC;                      \
        } } while(0)

/** @def CHECK_COND(param, err_code)
 *
 * A macro that checks whether condition \a param is true.
 *
 * If \a (param != true) it goes to error processing with \a err_code error code.
 *
 * @param[in] param  Parameter to check
 * @param[in] error error code
 */
#define CHECK_COND(param, err_code) do { \
        if (!(param)) {                  \
            error_value = err_code;      \
            CHECK_PRINT(err_code);       \
            goto ERROR_PROC;             \
        } } while(0)

/** @def CHECK_LESS_EQUAL(param, checker, err_code)
*
* A macro that checks whether \a param <= \a checker.
*
* If \a (param < checker) it goes to error processing with \a err_code error code.
*
* @param[in] param  Parameter to check
* @param[in] error error code
*/
#define CHECK_LESS_EQUAL(param, checker, err_code) do { \
        if ((param) > (checker)) {                      \
            error_value = err_code;                     \
            CHECK_PRINT(err_code);                      \
            goto ERROR_PROC;                            \
        } } while(0)

/** @def CHECK_NULL_BYTE_ARRAY_PTR(param, err_code)
 *
 * A macro that checks whether pointer to ByteArray \a param is not NULL and contains a valid pointer.
 *
 * If \a (param != checker) it goes to error processing with \a err_code error code.
 *
 * @param[in] param  Parameter to check
 * @param[in] err_code Error code
 */
#define CHECK_NULL_BYTE_ARRAY_PTR(param, err_code) do { \
        CHECK_NULL(param, err_code);                    \
        CHECK_NULL((param)->data, err_code);            \
        CHECK_NULL((param)->len, err_code);             \
    } while(0)

/** @def FUNCTION_INIT(...)
 * A macro for initializations function variables.
 *
 * If error occurs it goes to error processing.
 */
#define FUNCTION_INIT(...)                    \
    PKIError error_value = PKI_UNKNOWN_ERROR; \
    __VA_ARGS__;

/** @def FUNCTION_CLEAR(...)
 *
 * A macro for freeing  function variables.
 *
 * @return  0 if successful
 */
#define FUNCTION_CLEAR(...)    \
    error_value = PKI_SUCCESS; \
    ERROR_PROC:                \
    __VA_ARGS__                \
    return error_value;

/** @def CHECK_CALL(fn, ...)
 * A macro that checks \a fn function return code
 *
 * If function return error code it goes to error processing.
 *
 * @param[in] fn  Function to call
 */
#define CHECK_CALL(fn, ...) do {                    \
        error_value = fn(__VA_ARGS__);              \
        if ((int)error_value != (int)PKI_SUCCESS) { \
            CHECK_PRINT(error_value);               \
            goto ERROR_PROC;                        \
        } } while(0)

/** @def CHECK_INC_BYTE_ARRAY_PTR(array, size)
 *
 * Increments byte array pointer \a array by \a size with bound checking.
 *
 * @param array byte array pointer
 * @param size number of positions
 */
#undef CHECK_INC_BYTE_ARRAY_PTR
#define CHECK_INC_BYTE_ARRAY_PTR(array, size) do{   \
        if (size > ((array)->len)){                 \
            error_value = PKI_BUFFER_OVERFLOW;      \
            CHECK_PRINT(error_value);               \
            goto ERROR_PROC; }                      \
        INC_BYTE_ARRAY_PTR(array, size);            \
    }while(0)

/** @def CHECK_INC_BYTE_ARRAY(array, size)
 *
 * Increments byte array \a array by \a size with bound checking.
 *
 * @param array byte array pointer
 * @param size number of positions
 */
#undef CHECK_INC_BYTE_ARRAY
#define CHECK_INC_BYTE_ARRAY(array, size) do{   \
        if (size > ((array).len)) {             \
            error_value = PKI_BUFFER_OVERFLOW;  \
            CHECK_PRINT(error_value);           \
            goto ERROR_PROC; }                  \
        INC_BYTE_ARRAY(array, size);            \
    }while(0)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _PKI_ERRORS_H_
