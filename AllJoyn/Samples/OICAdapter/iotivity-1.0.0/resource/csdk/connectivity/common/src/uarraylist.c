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

#include <stdlib.h>
#include <string.h>
#include "uarraylist.h"
#include "logger.h"
#include "oic_malloc.h"

#define TAG "UARRAYLIST"

/**
 * Use this default capacity when initialized
 */
#define U_ARRAYLIST_DEFAULT_CAPACITY 1

u_arraylist_t *u_arraylist_create()
{
    u_arraylist_t *list = (u_arraylist_t *) OICCalloc(1, sizeof(u_arraylist_t));
    if (!list)
    {
        OIC_LOG(DEBUG, TAG, "Out of memory");
        return NULL;
    }

    list->capacity = U_ARRAYLIST_DEFAULT_CAPACITY;
    list->length = 0;

    list->data = (void **) OICMalloc(list->capacity * sizeof(list->data[0]));
    if (!list->data)
    {
        OIC_LOG(DEBUG, TAG, "Out of memory");
        OICFree(list);
        return NULL;
    }
    return list;
}

void u_arraylist_free(u_arraylist_t **list)
{
    if (!list || !(*list))
    {
        return;
    }

    OICFree((*list)->data);
    OICFree(*list);

    *list = NULL;
}

void u_arraylist_reserve(u_arraylist_t *list, size_t count)
{
    if (list && (count > list->capacity))
    {
        void *tmp = OICRealloc(list->data, count * sizeof(list->data[0]));
        if (!tmp)
        {
            OIC_LOG(DEBUG, TAG, "Memory reallocation failed.");
            // Note that this is considered non-fatal.
        }
        else
        {
            list->data = (void **) tmp;
            list->capacity = count;
        }
    }
}

void u_arraylist_shrink_to_fit(u_arraylist_t *list)
{
    if (!list)
    {
        return;
    }

    if ((list->capacity > list->length)
        && (list->length >= U_ARRAYLIST_DEFAULT_CAPACITY))
    {
        void *tmp = OICRealloc(list->data,
                               list->length * sizeof(list->data[0]));
        if (!tmp)
        {
            OIC_LOG(DEBUG, TAG, "Memory reallocation failed.");
            // Considered non-fatal as this call is non-binding.
        }
        else
        {
            list->data = (void **) tmp;
            list->capacity = list->length;
        }
    }
}

void *u_arraylist_get(const u_arraylist_t *list, uint32_t index)
{
    if (!list )
    {
        return NULL;
    }

    if ((index < list->length) && (list->data))
    {
        return list->data[index];
    }

    return NULL;
}

bool u_arraylist_add(u_arraylist_t *list, void *data)
{
    if (!list)
    {
        return false;
    }

    if (list->capacity <= list->length)
    {
        // Does a non-FP calcuation of the 1.5 growth factor. Helpful for
        // certain limited platforms.
        size_t new_capacity = ((list->capacity * 3) + 1) / 2;

        // In case the re-alloc returns null, use a local variable to avoid
        // losing the current block of memory.
        void *tmp = OICRealloc(list->data,
                               new_capacity * sizeof(list->data[0]));
        if (!tmp)
        {
            OIC_LOG(DEBUG, TAG, "Memory reallocation failed.");
            return false;
        }
        list->data = (void **) tmp;
        memset(list->data + list->capacity, 0,
               (new_capacity - list->capacity) * sizeof(list->data[0]));
        list->capacity = (uint32_t)new_capacity;
    }

    list->data[list->length] = data;
    list->length++;

    return true;
}

void *u_arraylist_remove(u_arraylist_t *list, uint32_t index)
{
    void *removed = NULL;

    if (!list || (index >= list->length))
    {
        return NULL;
    }

    removed = list->data[index];

    if (index < list->length - 1)
    {
        memmove(&list->data[index],
                &list->data[index + 1],
                (list->length - index - 1) * sizeof(list->data[0]));
    }

    list->length--;

    return removed;
}

uint32_t u_arraylist_length(const u_arraylist_t *list)
{
    if (!list)
    {
        OIC_LOG(DEBUG, TAG, "Invalid Parameter");
        return 0;
    }
    return list->length;
}

bool u_arraylist_contains(const u_arraylist_t *list, const void *data)
{
    if (!list)
    {
        return false;
    }

    for (uint32_t i = 0; i < list->length; i++)
    {
        if (data == list->data[i])
        {
            return true;
        }
    }

    return false;
}

// Assumes elements are shallow (have no pointers to allocated memory)
void u_arraylist_destroy(u_arraylist_t *list)
{
    if (!list)
    {
        return;
    }
    for (uint32_t i = 0; i < list->length; i++)
    {
        OICFree(list->data[i]);
    }
    (void)u_arraylist_free(&list);
}
