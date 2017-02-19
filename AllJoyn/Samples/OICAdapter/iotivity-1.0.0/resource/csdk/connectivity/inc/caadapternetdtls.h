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
#ifndef CA_ADAPTER_NET_DTLS_H_
#define CA_ADAPTER_NET_DTLS_H_

#include "dtls.h"
#include "uarraylist.h"
#include "camutex.h"
#include "caadapterutils.h"
#include "cainterface.h"
#include "cacommon.h"

/**
 * Currently DTLS supported adapters(2) WIFI and ETHENET for linux platform.
 */
#define MAX_SUPPORTED_ADAPTERS 2

typedef void (*CAPacketReceivedCallback)(const CASecureEndpoint_t *sep,
                                         const void *data, uint32_t dataLength);

typedef void (*CAPacketSendCallback)(CAEndpoint_t *endpoint,
                                         const void *data, uint32_t dataLength);

/**
 * Data structure for holding the send and recv callbacks.
 */
typedef struct CAAdapterCallbacks
{
    CAPacketReceivedCallback recvCallback;  /**< Callback used to send data to upper layer. */
    CAPacketSendCallback sendCallback;      /**< Callback used to send data to socket layer. */
} stCAAdapterCallbacks_t;

/**
 * Data structure for holding the tinyDTLS interface related info.
 */
typedef struct stCADtlsContext
{
    u_arraylist_t *peerInfoList;         /**< peerInfo list which holds the mapping between
                                              peer id to it's n/w address. */
    u_arraylist_t *cacheList;            /**< PDU's are cached until DTLS session is formed. */
    struct dtls_context_t *dtlsContext;  /**< Pointer to tinyDTLS context. */
    struct stPacketInfo *packetInfo;     /**< used by callback during
                                              decryption to hold address/length. */
    dtls_handler_t callbacks;            /**< Pointer to callbacks needed by tinyDTLS. */
    stCAAdapterCallbacks_t adapterCallbacks[MAX_SUPPORTED_ADAPTERS];
} stCADtlsContext_t;

/**
 * Data structure for holding the decrypted data address
 * and length provided by tinyDTLS callback interface.
 */
typedef struct stPacketInfo
{
    uint8_t *dataAddress;
    uint16_t dataLen;
} stPacketInfo_t;

/**
 * tinyDTLS library error codes.
 *
 */
typedef enum
{
    DTLS_OK = 0,
    DTLS_FAIL,
    DTLS_SESSION_INITIATED,
    DTLS_HS_MSG
} eDtlsRet_t;


/** Structure to have address information which will match with DTLS session_t structure.*/
typedef struct
{
    socklen_t size;                 /**< Size of address. */
    union
    {
        struct sockaddr     sa;
        struct sockaddr_storage st;
        struct sockaddr_in  sin;
        struct sockaddr_in6 sin6;
    } addr;                         /**< Address information. */
    uint8_t ifIndex;                /**< Holds adapter index to get callback info. */
} stCADtlsAddrInfo_t;

/**
 * structure to holds the information of cache message and address info.
 */
typedef struct CACacheMessage
{
    void *data;
    uint32_t dataLen;
    stCADtlsAddrInfo_t destSession;
} stCACacheMessage_t;


/**
 * Used set send and recv callbacks for different adapters(WIFI,EtherNet).
 *
 * @param[in]  recvCallback    packet received callback.
 * @param[in]  sendCallback    packet sent callback.
 * @param[in]  type  type of adapter.
 *
 */
void CADTLSSetAdapterCallbacks(CAPacketReceivedCallback recvCallback,
                               CAPacketSendCallback sendCallback,
                               CATransportAdapter_t type);

/**
 * Register callback to get DTLS PSK credentials.
 * @param[in]  credCallback    callback to get DTLS PSK credentials.
 */
void CADTLSSetCredentialsCallback(CAGetDTLSPskCredentialsHandler credCallback);

/**
 * Select the cipher suite for dtls handshake
 *
 * @param[in] cipher    cipher suite
 *                             0xC018 : TLS_ECDH_anon_WITH_AES_128_CBC_SHA_256
 *                             0xC0A8 : TLS_PSK_WITH_AES_128_CCM_8
 *                             0xC0AE : TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8
 *
 * @retval  ::CA_STATUS_OK for success, otherwise some error value
 */
CAResult_t CADtlsSelectCipherSuite(const dtls_cipher_t cipher);

/**
 * Enable anonymous ECDH cipher suite for dtls handshake
 *
 * @param[in] enable  TRUE/FALSE enables/disables anonymous cipher suite
 *
 * @retval  ::CA_STATUS_OK for success, otherwise some error value
 */
CAResult_t CADtlsEnableAnonECDHCipherSuite(const bool enable);

/**
 * Initiate DTLS handshake with selected cipher suite
 *
 * @param[in] endpoint  information of network address
 *
 * @retval  ::CA_STATUS_OK for success, otherwise some error value
 */
CAResult_t CADtlsInitiateHandshake(const CAEndpoint_t *endpoint);

/**
 * Close the DTLS session
 *
 * @param[in] endpoint  information of network address
 *
 * @retval  ::CA_STATUS_OK for success, otherwise some error value
 */
CAResult_t CADtlsClose(const CAEndpoint_t *endpoint);

/**
 * Generate ownerPSK using PRF
 * OwnerPSK = TLS-PRF('master key' , 'oic.sec.doxm.jw',
 *                                    'ID of new device(Resource Server)',
 *                                    'ID of owner smart-phone(Provisioning Server)')
 *
 * @param[in] endpoint  information of network address
 * @param[in] label  Ownership transfer method e.g)"oic.sec.doxm.jw"
 * @param[in] labelLen  Byte length of label
 * @param[in] rsrcServerDeviceID  ID of new device(Resource Server)
 * @param[in] rsrcServerDeviceIDLen  Byte length of rsrcServerDeviceID
 * @param[in] provServerDeviceID  label of previous owner
 * @param[in] provServerDeviceIDLen  byte length of provServerDeviceID
 * @param[in,out] ownerPSK  Output buffer for owner PSK
 * @param[in] ownerPSKSize  Byte length of the ownerPSK to be generated
 *
 * @retval  ::CA_STATUS_OK for success, otherwise some error value
 */
CAResult_t CADtlsGenerateOwnerPSK(const CAEndpoint_t *endpoint,
                    const uint8_t* label, const size_t labelLen,
                    const uint8_t* rsrcServerDeviceID, const size_t rsrcServerDeviceIDLen,
                    const uint8_t* provServerDeviceID, const size_t provServerDeviceIDLen,
                    uint8_t* ownerPSK, const size_t ownerPSKSize);
;

/**
 * initialize tinyDTLS library and other necessary initialization.
 *
 * @return  0 on success otherwise a positive error value.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 *
 */
CAResult_t CAAdapterNetDtlsInit();

/**
 * de-inits tinyDTLS library and free the allocated memory.
 */
void CAAdapterNetDtlsDeInit();

/**
 * Performs DTLS encryption of the CoAP PDU. If a DTLS session does not exist yet
 * with the @dst, a DTLS handshake will be started. In case where a new DTLS handshake
 * is started, pdu info is cached to be send when session setup is finished.
 *
 * @param[in]  endpoint  address to which data will be sent.
 * @param[in]  port  port to which data will be sent.
 * @param[in]  data  length of data.
 * @param[in]  dataLen  length of given data
 *
 * @return  0 on success otherwise a positive error value.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 *
 */

CAResult_t CAAdapterNetDtlsEncrypt(const CAEndpoint_t *endpoint,
                                   void *data,
                                   uint32_t dataLen);

/**
 * Performs DTLS decryption of the data received on
 * secure port. This method performs in-place decryption
 * of the cipher-text buffer. If a DTLS handshake message
 * is received or decryption failure happens, this method
 * returns -1. If a valid application PDU is decrypted, it
 * returns the length of the decrypted pdu.
 *
 * @return  0 on success otherwise a positive error value.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 *
 */
CAResult_t CAAdapterNetDtlsDecrypt(const CASecureEndpoint_t *sep,
                                   uint8_t *data,
                                   uint32_t dataLen);

#endif /* CA_ADAPTER_NET_DTLS_H_ */


