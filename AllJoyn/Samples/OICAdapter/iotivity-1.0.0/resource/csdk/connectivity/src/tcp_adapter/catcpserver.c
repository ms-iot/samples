/* ****************************************************************j
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <sys/poll.h>

#ifndef WITH_ARDUINO
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "catcpinterface.h"
#include "pdu.h"
#include "caadapterutils.h"
#include "camutex.h"
#include "oic_malloc.h"
#include "oic_string.h"

/**
 * Logging tag for module name.
 */
#define TAG "TCP_SERVER"

/**
 * Server port number for local test.
 */
#define SERVER_PORT 8000

/**
 * Maximum CoAP over TCP header length
 * to know the total data length.
 */
#define TCP_MAX_HEADER_LEN  6

/**
 * Default Thread Counts in TCP adapter
 */
#define CA_TCP_DEFAULT_THREAD_COUNTS    2

/**
 * Accept server file descriptor.
 */
static int g_acceptServerFD = -1;

/**
 * Mutex to synchronize device object list.
 */
static ca_mutex g_mutexObjectList = NULL;

/**
 * Conditional mutex to synchronize.
 */
static ca_cond g_condObjectList = NULL;

/**
 * Maintains the current running thread counts.
 */
static uint32_t g_threadCounts = CA_TCP_DEFAULT_THREAD_COUNTS;

/**
 * Maintains the callback to be notified when data received from remote device.
 */
static CATCPPacketReceivedCallback g_packetReceivedCallback;

/**
 * Error callback to update error in TCP.
 */
static CATCPErrorHandleCallback g_TCPErrorHandler = NULL;

static CAResult_t CATCPCreateMutex();
static void CATCPDestroyMutex();
static CAResult_t CATCPCreateCond();
static void CATCPDestroyCond();
static void CAAcceptHandler(void *data);
static void CAReceiveHandler(void *data);
static CAResult_t CAReceiveMessage();
static int CASetNonblocking(int fd);
static int CATCPCreateSocket(int family, CATCPServerInfo_t *TCPServerInfo);
static size_t CAGetTotalLengthFromHeader(const unsigned char *recvBuffer);
static void CATCPDisconnectAll();

static void CATCPDestroyMutex()
{
    if (g_mutexObjectList)
    {
        ca_mutex_free(g_mutexObjectList);
        g_mutexObjectList = NULL;
    }
}

static CAResult_t CATCPCreateMutex()
{
    if (!g_mutexObjectList)
    {
        g_mutexObjectList = ca_mutex_new();
        if (!g_mutexObjectList)
        {
            OIC_LOG(ERROR, TAG, "Failed to created mutex!");
            return CA_STATUS_FAILED;
        }
    }

    return CA_STATUS_OK;
}

static void CATCPDestroyCond()
{
    if (g_condObjectList)
    {
        ca_cond_free(g_condObjectList);
        g_condObjectList = NULL;
    }
}

static CAResult_t CATCPCreateCond()
{
    if (!g_condObjectList)
    {
        g_condObjectList = ca_cond_new();
        if (!g_condObjectList)
        {
            OIC_LOG(ERROR, TAG, "Failed to created cond!");
            return CA_STATUS_FAILED;
        }
    }
    return CA_STATUS_OK;
}

static void CATCPDisconnectAll()
{
    OIC_LOG(DEBUG, TAG, "IN");

    ca_mutex_lock(g_mutexObjectList);
    uint32_t length = u_arraylist_length(caglobals.tcp.svrlist);

    CATCPServerInfo_t *svritem = NULL;
    for (size_t i = 0; i < length; i++)
    {
        svritem = (CATCPServerInfo_t *) u_arraylist_get(caglobals.tcp.svrlist, i);
        if (svritem && svritem->u4tcp.fd >= 0)
        {
            shutdown(svritem->u4tcp.fd, SHUT_RDWR);
            close(svritem->u4tcp.fd);
        }
    }
    u_arraylist_destroy(caglobals.tcp.svrlist);
    caglobals.tcp.svrlist = NULL;
    ca_mutex_unlock(g_mutexObjectList);

    OIC_LOG(DEBUG, TAG, "OUT");
}

static void CAReceiveHandler(void *data)
{
    (void)data;
    OIC_LOG(DEBUG, TAG, "IN - CAReceiveHandler");

    while (!caglobals.tcp.terminate)
    {
        CAReceiveMessage();
    }

    ca_mutex_lock(g_mutexObjectList);
    // notify the thread
    g_threadCounts--;
    if (!g_threadCounts)
    {
        ca_cond_signal(g_condObjectList);
    }
    ca_mutex_unlock(g_mutexObjectList);

    OIC_LOG(DEBUG, TAG, "OUT - CAReceiveHandler");
}

static size_t CAGetTotalLengthFromHeader(const unsigned char *recvBuffer)
{
    OIC_LOG(DEBUG, TAG, "IN - CAGetTotalLengthFromHeader");

    coap_transport_type transport = coap_get_tcp_header_type_from_initbyte(
            ((unsigned char *)recvBuffer)[0] >> 4);
    size_t optPaylaodLen = coap_get_length_from_header((unsigned char *)recvBuffer,
                                                        transport);
    size_t headerLen = coap_get_tcp_header_length((unsigned char *)recvBuffer);

    OIC_LOG_V(DEBUG, TAG, "option/paylaod length [%d]", optPaylaodLen);
    OIC_LOG_V(DEBUG, TAG, "header length [%d]", headerLen);
    OIC_LOG_V(DEBUG, TAG, "total data length [%d]", headerLen + optPaylaodLen);

    OIC_LOG(DEBUG, TAG, "OUT - CAGetTotalLengthFromHeader");
    return headerLen + optPaylaodLen;
}

static CAResult_t CAReceiveMessage()
{
    uint32_t length = u_arraylist_length(caglobals.tcp.svrlist);

    size_t i = 0;
    unsigned char *recvBuffer = NULL;
    CATCPServerInfo_t *svritem = NULL;
    for (i = 0; i < length; i++)
    {
        svritem = (CATCPServerInfo_t *) u_arraylist_get(caglobals.tcp.svrlist, i);
        if (svritem->u4tcp.fd < 0)
        {
            continue;
        }

        size_t bufSize = TCP_MAX_HEADER_LEN;
        recvBuffer = (unsigned char *) OICCalloc(1, bufSize);
        if (!recvBuffer)
        {
            OIC_LOG(ERROR, TAG, "out of memory");
            goto exit;
        }

        bool isHeaderChecked = false;
        size_t totalLen = 0;
        size_t totalReceivedLen = 0;
        do
        {
            ssize_t recvLen = recv(svritem->u4tcp.fd, recvBuffer + totalReceivedLen,
                                   bufSize - totalReceivedLen, 0);
            if (recvLen <= 0)
            {
                if(EWOULDBLOCK != errno)
                {
                    OIC_LOG_V(ERROR, TAG, "Recvfrom failed %s", strerror(errno));
                    goto exit;
                }
                // if received data length is zero, we are breaking loop.
                // because we use non-blocking socket to receive data from remote device.
                if (!totalReceivedLen)
                {
                    break;
                }
                continue;
            }

            totalReceivedLen += recvLen;
            if (!isHeaderChecked && totalReceivedLen)
            {
                coap_transport_type transport = coap_get_tcp_header_type_from_initbyte(
                        ((unsigned char *)recvBuffer)[0] >> 4);
                size_t headerLen = coap_get_tcp_header_length_for_transport(transport);
                if (totalReceivedLen >= headerLen)
                {
                    // get actual data length from coap over tcp header
                    totalLen = CAGetTotalLengthFromHeader((unsigned char *) recvBuffer);
                    bufSize = totalLen;
                    unsigned char *newBuf = OICRealloc(recvBuffer, bufSize);
                    if (NULL == newBuf)
                    {
                        OIC_LOG(ERROR, TAG, "out of memory");
                        goto exit;
                    }
                    recvBuffer = newBuf;
                    isHeaderChecked = true;
                }
            }
            if (totalLen == totalReceivedLen)
            {
                CAEndpoint_t ep = { .adapter = CA_ADAPTER_TCP,
                                    .port = svritem->u4tcp.port };
                strncpy(ep.addr, svritem->addr, sizeof(ep.addr));

                if (g_packetReceivedCallback)
                {
                    g_packetReceivedCallback(&ep, recvBuffer, totalLen);
                }
                OIC_LOG_V(DEBUG, TAG, "received data len:%d", totalLen);
                break;
            }
        } while (!totalLen || totalLen > totalReceivedLen);

        OICFree(recvBuffer);
    }

    return CA_STATUS_OK;

exit:
    ca_mutex_lock(g_mutexObjectList);
    close(svritem->u4tcp.fd);
    u_arraylist_remove(caglobals.tcp.svrlist, i);
    ca_mutex_unlock(g_mutexObjectList);
    OICFree(recvBuffer);
    return CA_STATUS_FAILED;
}

// TODO: resolving duplication.
static int CASetNonblocking(int fd)
{
    int fl = fcntl(fd, F_GETFL);
    if (fl == -1)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to get existing flags, Error code: %s",
                  strerror(errno));
    }
    else if ((fl & O_NONBLOCK) != O_NONBLOCK)
    {
        fl = fcntl(fd, F_SETFL, fl | O_NONBLOCK);
        if (fl == -1)
        {
            OIC_LOG_V(ERROR, TAG, "Failed to set non-blocking mode, Error code: %s",
                      strerror(errno));
        }
    }

    return fl;
}

static int CATCPCreateSocket(int family, CATCPServerInfo_t *TCPServerInfo)
{
    // create tcp socket
    int fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == fd)
    {
        OIC_LOG_V(ERROR, TAG, "create socket failed: %s", strerror(errno));
        goto exit;
    }

    // set non-blocking socket
    if (-1 == CASetNonblocking(fd))
    {
        goto exit;
    }

    struct sockaddr_storage sa = { .ss_family = family };
    CAConvertNameToAddr(TCPServerInfo->addr, TCPServerInfo->u4tcp.port, &sa);
    socklen_t socklen = sizeof (struct sockaddr_in);

    // connect to TCP server
    int ret = connect(fd, (struct sockaddr *)&sa, socklen);
    if (0 == ret)
    {
        OIC_LOG(DEBUG, TAG, "connect socket success");
    }
    else if (EINPROGRESS == errno)
    {
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            OIC_LOG(ERROR, TAG, "getsockopt() error");
            goto exit;
        }

        if (error)
        {
            if (ECONNREFUSED == error)
            {
                OIC_LOG(ERROR, TAG, "connection refused");
                goto exit;
            }
            OIC_LOG(ERROR, TAG, "failed to connect socket");
            goto exit;
        }
        OIC_LOG(DEBUG, TAG, "connect socket success");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "failed to connect socket");
        goto exit;
    }

    return fd;

exit:
    if (fd >= 0)
    {
        close(fd);
    }
    return -1;
}

static void CAAcceptHandler(void *data)
{
    (void)data;
    OIC_LOG(DEBUG, TAG, "IN - CAAcceptHandler");

    int reuse = 1;
    struct sockaddr_in server = { .sin_addr.s_addr = INADDR_ANY,
                                  .sin_family = AF_INET,
                                  .sin_port = htons(SERVER_PORT) };

    g_acceptServerFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_acceptServerFD < 0)
    {
        OIC_LOG(ERROR, TAG, "Failed to create socket");
        goto exit;
    }

    if (-1 == setsockopt(g_acceptServerFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
    {
        OIC_LOG(ERROR, TAG, "setsockopt SO_REUSEADDR");
        goto exit;
    }

    int serverlen = sizeof(server);
    if (-1 == bind(g_acceptServerFD, (struct sockaddr *)&server, serverlen))
    {
        OIC_LOG(ERROR, TAG, "bind() error");
        goto exit;
    }

    if (listen(g_acceptServerFD, caglobals.tcp.listenBacklog) != 0)
    {
        OIC_LOG(ERROR, TAG, "listen() error");
        goto exit;
    }

    struct pollfd acceptServerFD = { .fd = g_acceptServerFD,
                                     .events = POLLIN };

    while (!caglobals.tcp.terminate)
    {
        int pollState = poll(&acceptServerFD, 1, caglobals.tcp.selectTimeout);
        if (pollState < 0)
        {
            OIC_LOG_V(FATAL, TAG, "polling error %s", strerror(errno));
            goto exit;
        }
        else if (!pollState)
        {
            continue;
        }

        if (acceptServerFD.revents & POLLIN)
        {
            struct sockaddr_storage clientaddr;
            socklen_t clientlen = sizeof (struct sockaddr_in);

            int sockfd = accept(g_acceptServerFD, (struct sockaddr *)&clientaddr, &clientlen);
            if (sockfd != -1)
            {
                CATCPServerInfo_t *svritem = (CATCPServerInfo_t *) OICMalloc(sizeof (*svritem));
                if (!svritem)
                {
                    OIC_LOG(ERROR, TAG, "Out of memory");
                    close(sockfd);
                    return;
                }

                // set non-blocking socket
                if (-1 == CASetNonblocking(sockfd))
                {
                    close(sockfd);
                    OICFree(svritem);
                    continue;
                }
                svritem->u4tcp.fd = sockfd;

                CAConvertAddrToName((struct sockaddr_storage *)&clientaddr,
                                    (char *) &svritem->addr, &svritem->u4tcp.port);

                ca_mutex_lock(g_mutexObjectList);
                bool res = u_arraylist_add(caglobals.tcp.svrlist, svritem);
                if (!res)
                {
                    OIC_LOG(ERROR, TAG, "u_arraylist_add failed.");
                    close(sockfd);
                    OICFree(svritem);
                    ca_mutex_unlock(g_mutexObjectList);
                    continue;
                }
                ca_mutex_unlock(g_mutexObjectList);
            }
        }
    }

    ca_mutex_lock(g_mutexObjectList);
    // notify the thread
    g_threadCounts--;
    if (!g_threadCounts)
    {
        ca_cond_signal(g_condObjectList);
    }
    ca_mutex_unlock(g_mutexObjectList);

    OIC_LOG(DEBUG, TAG, "OUT - CAAcceptHandler");

exit:
    if (g_acceptServerFD >= 0)
    {
        close(g_acceptServerFD);
    }
    ca_mutex_lock(g_mutexObjectList);
    g_threadCounts--;
    if (!g_threadCounts)
    {
        ca_cond_signal(g_condObjectList);
    }
    ca_mutex_unlock(g_mutexObjectList);
    return;
}

CAResult_t CATCPStartServer(const ca_thread_pool_t threadPool)
{
    if (caglobals.tcp.started)
    {
        return CA_STATUS_OK;
    }

    if (!caglobals.tcp.ipv4tcpenabled)
    {
        caglobals.tcp.ipv4tcpenabled = true;    // only needed to run CA tests
    }

    CAResult_t res = CATCPCreateMutex();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "failed to create mutex");
        return res;
    }

    res = CATCPCreateCond();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "failed to create cond");
        return res;
    }

    ca_mutex_lock(g_mutexObjectList);
    if (!caglobals.tcp.svrlist)
    {
        caglobals.tcp.svrlist = u_arraylist_create();
    }
    ca_mutex_unlock(g_mutexObjectList);

    caglobals.tcp.terminate = false;

    res = ca_thread_pool_add_task(threadPool, CAAcceptHandler, NULL);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "thread_pool_add_task failed");
        return res;
    }
    OIC_LOG(DEBUG, TAG, "CAAcceptHandler thread started successfully.");

    res = ca_thread_pool_add_task(threadPool, CAReceiveHandler, NULL);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "thread_pool_add_task failed");
        return res;
    }
    OIC_LOG(DEBUG, TAG, "CAReceiveHandler thread started successfully.");

    caglobals.tcp.started = true;

    g_threadCounts = CA_TCP_DEFAULT_THREAD_COUNTS;

    return CA_STATUS_OK;
}

void CATCPStopServer()
{
    OIC_LOG(DEBUG, TAG, "IN");

    // mutex lock
    ca_mutex_lock(g_mutexObjectList);

    // set terminate flag
    caglobals.tcp.terminate = true;
    caglobals.tcp.started = false;

    ca_cond_wait(g_condObjectList, g_mutexObjectList);

    // mutex unlock
    ca_mutex_unlock(g_mutexObjectList);

    if (-1 != g_acceptServerFD)
    {
        close(g_acceptServerFD);
        g_acceptServerFD = -1;
    }

    CATCPDisconnectAll();
    CATCPDestroyMutex();
    CATCPDestroyCond();

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CATCPSetPacketReceiveCallback(CATCPPacketReceivedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");

    g_packetReceivedCallback = callback;

    OIC_LOG(DEBUG, TAG, "OUT");
}

static size_t CACheckPayloadLength(const void *data, size_t dlen)
{
    coap_transport_type transport = coap_get_tcp_header_type_from_initbyte(
            ((unsigned char *)data)[0] >> 4);

    coap_pdu_t *pdu = coap_new_pdu(transport, dlen);
    if (!pdu)
    {
        OIC_LOG(ERROR, TAG, "outpdu is null");
        return 0;
    }

    int ret = coap_pdu_parse((unsigned char *) data, dlen, pdu, transport);
    if (0 >= ret)
    {
        OIC_LOG(ERROR, TAG, "pdu parse failed");
        coap_delete_pdu(pdu);
        return 0;
    }

    size_t payloadLen = 0;
    size_t headerSize = coap_get_tcp_header_length_for_transport(transport);
    OIC_LOG_V(DEBUG, TAG, "headerSize : %d, pdu length : %d",
              headerSize, pdu->length);
    if (pdu->length > headerSize)
    {
        payloadLen = (unsigned char *) pdu->hdr + pdu->length - pdu->data;
        OICFree(pdu);
    }

    return payloadLen;
}

static void sendData(const CAEndpoint_t *endpoint,
                     const void *data, size_t dlen)
{
    // #1. get TCP Server object from list
    uint32_t index = 0;
    CATCPServerInfo_t *svritem = CAGetTCPServerInfoFromList(endpoint->addr, endpoint->port,
                                                            &index);
    if (!svritem)
    {
        // if there is no connection info, connect to TCP Server
        svritem = CAConnectToTCPServer(endpoint);
        if (!svritem)
        {
            OIC_LOG(ERROR, TAG, "Failed to create TCP server object");
            g_TCPErrorHandler(endpoint, data, dlen, CA_SEND_FAILED);
            return;
        }
    }

    // #2. check payload length
    size_t payloadLen = CACheckPayloadLength(data, dlen);
    // if payload length is zero, disconnect from TCP server
    if (!payloadLen)
    {
        OIC_LOG(DEBUG, TAG, "payload length is zero, disconnect from remote device");
        CADisconnectFromTCPServer(endpoint);
        return;
    }

    // #3. check connection state
    if (svritem->u4tcp.fd < 0)
    {
        // if file descriptor value is wrong, remove TCP Server info from list
        OIC_LOG(ERROR, TAG, "Failed to connect to TCP server");
        CADisconnectFromTCPServer(endpoint);
        g_TCPErrorHandler(endpoint, data, dlen, CA_SEND_FAILED);
        return;
    }

    // #4. send data to TCP Server
    size_t remainLen = dlen;
    do
    {
        size_t len = send(svritem->u4tcp.fd, data, remainLen, 0);
        if (-1 == len)
        {
            if (EWOULDBLOCK != errno)
            {
                OIC_LOG_V(ERROR, TAG, "unicast ipv4tcp sendTo failed: %s", strerror(errno));
                g_TCPErrorHandler(endpoint, data, dlen, CA_SEND_FAILED);
                return;
            }
            continue;
        }
        data += len;
        remainLen -= len;
    } while (remainLen > 0);

    OIC_LOG_V(INFO, TAG, "unicast ipv4tcp sendTo is successful: %d bytes", dlen);
}

void CATCPSendData(CAEndpoint_t *endpoint, const void *data, uint32_t datalen,
                   bool isMulticast)
{
    VERIFY_NON_NULL_VOID(endpoint, TAG, "endpoint is NULL");
    VERIFY_NON_NULL_VOID(data, TAG, "data is NULL");

    if (!isMulticast)
    {
        if (caglobals.tcp.ipv4tcpenabled && (endpoint->adapter & CA_ADAPTER_TCP))
        {
            sendData(endpoint, data, datalen);
        }
    }
}

CAResult_t CAGetTCPInterfaceInformation(CAEndpoint_t **info, uint32_t *size)
{
    OIC_LOG(DEBUG, TAG, "IN");

    VERIFY_NON_NULL(info, TAG, "info is NULL");
    VERIFY_NON_NULL(size, TAG, "size is NULL");

    return CA_NOT_SUPPORTED;
}

CATCPServerInfo_t *CAConnectToTCPServer(const CAEndpoint_t *TCPServerInfo)
{
    VERIFY_NON_NULL_RET(TCPServerInfo, TAG, "TCPServerInfo is NULL", NULL);

    // #1. create TCP server object
    CATCPServerInfo_t *svritem = (CATCPServerInfo_t *) OICMalloc(sizeof (*svritem));
    if (!svritem)
    {
        OIC_LOG(ERROR, TAG, "Out of memory");
        return NULL;
    }
    memcpy(svritem->addr, TCPServerInfo->addr, sizeof(svritem->addr));
    svritem->u4tcp.port = TCPServerInfo->port;

    // #2. create the socket and connect to TCP server
    if (caglobals.tcp.ipv4tcpenabled)
    {
        svritem->u4tcp.fd = CATCPCreateSocket(AF_INET, svritem);
        if (-1 == svritem->u4tcp.fd)
        {
            OICFree(svritem);
            return NULL;
        }
    }

    // #3. add TCP connection info to list
    ca_mutex_lock(g_mutexObjectList);
    if (caglobals.tcp.svrlist)
    {
        bool res = u_arraylist_add(caglobals.tcp.svrlist, svritem);
        if (!res)
        {
            OIC_LOG(ERROR, TAG, "u_arraylist_add failed.");
            close(svritem->u4tcp.fd);
            OICFree(svritem);
            ca_mutex_unlock(g_mutexObjectList);
            return NULL;
        }
    }
    ca_mutex_unlock(g_mutexObjectList);

    return svritem;
}

CAResult_t CADisconnectFromTCPServer(const CAEndpoint_t *TCPServerInfo)
{
    VERIFY_NON_NULL(TCPServerInfo, TAG, "TCP server info is NULL");

    // #1. get server info
    uint32_t index = 0;
    ca_mutex_lock(g_mutexObjectList);
    CATCPServerInfo_t *svritem = CAGetTCPServerInfoFromList(TCPServerInfo->addr,
                                                            TCPServerInfo->port,
                                                            &index);
    if (!svritem)
    {
        OIC_LOG(ERROR, TAG, "there is no connection info");
        ca_mutex_unlock(g_mutexObjectList);
        return CA_STATUS_FAILED;
    }

    // #2. close the socket and remove TCP connection info in list
    if (svritem->u4tcp.fd >= 0)
    {
        close(svritem->u4tcp.fd);
    }
    u_arraylist_remove(caglobals.tcp.svrlist, index);
    ca_mutex_unlock(g_mutexObjectList);

    return CA_STATUS_OK;
}

CATCPServerInfo_t *CAGetTCPServerInfoFromList(const char *addr, const uint16_t port,
                                              uint32_t *index)
{
    VERIFY_NON_NULL_RET(addr, TAG, "addr is NULL", NULL);
    VERIFY_NON_NULL_RET(index, TAG, "index is NULL", NULL);

    // get connection info from list
    uint32_t length = u_arraylist_length(caglobals.tcp.svrlist);

    for (size_t i = 0; i < length; i++)
    {
        CATCPServerInfo_t *svritem = (CATCPServerInfo_t *) u_arraylist_get(
                caglobals.tcp.svrlist, i);
        if (!svritem)
        {
            continue;
        }

        if (!strncmp(svritem->addr, addr, sizeof(svritem->addr))
                && (svritem->u4tcp.port == port))
        {
            *index = i;
            return svritem;
        }
    }

    return NULL;
}

void CATCPSetErrorHandler(CATCPErrorHandleCallback errorHandleCallback)
{
    g_TCPErrorHandler = errorHandleCallback;
}
