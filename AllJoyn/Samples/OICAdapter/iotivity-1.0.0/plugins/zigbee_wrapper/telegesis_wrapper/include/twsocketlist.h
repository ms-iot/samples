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

#ifndef TWSOCKETLIST_H_
#define TWSOCKETLIST_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "plugintypes.h"
#include "telegesis_socket.h"
#include "utlist.h"

// TODO: Use OICThread instead of pthread directly.
// TODO: Use OICMutex instead of mutex directly.
#include <pthread.h>

typedef struct TWSock
{
    PIPlugin * plugin; // Handle
    char * eui; // The associated Zigbee radio's EUI.
    int fd;
    char * buffer;
    /** 'queue' MUST BE ACCESSED THREAD SAFE **/
    TWEntry * queue;
    pthread_mutex_t mutex; // TODO: Use OIC_MUTEX instead.
    pthread_cond_t  queueCV;
    bool isActive;
    /** 'queue' MUST BE ACCESSED THREAD SAFE **/
    pthread_t threadHandle;
    pthread_attr_t threadAttr;
    struct TWSock * next;
} TWSock;

TWResultCode TWAddTWSock(TWSock * sock, PIPlugin * plugin, const char * fileLoc);

TWSock * TWGetSock(PIPlugin * plugin);

TWResultCode TWDeleteTWSock(TWSock * sock);

TWResultCode TWDeleteAllTWSock();

TWResultCode TWFreeQueue(PIPlugin * plugin);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TWSOCKETLIST_H_ */
