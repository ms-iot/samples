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

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <oic_malloc.h>

#include "camutex.h"
#include "logger.h"

/**
 * TAG
 * Logging tag for module name
 */
#define TAG PCF("UMUTEX")

#ifdef __ANDROID__
/**
 * Android has pthread_condattr_setclock() only in version >= 5.0, older
 * version do have a function called __pthread_cond_timedwait_relative()
 * which waits *for* the given timespec, this function is not visible in
 * android version >= 5.0 anymore. This is the same way as it is handled in
 * QT 5.5.0 in
 * http://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/thread/qwaitcondition_unix.cpp?h=v5.5.0#n54
 */
static int camutex_condattr_setclock(pthread_condattr_t *, clockid_t)
        __attribute__ ((weakref("pthread_condattr_setclock")));

static int camutex_cond_timedwait_relative(pthread_cond_t*, pthread_mutex_t*, const struct timespec*)
        __attribute__ ((weakref("__pthread_cond_timedwait_relative")));
#endif /* __ANDROID__ */

static const uint64_t USECS_PER_SEC         = 1000000;
static const uint64_t NANOSECS_PER_USECS    = 1000;
static const uint64_t NANOSECS_PER_SEC      = 1000000000L;

typedef struct _tagMutexInfo_t
{
    pthread_mutex_t mutex;
} ca_mutex_internal;

typedef struct _tagEventInfo_t
{
    pthread_cond_t cond;
    pthread_condattr_t condattr;
} ca_cond_internal;

ca_mutex ca_mutex_new(void)
{
    ca_mutex retVal = NULL;
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) OICMalloc(sizeof(ca_mutex_internal));
    if (NULL != mutexInfo)
    {
        // create the mutex with the attributes set
        int ret=pthread_mutex_init(&(mutexInfo->mutex), PTHREAD_MUTEX_DEFAULT);
        if (0 == ret)
        {
            retVal = (ca_mutex) mutexInfo;
        }
        else
        {
            OIC_LOG_V(ERROR, TAG, "%s Failed to initialize mutex !", __func__);
            OICFree(mutexInfo);
        }
    }

    return retVal;
}

bool ca_mutex_free(ca_mutex mutex)
{
    bool bRet=false;

    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;
    if (mutexInfo)
    {
        int ret = pthread_mutex_destroy(&mutexInfo->mutex);
        if (0 == ret)
        {
            OICFree(mutexInfo);
            bRet=true;
        }
        else
        {
            OIC_LOG_V(ERROR, TAG, "%s Failed to free mutex !", __func__);
        }
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
        int ret = pthread_mutex_lock(&mutexInfo->mutex);
        assert(0 == ret);
        if(ret != 0)
        {
            OIC_LOG_V(ERROR, TAG, "Pthread Mutex lock failed: %d", ret);
            exit(ret);
        }
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

    int result = pthread_mutex_trylock(&mutexInfo->mutex);

    switch (result)
    {
        case 0:
            // Success
            bRet = true;
            break;
        case EINVAL:
            OIC_LOG_V(ERROR, TAG, "%s: Invalid mutex !", __func__);
            break;
        case EBUSY:
        default:
            break;
    }

    return bRet;
}

void ca_mutex_unlock(ca_mutex mutex)
{
    ca_mutex_internal *mutexInfo = (ca_mutex_internal*) mutex;
    if (mutexInfo)
    {
        int ret = pthread_mutex_unlock(&mutexInfo->mutex);

        assert ( 0 == ret);
        if(ret != 0)
        {
            OIC_LOG_V(ERROR, TAG, "Pthread Mutex unlock failed: %d", ret);
            exit(ret);
        }
        (void)ret;
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
        int ret = pthread_condattr_init(&(eventInfo->condattr));
        if(0 != ret)
        {
            OIC_LOG_V(ERROR, TAG, "%s: Failed to initialize condition variable attribute %d!",
                    __func__, ret);
            OICFree(eventInfo);
            return retVal;
        }

#if defined(__ANDROID__) || _POSIX_TIMERS > 0
#ifdef __ANDROID__
        if (camutex_condattr_setclock) {
            ret = camutex_condattr_setclock(&(eventInfo->condattr), CLOCK_MONOTONIC);
#else
        {
            ret = pthread_condattr_setclock(&(eventInfo->condattr), CLOCK_MONOTONIC);
#endif /*  __ANDROID__ */
            if(0 != ret)
            {
                OIC_LOG_V(ERROR, TAG, "%s: Failed to set condition variable clock %d!",
                        __func__, ret);
                pthread_condattr_destroy(&(eventInfo->condattr));
                OICFree(eventInfo);
                return retVal;
            }
        }
#endif /* defined(__ANDROID__) || _POSIX_TIMERS > 0 */
        ret = pthread_cond_init(&(eventInfo->cond), &(eventInfo->condattr));
        if (0 == ret)
        {
            retVal = (ca_cond) eventInfo;
        }
        else
        {
            OIC_LOG_V(ERROR, TAG, "%s: Failed to initialize condition variable %d!", __func__, ret);
            pthread_condattr_destroy(&(eventInfo->condattr));
            OICFree(eventInfo);
        }
    }

    return retVal;
}

void ca_cond_free(ca_cond cond)
{
    ca_cond_internal *eventInfo = (ca_cond_internal*) cond;
    if (eventInfo != NULL)
    {
        int ret = pthread_cond_destroy(&(eventInfo->cond));
        int ret2 = pthread_condattr_destroy(&(eventInfo->condattr));
        if (0 == ret && 0 == ret2)
        {
            OICFree(cond);
        }
        else
        {
            OIC_LOG_V(ERROR, TAG, "%s: Failed to destroy condition variable %d, %d",
                    __func__, ret, ret2);
        }
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
        int ret = pthread_cond_signal(&(eventInfo->cond));
        if (0 != ret)
        {
            OIC_LOG_V(ERROR, TAG, "%s: Failed to signal condition variable", __func__);
        }
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
        int ret = pthread_cond_broadcast(&(eventInfo->cond));
        if (0 != ret)
        {
            OIC_LOG_V(ERROR, TAG, "%s: failed to signal condition variable", __func__);
        }
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

struct timespec ca_get_current_time()
{
#if defined(__ANDROID__) || _POSIX_TIMERS > 0
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct timespec ts;
    TIMEVAL_TO_TIMESPEC(&tv, &ts);
    return ts;
#endif
}

void ca_add_microseconds_to_timespec(struct timespec* ts, uint64_t microseconds)
{
    time_t secPart = microseconds/USECS_PER_SEC;
    uint64_t nsecPart = (microseconds % USECS_PER_SEC) * NANOSECS_PER_USECS;
    uint64_t totalNs = ts->tv_nsec + nsecPart;
    time_t secOfNs = totalNs/NANOSECS_PER_SEC;

    ts->tv_nsec = (totalNs)% NANOSECS_PER_SEC;
    ts->tv_sec += secPart + secOfNs;
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
        int ret;
        struct timespec abstime;

#ifdef __ANDROID__
        if (camutex_cond_timedwait_relative) {
            abstime.tv_sec = microseconds / USECS_PER_SEC;
            abstime.tv_nsec = (microseconds % USECS_PER_SEC) * NANOSECS_PER_USECS;
            //Wait for the given time
            ret = camutex_cond_timedwait_relative(&(eventInfo->cond), &(mutexInfo->mutex), &abstime);
        } else
#endif
        {
             abstime = ca_get_current_time();
            ca_add_microseconds_to_timespec(&abstime, microseconds);

            //Wait for the given time
            ret = pthread_cond_timedwait(&(eventInfo->cond), &(mutexInfo->mutex), &abstime);
        }

        switch (ret)
        {
            case 0:
                // Success
                retVal = CA_WAIT_SUCCESS;
                break;
            case ETIMEDOUT:
                retVal = CA_WAIT_TIMEDOUT;
                break;
            case EINVAL:
                OIC_LOG_V(ERROR, TAG, "%s: condition, mutex, or abstime is Invalid", __func__);
                retVal = CA_WAIT_INVAL;
                break;
            default:
                OIC_LOG_V(ERROR, TAG, "%s: pthread_cond_timedwait returned %d", __func__, retVal);
                retVal = CA_WAIT_INVAL;
                break;
        }
    }
    else
    {
        // Wait forever
        int ret = pthread_cond_wait(&eventInfo->cond, &mutexInfo->mutex);
        retVal = ret == 0 ? CA_WAIT_SUCCESS : CA_WAIT_INVAL;
    }

    return retVal;
}

