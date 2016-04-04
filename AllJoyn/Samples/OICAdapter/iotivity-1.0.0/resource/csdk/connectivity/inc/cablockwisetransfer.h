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
 * This file contains common function for block messages.
 */

#ifndef CA_BLOCKWISETRANSFER_H_
#define CA_BLOCKWISETRANSFER_H_

#include <stdint.h>

#include "coap.h"
#include "cathreadpool.h"
#include "camutex.h"
#include "uarraylist.h"
#include "cacommon.h"
#include "caprotocolmessage.h"

/**
 * Callback to send block data.
 * @param[in]   data    send data.
 */
typedef void (*CASendThreadFunc)(CAData_t *data);

/**
 * Callback to notify received data from the remote endpoint.
 * @param[in]   data    received data.
 */
typedef void (*CAReceiveThreadFunc)(CAData_t *data);

/**
 * context of blockwise transfer.
 */
typedef struct
{
    /** send method for block data. **/
    CASendThreadFunc sendThreadFunc;

    /** callback function for received message. **/
    CAReceiveThreadFunc receivedThreadFunc;

    /** array list on which the thread is operating. **/
    u_arraylist_t *dataList;

    /** data list mutex for synchronization. **/
    ca_mutex blockDataListMutex;

    /** sender mutex for synchronization. **/
    ca_mutex blockDataSenderMutex;
} CABlockWiseContext_t;

/**
 * ID set of Blockwise transfer data set(::CABlockData_t).
 */
typedef struct
{
    uint8_t* id;                       /**< blockData ID for CA. */
    size_t idLength;                   /**< length of blockData ID. */
} CABlockDataID_t;

/**
 * Block Data Set.
 */
typedef struct
{
    coap_block_t block1;                /**< block1 option. */
    coap_block_t block2;                /**< block2 option. */
    uint16_t type;                      /**< block option type. */
    CABlockDataID_t* blockDataId;        /**< ID set of CABlockData. */
    CAData_t *sentData;                 /**< sent request or response data information. */
    CAPayload_t payload;                /**< payload buffer. */
    size_t payloadLength;               /**< the total payload length to be received. */
    size_t receivedPayloadLen;          /**< currently received payload length. */
} CABlockData_t;

/**
 * state of received block message from remote endpoint.
 */
typedef enum
{
    CA_BLOCK_UNKNOWN = 0,
    CA_OPTION1_ACK,
    CA_OPTION1_NO_ACK_LAST_BLOCK,
    CA_OPTION1_NO_ACK_BLOCK,
    CA_OPTION2_FIRST_BLOCK,
    CA_OPTION2_LAST_BLOCK,
    CA_OPTION2_ACK,
    CA_OPTION2_NON,
    CA_OPTION2_CON,
    CA_SENT_PREVIOUS_NON_MSG,
    CA_BLOCK_INCOMPLETE,
    CA_BLOCK_TOO_LARGE,
    CA_BLOCK_RECEIVED_ALREADY
} CABlockState_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initializes the block-wise transfer context.
 * @param[in]  CASendThreadFunc    function point to add data in send queue thread.
 * @param[in]  CAReceiveThreadFunc function point to add data in receive queue thread.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAInitializeBlockWiseTransfer(CASendThreadFunc blockSendMethod,
                                         CAReceiveThreadFunc receivedDataCallback);

/**
 * Terminate the block-wise transfer context.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CATerminateBlockWiseTransfer();

/**
 * initialize mutex.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAInitBlockWiseMutexVariables();

/**
 * terminate mutex.
 */
void CATerminateBlockWiseMutexVariables();

/**
 * Pass the bulk data. if block-wise transfer process need,
 *          bulk data will be sent to block messages.
 * @param[in]   data    data for sending.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CASendBlockWiseData(const CAData_t *data);

/**
 * Add the data to send thread queue.
 * @param[in]   sendData    data for sending.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAddSendThreadQueue(const CAData_t *sendData, const CABlockDataID_t *blockID);

/**
 * Check the block option type. If it has to be sent to a block,
 *          block option will be set.
 * @param[in]   currData    block data.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CACheckBlockOptionType(CABlockData_t *currData);

/**
 * Pass the received pdu data. and check if block option is set.
 * @param[in]   pdu    received pdu binary data.
 * @param[in]   endpoint    information of remote device.
 * @param[in]   receivedData    received CAData.
 * @param[in]   dataLen    received data length.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAReceiveBlockWiseData(coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                  const CAData_t *receivedData, size_t dataLen);

/**
 * process next step by block-wise state.
 * @param[in]   pdu    received pdu binary data.
 * @param[in]   receivedData    received CAData.
 * @param[in]   blockWiseStatus    block-wise state to move next step.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAProcessNextStep(const coap_pdu_t *pdu, const CAData_t *receivedData,
                             uint8_t blockWiseStatus, const CABlockDataID_t *blockID);

/**
 * send block message to remote device.
 * @param[in]   pdu    received pdu binary data.
 * @param[in]   msgType    the message type of the block.
 * @param[in]   status    block-wise state to move next step.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CASendBlockMessage(const coap_pdu_t *pdu, CAMessageType_t msgType,
                              uint8_t status, const CABlockDataID_t *blockID);

/**
 * send error message to remote device.
 * @param[in]   pdu    received pdu binary data.
 * @param[in]   status    block-wise state to move next step.
 * @param[in]   responseResult   response result code.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CASendErrorMessage(const coap_pdu_t *pdu, uint8_t status,
                              CAResponseResult_t responseResult,
                              const CABlockDataID_t *blockID);

/**
 * receive last block data.
 * @param[in]   blockID     ID set of CABlockData.
 * @param[in]   receivedData    received CAData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAReceiveLastBlock(const CABlockDataID_t *blockID,
                              const CAData_t *receivedData);

/**
 * set next block option 1.
 * @param[in]   pdu received pdu binary data.
 * @param[in]   endpoint  information of remote device.
 * @param[in]   receivedData    received CAData.
 * @param[in]   block   block option data.
 * @param[in]   dataLen received data length.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CASetNextBlockOption1(coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                 const CAData_t *receivedData, coap_block_t block,
                                 size_t dataLen);

/**
 * set next block option 2.
 * @param[in]   pdu received pdu binary data.
 * @param[in]   endpoint    information of remote device.
 * @param[in]   receivedData    received CAData.
 * @param[in]   block   block option data.
 * @param[in]   dataLen received data length.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CASetNextBlockOption2(coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                 const CAData_t *receivedData, coap_block_t block,
                                 size_t dataLen);

/**
 * Update the block option in block-wise transfer list.
 * @param[in]   currData   stored block data information.
 * @param[in]   block   block option to update.
 * @param[in]   msgType message type of pdu.
 * @param[in]   blockType   block option type.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CANegotiateBlockSize(CABlockData_t *currData, coap_block_t *block,
                                CAMessageType_t msgType, uint16_t blockType);

/**
 * Update the block option in block-wise transfer list.
 * @param[in]   currData    stored block data information.
 * @param[in]   block   block option of current message.
 * @param[in]   blockType   block option type.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAUpdateBlockData(CABlockData_t *currData, coap_block_t block,
                             uint16_t blockType);

/**
 * Update the messageId in block-wise transfer list.
 * @param[in]   pdu   received pdu binary data.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAUpdateMessageId(coap_pdu_t *pdu, const CABlockDataID_t *blockID);

/**
 * Update the block option items.
 * @param[in]   currData    stored block data information.
 * @param[in]   pdu received pdu binary data.
 * @param[in/out]   block   block option of current message.
 * @param[in]   blockType  block option type.
 * @param[in]   status  current flow status for block-wise transfer.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAUpdateBlockOptionItems(CABlockData_t *currData, const coap_pdu_t *pdu,
                                    coap_block_t *block, uint16_t blockType,
                                    uint32_t status);
/**
 * Set the M-bit of block option.
 * @param[in]   payloadLen  payload length of current bulk data.
 * @param[out]  block   block option.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetMoreBitFromBlock(size_t payloadLen, coap_block_t *block);

/**
 * check the block option what kind of option have to set.
 * @param[out]  pdu pdu object.
 * @param[in]   info    information of the request/response.
 * @param[in]   endpoint    port of transport.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAddBlockOption(coap_pdu_t **pdu, const CAInfo_t *info,
                            const CAEndpoint_t *endpoint, coap_list_t **options);

/**
 * Write the block option2 in pdu binary data.
 * @param[out]  pdu   pdu object.
 * @param[in]   info    information of the request/response.
 * @param[in]   dataLength  length of payload.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAddBlockOption2(coap_pdu_t **pdu, const CAInfo_t *info, size_t dataLength,
                             const CABlockDataID_t *blockID, coap_list_t **options);

/**
 * Write the block option1 in pdu binary data.
 * @param[out]  pdu    pdu object.
 * @param[in]   info    information of the request/response.
 * @param[in]   dataLength length of payload.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAddBlockOption1(coap_pdu_t **pdu, const CAInfo_t *info, size_t dataLength,
                             const CABlockDataID_t *blockID, coap_list_t **options);

/**
 * Add the block option in pdu data.
 * @param[in]   pdu    pdu object.
 * @param[out]  block    block data.
 * @param[in]   blockType   block option type.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAddBlockOptionImpl(coap_pdu_t *pdu, coap_block_t *block, uint8_t blockType,
                                coap_list_t **options);

/**
 * Add the size option in pdu data.
 * @param[in/out]   pdu    pdu object.
 * @param[in]   sizeType    size option type.
 * @param[in]   dataLength the total payload length to be sent.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAddBlockSizeOption(coap_pdu_t *pdu, uint16_t sizeType, size_t dataLength,
                                coap_list_t **options);

/**
 * Get the size option from pdu data.
 * @param[in]   pdu    pdu object.
 * @param[in]   sizeType    size option type.
 * @param[out]  totalPayloadLen the total payload length to be received.
 * @return true or false.
 */
bool CAIsPayloadLengthInPduWithBlockSizeOption(coap_pdu_t *pdu,
                                               uint16_t sizeType,
                                               size_t *totalPayloadLen);

/**
 * update the total payload with the received payload.
 * @param[in]   currData    stored block data information.
 * @param[in]   receivedData    received CAData.
 * @param[in]   status  block-wise state.
 * @param[in]   isSizeOption    size option.
 * @param[in]   blockType    block option type.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAUpdatePayloadData(CABlockData_t *currData, const CAData_t *receivedData,
                               uint8_t status, bool isSizeOption, uint16_t blockType);

/**
 * Generate CAData structure  from the given information.
 * @param[in]   pdu    pdu object.
 * @param[in]   endpoint    information of remote device.
 * @return generated CAData.
 */
CAData_t* CACreateNewDataSet(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint);

/**
 * Update the block option items.
 * @param[in/out]   blockblock option of current message.
 * @param[in]   blockType   block option type.
 * @param[in]   responseResult  result code of pdu.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAHandleBlockErrorResponse(coap_block_t *block, uint16_t blockType,
                                      uint32_t responseResult);

/**
 * Check the received payload and if an error happens, return error type.
 * @param[in/out]   currData    stored block data information.
 * @param[in]   receivedBlock   received block option.
 * @param[in]   receivedData    message type of pdu.
 * @param[in]   blockType   block option type.
 * @param[in]   dataLen received data length.
 * @return block state.
 */
uint8_t CACheckBlockErrorType(CABlockData_t *currData, coap_block_t *receivedBlock,
                              const CAData_t *receivedData, uint16_t blockType,
                              size_t dataLen);

/**
 * Destroys the given CAData.
 * @param[in]   data    CAData to destroy.
 * @return generated CAData.
 */
void CADestroyDataSet(CAData_t* data);

/**
 * Create the blockId for CABlockData.
 * @param[in]   token   token of current message.
 * @param[in]   tokenLength   token length of current message.
 * @param[in]   portNumber   port.
 * @return ID set of CABlockData.
 */
CABlockDataID_t* CACreateBlockDatablockId(const CAToken_t token, uint8_t tokenLength,
                                          uint16_t portNumber);

/**
 * Destroy the blockId set.
 * @param[in]   blockID     ID set of CABlockData.
 */
void CADestroyBlockID(CABlockDataID_t *blockID);

/**
 * check whether Block ID is matched or not.
 * @param[in]   currData    block data.
 * @param[in]   blockID     ID set of CABlockData.
 * @return true or false.
 */
bool CABlockidMatches(const CABlockData_t *currData, const CABlockDataID_t *blockID);
/**
 * Print the given block option information.
 * @param[in]   block   block option information.
 */
void CALogBlockInfo(coap_block_t *block);

/**
 * Create new CAData structure from the input information.
 * @param[in]   data    CAData information that needs to be duplicated.
 * @return created CAData structure.
 */
CAData_t *CACloneCAData(const CAData_t *data);

/**
 * Update payload from the input information.
 * @param[in]   data    CAData information that needs to be updated.
 * @param[in]   payload received new payload from the remote endpoint.
 * @param[in]   payloadLen  received full payload length.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAUpdatePayloadToCAData(CAData_t *data, const CAPayload_t payload,
                                   size_t payloadLen);

/**
 * Get payload and payload length from the input information.
 * @param[in]   data    CAData information.
 * @param[out]  payloadLen  The payload length is stored.
 * @return payload.
 */
CAPayload_t CAGetPayloadInfo(const CAData_t *data, size_t *payloadLen);

/**
 * Set the block option type.
 * @param[in]   blockID     ID set of CABlockData.
 * @param[in]   blockType   block option type.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAUpdateBlockOptionType(const CABlockDataID_t *blockID,
                                   uint8_t blockType);

/**
 * Get the block option type from block-wise transfer list.
 * @param[in]   blockID     ID set of CABlockData.
 * @return block option type.
 */
uint8_t CAGetBlockOptionType(const CABlockDataID_t *blockID);

/**
 * Get the block data from block-wise transfer list.
 * @param[in]   blockID     ID set of CABlockData.
 * @return CAData structure.
 */
CAData_t *CAGetDataSetFromBlockDataList(const CABlockDataID_t *blockID);

/**
 * Get token from block-wise transfer list.
 * @param[in]   pdu    received pdu binary data.
 * @param[in]   endpoint    remote endpoint information.
 * @param[in]   responseInfo    received response information.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetTokenFromBlockDataList(const coap_pdu_t *pdu, const CAEndpoint_t *endpoint,
                                       CAResponseInfo_t *responseInfo);

/**
 * check whether the block data is valid or not.
 * @param[in]   sendData    CAData information.
 * @param[out]  blockData   block data when it is valid.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CACheckBlockDataValidation(const CAData_t *sendData, CABlockData_t **blockData);

/**
 * Get the block data from block-wise transfer list.
 * @param[in]   blockID     ID set of CABlockData.
 * @return CABlockData_t structure.
 */
CABlockData_t *CAGetBlockDataFromBlockDataList(const CABlockDataID_t *blockID);

/**
 * Get the block option from block-wise transfer list.
 * @param[in]   blockID     ID set of CABlockData.
 * @param[in]   blockType    block option type.
 * @return coap_block_t structure.
 */
coap_block_t *CAGetBlockOption(const CABlockDataID_t *blockID,
                               uint16_t blockType);

/**
 * Get the full payload from block-wise list.
 * @param[in]   blockID     ID set of CABlockData.
 * @param[out]  fullPayloadLen  received full payload length.
 * @return payload.
 */
CAPayload_t CAGetPayloadFromBlockDataList(const CABlockDataID_t *blockID,
                                          size_t *fullPayloadLen);

/**
 * Create the block data from given data and add the data in block-wise transfer list.
 * @param[in]   sendData    data to be added to a list.
 * @return created CABlockData_t structure.
 *         and NULL point will be returned if there is error case..
 */
CABlockData_t *CACreateNewBlockData(const CAData_t *sendData);

/**
 * Remove the block data in block-wise transfer list.
 * @param[in]   blockID     ID set of CABlockData.
 * @return ::CASTATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARemoveBlockDataFromList(const CABlockDataID_t *blockID);

/**
 * Check if data exist in block-wise transfer list.
 * @param[in]   blockID     ID set of CABlockData.
 * @return true or false.
 */
bool CAIsBlockDataInList(const CABlockDataID_t *blockID);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // CA_BLOCKWISETRANSFER_H_
