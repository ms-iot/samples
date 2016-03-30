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
 * This file contains the Security APIs for Resource Model to use.
 */

#ifndef CA_SECURITY_INTERFACE_H_
#define CA_SECURITY_INTERFACE_H_

#ifdef __WITH_X509__
#include "pki.h"
#endif //__WITH_X509__


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __WITH_DTLS__
/**
 * @enum CADtlsPskCredType_t
 * Type of PSK credential required during DTLS handshake
 * It does not make much sense in bringing in all definitions from dtls.h into here.
 * Therefore, redefining them here.
 */
typedef enum
{
    CA_DTLS_PSK_HINT,
    CA_DTLS_PSK_IDENTITY,
    CA_DTLS_PSK_KEY
} CADtlsPskCredType_t;

/**
 * This internal callback is used by CA layer to
 * retrieve PSK credentials from SRM.
 *
 * @param[in]  type type of PSK data required by CA layer during DTLS handshake set.
 * @param[in]  desc    Additional request information.
 * @param[in]  desc_len The actual length of desc.
 * @param[out] result  Must be filled with the requested information.
 * @param[in]  result_length  Maximum size of @p result.
 *
 * @return The number of bytes written to @p result or a value
 *         less than zero on error.
 */
typedef int (*CAGetDTLSPskCredentialsHandler)( CADtlsPskCredType_t type,
		      const unsigned char *desc, size_t desc_len,
		      unsigned char *result, size_t result_length);

/**
 * Register callback to get DTLS PSK credentials.
 * @param[in]   GetDTLSCredentials    GetDTLS Credetials callback.
 * @return  ::CA_STATUS_OK
 */
CAResult_t CARegisterDTLSCredentialsHandler(CAGetDTLSPskCredentialsHandler GetDTLSCredentials);

#endif //__WITH_DTLS__

#ifdef __WITH_X509__
/**
 * Binary structure containing certificate chain and certificate credentials
 * for this device.
 */
typedef struct
{
    // certificate message  for DTLS
    unsigned char certificateChain[MAX_CERT_MESSAGE_LEN];
    // length of the certificate message
    uint32_t  certificateChainLen;
    // number of certificates in  certificate message
    uint8_t   chainLen;
    // x component of EC public key
    uint8_t   rootPublicKeyX[PUBLIC_KEY_SIZE / 2];
    // y component of EC public key
    uint8_t   rootPublicKeyY[PUBLIC_KEY_SIZE / 2];
    // EC private key
    uint8_t   devicePrivateKey[PRIVATE_KEY_SIZE];

} CADtlsX509Creds_t;

/**
 * @brief   Callback function type for getting certificate credentials.
 * @param   credInfo          [OUT] Certificate credentials info. Handler has to allocate new memory for
 *                                  credInfo which is then freed by CA
 * @return  NONE
 */
typedef int (*CAGetDTLSX509CredentialsHandler)(CADtlsX509Creds_t *credInfo);
/**
 * @brief   Callback function type for getting CRL.
 * @param   crlInfo          [OUT] Certificate credentials info. Handler has to allocate new memory for
 *                                  credInfo which is then freed by CA
 * @return  NONE
 */
typedef void (*CAGetDTLSCrlHandler)(ByteArray crlInfo);

/**
 * @brief   Register callback to get DTLS Cert credentials.
 * @param   GetCertCredentials   [IN] GetCert Credetials callback
 * @return  #CA_STATUS_OK
 */
CAResult_t CARegisterDTLSX509CredentialsHandler(CAGetDTLSX509CredentialsHandler GetX509Credentials);
/**
 * @brief   Register callback to get CRL.
 * @param   GetCrl   [IN] GetCrl callback
 * @return  #CA_STATUS_OK
 */
CAResult_t CARegisterDTLSCrlHandler(CAGetDTLSCrlHandler GetCrl);
#endif //__WITH_X509__


#ifdef __WITH_DTLS__

/**
 * Select the cipher suite for dtls handshake.
 *
 * @param[in] cipher  cipher suite (Note : Make sure endianness).
 *                    0xC018 : TLS_ECDH_anon_WITH_AES_128_CBC_SHA
 *                    0xC0A8 : TLS_PSK_WITH_AES_128_CCM_8
 *                    0xC0AE : TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8
 *
 * @retval  ::CA_STATUS_OK    Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CASelectCipherSuite(const uint16_t cipher);

/**
 * Enable TLS_ECDH_anon_WITH_AES_128_CBC_SHA cipher suite in dtls.
 *
 * @param[in] enable  TRUE/FALSE enables/disables anonymous cipher suite.
 *
 * @retval  ::CA_STATUS_OK    Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 *
 * @note anonymous cipher suite should only be enabled for 'JustWorks' provisioning.
 */
CAResult_t CAEnableAnonECDHCipherSuite(const bool enable);


/**
 * Generate ownerPSK using PRF.
 * OwnerPSK = TLS-PRF('master key' , 'oic.sec.doxm.jw',
 *                    'ID of new device(Resource Server)',
 *                    'ID of owner smart-phone(Provisioning Server)')
 *
 * @param[in] endpoint  information of network address.
 * @param[in] label  Ownership transfer method e.g)"oic.sec.doxm.jw".
 * @param[in] labelLen  Byte length of label.
 * @param[in] rsrcServerDeviceID  ID of new device(Resource Server).
 * @param[in] rsrcServerDeviceIDLen  Byte length of rsrcServerDeviceID.
 * @param[in] provServerDeviceID  label of previous owner.
 * @param[in] provServerDeviceIDLen  byte length of provServerDeviceID.
 * @param[in,out] ownerPSK  Output buffer for owner PSK.
 * @param[in] ownerPSKSize  Byte length of the ownerPSK to be generated.
 *
 * @retval  ::CA_STATUS_OK    Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAGenerateOwnerPSK(const CAEndpoint_t *endpoint,
                              const uint8_t* label, const size_t labelLen,
                              const uint8_t* rsrcServerDeviceID,
                              const size_t rsrcServerDeviceIDLen,
                              const uint8_t* provServerDeviceID,
                              const size_t provServerDeviceIDLen,
                              uint8_t* ownerPSK, const size_t ownerPSKSize);

/**
 * Initiate DTLS handshake with selected cipher suite.
 *
 * @param[in] endpoint  information of network address.
 *
 * @retval  ::CA_STATUS_OK    Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAInitiateHandshake(const CAEndpoint_t *endpoint);

/**
 * Close the DTLS session.
 *
 * @param[in] endpoint  information of network address.
 *
 * @retval  ::CA_STATUS_OK    Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CACloseDtlsSession(const CAEndpoint_t *endpoint);

#endif /* __WITH_DTLS__ */


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CA_SECURITY_INTERFACE_H_ */

