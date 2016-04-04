/******************************************************************
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
#include "uqueue.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "logger.h"
#include "oic_malloc.h"

/**
 * @def NO_MESSAGES
 * @brief Number of messages in the queue
 */
#define NO_MESSAGES 0

/**
 * @def TAG
 * @brief Logging tag for module name
 */
#define TAG "UQUEUE"

u_queue_t *u_queue_create()
{
    u_queue_t *queuePtr = (u_queue_t *) OICMalloc(sizeof(u_queue_t));
    if (NULL == queuePtr)
    {
        OIC_LOG(DEBUG, TAG, "QueueCreate FAIL");
        return NULL;
    }

    queuePtr->count = NO_MESSAGES;
    queuePtr->element = NULL;

    return queuePtr;
}

CAResult_t u_queue_add_element(u_queue_t *queue, u_queue_message_t *message)
{
    u_queue_element *element = NULL;
    u_queue_element *ptr = NULL;

    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueAddElement FAIL, Invalid Queue");
        return CA_STATUS_FAILED;
    }

    if (NULL == message)
    {
        OIC_LOG(DEBUG, TAG, "QueueAddElement : FAIL, NULL Message");
        return CA_STATUS_FAILED;
    }

    element = (u_queue_element *) OICMalloc(sizeof(u_queue_element));
    if (NULL == element)
    {
        OIC_LOG(DEBUG, TAG, "QueueAddElement FAIL, memory allocation failed");
        return CA_MEMORY_ALLOC_FAILED;
    }

    element->message = message;
    element->next = NULL;

    ptr = queue->element;

    if (NULL != ptr)
    {
        while (NULL != ptr->next)
        {
            ptr = ptr->next;
        }

        ptr->next = element;
        queue->count++;

        OIC_LOG_V(DEBUG, TAG, "Queue Count : %d", queue->count);
    }
    else
    {
        if (NO_MESSAGES != queue->count)
        {
            OIC_LOG(DEBUG, TAG, "QueueAddElement : FAIL, count is not zero");

            /* error in queue, free the allocated memory*/
            OICFree(element);
            return CA_STATUS_FAILED;
        }

        queue->element = element;
        queue->count++;
        OIC_LOG_V(DEBUG, TAG, "Queue Count : %d", queue->count);
    }

    return CA_STATUS_OK;
}

u_queue_message_t *u_queue_get_element(u_queue_t *queue)
{
    u_queue_element *element = NULL;
    u_queue_message_t *message = NULL;

    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueAddElement FAIL, Invalid Queue");
        return NULL;
    }

    element = queue->element;

    if (NULL == element)
    {
        return NULL;
    }

    queue->element = element->next;;
    queue->count--;

    message = element->message;
    OICFree(element);
    return message;
}

CAResult_t u_queue_remove_element(u_queue_t *queue)
{
    u_queue_element *next = NULL;
    u_queue_element *remove = NULL;

    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueRemoveElement FAIL, Invalid Queue");
        return CA_STATUS_FAILED;
    }

    remove = queue->element;

    if (NULL == remove)
    {
        OIC_LOG(DEBUG, TAG, "QueueRemoveElement : no messages");
        return CA_STATUS_OK;
    }

    next = remove->next;

    OICFree(remove->message);
    OICFree(remove);

    queue->element = next;
    queue->count--;

    return CA_STATUS_OK;
}

uint32_t u_queue_get_size(u_queue_t *queue)
{
    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueGetSize FAIL, Invalid Queue");
        return NO_MESSAGES;
    }

    return queue->count;
}

CAResult_t u_queue_reset(u_queue_t *queue)
{
    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueReset FAIL, Invalid Queue");
        return CA_STATUS_FAILED;
    }

    if (NO_MESSAGES == queue->count)
    {
        OIC_LOG(DEBUG, TAG, "QueueReset, no elements in the queue");
        return CA_STATUS_OK;
    }

    while (NULL != queue->element)
    {
       u_queue_remove_element(queue);
    }

    if (NO_MESSAGES != queue->count)
    {
        OIC_LOG(DEBUG, TAG, "QueueReset : FAIL, count is non zero");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;

}

CAResult_t u_queue_delete(u_queue_t *queue)
{
    CAResult_t error = CA_STATUS_FAILED;

    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueDelete FAIL, Invalid Queue");
        return CA_STATUS_FAILED;
    }

    error = u_queue_reset(queue);
    if (error != CA_STATUS_OK)
    {
        OIC_LOG(DEBUG, TAG, "QueueDelete : FAIL, error in QueueReset");
        return error;
    }

    OICFree(queue);
    return (CA_STATUS_OK);
}

u_queue_message_t *u_queue_get_head(u_queue_t *queue)
{
    if (NULL == queue)
    {
        OIC_LOG(DEBUG, TAG, "QueueGetHead FAIL, Invalid Queue");
        return NULL;
    }

    if (NULL == queue->element)
    {
        OIC_LOG(DEBUG, TAG, "QueueGetHead : no messages in queue");
        return NULL;
    }
    return queue->element->message;
}

