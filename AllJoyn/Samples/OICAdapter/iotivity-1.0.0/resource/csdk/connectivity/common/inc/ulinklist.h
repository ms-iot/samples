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

#ifndef U_LINKLIST_H_
#define U_LINKLIST_H_

/**
 * @todo Do performance comparision with array list.
 */

#include <stdint.h>
#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef OICDEFINE
#include <stdbool.h>

#endif

/**
 * link list structure
 */
typedef struct linked_list_data u_linklist_data_t;

typedef struct linked_list_data u_linklist_iterator_t;

typedef struct u_linklist
{
    u_linklist_data_t *list;
    int size;
}u_linklist_t;

struct linked_list_data
{
    void *data;
    u_linklist_data_t *next;
};

/**
 * API to create link list and initializes the elements.
 * @return  u_linklist_t if Success, NULL otherwise.
 */
u_linklist_t *u_linklist_create();

/**
 * Resets and deletes the link list
 * Linklist elements are deleted, Calling function must take care of freeing
 * dynamic memory allocated before freeing the linklist.
 * @param[in,out] list      u_linklist pointer.
 * @return ::CA_STATUS_OK if Success, ::CA_STATUS_INVALID_PARAM if pointer to list is NULL.
 */
CAResult_t u_linklist_free(u_linklist_t **list);

/**
 * Add data to the head of the link list.
 * @param[in/out]  list      pointer of link list.
 * @param[in]      data      pointer of data.
 * @return ::CA_STATUS_OK if Success, ::CA_MEMORY_ALLOC_FAILED if memory allocation fails.
 */
CAResult_t u_linklist_add_head(u_linklist_t *list, void *data);

/**
 * Add data to the tail of the link list.
 * @param[in/out]  list      pointer of link list.
 * @param[in]      data      pointer of data.
 * @return ::CA_STATUS_OK if Success, ::CA_MEMORY_ALLOC_FAILED if memory allocation fails.
 */
CAResult_t u_linklist_add(u_linklist_t *list, void *data);


/**
 * This api deletes node pointed by iterator.
 * Advances iterator to next node in the list.
 * @param[in/out]   list         pointer of link list.
 * @param[in/out]   iter         pointer of iterator pointing to previous node.
 * @return ::CA_STATUS_OK if Success, ::CA_STATUS_INVALID_PARAM if iterator is NULL.
 */
CAResult_t u_linklist_remove(u_linklist_t *list, u_linklist_iterator_t **iter);


/**
 * Returns the length of the link list.
 * @param[in] list               pointer of link list.
 * @return length of the link list.
 */
uint32_t u_linklist_length(const u_linklist_t *list);

/**
 * Initializes the iterator, need to be called before calling u_linklist_get_next.
 * @param[in]       list             pointer of link list.
 * @param[in,out]   iter             iterator of link list.
 * @return NONE
 */
void u_linklist_init_iterator(const u_linklist_t *list, u_linklist_iterator_t **iter);

/**
 * Returns the data of the node iterator points to from the link list.
 * @param[in] iter             iterator of link list.
 * @return the data of node to which iterator is pointing.
 */
void *u_linklist_get_data(const u_linklist_iterator_t *iter);

/**
 * Returns the data of the next node iterator points to from the link list
 * Advances iterator to next node in the list.
 * @param[in,out] iter             iterator of link list.
 * @return the data of next node.
 */
void *u_linklist_get_next(u_linklist_iterator_t **iter);

#ifdef __cplusplus
}
#endif

#endif /* U_LINKLIST_H_ */

