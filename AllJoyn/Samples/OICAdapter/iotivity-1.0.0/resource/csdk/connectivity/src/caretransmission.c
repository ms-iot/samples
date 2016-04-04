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

// Defining _BSD_SOURCE or _DEFAULT_SOURCE causes header files to expose
// definitions that may otherwise be skipped. Skipping can cause implicit
// declaration warnings and/or bugs and subtle problems in code execution.
// For glibc information on feature test macros,
// Refer http://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
//
// This file requires #define use due to random()
// For details on compatibility and glibc support,
// Refer http://www.gnu.org/software/libc/manual/html_node/BSD-Random.html
#define _DEFAULT_SOURCE

// Defining _POSIX_C_SOURCE macro with 199309L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1b, Real-time extensions
// (IEEE Std 1003.1b-1993) specification
//
// For this specific file, see use of clock_gettime,
// Refer to http://pubs.opengroup.org/stage7tc1/functions/clock_gettime.html
// and to http://man7.org/linux/man-pages/man2/clock_gettime.2.html
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SINGLE_THREAD
#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <sys/timeb.h>
#endif
#include <time.h>
#endif

#if defined(__ANDROID__)
#include <linux/time.h>
#endif

#include "caretransmission.h"
#include "caremotehandler.h"
#include "caprotocolmessage.h"
#include "oic_malloc.h"
#include "logger.h"

#define TAG "CA_RETRANS"

typedef struct
{
    uint64_t timeStamp;                 /**< last sent time. microseconds */
#ifndef SINGLE_THREAD
    uint64_t timeout;                   /**< timeout value. microseconds */
#endif
    uint8_t triedCount;                 /**< retransmission count */
    uint16_t messageId;                 /**< coap PDU message id */
    CAEndpoint_t *endpoint;             /**< remote endpoint */
    void *pdu;                          /**< coap PDU */
    uint32_t size;                      /**< coap PDU size */
} CARetransmissionData_t;

static const uint64_t USECS_PER_SEC = 1000000;

/**
 * @brief   getCurrent monotonic time
 * @return  current time in microseconds
 */
uint64_t getCurrentTimeInMicroSeconds();

#ifndef SINGLE_THREAD
/**
 * @brief   timeout value is
 *          between DEFAULT_ACK_TIMEOUT_SEC and
 *          (DEFAULT_ACK_TIMEOUT_SEC * DEFAULT_RANDOM_FACTOR) second.
 *          DEFAULT_RANDOM_FACTOR       1.5 (CoAP)
 * @return  microseconds.
 */
static uint64_t CAGetTimeoutValue()
{
#ifdef WIN32
    return ((DEFAULT_ACK_TIMEOUT_SEC * 1000) + ((1000 * (rand() & 0xFF)) >> 8)) *
        (uint64_t)1000;
#else
    return ((DEFAULT_ACK_TIMEOUT_SEC * 1000) + ((1000 * (random() & 0xFF)) >> 8)) *
            (uint64_t) 1000;
#endif
}

CAResult_t CARetransmissionStart(CARetransmission_t *context)
{
    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "context is empty");
        return CA_STATUS_INVALID_PARAM;
    }

    if (NULL == context->threadPool)
    {
        OIC_LOG(ERROR, TAG, "thread pool handle is empty..");
        return CA_STATUS_INVALID_PARAM;
    }

    CAResult_t res = ca_thread_pool_add_task(context->threadPool, CARetransmissionBaseRoutine,
                                             context);

    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "thread pool add task error(send thread).");
        return res;
    }

    return res;
}
#endif

/**
 * @brief   check timeout routine
 * @param   currentTime     [IN]microseconds
 * @param   retData         [IN]retransmission data
 * @return  true if the timeout period has elapsed, false otherwise
 */
static bool CACheckTimeout(uint64_t currentTime, CARetransmissionData_t *retData)
{
#ifndef SINGLE_THREAD
    // #1. calculate timeout
    uint32_t milliTimeoutValue = retData->timeout * 0.001;
    uint64_t timeout = (milliTimeoutValue << retData->triedCount) * (uint64_t) 1000;

    if (currentTime >= retData->timeStamp + timeout)
    {
        OIC_LOG_V(DEBUG, TAG, "%d microseconds time out!!, tried count(%ld)",
                  timeout, retData->triedCount);
        return true;
    }
#else
    // #1. calculate timeout
    uint64_t timeOut = (2 << retData->triedCount) * 1000000;

    if (currentTime >= retData->timeStamp + timeOut)
    {
        OIC_LOG_V(DEBUG, TAG, "timeout=%d, tried cnt=%d",
                  (2 << retData->triedCount), retData->triedCount);
        return true;
    }
#endif
    return false;
}

static void CACheckRetransmissionList(CARetransmission_t *context)
{
    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "context is null");
        return;
    }

    // mutex lock
    ca_mutex_lock(context->threadMutex);

    uint32_t i = 0;
    uint32_t len = u_arraylist_length(context->dataList);

    for (i = 0; i < len; i++)
    {
        CARetransmissionData_t *retData = u_arraylist_get(context->dataList, i);

        if (NULL == retData)
        {
            continue;
        }

        uint64_t currentTime = getCurrentTimeInMicroSeconds();

        if (CACheckTimeout(currentTime, retData))
        {
            // #2. if time's up, send the data.
            if (NULL != context->dataSendMethod)
            {
                OIC_LOG_V(DEBUG, TAG, "retransmission CON data!!, msgid=%d",
                          retData->messageId);
                context->dataSendMethod(retData->endpoint, retData->pdu, retData->size);
            }

            // #3. increase the retransmission count and update timestamp.
            retData->timeStamp = currentTime;
            retData->triedCount++;
        }

        // #4. if tried count is max, remove the retransmission data from list.
        if (retData->triedCount >= context->config.tryingCount)
        {
            CARetransmissionData_t *removedData = u_arraylist_remove(context->dataList, i);
            if (NULL == removedData)
            {
                OIC_LOG(ERROR, TAG, "Removed data is NULL");
                // mutex unlock
                ca_mutex_unlock(context->threadMutex);
                return;
            }
            OIC_LOG_V(DEBUG, TAG, "max trying count, remove RTCON data,"
                      "msgid=%d", removedData->messageId);

            // callback for retransmit timeout
            if (NULL != context->timeoutCallback)
            {
                context->timeoutCallback(removedData->endpoint, removedData->pdu,
                                         removedData->size);
            }

            CAFreeEndpoint(removedData->endpoint);
            OICFree(removedData->pdu);

            OICFree(removedData);

            // modify loop value.
            len = u_arraylist_length(context->dataList);
            --i;
        }
    }

    // mutex unlock
    ca_mutex_unlock(context->threadMutex);
}

void CARetransmissionBaseRoutine(void *threadValue)
{
    OIC_LOG(DEBUG, TAG, "retransmission main thread start");

    CARetransmission_t *context = (CARetransmission_t *) threadValue;

    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "thread data passing error");

        return;
    }

#ifdef SINGLE_THREAD
    if (true == context->isStop)
    {
        OIC_LOG(DEBUG, TAG, "thread stopped");
        return;
    }
    CACheckRetransmissionList(context);
#else

    while (!context->isStop)
    {
        // mutex lock
        ca_mutex_lock(context->threadMutex);

        if (!context->isStop && u_arraylist_length(context->dataList) <= 0)
        {
            // if list is empty, thread will wait
            OIC_LOG(DEBUG, TAG, "wait..there is no retransmission data.");

            // wait
            ca_cond_wait(context->threadCond, context->threadMutex);

            OIC_LOG(DEBUG, TAG, "wake up..");
        }
        else if (!context->isStop)
        {
            // check each RETRANSMISSION_CHECK_PERIOD_SEC time.
            OIC_LOG_V(DEBUG, TAG, "wait..(%ld)microseconds",
                      RETRANSMISSION_CHECK_PERIOD_SEC * (uint64_t) USECS_PER_SEC);

            // wait
            uint64_t absTime = RETRANSMISSION_CHECK_PERIOD_SEC * (uint64_t) USECS_PER_SEC;
            ca_cond_wait_for(context->threadCond, context->threadMutex, absTime );
        }
        else
        {
            // we are stopping, so we want to unlock and finish stopping
        }

        // mutex unlock
        ca_mutex_unlock(context->threadMutex);

        // check stop flag
        if (context->isStop)
        {
            continue;
        }

        CACheckRetransmissionList(context);
    }

    ca_mutex_lock(context->threadMutex);
    ca_cond_signal(context->threadCond);
    ca_mutex_unlock(context->threadMutex);

#endif
    OIC_LOG(DEBUG, TAG, "retransmission main thread end");

}

CAResult_t CARetransmissionInitialize(CARetransmission_t *context,
                                      ca_thread_pool_t handle,
                                      CADataSendMethod_t retransmissionSendMethod,
                                      CATimeoutCallback_t timeoutCallback,
                                      CARetransmissionConfig_t* config)
{
    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "thread instance is empty");
        return CA_STATUS_INVALID_PARAM;
    }
#ifndef SINGLE_THREAD
    if (NULL == handle)
    {
        OIC_LOG(ERROR, TAG, "thread pool handle is empty");
        return CA_STATUS_INVALID_PARAM;
    }
#endif
    OIC_LOG(DEBUG, TAG, "thread initialize");

    memset(context, 0, sizeof(CARetransmission_t));

    CARetransmissionConfig_t cfg = { .supportType = DEFAULT_RETRANSMISSION_TYPE,
                                     .tryingCount = DEFAULT_RETRANSMISSION_COUNT };

    if (config)
    {
        cfg = *config;
    }

    // set send thread data
    context->threadPool = handle;
    context->threadMutex = ca_mutex_new();
    context->threadCond = ca_cond_new();
    context->dataSendMethod = retransmissionSendMethod;
    context->timeoutCallback = timeoutCallback;
    context->config = cfg;
    context->isStop = false;
    context->dataList = u_arraylist_create();

    return CA_STATUS_OK;
}

CAResult_t CARetransmissionSentData(CARetransmission_t *context,
                                    const CAEndpoint_t *endpoint,
                                    const void *pdu, uint32_t size)
{
    if (NULL == context || NULL == endpoint || NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "invalid parameter");
        return CA_STATUS_INVALID_PARAM;
    }

    // #0. check support transport type
    if (!(context->config.supportType & endpoint->adapter))
    {
        OIC_LOG_V(DEBUG, TAG, "not supported transport type=%d", endpoint->adapter);
        return CA_NOT_SUPPORTED;
    }

    // #1. check PDU method type and get message id.
    CAMessageType_t type = CAGetMessageTypeFromPduBinaryData(pdu, size);
    uint16_t messageId = CAGetMessageIdFromPduBinaryData(pdu, size);

    OIC_LOG_V(DEBUG, TAG, "sent pdu, msgtype=%d, msgid=%d", type, messageId);

    if (CA_MSG_CONFIRM != type)
    {
        OIC_LOG(DEBUG, TAG, "not supported message type");
        return CA_NOT_SUPPORTED;
    }

    // create retransmission data
    CARetransmissionData_t *retData = (CARetransmissionData_t *) OICCalloc(
                                          1, sizeof(CARetransmissionData_t));

    if (NULL == retData)
    {
        OIC_LOG(ERROR, TAG, "memory error");
        return CA_MEMORY_ALLOC_FAILED;
    }

    // copy PDU data
    void *pduData = (void *) OICMalloc(size);
    if (NULL == pduData)
    {
        OICFree(retData);
        OIC_LOG(ERROR, TAG, "memory error");
        return CA_MEMORY_ALLOC_FAILED;
    }
    memcpy(pduData, pdu, size);

    // clone remote endpoint
    CAEndpoint_t *remoteEndpoint = CACloneEndpoint(endpoint);
    if (NULL == remoteEndpoint)
    {
        OICFree(retData);
        OICFree(pduData);
        OIC_LOG(ERROR, TAG, "memory error");
        return CA_MEMORY_ALLOC_FAILED;
    }

    // #2. add additional information. (time stamp, retransmission count...)
    retData->timeStamp = getCurrentTimeInMicroSeconds();
#ifndef SINGLE_THREAD
    retData->timeout = CAGetTimeoutValue();
#endif
    retData->triedCount = 0;
    retData->messageId = messageId;
    retData->endpoint = remoteEndpoint;
    retData->pdu = pduData;
    retData->size = size;
#ifndef SINGLE_THREAD
    // mutex lock
    ca_mutex_lock(context->threadMutex);

    uint32_t i = 0;
    uint32_t len = u_arraylist_length(context->dataList);

    // #3. add data into list
    for (i = 0; i < len; i++)
    {
        CARetransmissionData_t *currData = u_arraylist_get(context->dataList, i);

        if (NULL == currData)
        {
            continue;
        }

        // found index
        if (NULL != currData->endpoint && currData->messageId == messageId
            && (currData->endpoint->adapter == endpoint->adapter))
        {
            OIC_LOG(ERROR, TAG, "Duplicate message ID");

            // mutex unlock
            ca_mutex_unlock(context->threadMutex);

            OICFree(retData);
            OICFree(pduData);
            OICFree(remoteEndpoint);
            return CA_STATUS_FAILED;
        }
    }

    u_arraylist_add(context->dataList, (void *) retData);

    // notify the thread
    ca_cond_signal(context->threadCond);

    // mutex unlock
    ca_mutex_unlock(context->threadMutex);

#else
    u_arraylist_add(context->dataList, (void *) retData);

    CACheckRetransmissionList(context);
#endif
    return CA_STATUS_OK;
}

CAResult_t CARetransmissionReceivedData(CARetransmission_t *context,
                                        const CAEndpoint_t *endpoint, const void *pdu,
                                        uint32_t size, void **retransmissionPdu)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (NULL == context || NULL == endpoint || NULL == pdu || NULL == retransmissionPdu)
    {
        OIC_LOG(ERROR, TAG, "invalid parameter");
        return CA_STATUS_INVALID_PARAM;
    }

    // #0. check support transport type
    if (!(context->config.supportType & endpoint->adapter))
    {
        OIC_LOG_V(DEBUG, TAG, "not supported transport type=%d", endpoint->adapter);
        return CA_STATUS_OK;
    }

    // #1. check PDU method type and get message id.
    // ACK, RST --> remove the CON data
    CAMessageType_t type = CAGetMessageTypeFromPduBinaryData(pdu, size);
    uint16_t messageId = CAGetMessageIdFromPduBinaryData(pdu, size);

    OIC_LOG_V(DEBUG, TAG, "received pdu, msgtype=%d, msgid=%d", type, messageId);

    if ((CA_MSG_ACKNOWLEDGE != type) && (CA_MSG_RESET != type))
    {
        return CA_STATUS_OK;
    }

    // mutex lock
    ca_mutex_lock(context->threadMutex);
    uint32_t len = u_arraylist_length(context->dataList);

    // find index
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        CARetransmissionData_t *retData = (CARetransmissionData_t *) u_arraylist_get(
                context->dataList, i);

        if (NULL == retData)
        {
            continue;
        }

        // found index
        if (NULL != retData->endpoint && retData->messageId == messageId
            && (retData->endpoint->adapter == endpoint->adapter))
        {
            // get pdu data for getting token when CA_EMPTY(RST/ACK) is received from remote device
            // if retransmission was finish..token will be unavailable.
            if (CA_EMPTY == CAGetCodeFromPduBinaryData(pdu, size))
            {
                OIC_LOG(DEBUG, TAG, "code is CA_EMPTY");

                if (NULL == retData->pdu)
                {
                    OIC_LOG(ERROR, TAG, "retData->pdu is null");
                    OICFree(retData);
                    // mutex unlock
                    ca_mutex_unlock(context->threadMutex);

                    return CA_STATUS_FAILED;
                }

                // copy PDU data
                (*retransmissionPdu) = (void *) OICCalloc(1, retData->size);
                if ((*retransmissionPdu) == NULL)
                {
                    OICFree(retData);
                    OIC_LOG(ERROR, TAG, "memory error");

                    // mutex unlock
                    ca_mutex_unlock(context->threadMutex);

                    return CA_MEMORY_ALLOC_FAILED;
                }
                memcpy((*retransmissionPdu), retData->pdu, retData->size);
            }

            // #2. remove data from list
            CARetransmissionData_t *removedData = u_arraylist_remove(context->dataList, i);
            if (NULL == removedData)
            {
                OIC_LOG(ERROR, TAG, "Removed data is NULL");

                // mutex unlock
                ca_mutex_unlock(context->threadMutex);

                return CA_STATUS_FAILED;
            }

            OIC_LOG_V(DEBUG, TAG, "remove RTCON data!!, msgid=%d", messageId);

            CAFreeEndpoint(removedData->endpoint);
            OICFree(removedData->pdu);
            OICFree(removedData);

            break;
        }
    }

    // mutex unlock
    ca_mutex_unlock(context->threadMutex);

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CARetransmissionStop(CARetransmission_t *context)
{
    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "context is empty..");
        return CA_STATUS_INVALID_PARAM;
    }

    OIC_LOG(DEBUG, TAG, "retransmission stop request!!");

    // mutex lock
    ca_mutex_lock(context->threadMutex);

    // set stop flag
    context->isStop = true;

    // notify the thread
    ca_cond_signal(context->threadCond);

    ca_cond_wait(context->threadCond, context->threadMutex);

    // mutex unlock
    ca_mutex_unlock(context->threadMutex);

    return CA_STATUS_OK;
}

CAResult_t CARetransmissionDestroy(CARetransmission_t *context)
{
    if (NULL == context)
    {
        OIC_LOG(ERROR, TAG, "context is empty..");
        return CA_STATUS_INVALID_PARAM;
    }

    OIC_LOG(DEBUG, TAG, "retransmission context destroy..");

    ca_mutex_free(context->threadMutex);
    context->threadMutex = NULL;
    ca_cond_free(context->threadCond);
    u_arraylist_free(&context->dataList);

    return CA_STATUS_OK;
}

uint64_t getCurrentTimeInMicroSeconds()
{
    OIC_LOG(DEBUG, TAG, "IN");
    uint64_t currentTime = 0;

#ifdef __ANDROID__
    struct timespec getTs;

    clock_gettime(CLOCK_MONOTONIC, &getTs);

    currentTime = (getTs.tv_sec * (uint64_t)1000000000 + getTs.tv_nsec)/1000;
    OIC_LOG_V(DEBUG, TAG, "current time = %ld", currentTime);
#elif defined __ARDUINO__
    currentTime = millis() * 1000;
    OIC_LOG_V(DEBUG, TAG, "currtime=%lu", currentTime);
#else
#if _POSIX_TIMERS > 0
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    currentTime = ts.tv_sec * USECS_PER_SEC + ts.tv_nsec / 1000;
#else
#ifdef WIN32
    struct __timeb64 tb;
    _ftime64_s(&tb);
    currentTime = tb.time * USECS_PER_SEC + tb.millitm * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    currentTime = tv.tv_sec * USECS_PER_SEC + tv.tv_usec;
#endif
#endif
#endif

    OIC_LOG(DEBUG, TAG, "OUT");
    return currentTime;
}
