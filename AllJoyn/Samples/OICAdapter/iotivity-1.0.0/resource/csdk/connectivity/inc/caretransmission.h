/******************************************************************
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
 * This file contains common function for retransmission messages.
 */

#ifndef CA_RETRANSMISSION_H_
#define CA_RETRANSMISSION_H_

#include <stdint.h>

#include "cathreadpool.h"
#include "camutex.h"
#include "uarraylist.h"
#include "cacommon.h"

/** IP, EDR, LE. **/
#define DEFAULT_RETRANSMISSION_TYPE (CA_ADAPTER_IP | \
                                     CA_ADAPTER_RFCOMM_BTEDR | \
                                     CA_ADAPTER_GATT_BTLE)

/** default ACK time is 2 sec(CoAP). **/
#define DEFAULT_ACK_TIMEOUT_SEC     2

/** default max retransmission trying count is 4(CoAP). **/
#define DEFAULT_RETRANSMISSION_COUNT      4

/** check period is 1 sec. **/
#define RETRANSMISSION_CHECK_PERIOD_SEC     1

/** retransmission data send method type. **/
typedef CAResult_t (*CADataSendMethod_t)(const CAEndpoint_t *endpoint,
                                         const void *pdu,
                                         uint32_t size);

/** retransmission timeout callback type. **/
typedef void (*CATimeoutCallback_t)(const CAEndpoint_t *endpoint,
                                    const void *pdu,
                                    uint32_t size);

typedef struct
{
    /** retransmission support transport type. **/
    CATransportAdapter_t supportType;

    /** retransmission trying count. **/
    uint8_t tryingCount;

} CARetransmissionConfig_t;

typedef struct
{
    /** Thread pool of the thread started. **/
    ca_thread_pool_t threadPool;

    /** mutex for synchronization. **/
    ca_mutex threadMutex;

    /** conditional mutex for synchronization. **/
    ca_cond threadCond;

    /** send method for retransmission data. **/
    CADataSendMethod_t dataSendMethod;

    /** callback function for retransmit timeout. **/
    CATimeoutCallback_t timeoutCallback;

    /** retransmission configure data. **/
    CARetransmissionConfig_t config;

    /** Variable to inform the thread to stop. **/
    bool isStop;

    /** array list on which the thread is operating. **/
    u_arraylist_t *dataList;

} CARetransmission_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initializes the retransmission context.
 * @param[in]   context                      context for retransmission.
 * @param[in]   handle                       thread pool handle.
 * @param[in]   retransmissionSendMethod     function to be called for retransmission.
 * @param[in]   timeoutCallback              callback for retransmit timeout.
 * @param[in]   config                       configuration for retransmission.
 *                                           if NULL is coming, it will set default values.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARetransmissionInitialize(CARetransmission_t *context,
                                      ca_thread_pool_t handle,
                                      CADataSendMethod_t retransmissionSendMethod,
                                      CATimeoutCallback_t timeoutCallback,
                                      CARetransmissionConfig_t* config);

/**
 * Starting the retransmission context.
 * @param[in]   context      context for retransmission.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARetransmissionStart(CARetransmission_t *context);

/**
 * Pass the sent pdu data. if retransmission process need, internal thread will wake up and
 * process the retransmission data.
 * @param[in]   context      context for retransmission.
 * @param[in]   endpoint     endpoint information.
 * @param[in]   pdu          sent pdu binary data.
 * @param[in]   size         sent pdu binary data size.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARetransmissionSentData(CARetransmission_t* context,
                                    const CAEndpoint_t* endpoint,
                                    const void* pdu, uint32_t size);

/**
 * Pass the received pdu data. if received pdu is ACK data for the retransmission CON data,
 * the specified CON data will remove on retransmission list.
 * @param[in]   context              context for retransmission.
 * @param[in]   endpoint             endpoint information.
 * @param[in]   pdu                  received pdu binary data.
 * @param[in]   size                 received pdu binary data size.
 * @param[out]  retransmissionPdu    pdu data of the request for reset and ack.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARetransmissionReceivedData(CARetransmission_t *context,
                                        const CAEndpoint_t *endpoint, const void *pdu,
                                        uint32_t size, void **retransmissionPdu);

/**
 * Stopping the retransmission context.
 * @param[in]   context         context for retransmission.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARetransmissionStop(CARetransmission_t *context);

/**
 * Terminating the retransmission context.
 * @param[in]   context         context for retransmission.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CARetransmissionDestroy(CARetransmission_t *context);

/**
 * Invoke Retransmission according to TimedAction Response.
 * @param[in]   threadValue     context for retransmission.
 */
void CARetransmissionBaseRoutine(void *threadValue);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* CA_RETRANSMISSION_H_ */
