//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
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

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include "oic_malloc.h"

// Enable extra debug logging for malloc.  Comment out to disable
#ifdef ENABLE_MALLOC_DEBUG
#include "logger.h"
#define TAG "OICMalloc"
#endif

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Internal API function
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public APIs
//-----------------------------------------------------------------------------
#ifdef ENABLE_MALLOC_DEBUG
static uint32_t count;
#endif

void *OICMalloc(size_t size)
{
    if (0 == size)
    {
        return NULL;
    }

#ifdef ENABLE_MALLOC_DEBUG
    void *ptr = malloc(size);
    if (ptr)
    {
        count++;
    }
    OIC_LOG_V(INFO, TAG, "malloc: ptr=%p, size=%u, count=%u", ptr, size, count);
    return ptr;
#else
    return malloc(size);
#endif
}

void *OICCalloc(size_t num, size_t size)
{
    if (0 == size || 0 == num)
    {
        return NULL;
    }

#ifdef ENABLE_MALLOC_DEBUG
    void *ptr = calloc(num, size);
    if (ptr)
    {
        count++;
    }
    OIC_LOG_V(INFO, TAG, "calloc: ptr=%p, num=%u, size=%u, count=%u", ptr, num, size, count);
    return ptr;
#else
    return calloc(num, size);
#endif
}

void *OICRealloc(void* ptr, size_t size)
{
    // Override realloc() behavior for NULL pointer which normally would
    // work as per malloc(), however we suppress the behavior of possibly
    // returning a non-null unique pointer.
    if (ptr == NULL)
    {
        return OICMalloc(size);
    }

    // Otherwise leave the behavior up to realloc() itself:

#ifdef ENABLE_MALLOC_DEBUG
    void* newptr = realloc(ptr, size);
    OIC_LOG_V(INFO, TAG, "realloc: ptr=%p, newptr=%p, size=%u", ptr, newptr, size);
    // Very important to return the correct pointer here, as it only *somtimes*
    // differs and thus can be hard to notice/test:
    return newptr;
#else
    return realloc(ptr, size);
#endif
}

void OICFree(void *ptr)
{
#ifdef ENABLE_MALLOC_DEBUG
    // Since OICMalloc() did not increment count if it returned NULL,
    // guard the decrement:
    if (ptr)
    {
        count--;
    }
    OIC_LOG_V(INFO, TAG, "free: ptr=%p, count=%u", ptr, count);
#endif

    free(ptr);
}
