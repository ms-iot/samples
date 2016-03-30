/* ****************************************************************
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

/**
 * @file
 *
 * This file provides APIs related to thread pool.  Implementations are provided
 * by adding a new .c file for each implementation, and adding them conditionally
 * via the SCONS build script.  Currently, cathreadpool_pthreads.c is implemented,
 * with cathreadpool_winthreads.c being considered.  RTOS implementations should use
 * a name that best describes the used technology, not the OS.
 */

#ifndef CA_THREAD_POOL_H_
#define CA_THREAD_POOL_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/**
 * Callback type can be registered to thread pool.
 */
typedef void (*ca_thread_func)(void *);

struct ca_thread_pool_details_t;
/**
 * Thread pool type.
 */
typedef struct ca_thread_pool
{
    struct ca_thread_pool_details_t* details;
}*ca_thread_pool_t;

/**
 * This function creates a newly allocated thread pool.
 *
 * @param num_of_threads The number of worker thread used in this pool.
 * @param thread_pool_handle Handle to newly create thread pool.
 * @return Error code, CA_STATUS_OK if success, else error number.
 */
CAResult_t ca_thread_pool_init(int32_t num_of_threads, ca_thread_pool_t *thread_pool_handle);

/**
 * This function adds a routine to be executed by the thread pool at some future time.
 *
 * @param thread_pool The thread pool structure.
 * @param method The routine to be executed.
 * @param data The data to be passed to the routine.
 *
 * @return CA_STATUS_OK on success.
 * @return Error on failure.
 */
CAResult_t ca_thread_pool_add_task(ca_thread_pool_t thread_pool, ca_thread_func method,
                    void *data);

/**
 * This function stops all the worker threads (stop & exit). And frees all the allocated memory.
 * Function will return only after joining all threads executing the currently scheduled tasks.
 *
 * @param thread_pool The thread pool structure.
 */
void ca_thread_pool_free(ca_thread_pool_t thread_pool);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* CA_THREAD_POOL_H_ */

