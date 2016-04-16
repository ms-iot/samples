/* ****************************************************************
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
#include <stdio.h>
#include <stdlib.h>
#include "ulinklist.h"
#include "logger.h"
#include "oic_malloc.h"
#include "caadapterutils.h"

/**
 * Logging tag for module name.
 */
#define TAG "ULINKLIST"

u_linklist_t *u_linklist_create()
{
    u_linklist_t *header = (u_linklist_t *)OICMalloc(sizeof(u_linklist_t));
    if (!header)
    {
        OIC_LOG(ERROR, TAG, "Out of memory");
        return NULL;
    }
    header->list=NULL;
    header->size=0;
    return header;
}

CAResult_t u_linklist_add_head(u_linklist_t *linklist, void *data)
{
    VERIFY_NON_NULL(linklist, TAG, "list is null");
    VERIFY_NON_NULL(data, TAG, "data is null");

    u_linklist_data_t *add_node = NULL;
    add_node = (u_linklist_data_t *) OICMalloc(sizeof(u_linklist_data_t));
    if (NULL == add_node)
    {
        OIC_LOG(DEBUG, TAG, "LinklistAdd FAIL, memory allocation failed");
        return CA_MEMORY_ALLOC_FAILED;
    }
    add_node->data = data;
    add_node->next = linklist->list;
    linklist->list = add_node;
    linklist->size += 1;
    return CA_STATUS_OK;
}

CAResult_t u_linklist_add(u_linklist_t *linklist, void *data)
{
    VERIFY_NON_NULL(linklist, TAG, "list is null");
    VERIFY_NON_NULL(data, TAG, "data is null");

    u_linklist_data_t *add_node = NULL;
    u_linklist_data_t *node = linklist->list;
    add_node = (u_linklist_data_t *) OICMalloc(sizeof(u_linklist_data_t));
    if (NULL == add_node)
    {
        OIC_LOG(DEBUG, TAG, "LinklistAdd FAIL, memory allocation failed");
        return CA_MEMORY_ALLOC_FAILED;
    }

    add_node->data = data;
    add_node->next = NULL;

    if (NULL == node)
    {
        linklist->list = add_node;
        linklist->size += 1;
    }
    else
    {
        //else loop through the list and find the last node, insert next to it
        while (true)
        {
            if(node->next == NULL)
            {
                node->next = add_node;
                linklist->size += 1;
                break;
            }
            node = node->next;
        };
    }

    return CA_STATUS_OK;
}

CAResult_t u_linklist_free(u_linklist_t **linklist)
{
    VERIFY_NON_NULL(linklist, TAG, "linklist is null");
    if (!(*linklist))
    {
        OIC_LOG(DEBUG, TAG, "List is already Empty");
        return CA_STATUS_OK;
    }

    u_linklist_data_t *free_node=NULL;
    while((*linklist)->size)
    {
        free_node = (*linklist)->list;
        (*linklist)->list = (*linklist)->list->next;

        if(free_node != NULL)
        {
            OICFree(free_node);
            free_node=NULL;
        }

        (*linklist)->size -= 1;
    }
    *linklist=NULL;

    return CA_STATUS_OK;
}

CAResult_t u_linklist_remove(u_linklist_t *linklist, u_linklist_iterator_t **iter)
{
    VERIFY_NON_NULL(linklist, TAG, "list is null");
    VERIFY_NON_NULL(iter, TAG, "iterator is null");

    if (NULL == *iter)
    {
        return CA_STATUS_INVALID_PARAM;
    }

    // When node to be deleted is head node
    if (linklist->list == *iter)
    {
        // store address of next node
        linklist->list = linklist->list->next;

        // free memory
        linklist->size -=1;
        OICFree(*iter);

        *iter = linklist->list;

        return CA_STATUS_OK;
    }


    // When not first node, follow the normal deletion process find the previous node
    u_linklist_data_t *prev = linklist->list;
    while(NULL != prev->next && prev->next != *iter)
    {
        prev = prev->next;
    }

    // Check if node really exists in Linked List
    if (NULL == prev->next)
    {
        OIC_LOG(ERROR, TAG, " Given node is not present in Linked List\n");
        return CA_STATUS_FAILED;
    }

    // Remove node from Linked List
    prev->next = prev->next->next;
    linklist->size -=1;
    OICFree(*iter);
    *iter = prev->next;

    return CA_STATUS_OK;
}


uint32_t u_linklist_length(const u_linklist_t *linklist)
{
    if (NULL == linklist)
    {
        OIC_LOG(ERROR, TAG, "linklist is NULL");
        return 0;
    }
    return linklist->size;
}

void u_linklist_init_iterator(const u_linklist_t *linklist, u_linklist_iterator_t **iter)
{
    VERIFY_NON_NULL_VOID(linklist, TAG, "list is null");
    VERIFY_NON_NULL_VOID(iter, TAG, "iterator is null");

    *iter = linklist->list;
}

void *u_linklist_get_data(const u_linklist_iterator_t *iter)
{
    VERIFY_NON_NULL_RET(iter, TAG, "iterator is null", NULL);

    return iter->data;
}

void *u_linklist_get_next(u_linklist_iterator_t **iter)
{
    VERIFY_NON_NULL_RET(iter, TAG, "iterator is null", NULL);
    *iter = (*iter)->next;

    if (NULL != *iter)
    {
        return (*iter)->data;
    }
    else
    {
        return NULL;
    }
}
