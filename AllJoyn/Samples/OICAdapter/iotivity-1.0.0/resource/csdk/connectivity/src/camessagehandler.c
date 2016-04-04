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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cainterface.h"
#include "camessagehandler.h"
#include "caremotehandler.h"
#include "caprotocolmessage.h"
#include "logger.h"
#include "config.h" /* for coap protocol */
#include "oic_malloc.h"
#include "canetworkconfigurator.h"
#include "caadapterutils.h"
#include "cainterfacecontroller.h"
#include "caretransmission.h"

#ifdef WITH_BWT
#include "cablockwisetransfer.h"
#endif

#ifndef  SINGLE_THREAD
#include "uqueue.h"
#include "cathreadpool.h" /* for thread pool */
#include "caqueueingthread.h"

#define SINGLE_HANDLE
#define MAX_THREAD_POOL_SIZE    20

// thread pool handle
static ca_thread_pool_t g_threadPoolHandle = NULL;

// message handler main thread
static CAQueueingThread_t g_sendThread;
static CAQueueingThread_t g_receiveThread;

#else
#define CA_MAX_RT_ARRAY_SIZE    3
#endif  /* SINGLE_THREAD */

#define TAG "CA_MSG_HNDLR"

static CARetransmission_t g_retransmissionContext;

// handler field
static CARequestCallback g_requestHandler = NULL;
static CAResponseCallback g_responseHandler = NULL;
static CAErrorCallback g_errorHandler = NULL;

static void CAErrorHandler(const CAEndpoint_t *endpoint,
                           const void *data, uint32_t dataLen,
                           CAResult_t result);

static CAData_t* CAGenerateHandlerData(const CAEndpoint_t *endpoint,
                                       const CARemoteId_t *identity,
                                       const void *data, CADataType_t dataType);

static void CASendErrorInfo(const CAEndpoint_t *endpoint, const CAInfo_t *info,
                            CAResult_t result);

#ifdef SINGLE_THREAD
static void CAProcessReceivedData(CAData_t *data);
#endif
static void CADestroyData(void *data, uint32_t size);
static void CALogPayloadInfo(CAInfo_t *info);
static bool CADropSecondMessage(CAHistory_t *history, const CAEndpoint_t *endpoint, uint16_t id,
                                CAToken_t token, uint8_t tokenLength);

#ifdef WITH_BWT
void CAAddDataToSendThread(CAData_t *data)
{
    VERIFY_NON_NULL_VOID(data, TAG, "data");

    // add thread
    CAQueueingThreadAddData(&g_sendThread, data, sizeof(CAData_t));
}

void CAAddDataToReceiveThread(CAData_t *data)
{
    VERIFY_NON_NULL_VOID(data, TAG, "data");

    // add thread
    CAQueueingThreadAddData(&g_receiveThread, data, sizeof(CAData_t));
}
#endif

static bool CAIsSelectedNetworkAvailable()
{
    u_arraylist_t *list = CAGetSelectedNetworkList();
    if (!list || u_arraylist_length(list) == 0)
    {
        OIC_LOG(ERROR, TAG, "No selected network");
        return false;
    }

    return true;
}

static CAData_t* CAGenerateHandlerData(const CAEndpoint_t *endpoint,
                                       const CARemoteId_t *identity,
                                       const void *data, CADataType_t dataType)
{
    OIC_LOG(DEBUG, TAG, "CAGenerateHandlerData IN");
    CAInfo_t *info = NULL;
    CAData_t *cadata = (CAData_t *) OICCalloc(1, sizeof(CAData_t));
    if (!cadata)
    {
        OIC_LOG(ERROR, TAG, "memory allocation failed");
        return NULL;
    }

    CAEndpoint_t* ep = CACloneEndpoint(endpoint);
    if (!ep)
    {
        OIC_LOG(ERROR, TAG, "endpoint clone failed");
        OICFree(cadata);
        return NULL;
    }

    OIC_LOG_V(DEBUG, TAG, "address : %s", ep->addr);
    CAResult_t result;

    if(CA_RESPONSE_DATA == dataType)
    {
        CAResponseInfo_t* resInfo = (CAResponseInfo_t*)OICCalloc(1, sizeof(CAResponseInfo_t));
        if (!resInfo)
        {
            OIC_LOG(ERROR, TAG, "memory allocation failed");
            OICFree(cadata);
            CAFreeEndpoint(ep);
            return NULL;
        }

        result = CAGetResponseInfoFromPDU(data, resInfo, endpoint);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG(ERROR, TAG, "CAGetResponseInfoFromPDU Failed");
            CAFreeEndpoint(ep);
            CADestroyResponseInfoInternal(resInfo);
            OICFree(cadata);
            return NULL;
        }
        cadata->responseInfo = resInfo;
        info = &resInfo->info;
        if (identity)
        {
            info->identity = *identity;
        }
        OIC_LOG(DEBUG, TAG, "Response Info :");
        CALogPayloadInfo(info);
    }
    else if (CA_REQUEST_DATA == dataType)
    {
        CARequestInfo_t* reqInfo = (CARequestInfo_t*)OICCalloc(1, sizeof(CARequestInfo_t));
        if (!reqInfo)
        {
            OIC_LOG(ERROR, TAG, "memory allocation failed");
            OICFree(cadata);
            CAFreeEndpoint(ep);
            return NULL;
        }

        result = CAGetRequestInfoFromPDU(data, endpoint, reqInfo);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG(ERROR, TAG, "CAGetRequestInfoFromPDU failed");
            CAFreeEndpoint(ep);
            CADestroyRequestInfoInternal(reqInfo);
            OICFree(cadata);
            return NULL;
        }

        if (CADropSecondMessage(&caglobals.ca.requestHistory, endpoint, reqInfo->info.messageId,
                                reqInfo->info.token, reqInfo->info.tokenLength))
        {
            OIC_LOG(ERROR, TAG, "Second Request with same Token, Drop it");
            CAFreeEndpoint(ep);
            CADestroyRequestInfoInternal(reqInfo);
            OICFree(cadata);
            return NULL;
        }

        cadata->requestInfo = reqInfo;
        info = &reqInfo->info;
        if (identity)
        {
            info->identity = *identity;
        }
        OIC_LOG(DEBUG, TAG, "Request Info :");
        CALogPayloadInfo(info);
   }
    else if (CA_ERROR_DATA == dataType)
    {
        CAErrorInfo_t *errorInfo = (CAErrorInfo_t *)OICCalloc(1, sizeof (CAErrorInfo_t));
        if (!errorInfo)
        {
            OIC_LOG(ERROR, TAG, "Memory allocation failed!");
            OICFree(cadata);
            CAFreeEndpoint(ep);
            return NULL;
        }

        CAResult_t result = CAGetErrorInfoFromPDU(data, endpoint, errorInfo);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG(ERROR, TAG, "CAGetErrorInfoFromPDU failed");
            CAFreeEndpoint(ep);
            OICFree(errorInfo);
            OICFree(cadata);
            return NULL;
        }

        cadata->errorInfo = errorInfo;
        info = &errorInfo->info;
        if (identity)
        {
            info->identity = *identity;
        }
        OIC_LOG(DEBUG, TAG, "error Info :");
        CALogPayloadInfo(info);
    }

    cadata->remoteEndpoint = ep;
    cadata->dataType = dataType;

    return cadata;

    OIC_LOG(DEBUG, TAG, "CAGenerateHandlerData OUT");
}

static void CATimeoutCallback(const CAEndpoint_t *endpoint, const void *pdu, uint32_t size)
{
    VERIFY_NON_NULL_VOID(endpoint, TAG, "endpoint");
    VERIFY_NON_NULL_VOID(pdu, TAG, "pdu");

    CAEndpoint_t* ep = CACloneEndpoint(endpoint);
    if (!ep)
    {
        OIC_LOG(ERROR, TAG, "clone failed");
        return;
    }

    CAResponseInfo_t* resInfo = (CAResponseInfo_t*)OICCalloc(1, sizeof(CAResponseInfo_t));

    if (!resInfo)
    {
        OIC_LOG(ERROR, TAG, "calloc failed");
        CAFreeEndpoint(ep);
        return;
    }

    resInfo->result = CA_RETRANSMIT_TIMEOUT;
    resInfo->info.type = CAGetMessageTypeFromPduBinaryData(pdu, size);
    resInfo->info.messageId = CAGetMessageIdFromPduBinaryData(pdu, size);

    CAResult_t res = CAGetTokenFromPDU((const coap_hdr_t *) pdu, &(resInfo->info),
                                       endpoint);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "fail to get Token from retransmission list");
        CADestroyResponseInfoInternal(resInfo);
        CAFreeEndpoint(ep);
        return;
    }

    CAData_t *cadata = (CAData_t *) OICCalloc(1, sizeof(CAData_t));
    if (NULL == cadata)
    {
        OIC_LOG(ERROR, TAG, "memory allocation failed !");
        CAFreeEndpoint(ep);
        CADestroyResponseInfoInternal(resInfo);
        return;
    }

    cadata->type = SEND_TYPE_UNICAST;
    cadata->remoteEndpoint = ep;
    cadata->requestInfo = NULL;
    cadata->responseInfo = resInfo;

#ifdef SINGLE_THREAD
    CAProcessReceivedData(cadata);
#else
    CAQueueingThreadAddData(&g_receiveThread, cadata, sizeof(CAData_t));
#endif
}

static void CADestroyData(void *data, uint32_t size)
{
    OIC_LOG(DEBUG, TAG, "CADestroyData IN");
    if ((size_t)size < sizeof(CAData_t))
    {
        OIC_LOG_V(ERROR, TAG, "Destroy data too small %p %d", data, size);
    }
    CAData_t *cadata = (CAData_t *) data;

    if (NULL == cadata)
    {
        OIC_LOG(ERROR, TAG, "cadata is NULL");
        return;
    }

    if (NULL != cadata->remoteEndpoint)
    {
        CAFreeEndpoint(cadata->remoteEndpoint);
    }

    if (NULL != cadata->requestInfo)
    {
        CADestroyRequestInfoInternal((CARequestInfo_t *) cadata->requestInfo);
    }

    if (NULL != cadata->responseInfo)
    {
        CADestroyResponseInfoInternal((CAResponseInfo_t *) cadata->responseInfo);
    }

    if (NULL != cadata->errorInfo)
    {
        CADestroyErrorInfoInternal(cadata->errorInfo);
    }

    OICFree(cadata);
    OIC_LOG(DEBUG, TAG, "CADestroyData OUT");
}

#ifdef SINGLE_THREAD
static void CAProcessReceivedData(CAData_t *data)
{
    OIC_LOG(DEBUG, TAG, "CAProcessReceivedData IN");
    if (!data)
    {
        OIC_LOG(ERROR, TAG, "thread data error!!");
        return;
    }

    // parse the data and call the callbacks.
    // #1 parse the data
    // #2 get endpoint
    CAEndpoint_t *rep = (CAEndpoint_t *)(data->remoteEndpoint);
    if (!rep)
    {
        OIC_LOG(ERROR, TAG, "remoteEndpoint error!!");
        return;
    }

    if (data->requestInfo && g_requestHandler)
    {
        g_requestHandler(rep, data->requestInfo);
    }
    else if (data->responseInfo && g_responseHandler)
    {
        g_responseHandler(rep, data->responseInfo);
    }
    else if (data->errorInfo && g_errorHandler)
    {
        g_errorHandler(rep, data->errorInfo);
    }

#ifdef SINGLE_THREAD
    CADestroyData(data, sizeof(CAData_t));
#endif

    OIC_LOG(DEBUG, TAG, "CAProcessReceivedData OUT");
}
#endif

#ifndef SINGLE_THREAD

static void CAReceiveThreadProcess(void *threadData)
{
#ifndef SINGLE_HANDLE
    CAData_t *data = (CAData_t *) threadData;
    CAProcessReceivedData(data);
#else
    (void)threadData;
#endif
}
#endif

static CAResult_t CAProcessSendData(const CAData_t *data)
{
    VERIFY_NON_NULL(data, TAG, "data");
    VERIFY_NON_NULL(data->remoteEndpoint, TAG, "remoteEndpoint");

    CAResult_t res = CA_STATUS_FAILED;

    CASendDataType_t type = data->type;

    coap_pdu_t *pdu = NULL;
    CAInfo_t *info = NULL;
    coap_list_t *options = NULL;
    coap_transport_type transport;

    if (SEND_TYPE_UNICAST == type)
    {
        OIC_LOG(DEBUG,TAG,"Unicast message");
#ifdef ROUTING_GATEWAY
        /*
         * When forwarding a packet, do not attempt retransmission as its the responsibility of
         * packet originator node
         */
        bool skipRetransmission = false;
#endif

        if (NULL != data->requestInfo)
        {
            OIC_LOG(DEBUG, TAG, "requestInfo is available..");

            info = &data->requestInfo->info;
#ifdef ROUTING_GATEWAY
            skipRetransmission = data->requestInfo->info.skipRetransmission;
#endif
            pdu = CAGeneratePDU(data->requestInfo->method, info, data->remoteEndpoint,
                                &options, &transport);
        }
        else if (NULL != data->responseInfo)
        {
            OIC_LOG(DEBUG, TAG, "responseInfo is available..");

            info = &data->responseInfo->info;
#ifdef ROUTING_GATEWAY
            skipRetransmission = data->responseInfo->info.skipRetransmission;
#endif
            pdu = CAGeneratePDU(data->responseInfo->result, info, data->remoteEndpoint,
                                &options, &transport);
        }
        else
        {
            OIC_LOG(DEBUG, TAG, "request info, response info is empty");
            return CA_STATUS_INVALID_PARAM;
        }

        // interface controller function call.
        if (NULL != pdu)
        {
#ifdef WITH_BWT
            if (CA_ADAPTER_GATT_BTLE != data->remoteEndpoint->adapter
#ifdef TCP_ADAPTER
                    && CA_ADAPTER_TCP != data->remoteEndpoint->adapter
#endif
                    )
            {
                // Blockwise transfer
                if (NULL != info)
                {
                    CAResult_t res = CAAddBlockOption(&pdu, info,
                                                      data->remoteEndpoint,
                                                      &options);
                    if (CA_STATUS_OK != res)
                    {
                        OIC_LOG(INFO, TAG, "to write block option has failed");
                        CAErrorHandler(data->remoteEndpoint, pdu->hdr, pdu->length, res);
                        coap_delete_list(options);
                        coap_delete_pdu(pdu);
                        return res;
                    }
                }
            }
#endif
            CALogPDUInfo(pdu, data->remoteEndpoint);

            res = CASendUnicastData(data->remoteEndpoint, pdu->hdr, pdu->length);
            if (CA_STATUS_OK != res)
            {
                OIC_LOG_V(ERROR, TAG, "send failed:%d", res);
                CAErrorHandler(data->remoteEndpoint, pdu->hdr, pdu->length, res);
                coap_delete_list(options);
                coap_delete_pdu(pdu);
                return res;
            }

#ifdef TCP_ADAPTER
            if (CA_ADAPTER_TCP == data->remoteEndpoint->adapter)
            {
                OIC_LOG(INFO, TAG, "retransmission will be not worked");
            }
            else
#endif
#ifdef ROUTING_GATEWAY
            if(!skipRetransmission)
#endif
            {
                // for retransmission
                res = CARetransmissionSentData(&g_retransmissionContext, data->remoteEndpoint,
                                               pdu->hdr, pdu->length);
                if ((CA_STATUS_OK != res) && (CA_NOT_SUPPORTED != res))
                {
                    //when retransmission not supported this will return CA_NOT_SUPPORTED, ignore
                    OIC_LOG_V(INFO, TAG, "retransmission is not enabled due to error, res : %d", res);
                    coap_delete_list(options);
                    coap_delete_pdu(pdu);
                    return res;
                }
            }

            coap_delete_list(options);
            coap_delete_pdu(pdu);
        }
        else
        {
            OIC_LOG(ERROR,TAG,"Failed to generate unicast PDU");
            CASendErrorInfo(data->remoteEndpoint, info, CA_SEND_FAILED);
            return CA_SEND_FAILED;
        }
    }
    else if (SEND_TYPE_MULTICAST == type)
    {
        OIC_LOG(DEBUG,TAG,"Multicast message");
        if (NULL != data->requestInfo)
        {
            OIC_LOG(DEBUG, TAG, "requestInfo is available..");

            info = &data->requestInfo->info;
            pdu = CAGeneratePDU(CA_GET, info, data->remoteEndpoint, &options, &transport);

            if (NULL != pdu)
            {
#ifdef WITH_BWT
                if (CA_ADAPTER_GATT_BTLE != data->remoteEndpoint->adapter
#ifdef TCP_ADAPTER
                        && CA_ADAPTER_TCP != data->remoteEndpoint->adapter
#endif
                        )
                {
                    // Blockwise transfer
                    CAResult_t res = CAAddBlockOption(&pdu, &data->requestInfo->info,
                                                      data->remoteEndpoint,
                                                      &options);
                    if (CA_STATUS_OK != res)
                    {
                        OIC_LOG(DEBUG, TAG, "CAAddBlockOption has failed");
                        CAErrorHandler(data->remoteEndpoint, pdu->hdr, pdu->length, res);
                        coap_delete_list(options);
                        coap_delete_pdu(pdu);
                        return res;
                    }
                }
#endif
            }
            else
            {
                OIC_LOG(ERROR,TAG,"Failed to generate multicast PDU");
                CASendErrorInfo(data->remoteEndpoint, info, CA_SEND_FAILED);
                return CA_SEND_FAILED;
            }
        }
        else if (NULL != data->responseInfo)
        {
            OIC_LOG(DEBUG, TAG, "responseInfo is available..");

            info = &data->responseInfo->info;
            pdu = CAGeneratePDU(data->responseInfo->result, info, data->remoteEndpoint,
                                &options, &transport);

            if (NULL != pdu)
            {
#ifdef WITH_BWT
                if (CA_ADAPTER_GATT_BTLE != data->remoteEndpoint->adapter
#ifdef TCP_ADAPTER
                        && CA_ADAPTER_TCP != data->remoteEndpoint->adapter
#endif
                        )
                {
                    // Blockwise transfer
                    if (NULL != info)
                    {
                        CAResult_t res = CAAddBlockOption(&pdu, info,
                                                          data->remoteEndpoint,
                                                          &options);
                        if (CA_STATUS_OK != res)
                        {
                            OIC_LOG(INFO, TAG, "to write block option has failed");
                            CAErrorHandler(data->remoteEndpoint, pdu->hdr, pdu->length, res);
                            coap_delete_list(options);
                            coap_delete_pdu(pdu);
                            return res;
                        }
                    }
                }
#endif
            }
            else
            {
                OIC_LOG(ERROR,TAG,"Failed to generate multicast PDU");
                CASendErrorInfo(data->remoteEndpoint, info, CA_SEND_FAILED);
                return CA_SEND_FAILED;
            }
        }
        else
        {
            OIC_LOG(ERROR, TAG, "request or response info is empty");
            return CA_SEND_FAILED;
        }

        CALogPDUInfo(pdu, data->remoteEndpoint);

        OIC_LOG(DEBUG, TAG, "pdu to send :");
        OIC_LOG_BUFFER(DEBUG, TAG,  pdu->hdr, pdu->length);

        res = CASendMulticastData(data->remoteEndpoint, pdu->hdr, pdu->length);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG_V(ERROR, TAG, "send failed:%d", res);
            CAErrorHandler(data->remoteEndpoint, pdu->hdr, pdu->length, res);
            coap_delete_list(options);
            coap_delete_pdu(pdu);
            return res;
        }

        coap_delete_list(options);
        coap_delete_pdu(pdu);
    }

    return CA_STATUS_OK;
}

#ifndef SINGLE_THREAD
static void CASendThreadProcess(void *threadData)
{
    CAData_t *data = (CAData_t *) threadData;
    CAProcessSendData(data);
}

#endif

/*
 * If a second message arrives with the same message ID, token and the other address
 * family, drop it.  Typically, IPv6 beats IPv4, so the IPv4 message is dropped.
 */
static bool CADropSecondMessage(CAHistory_t *history, const CAEndpoint_t *ep, uint16_t id,
                                CAToken_t token, uint8_t tokenLength)
{
    if (!ep)
    {
        return true;
    }
    if (ep->adapter != CA_ADAPTER_IP)
    {
        return false;
    }
    if (!caglobals.ip.dualstack)
    {
        return false;
    }

    if (tokenLength > CA_MAX_TOKEN_LEN)
    {
        /*
         * If token length is more than CA_MAX_TOKEN_LEN,
         * we compare the first CA_MAX_TOKEN_LEN bytes only.
         */
        tokenLength = CA_MAX_TOKEN_LEN;
    }

    bool ret = false;
    CATransportFlags_t familyFlags = ep->flags & CA_IPFAMILY_MASK;

    for (size_t i = 0; i < sizeof(history->items) / sizeof(history->items[0]); i++)
    {
        CAHistoryItem_t *item = &(history->items[i]);
        if (id == item->messageId && tokenLength == item->tokenLength
            && memcmp(item->token, token, tokenLength) == 0)
        {
            if ((familyFlags ^ item->flags) == CA_IPFAMILY_MASK)
            {
                OIC_LOG_V(INFO, TAG, "IPv%c duplicate message ignored",
                                            familyFlags & CA_IPV6 ? '6' : '4');
                ret = true;
                break;
            }
        }
    }

    history->items[history->nextIndex].flags = familyFlags;
    history->items[history->nextIndex].messageId = id;
    if (token && tokenLength)
    {
        memcpy(history->items[history->nextIndex].token, token, tokenLength);
        history->items[history->nextIndex].tokenLength = tokenLength;
    }

    if (++history->nextIndex >= HISTORYSIZE)
    {
        history->nextIndex = 0;
    }

    return ret;
}

static void CAReceivedPacketCallback(const CASecureEndpoint_t *sep,
                                     const void *data, uint32_t dataLen)
{
    VERIFY_NON_NULL_VOID(sep, TAG, "remoteEndpoint");
    VERIFY_NON_NULL_VOID(data, TAG, "data");

    OIC_LOG(DEBUG, TAG, "received pdu data :");
    OIC_LOG_BUFFER(DEBUG, TAG,  data, dataLen);

    uint32_t code = CA_NOT_FOUND;
    CAData_t *cadata = NULL;

    coap_pdu_t *pdu = (coap_pdu_t *) CAParsePDU((const char *) data, dataLen, &code,
                                                &(sep->endpoint));
    if (NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "Parse PDU failed");
        return;
    }

    OIC_LOG_V(DEBUG, TAG, "code = %d", code);
    if (CA_GET == code || CA_POST == code || CA_PUT == code || CA_DELETE == code)
    {
        cadata = CAGenerateHandlerData(&(sep->endpoint), &(sep->identity), pdu, CA_REQUEST_DATA);
        if (!cadata)
        {
            OIC_LOG(ERROR, TAG, "CAReceivedPacketCallback, CAGenerateHandlerData failed!");
            coap_delete_pdu(pdu);
            return;
        }
    }
    else
    {
        cadata = CAGenerateHandlerData(&(sep->endpoint), &(sep->identity), pdu, CA_RESPONSE_DATA);
        if (!cadata)
        {
            OIC_LOG(ERROR, TAG, "CAReceivedPacketCallback, CAGenerateHandlerData failed!");
            coap_delete_pdu(pdu);
            return;
        }

#ifdef TCP_ADAPTER
        if (CA_ADAPTER_TCP == sep->endpoint.adapter)
        {
            OIC_LOG(INFO, TAG, "retransmission is not supported");
        }
        else
#endif
        {
            // for retransmission
            void *retransmissionPdu = NULL;
            CARetransmissionReceivedData(&g_retransmissionContext, cadata->remoteEndpoint, pdu->hdr,
                                         pdu->length, &retransmissionPdu);

            // get token from saved data in retransmission list
            if (retransmissionPdu && CA_EMPTY == code)
            {
                if (cadata->responseInfo)
                {
                    CAInfo_t *info = &cadata->responseInfo->info;
                    CAResult_t res = CAGetTokenFromPDU((const coap_hdr_t *)retransmissionPdu,
                                                       info, &(sep->endpoint));
                    if (CA_STATUS_OK != res)
                    {
                        OIC_LOG(ERROR, TAG, "fail to get Token from retransmission list");
                        OICFree(info->token);
                        info->tokenLength = 0;
                    }
                }
            }
            OICFree(retransmissionPdu);
        }
    }

    cadata->type = SEND_TYPE_UNICAST;

#ifdef SINGLE_THREAD
    CAProcessReceivedData(cadata);
#else
#ifdef WITH_BWT
    if (CA_ADAPTER_GATT_BTLE != sep->endpoint.adapter
#ifdef TCP_ADAPTER
            && CA_ADAPTER_TCP != sep->endpoint.adapter
#endif
            )
    {
        CAResult_t res = CAReceiveBlockWiseData(pdu, &(sep->endpoint), cadata, dataLen);
        if (CA_NOT_SUPPORTED == res)
        {
            OIC_LOG(ERROR, TAG, "this message does not have block option");
            CAQueueingThreadAddData(&g_receiveThread, cadata, sizeof(CAData_t));
        }
        else
        {
            CADestroyData(cadata, sizeof(CAData_t));
        }
    }
    else
#endif
    {
        CAQueueingThreadAddData(&g_receiveThread, cadata, sizeof(CAData_t));
    }
#endif

    coap_delete_pdu(pdu);
}

static void CANetworkChangedCallback(const CAEndpoint_t *info, CANetworkStatus_t status)
{
    (void)info;
    (void)status;
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAHandleRequestResponseCallbacks()
{
#ifdef SINGLE_THREAD
    CAReadData();
    CARetransmissionBaseRoutine((void *)&g_retransmissionContext);
#else
#ifdef SINGLE_HANDLE
    // parse the data and call the callbacks.
    // #1 parse the data
    // #2 get endpoint

    ca_mutex_lock(g_receiveThread.threadMutex);

    u_queue_message_t *item = u_queue_get_element(g_receiveThread.dataQueue);

    ca_mutex_unlock(g_receiveThread.threadMutex);

    if (NULL == item)
    {
        return;
    }

    // get values
    void *msg = item->msg;

    if (NULL == msg)
    {
        return;
    }

    // get endpoint
    CAData_t *td = (CAData_t *) msg;

    if (td->requestInfo && g_requestHandler)
    {
        OIC_LOG_V(DEBUG, TAG, "request callback : %d", td->requestInfo->info.numOptions);
        g_requestHandler(td->remoteEndpoint, td->requestInfo);
    }
    else if (td->responseInfo && g_responseHandler)
    {
        OIC_LOG_V(DEBUG, TAG, "response callback : %d", td->responseInfo->info.numOptions);
        g_responseHandler(td->remoteEndpoint, td->responseInfo);
    }
    else if (td->errorInfo && g_errorHandler)
    {
        OIC_LOG_V(DEBUG, TAG, "error callback error: %d", td->errorInfo->result);
        g_errorHandler(td->remoteEndpoint, td->errorInfo);
    }

    CADestroyData(msg, sizeof(CAData_t));
    OICFree(item);

#endif /* SINGLE_HANDLE */
#endif
}

static CAData_t* CAPrepareSendData(const CAEndpoint_t *endpoint, const void *sendData,
                                   CADataType_t dataType)
{
    OIC_LOG(DEBUG, TAG, "CAPrepareSendData IN");

    CAData_t *cadata = (CAData_t *) OICCalloc(1, sizeof(CAData_t));
    if (!cadata)
    {
        OIC_LOG(ERROR, TAG, "memory allocation failed");
        return NULL;
    }

    if(CA_REQUEST_DATA == dataType)
    {
        // clone request info
        CARequestInfo_t *request = CACloneRequestInfo((CARequestInfo_t *)sendData);

        if(!request)
        {
            OIC_LOG(ERROR, TAG, "CACloneRequestInfo failed");
            OICFree(cadata);
            return NULL;
        }

        cadata->type = request->isMulticast ? SEND_TYPE_MULTICAST : SEND_TYPE_UNICAST;
        cadata->requestInfo =  request;
    }
    else if(CA_RESPONSE_DATA == dataType)
    {
        // clone response info
        CAResponseInfo_t *response = CACloneResponseInfo((CAResponseInfo_t *)sendData);

        if(!response)
        {
            OIC_LOG(ERROR, TAG, "CACloneResponseInfo failed");
            OICFree(cadata);
            return NULL;
        }

        cadata->type = response->isMulticast ? SEND_TYPE_MULTICAST : SEND_TYPE_UNICAST;
        cadata->responseInfo = response;
    }
    else
    {
        OIC_LOG(ERROR, TAG, "CAPrepareSendData unknown data type");
        OICFree(cadata);
        return NULL;
    }

    CAEndpoint_t* ep = CACloneEndpoint(endpoint);
    if (!ep)
    {
        OIC_LOG(ERROR, TAG, "endpoint clone failed");
        CADestroyData(cadata, sizeof(CAData_t));
        return NULL;
    }

    cadata->remoteEndpoint = ep;
    cadata->dataType = dataType;
    return cadata;
}

CAResult_t CADetachRequestMessage(const CAEndpoint_t *object, const CARequestInfo_t *request)
{
    VERIFY_NON_NULL(object, TAG, "object");
    VERIFY_NON_NULL(request, TAG, "request");

    if (false == CAIsSelectedNetworkAvailable())
    {
        return CA_STATUS_FAILED;
    }

#ifdef ARDUINO
    // If max retransmission queue is reached, then don't handle new request
    if (CA_MAX_RT_ARRAY_SIZE == u_arraylist_length(g_retransmissionContext.dataList))
    {
        OIC_LOG(ERROR, TAG, "max RT queue size reached!");
        return CA_SEND_FAILED;
    }
#endif /* ARDUINO */

    CAData_t *data = CAPrepareSendData(object, request, CA_REQUEST_DATA);
    if(!data)
    {
        OIC_LOG(ERROR, TAG, "CAPrepareSendData failed");
        return CA_MEMORY_ALLOC_FAILED;
    }

#ifdef SINGLE_THREAD
    CAResult_t result = CAProcessSendData(data);
    if(CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TAG, "CAProcessSendData failed");
        return result;
    }

    CADestroyData(data, sizeof(CAData_t));
#else
#ifdef WITH_BWT
    if (CA_ADAPTER_GATT_BTLE != object->adapter
#ifdef TCP_ADAPTER
            && CA_ADAPTER_TCP != object->adapter
#endif
            )
    {
        // send block data
        CAResult_t res = CASendBlockWiseData(data);
        if(CA_NOT_SUPPORTED == res)
        {
            OIC_LOG(DEBUG, TAG, "normal msg will be sent");
            CAQueueingThreadAddData(&g_sendThread, data, sizeof(CAData_t));
            return CA_STATUS_OK;
        }
        else
        {
            CADestroyData(data, sizeof(CAData_t));
        }
        return res;
    }
    else
#endif
    {
        CAQueueingThreadAddData(&g_sendThread, data, sizeof(CAData_t));
    }
#endif

    return CA_STATUS_OK;
}

CAResult_t CADetachResponseMessage(const CAEndpoint_t *object,
                                   const CAResponseInfo_t *response)
{
    VERIFY_NON_NULL(object, TAG, "object");
    VERIFY_NON_NULL(response, TAG, "response");

    if (false == CAIsSelectedNetworkAvailable())
    {
        return CA_STATUS_FAILED;
    }

    CAData_t *data = CAPrepareSendData(object, response, CA_RESPONSE_DATA);
    if(!data)
    {
        OIC_LOG(ERROR, TAG, "CAPrepareSendData failed");
        return CA_MEMORY_ALLOC_FAILED;
    }

#ifdef SINGLE_THREAD
    CAResult_t result = CAProcessSendData(data);
    if(result != CA_STATUS_OK)
    {
        OIC_LOG(ERROR, TAG, "CAProcessSendData failed");
        return result;
    }

    CADestroyData(data, sizeof(CAData_t));
#else
#ifdef WITH_BWT
    if (CA_ADAPTER_GATT_BTLE != object->adapter
#ifdef TCP_ADAPTER
            && CA_ADAPTER_TCP != object->adapter
#endif
            )
    {
        // send block data
        CAResult_t res = CASendBlockWiseData(data);
        if(CA_NOT_SUPPORTED == res)
        {
            OIC_LOG(DEBUG, TAG, "normal msg will be sent");
            CAQueueingThreadAddData(&g_sendThread, data, sizeof(CAData_t));
            return CA_STATUS_OK;
        }
        else
        {
            CADestroyData(data, sizeof(CAData_t));
        }
        return res;
    }
    else
#endif
    {
        CAQueueingThreadAddData(&g_sendThread, data, sizeof(CAData_t));
    }
#endif

    return CA_STATUS_OK;
}

CAResult_t CADetachMessageResourceUri(const CAURI_t resourceUri, const CAToken_t token,
                                      uint8_t tokenLength, const CAHeaderOption_t *options,
                                      uint8_t numOptions)
{
    (void)resourceUri;
    (void)token;
    (void)tokenLength;
    (void)options;
    (void)numOptions;
    return CA_NOT_SUPPORTED;
}

void CASetInterfaceCallbacks(CARequestCallback ReqHandler, CAResponseCallback RespHandler,
                             CAErrorCallback errorHandler)
{
    g_requestHandler = ReqHandler;
    g_responseHandler = RespHandler;
    g_errorHandler = errorHandler;
}

CAResult_t CAInitializeMessageHandler()
{
    CASetPacketReceivedCallback(CAReceivedPacketCallback);

    CASetNetworkChangeCallback(CANetworkChangedCallback);
    CASetErrorHandleCallback(CAErrorHandler);

#ifndef SINGLE_THREAD
    // create thread pool
    CAResult_t res = ca_thread_pool_init(MAX_THREAD_POOL_SIZE, &g_threadPoolHandle);

    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "thread pool initialize error.");
        return res;
    }

    // send thread initialize
    if (CA_STATUS_OK != CAQueueingThreadInitialize(&g_sendThread, g_threadPoolHandle,
                                                   CASendThreadProcess, CADestroyData))
    {
        OIC_LOG(ERROR, TAG, "Failed to Initialize send queue thread");
        return CA_STATUS_FAILED;
    }

    // start send thread
    res = CAQueueingThreadStart(&g_sendThread);

    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "thread start error(send thread).");
        ca_thread_pool_free(g_threadPoolHandle);
        g_threadPoolHandle = NULL;
        return res;
    }

    // receive thread initialize
    if (CA_STATUS_OK != CAQueueingThreadInitialize(&g_receiveThread, g_threadPoolHandle,
                                                   CAReceiveThreadProcess, CADestroyData))
    {
        OIC_LOG(ERROR, TAG, "Failed to Initialize receive queue thread");
        return CA_STATUS_FAILED;
    }

#ifndef SINGLE_HANDLE // This will be enabled when RI supports multi threading
    // start receive thread
    res = CAQueueingThreadStart(&gReceiveThread);

    if (res != CA_STATUS_OK)
    {
        OIC_LOG(ERROR, TAG, "thread start error(receive thread).");
        return res;
    }
#endif /* SINGLE_HANDLE */

    // retransmission initialize
    CARetransmissionInitialize(&g_retransmissionContext, g_threadPoolHandle, CASendUnicastData,
                               CATimeoutCallback, NULL);

#ifdef WITH_BWT
    // block-wise transfer initialize
    CAInitializeBlockWiseTransfer(CAAddDataToSendThread, CAAddDataToReceiveThread);
#endif

    // start retransmission
    res = CARetransmissionStart(&g_retransmissionContext);

    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "thread start error(retransmission thread).");
        return res;
    }

    // initialize interface adapters by controller
    CAInitializeAdapters(g_threadPoolHandle);
#else
    // retransmission initialize
    CARetransmissionInitialize(&g_retransmissionContext, NULL, CASendUnicastData,
                               CATimeoutCallback, NULL);
    CAInitializeAdapters();
#endif

    return CA_STATUS_OK;
}

void CATerminateMessageHandler()
{
#ifndef SINGLE_THREAD
    CATransportAdapter_t connType;
    u_arraylist_t *list = CAGetSelectedNetworkList();
    uint32_t length = u_arraylist_length(list);

    uint32_t i = 0;
    for (i = 0; i < length; i++)
    {
        void* ptrType = u_arraylist_get(list, i);

        if (NULL == ptrType)
        {
            continue;
        }

        connType = *(CATransportAdapter_t *)ptrType;
        CAStopAdapter(connType);
    }

    // stop retransmission
    if (NULL != g_retransmissionContext.threadMutex)
    {
        CARetransmissionStop(&g_retransmissionContext);
    }

    // stop thread
    // delete thread data
    if (NULL != g_sendThread.threadMutex)
    {
        CAQueueingThreadStop(&g_sendThread);
    }

    // stop thread
    // delete thread data
    if (NULL != g_receiveThread.threadMutex)
    {
#ifndef SINGLE_HANDLE // This will be enabled when RI supports multi threading
        CAQueueingThreadStop(&gReceiveThread);
#endif /* SINGLE_HANDLE */
    }

    // destroy thread pool
    if (NULL != g_threadPoolHandle)
    {
        ca_thread_pool_free(g_threadPoolHandle);
        g_threadPoolHandle = NULL;
    }

#ifdef WITH_BWT
    CATerminateBlockWiseTransfer();
#endif
    CARetransmissionDestroy(&g_retransmissionContext);
    CAQueueingThreadDestroy(&g_sendThread);
    CAQueueingThreadDestroy(&g_receiveThread);

    // terminate interface adapters by controller
    CATerminateAdapters();
#else
    // terminate interface adapters by controller
    CATerminateAdapters();

    // stop retransmission
    CARetransmissionStop(&g_retransmissionContext);
    CARetransmissionDestroy(&g_retransmissionContext);
#endif
}

void CALogPDUInfo(coap_pdu_t *pdu, const CAEndpoint_t *endpoint)
{
    VERIFY_NON_NULL_VOID(pdu, TAG, "pdu");

    OIC_LOG_V(DEBUG, TAG, "PDU Maker - payload : %s", pdu->data);

#ifdef TCP_ADAPTER
    if (CA_ADAPTER_TCP == endpoint->adapter)
    {
        OIC_LOG(DEBUG, TAG, "pdu header data :");
        OIC_LOG_BUFFER(DEBUG, TAG,  pdu->hdr, pdu->length);
    }
    else
#endif
    {
        OIC_LOG_V(DEBUG, TAG, "PDU Maker - type : %d", pdu->hdr->coap_hdr_udp_t.type);

        OIC_LOG_V(DEBUG, TAG, "PDU Maker - code : %d", pdu->hdr->coap_hdr_udp_t.code);

        OIC_LOG(DEBUG, TAG, "PDU Maker - token :");

        OIC_LOG_BUFFER(DEBUG, TAG, pdu->hdr->coap_hdr_udp_t.token,
                       pdu->hdr->coap_hdr_udp_t.token_length);
    }
}

static void CALogPayloadInfo(CAInfo_t *info)
{
    if(info)
    {
        if (info->options)
        {
            for (uint32_t i = 0; i < info->numOptions; i++)
            {
                OIC_LOG_V(DEBUG, TAG, "optionID: %d", info->options[i].optionID);

                OIC_LOG_V(DEBUG, TAG, "list: %s", info->options[i].optionData);
            }
        }

        if (info->payload)
        {
            OIC_LOG_V(DEBUG, TAG, "payload: %p(%u)", info->payload,
                      info->payloadSize);
        }

        if (info->token)
        {
            OIC_LOG(DEBUG, TAG, "token:");
            OIC_LOG_BUFFER(DEBUG, TAG, (const uint8_t *) info->token,
                           info->tokenLength);
        }
        OIC_LOG_V(DEBUG, TAG, "msgID: %d", info->messageId);
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "info is NULL, cannot output log data");
    }
}

void CAErrorHandler(const CAEndpoint_t *endpoint,
                    const void *data, uint32_t dataLen,
                    CAResult_t result)
{
    OIC_LOG(DEBUG, TAG, "CAErrorHandler IN");

#ifndef SINGLE_THREAD

    VERIFY_NON_NULL_VOID(endpoint, TAG, "remoteEndpoint");
    VERIFY_NON_NULL_VOID(data, TAG, "data");

    uint32_t code = CA_NOT_FOUND;
    //Do not free remoteEndpoint and data. Currently they will be freed in data thread
    //Get PDU data
    coap_pdu_t *pdu = (coap_pdu_t *)CAParsePDU((const char *)data, dataLen, &code, endpoint);
    if (NULL == pdu)
    {
        OIC_LOG(ERROR, TAG, "Parse PDU failed");
        return;
    }

    CAData_t *cadata = CAGenerateHandlerData(endpoint, NULL, pdu, CA_ERROR_DATA);
    if(!cadata)
    {
        OIC_LOG(ERROR, TAG, "CAErrorHandler, CAGenerateHandlerData failed!");
        coap_delete_pdu(pdu);
        return;
    }

    cadata->errorInfo->result = result;

    CAQueueingThreadAddData(&g_receiveThread, cadata, sizeof(CAData_t));
    coap_delete_pdu(pdu);
#endif

    OIC_LOG(DEBUG, TAG, "CAErrorHandler OUT");
    return;
}

static void CASendErrorInfo(const CAEndpoint_t *endpoint, const CAInfo_t *info, CAResult_t result)
{
    OIC_LOG(DEBUG, TAG, "CASendErrorInfo IN");
#ifndef SINGLE_THREAD
    CAData_t *cadata = (CAData_t *) OICCalloc(1, sizeof(CAData_t));
    if (!cadata)
    {
        OIC_LOG(ERROR, TAG, "memory allocation failed");
        return;
    }

    CAEndpoint_t* ep = CACloneEndpoint(endpoint);
    if (!ep)
    {
        OIC_LOG(ERROR, TAG, "endpoint clone failed");
        OICFree(cadata);
        return;
    }

    CAErrorInfo_t *errorInfo = (CAErrorInfo_t *)OICCalloc(1, sizeof (CAErrorInfo_t));
    if (!errorInfo)
    {
        OICFree(cadata);
        CAFreeEndpoint(ep);
        return;
    }

    CAResult_t res = CACloneInfo(info, &errorInfo->info);
    if (CA_STATUS_OK != res)
    {
        OICFree(cadata);
        OICFree(errorInfo);
        CAFreeEndpoint(ep);
        return;
    }

    errorInfo->result = result;
    cadata->remoteEndpoint = ep;
    cadata->errorInfo = errorInfo;
    cadata->dataType = CA_ERROR_DATA;

    CAQueueingThreadAddData(&g_receiveThread, cadata, sizeof(CAData_t));
#endif
    OIC_LOG(DEBUG, TAG, "CASendErrorInfo OUT");
}
