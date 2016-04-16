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
 * This file provides APIs related to mutex and semaphores.
 */

// Defining _POSIX_C_SOURCE macro with 199309L (or greater) as value
// causes header files to expose definitions
// corresponding to the POSIX.1b, Real-time extensions
// (IEEE Std 1003.1b-1993) specification
//
// For this specific file, see use of clock_gettime and PTHREAD_MUTEX_DEFAULT
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <Windows.h>
#include <WinBase.h>
#include <sys/timeb.h>

#define __func__ __FUNCTION__

#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <oic_malloc.h>

#include "camutex.h"
#include "logger.h"

/**
 * TAG
 * Logging tag for module name
 */
#define TAG PCF("UMUTEX")

static const uint64_t USECS_PER_SEC         = 1000000;
static const uint64_t NANOSECS_PER_USECS    = 1000;
static const uint64_t NANOSECS_PER_SEC      = 1000000000L;

typedef struct _tagMutexInfo_t
{
    CRITICAL_SECTION mutex;
} ca_mutex_internal;

typedef struct _tagEventInfo_t
{
    CONDITION_VARIABLE  cond;
} ca_cond_internal;

ca_mutex ca_mutex_new(void)
{
    ca_mutex retVal = NULL;
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) OICMalloc(sizeof(ca_mutex_internal));
    if (NULL != mutexInfo)
    {
        InitializeCriticalSection(&(mutexInfo->mutex));
        retVal = (ca_mutex)mutexInfo;
    }
    return retVal;
}

bool ca_mutex_free(ca_mutex mutex)
{
    bool bRet=false;
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;
    if (mutexInfo)
    {
        DeleteCriticalSection(&mutexInfo->mutex);
        OICFree(mutexInfo);
        bRet = true;
    }
    else
    {
        OIC_LOG_V(ERROR, TAG, "%s Invalid mutex !", __func__);
    }

    return bRet;
}

void ca_mutex_lock(ca_mutex mutex)
{
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;
    if (mutexInfo)
    {
        EnterCriticalSection(&mutexInfo->mutex);
    }
    else
    {
        OIC_LOG_V(ERROR, TAG, "%s Invalid mutex !", __func__);
        return;
    }
}

bool ca_mutex_trylock(ca_mutex mutex)
{
    if (NULL == mutex)
    {
        OIC_LOG_V(ERROR, TAG, "%s Invalid mutex !", __func__);
        return false;
    }

    bool bRet = false;

    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;

    bRet = (TryEnterCriticalSection(&mutexInfo->mutex) == FALSE) ? false : true;
    return bRet;
}

void ca_mutex_unlock(ca_mutex mutex)
{
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;
    if (mutexInfo)
    {
        LeaveCriticalSection(&mutexInfo->mutex);
    }
    else
    {
          OIC_LOG_V(ERROR, TAG, "%s: Invalid mutex !", __func__);
          return;
    }
}

ca_cond ca_cond_new(void)
{
    ca_cond retVal = NULL;
    ca_cond_internal *eventInfo = (ca_cond_internal*) OICMalloc(sizeof(ca_cond_internal));
    if (NULL != eventInfo)
    {
        InitializeConditionVariable(&(eventInfo->cond));
        retVal = (ca_cond)eventInfo;
    }
    return retVal;
}

void ca_cond_free(ca_cond cond)
{
    ca_cond_internal *eventInfo = (ca_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        OICFree(cond);
    }
    else
    {
        OIC_LOG_V(ERROR, TAG, "%s: Invalid parameter", __func__);
    }
}

void ca_cond_signal(ca_cond cond)
{
    ca_cond_internal *eventInfo = (ca_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        WakeConditionVariable(&(eventInfo->cond));
    }
    else
    {
        OIC_LOG_V(ERROR, TAG, "%s: Invalid parameter", __func__);
    }
}

void ca_cond_broadcast(ca_cond cond)
{
    ca_cond_internal* eventInfo = (ca_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        WakeAllConditionVariable(&(eventInfo->cond));
    }
    else
    {
        OIC_LOG_V(ERROR, TAG, "%s: Invalid parameter", __func__);
    }
}

void ca_cond_wait(ca_cond cond, ca_mutex mutex)
{
    ca_cond_wait_for(cond, mutex, 0L);
}

CAWaitResult_t ca_cond_wait_for(ca_cond cond, ca_mutex mutex, uint64_t microseconds)
{
    CAWaitResult_t retVal = CA_WAIT_INVAL;

    ca_cond_internal *eventInfo = (ca_cond_internal*) cond;
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;

    if (NULL == mutexInfo)
    {
        OIC_LOG_V(ERROR, TAG, "%s: Invalid mutex", __func__);
        return CA_WAIT_INVAL;
    }

    if (NULL == eventInfo)
    {
        OIC_LOG_V(ERROR, TAG, "%s: Invalid condition", __func__);
        return CA_WAIT_INVAL;
    }

    if (microseconds > 0)
    {
        DWORD tm = microseconds / 1000;
        if (!SleepConditionVariableCS(&(eventInfo->cond), &(mutexInfo->mutex), tm))
        {
            DWORD dwErr = GetLastError();
            retVal = (dwErr == ERROR_TIMEOUT) ? CA_WAIT_TIMEDOUT : CA_WAIT_INVAL;
        }
        else
        {
            retVal = CA_WAIT_SUCCESS;;
        }
    }
    else
    {
        // Wait forever
        BOOL ret = SleepConditionVariableCS(&(eventInfo->cond), &(mutexInfo->mutex), INFINITE);
        retVal = (ret != FALSE) ? CA_WAIT_SUCCESS : CA_WAIT_INVAL;
    }
    return retVal;
}

