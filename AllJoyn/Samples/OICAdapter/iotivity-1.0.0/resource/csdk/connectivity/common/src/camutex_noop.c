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
//
//
//*********************************************************************

/**
 * @file
 * This file provides APIs related to mutex with no operation
 * for Singlethread implementation.
 */

#include "camutex.h"

/**
 * TAG
 * Logging tag for module name
 */
#define TAG "UMUTEX"

typedef struct _tagMutexInfo_t
{
} ca_mutex_internal;

typedef struct _tagEventInfo_t
{
} ca_cond_internal;

/**
 * @var g_mutexInfo
 * @brief This is used to return a non NULL value for ca_mutex_new().
 */
static ca_mutex_internal g_mutexInfo = { 0 };

/**
 * @var g_condInfo
 * @brief This is used to return a non NULL value for ca_cond_new().
 */
static ca_cond_internal g_condInfo = { 0 };

ca_mutex ca_mutex_new(void)
{
    return &g_mutexInfo;
}

bool ca_mutex_free(ca_mutex mutex)
{
    return true;
}

void ca_mutex_lock(ca_mutex mutex)
{
    return;
}

bool ca_mutex_trylock(ca_mutex mutex)
{
    return true;
}

void ca_mutex_unlock(ca_mutex mutex)
{
    return;
}

ca_cond ca_cond_new(void)
{
    return &g_condInfo;
}

void ca_cond_free(ca_cond cond)
{
    return;
}

void ca_cond_signal(ca_cond cond)
{
    return;
}

void ca_cond_broadcast(ca_cond cond)
{
    return;
}

void ca_cond_wait(ca_cond cond, ca_mutex mutex)
{
    return;
}

CAWaitResult_t ca_cond_wait_for(ca_cond cond, ca_mutex mutex, uint64_t microseconds)
{
    return CA_WAIT_SUCCESS;
}

