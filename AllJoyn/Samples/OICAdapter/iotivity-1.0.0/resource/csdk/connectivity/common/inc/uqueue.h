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
 * This file contains the APIs for queue to be implemented.
 */

#ifndef U_QUEUE_H_
#define U_QUEUE_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * Queue message format.
 */
typedef struct u_queue_message_t
{
    /** Pointer to message. */
    void *msg;
    /** message size. */
    uint32_t size;
} u_queue_message_t;

typedef struct u_queue_element_t u_queue_element;

/**
 * Queue element format.
 */
struct u_queue_element_t
{
    /** pointer to queue message. */
    u_queue_message_t *message;
    /** Pointer to next queue element. */
    u_queue_element *next;
};

/**
 * Queue structure.
 */
typedef struct u_queue_t
{
    /** Head of the queue. */
    u_queue_element *element;
    /** Number of messages in Queue. */
    uint32_t count;
} u_queue_t;

/**
 * API to creates queue and initializes the elements.
 * @return  u_queue_t pointer if Success, NULL otherwise.
 */
u_queue_t *u_queue_create();

/**
 * Resets and deletes the queue.
 * @param queue- queue pointer.
 * @return ::CA_STATUS_OK if Success, ::CA_STATUS_FAILED otherwise.
 */
CAResult_t u_queue_delete(u_queue_t *queue);

/**
 * Adds message at the end of the queue.
 * @param queue pointer to queue.
 * @param message Pointer to message.
 * @return ::CA_STATUS_OK if Success, ::CA_STATUS_FAILED otherwise.
 */
CAResult_t u_queue_add_element(u_queue_t *queue, u_queue_message_t *message);

/**
 * Returns the first message in the queue and removes queue element.
 * Head is moved to next element.
 * @param queue pointer to queue.
 * @return pointer to Message if Success, NULL otherwise.
 */
u_queue_message_t *u_queue_get_element(u_queue_t *queue);

/**
 * Removes head element of the queue.
 * @param queue pointer to queue.
 * @return ::CA_STATUS_OK if Success, ::CA_STATUS_FAILED otherwise.
 */
CAResult_t u_queue_remove_element(u_queue_t *queue);

/**
 * @param queue pointer to queue.
 * @return number of elements in queue.
 */
uint32_t u_queue_get_size(u_queue_t *queue);

/**
 * Removes all the messages from Queue and reset message count.
 * @param queue pointer to queue.
 * @return ::CA_STATUS_OK if Success, ::CA_STATUS_FAILED otherwise.
 */
CAResult_t u_queue_reset(u_queue_t *queue);

/**
 * Returns the first message in queue, but not remove the element.
 * @param queue pointer to queue.
 * @return pointer to Message if Success, NULL otherwise.
 */
u_queue_message_t *u_queue_get_head(u_queue_t *queue);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* U_QUEUE_H_ */
