/* *****************************************************************
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
 * This file contains message functionality.
 */

#ifndef CA_MESSAGE_HANDLER_H_
#define CA_MESSAGE_HANDLER_H_

#include "cacommon.h"
#include "coap.h"

#define CA_MEMORY_ALLOC_CHECK(arg) { if (NULL == arg) {OIC_LOG(ERROR, TAG, "Out of memory"); \
goto memory_error_exit;} }

typedef enum
{
    SEND_TYPE_MULTICAST = 0,
    SEND_TYPE_UNICAST
} CASendDataType_t;

typedef enum
{
    CA_REQUEST_DATA = 1,
    CA_RESPONSE_DATA = 2,
    CA_ERROR_DATA = 3,
} CADataType_t;

typedef struct
{
    CASendDataType_t type;
    CAEndpoint_t *remoteEndpoint;
    CARequestInfo_t *requestInfo;
    CAResponseInfo_t *responseInfo;
    CAErrorInfo_t *errorInfo;
    CADataType_t dataType;
} CAData_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Detaches control from the caller for sending unicast request.
 * @param[in] endpoint    endpoint information where the data has to be sent.
 * @param[in] request     request that needs to be sent.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CADetachRequestMessage(const CAEndpoint_t *endpoint,
                                  const CARequestInfo_t *request);

/**
 * Detaches control from the caller for sending multicast request.
 * @param[in] object     Group endpoint information where the data has to be sent.
 * @param[in] request    request that needs to be sent.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CADetachRequestToAllMessage(const CAEndpoint_t *object,
                                       const CARequestInfo_t *request);

/**
 * Detaches control from the caller for sending response.
 * @param[in] endpoint    endpoint information where the data has to be sent.
 * @param[in] response    response that needs to be sent.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CADetachResponseMessage(const CAEndpoint_t *endpoint,
                                   const CAResponseInfo_t *response);

/**
 * Detaches control from the caller for sending request.
 * @param[in] resourceUri    resource uri that needs to  be sent in the request.
 * @param[in] token          token information of the request.
 * @param[in] tokenLength    length of the token.
 * @param[in] options        header options that need to be append in the request.
 * @param[in] numOptions     number of options be appended.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CADetachMessageResourceUri(const CAURI_t resourceUri, const CAToken_t token,
                                      uint8_t tokenLength, const CAHeaderOption_t *options,
                                      uint8_t numOptions);

/**
 * Setting the request and response callbacks for network packets.
 * @param[in] ReqHandler      callback for receiving the requests.
 * @param[in] RespHandler     callback for receiving the response.
 * @param[in] ErrorHandler    callback for receiving error response.
 */
void CASetInterfaceCallbacks(CARequestCallback ReqHandler, CAResponseCallback RespHandler,
                             CAErrorCallback ErrorHandler);

/**
 * Initialize the message handler by starting thread pool and initializing the
 * send and receive queue.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAInitializeMessageHandler();

/**
 * Terminate the message handler by stopping  the thread pool and destroying the queues.
 */
void CATerminateMessageHandler();

/**
 * Handler for receiving request and response callback in single thread model.
 */
void CAHandleRequestResponseCallbacks();

/**
 * To log the PDU data.
 * @param[in] pdu    pdu data.
 * @param[in] endpoint  endpoint
 */
void CALogPDUInfo(coap_pdu_t *pdu, const CAEndpoint_t *endpoint);

#ifdef WITH_BWT
/**
 * Add the data to the send queue thread.
 * @param[in] data    send data.
 */
void CAAddDataToSendThread(CAData_t *data);

/**
 * Add the data to the receive queue thread to notify received data.
 * @param[in] data    received data.
 */
void CAAddDataToReceiveThread(CAData_t *data);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_MESSAGE_HANDLER_H_ */
