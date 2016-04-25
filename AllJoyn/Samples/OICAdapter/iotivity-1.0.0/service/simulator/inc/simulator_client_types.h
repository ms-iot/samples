/******************************************************************
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

#ifndef SIMULATOR_CLIENT_TYPES_H_
#define SIMULATOR_CLIENT_TYPES_H_

#include <iostream>
#include <functional>
#include <memory>
#include "simulator_error_codes.h"

enum class ObserveType
{
    OBSERVE,
    OBSERVE_ALL
};

enum class RequestType
{
    RQ_TYPE_GET,
    RQ_TYPE_PUT,
    RQ_TYPE_POST,
    RQ_TYPE_DELETE
};

typedef struct
{
    bool isVerified;
    SimulatorResult errorCode;
} ValidationStatus;

typedef enum
{
    OP_START,
    OP_COMPLETE,
    OP_ABORT
} OperationState;

typedef enum
{
    /** use when defaults are ok. */
    SIMULATOR_CT_DEFAULT = 0,

    /** IPv4 and IPv6, including 6LoWPAN.*/
    SIMULATOR_CT_ADAPTER_IP           = (1 << 16),

    /** GATT over Bluetooth LE.*/
    SIMULATOR_CT_ADAPTER_GATT_BTLE    = (1 << 17),

    /** RFCOMM over Bluetooth EDR.*/
    SIMULATOR_CT_ADAPTER_RFCOMM_BTEDR = (1 << 18),

#ifdef RA_ADAPTER
    /** Remote Access over XMPP.*/
    SIMULATOR_CT_ADAPTER_REMOTE_ACCESS = (1 << 19),
#endif

    /** Insecure transport is the default (subject to change).*/

    /** secure the transport path.*/
    SIMULATOR_CT_FLAG_SECURE     = (1 << 4),

    /** IPv4 & IPv6 autoselection is the default.*/

    /** IP adapter only.*/
    SIMULATOR_CT_IP_USE_V6       = (1 << 5),

    /** IP adapter only.*/
    SIMULATOR_CT_IP_USE_V4       = (1 << 6),

    /** Link-Local multicast is the default multicast scope for IPv6.
     * These are placed here to correspond to the IPv6 address bits.*/

    /** IPv6 Interface-Local scope(loopback).*/
    SIMULATOR_CT_SCOPE_INTERFACE = 0x1,

    /** IPv6 Link-Local scope (default).*/
    SIMULATOR_CT_SCOPE_LINK      = 0x2,

    /** IPv6 Realm-Local scope.*/
    SIMULATOR_CT_SCOPE_REALM     = 0x3,

    /** IPv6 Admin-Local scope.*/
    SIMULATOR_CT_SCOPE_ADMIN     = 0x4,

    /** IPv6 Site-Local scope.*/
    SIMULATOR_CT_SCOPE_SITE      = 0x5,

    /** IPv6 Organization-Local scope.*/
    SIMULATOR_CT_SCOPE_ORG       = 0x8,

    /** IPv6 Global scope.*/
    SIMULATOR_CT_SCOPE_GLOBAL    = 0xE,
} SimulatorConnectivityType;

class SimulatorRemoteResource;
typedef std::function<void(std::shared_ptr<SimulatorRemoteResource>)>
ResourceFindCallback;

#endif
