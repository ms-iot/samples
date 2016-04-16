//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

#ifndef _RESOURCE_DIRECTORY_CLIENT_H_
#define _RESOURCE_DIRECTORY_CLIENT_H_

// Iotivity Base CAPI
#include "ocstack.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** Max ADDR SIZE */
#define MAX_ADDR_STR_SIZE                (40)

/** Callback function for returning RDDiscovery Result. */
typedef int (* OCRDBiasFactorCB)(char addr[MAX_ADDR_STR_SIZE], uint16_t port);

/** Context structure used sending it as part of the callback context. */
typedef struct
{
    /** Stores the context value of the message sent. */
    void *context;
    /** Pointing to the callback function that OCRDDiscover() received. */
    OCRDBiasFactorCB cbFunc;
} OCRDClientContextCB;

/**
 * Discovers the resource directory.
 * This function searches a RD server and obtain the bias factor.
 *
 * @param cbBiasFactor callback function invoked when bias factor is returned by
 *                     the Resource Directory Server
 *
 * @return ::OC_STACK_OK upon success, ::OC_STACK_ERROR in case of error.
 */
OCStackResult OCRDDiscover(OCRDBiasFactorCB cbBiasFactor);

/**
 * Publish resource on the RD.
 *
 * @param addr The IP address of the RD, it could be either retrieved via OCRDDiscover().
 * @param port The port of the RD.
 * @param num This denotes the number of registered resource handles being passed
 *            for the remaining arguments.
 *
 * @returns ::OC_STACK_OK when successful and OC_STACK_ERROR when query failed.
 */
OCStackResult OCRDPublish(char *addr, uint16_t port, int num, ...);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_RESOURCE_DIRECTORY_CLIENT_H_
