//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/**
 * @file
 *
 * This API only works with:
 *      Telegesis ETRX357
 *      CICIE R310 B110615
 *
 */

#include "twsocketlist.h"
#include "logger.h"
#include "oic_malloc.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#define TAG "TWSocketList"


/**
 *
 * Apply terminal settings.
 *
 */
static int SetTerminalInfo(int fd, int speed, int parity, int shouldBlock);

/**
 *
 * Internal function to close socket. For outside callers to close a socket, they must properly
 * call TWDeleteTWSock() for the same affect to ensure all things are cleaned up on shutdown.
 *
 */
TWResultCode TWCloseTWSock(TWSock * sock);

static TWSock * g_twSockList = NULL;

TWResultCode TWAddTWSock(TWSock * sock, PIPlugin * plugin, const char * fileLoc)
{
    if(!sock || !plugin || !fileLoc)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }

    TWSock * out = NULL;
    TWSock * temp = NULL;
    LL_FOREACH_SAFE(g_twSockList, out, temp)
    {
        if(out == sock)
        {
            // Ignore requests to add a socket that's already in the queue.
            return TW_RESULT_OK;
        }
    }

    sock->plugin = plugin;
    sock->fd = open(fileLoc, O_RDWR | O_NOCTTY | O_SYNC);
    if(sock->fd <= 0)
    {
        OC_LOG_V(INFO, TAG, "Could not open port. Errno is: %d\n", errno);
        return TW_RESULT_ERROR;
    }

    // set speed to 19,200 bps, 8n1 (no parity), no blocking.
    int ret = SetTerminalInfo(sock->fd, DEVICE_BAUDRATE, 0, 0);
    if(ret != 0)
    {
        TWResultCode result = TWCloseTWSock(sock);
        if(result != TW_RESULT_OK)
        {
            return result;
        }
        return TW_RESULT_ERROR;
    }

    sock->buffer = NULL;
    sock->queue = NULL;
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&(sock->mutex), &mutexAttr); // TODO: Use OIC_MUTEX instead.
    pthread_cond_init(&(sock->queueCV), NULL);
    sock->next = NULL;
    sock->isActive = true;

    LL_APPEND(g_twSockList, sock);
    return TW_RESULT_OK;
}

TWSock * TWGetSock(PIPlugin * plugin)
{
    if(!plugin)
    {
        return NULL;
    }
    TWSock * out = NULL;
    TWSock * tmp = NULL;
    LL_FOREACH_SAFE(g_twSockList, out, tmp)
    {
        if(out->plugin == plugin)
        {
            return out;
        }
    }
    return NULL;
}

TWResultCode TWCloseTWSock(TWSock * sock)
{
    if(!sock)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    int ret = close(sock->fd);
    if(ret != 0)
    {
        OC_LOG_V(ERROR, TAG, "Could not close port. Errno is: %d", errno);
        return TW_RESULT_ERROR;
    }
    return TW_RESULT_OK;
}

TWResultCode TWDeleteTWSock(TWSock * sock)
{
    if(!sock)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWSock * out = NULL;
    TWSock * tmp = NULL;
    LL_FOREACH_SAFE(g_twSockList, out, tmp)
    {
        if(out == sock)
        {
            LL_DELETE(g_twSockList, out);
        }
    }

    OICFree(sock->buffer);
    OICFree(sock->eui);
    TWFreeQueue(sock->plugin);

    int mutexRet = pthread_mutex_destroy(&(sock->mutex));
    if(mutexRet != 0)
    {
        OC_LOG_V(ERROR, TAG, "Failed to destroy mutex. Error: %d", mutexRet);
        return TW_RESULT_ERROR;
    }
    TWResultCode result = TWCloseTWSock(sock);

    return result;
}

TWResultCode TWDeleteAllTWSock()
{
    TWSock * out = NULL;
    TWSock * tmp = NULL;
    LL_FOREACH_SAFE(g_twSockList, out, tmp)
    {
        TWDeleteTWSock(out);
    }
    return TW_RESULT_OK;
}

/**
 *
 * Apply interface attribute values to terminal settings.
 *
 */
int SetTerminalInfo(int fd, int speed, int parity, int shouldBlock)
{
    OC_LOG(INFO, TAG, "Enter SetTerminalInfo()");

    int ret = 0;
    struct termios terminalInfo = {
                                   .c_iflag = 0
                                  };

    errno = 0;
    ret = tcgetattr(fd, &terminalInfo);
    if (ret != 0)
    {
        OC_LOG_V(ERROR, TAG, "tcgetattr() - ret=%d errno=%d", ret, errno);
        ret = -1;
        goto exit;
    }

    errno = 0;
    ret = cfsetispeed (&terminalInfo, speed);
    if (ret != 0)
    {
        OC_LOG_V(ERROR, TAG, "cfsetispeed() - ret=%d errno=%d", ret, errno);
        ret = -1;
        goto exit;
    }

    errno = 0;
    ret = cfsetospeed (&terminalInfo, speed);
    if (ret != 0)
    {
        OC_LOG_V(ERROR, TAG, "cfsetospeed() - ret=%d errno=%d", ret, errno);
        ret = -1;
        goto exit;
    }

    terminalInfo.c_cflag = (terminalInfo.c_cflag & ~CSIZE);     //byte size
    terminalInfo.c_cflag |= CS8;        //byte size is 8

    terminalInfo.c_cflag &= ~PARENB;    //no parity
    terminalInfo.c_cflag |= parity;     //no parity

    terminalInfo.c_cflag &= ~CSTOPB;    //1 stop bit

    terminalInfo.c_cflag |= CREAD;      //enable the receiver

    //Input Control Settings
    terminalInfo.c_iflag &= ~IGNBRK;    //break condition

    //Local Mode Settings
    terminalInfo.c_lflag = 0;

    // whether to block on read and read time-out
    terminalInfo.c_cc[VMIN]  = (shouldBlock >= 1) ? 1 : 0;
    terminalInfo.c_cc[VTIME] = 5;

    //Input Control Settings
    terminalInfo.c_oflag = 0;

    errno = 0;
    ret = tcsetattr (fd, TCSANOW, &terminalInfo);
    if (ret != 0)
    {
        OC_LOG_V(ERROR, TAG, "tcsetattr - ret=%d errno=%d", ret, errno);
        ret = -1;
    }

exit:
    OC_LOG_V(INFO, TAG, "Leave SetTerminalInfo() with ret=%d", ret);
    return ret;
}
