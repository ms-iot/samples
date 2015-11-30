/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2003 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

/** @file keylist.c  Keyed Linked List Library */

/* */
/* This is an enhanced array of pointers to data. */
/* The list is sorted, indexed, and keyed. */
/* The array is much faster than a linked list. */
/* It stores a pointer to data, which you must */
/* malloc and free on your own, or just use */
/* static data */

#include <stdlib.h>

#include "keylist.h"    /* check for valid prototypes */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/******************************************************************** */
/* Generic node routines */
/******************************************************************** */

/* grab memory for a node */
static struct Keylist_Node *NodeCreate(
    void)
{
    return calloc(1, sizeof(struct Keylist_Node));
}

/* grab memory for a list */
static struct Keylist *KeylistCreate(
    void)
{
    return calloc(1, sizeof(struct Keylist));
}

/* check to see if the array is big enough for an addition */
/* or is too big when we are deleting and we can shrink */
/* returns TRUE if success, FALSE if failed */
static int CheckArraySize(
    OS_Keylist list)
{
    int new_size = 0;   /* set it up so that no size change is the default */
    const int chunk = 8;        /* minimum number of nodes to allocate memory for */
    struct Keylist_Node **new_array;    /* new array of nodes, if needed */
    int i;      /* counter */
    if (!list)
        return FALSE;

    /* indicates the need for more memory allocation */
    if (list->count == list->size)
        new_size = list->size + chunk;

    /* allow for shrinking memory */
    else if ((list->size > chunk) && (list->count < (list->size - chunk)))
        new_size = list->size - chunk;
    if (new_size) {

        /* Allocate more room for node pointer array */
        new_array = calloc((size_t) new_size, sizeof(new_array));

        /* See if we got the memory we wanted */
        if (!new_array)
            return FALSE;

        /* copy the nodes from the old array to the new array */
        if (list->array) {
            for (i = 0; i < list->count; i++) {
                new_array[i] = list->array[i];
            }
            free(list->array);
        }
        list->array = new_array;
        list->size = new_size;
    }
    return TRUE;
}


/* find the index of the key that we are looking for */
/* since it is sorted, we can optimize the search */
/* returns TRUE if found, and FALSE not found */
/* returns the found key and the index where it was found in parameters */
/* If the key is not found, the nearest index from the bottom will be returned, */
/* allowing the ability to find where an key should go into the list. */
static int FindIndex(
    OS_Keylist list,
    KEY key,
    int *pIndex)
{
    struct Keylist_Node *node;  /* holds the new node */
    int left = 0;       /* the left branch of tree, beginning of list */
    int right = 0;      /* the right branch on the tree, end of list */
    int index = 0;      /* our current search place in the array */
    KEY current_key = 0;        /* place holder for current node key */
    int status = FALSE; /* return value */
    if (!list || !list->array || !list->count) {
        *pIndex = 0;
        return (FALSE);
    }
    right = list->count - 1;
    /* assume that the list is sorted */
    do {

        /* A binary search */
        index = (left + right) / 2;
        node = list->array[index];
        if (!node)
            break;
        current_key = node->key;
        if (key < current_key)
            right = index - 1;

        else
            left = index + 1;
    }
    while ((key != current_key) && (left <= right));
    if (key == current_key) {
        status = TRUE;
        *pIndex = index;
    }

    else {

        /* where the index should be */
        if (key > current_key)
            *pIndex = index + 1;

        else
            *pIndex = index;
    }
    return (status);
}


/******************************************************************** */
/* list data functions */
/******************************************************************** */
/* inserts a node into its sorted position */
int Keylist_Data_Add(
    OS_Keylist list,
    KEY key,
    void *data)
{
    struct Keylist_Node *node;  /* holds the new node */
    int index = -1;     /* return value */
    int i;      /* counts through the array */

    if (list && CheckArraySize(list)) {
        /* figure out where to put the new node */
        if (list->count) {
            (void) FindIndex(list, key, &index);
            /* Add to the beginning of the list */
            if (index < 0)
                index = 0;

            /* Add to the end of the list */
            else if (index > list->count)
                index = list->count;

            /* Move all the items up to make room for the new one */
            for (i = list->count; i > index; i--) {
                list->array[i] = list->array[i - 1];
            }
        }

        else {
            index = 0;
        }

        /* create and add the node */
        node = NodeCreate();
        if (node) {
            list->count++;
            node->key = key;
            node->data = data;
            list->array[index] = node;
        }
    }
    return index;
}

/* deletes a node specified by its index */
/* returns the data from the node */
void *Keylist_Data_Delete_By_Index(
    OS_Keylist list,
    int index)
{
    struct Keylist_Node *node;
    void *data = NULL;

    if (list && list->array && list->count && (index >= 0) &&
        (index < list->count)) {
        node = list->array[index];
        if (node)
            data = node->data;

        /* move the nodes to account for the deleted one */
        if (list->count == 1) {

            /* There is no node shifting to do */
        }
        /* We are the last one */
        else if (index == (list->count - 1)) {

            /* There is no node shifting to do */
        }
        /* Move all the nodes down one */
        else {
            int i;      /* counter */
            int count = list->count - 1;
            for (i = index; i < count; i++) {
                list->array[i] = list->array[i + 1];
            }
        }
        list->count--;
        if (node)
            free(node);

        /* potentially reduce the size of the array */
        (void) CheckArraySize(list);
    }
    return (data);
}


/* deletes a node specified by its key */
/* returns the data from the node */
void *Keylist_Data_Delete(
    OS_Keylist list,
    KEY key)
{
    void *data = NULL;  /* return value */
    int index;  /* where the node is in the array */

    if (list) {
        if (FindIndex(list, key, &index))
            data = Keylist_Data_Delete_By_Index(list, index);
    }

    return data;
}

/* returns the data from last node, and removes it from the list */
void *Keylist_Data_Pop(
    OS_Keylist list)
{
    void *data = NULL;  /* return value */
    int index;  /* position in the array */

    if (list && list->count) {
        index = list->count - 1;
        data = Keylist_Data_Delete_By_Index(list, index);
    }

    return data;
}

/* returns the data from the node specified by key */
void *Keylist_Data(
    OS_Keylist list,
    KEY key)
{
    struct Keylist_Node *node = NULL;
    int index = 0;      /* used to look up the index of node */

    if (list && list->array && list->count) {
        if (FindIndex(list, key, &index))
            node = list->array[index];
    }

    return node ? node->data : NULL;
}

/* returns the index from the node specified by key */
int Keylist_Index(
    OS_Keylist list,
    KEY key)
{
    int index = -1;      /* used to look up the index of node */

    if (list && list->array && list->count) {
        if (!FindIndex(list, key, &index)) {
            index = -1;
        }
    }

    return index;
}


/* returns the data specified by key */
void *Keylist_Data_Index(
    OS_Keylist list,
    int index)
{
    struct Keylist_Node *node = NULL;

    if (list && list->array && list->count && (index >= 0) &&
        (index < list->count))
        node = list->array[index];

    return node ? node->data : NULL;
}

/* return the key at the given index */
KEY Keylist_Key(
    OS_Keylist list,
    int index)
{
    KEY key = 0;        /* return value */
    struct Keylist_Node *node;

    if (list && list->array && list->count && (index >= 0) &&
        (index < list->count)) {
        node = list->array[index];
        if (node)
            key = node->key;
    }

    return key;
}

/* returns the next empty key from the list */
KEY Keylist_Next_Empty_Key(
    OS_Keylist list,
    KEY key)
{
    int index;

    if (list) {
        while (FindIndex(list, key, &index)) {
            if (KEY_LAST(key))
                break;
            key++;
        }
    }

    return key;
}

/* return the number of nodes in this list */
int Keylist_Count(
    OS_Keylist list)
{
    return list->count;
}

/******************************************************************** */
/* Public List functions */
/******************************************************************** */

/* returns head of the list or NULL on failure. */
OS_Keylist Keylist_Create(
    void)
{
    struct Keylist *list;

    list = KeylistCreate();
    if (list)
        CheckArraySize(list);

    return list;
}

/* delete specified list */
void Keylist_Delete(
    OS_Keylist list)
{       /* list number to be deleted */
    if (list) {
        /* clean out the list */
        while (list->count) {
            (void) Keylist_Data_Delete_By_Index(list, 0);
        }
        if (list->array)
            free(list->array);
        free(list);
    }

    return;
}

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

/* test the FIFO */
static void testKeyListFIFO(
    Test * pTest)
{
    OS_Keylist list;
    KEY key;
    int index;
    char *data1 = "Joshua";
    char *data2 = "Anna";
    char *data3 = "Mary";
    char *data;

    list = Keylist_Create();
    ct_test(pTest, list != NULL);

    key = 0;
    index = Keylist_Data_Add(list, key, data1);
    ct_test(pTest, index == 0);
    index = Keylist_Data_Add(list, key, data2);
    ct_test(pTest, index == 0);
    index = Keylist_Data_Add(list, key, data3);
    ct_test(pTest, index == 0);

    ct_test(pTest, Keylist_Count(list) == 3);

    data = Keylist_Data_Pop(list);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);
    data = Keylist_Data_Pop(list);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data2) == 0);
    data = Keylist_Data_Pop(list);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data3) == 0);
    data = Keylist_Data_Pop(list);
    ct_test(pTest, data == NULL);
    data = Keylist_Data_Pop(list);
    ct_test(pTest, data == NULL);

    Keylist_Delete(list);

    return;
}

/* test the FILO */
static void testKeyListFILO(
    Test * pTest)
{
    OS_Keylist list;
    KEY key;
    int index;
    char *data1 = "Joshua";
    char *data2 = "Anna";
    char *data3 = "Mary";
    char *data;

    list = Keylist_Create();
    ct_test(pTest, list != NULL);

    key = 0;
    index = Keylist_Data_Add(list, key, data1);
    ct_test(pTest, index == 0);
    index = Keylist_Data_Add(list, key, data2);
    ct_test(pTest, index == 0);
    index = Keylist_Data_Add(list, key, data3);
    ct_test(pTest, index == 0);

    ct_test(pTest, Keylist_Count(list) == 3);

    data = Keylist_Data_Delete_By_Index(list, 0);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data3) == 0);

    data = Keylist_Data_Delete_By_Index(list, 0);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data2) == 0);

    data = Keylist_Data_Delete_By_Index(list, 0);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);

    data = Keylist_Data_Delete_By_Index(list, 0);
    ct_test(pTest, data == NULL);

    data = Keylist_Data_Delete_By_Index(list, 0);
    ct_test(pTest, data == NULL);

    Keylist_Delete(list);

    return;
}

static void testKeyListDataKey(
    Test * pTest)
{
    OS_Keylist list;
    KEY key;
    KEY test_key;
    int index;
    char *data1 = "Joshua";
    char *data2 = "Anna";
    char *data3 = "Mary";
    char *data;

    list = Keylist_Create();
    ct_test(pTest, list != NULL);

    key = 1;
    index = Keylist_Data_Add(list, key, data1);
    ct_test(pTest, index == 0);
    test_key = Keylist_Key(list, index);
    ct_test(pTest, test_key == key);

    key = 2;
    index = Keylist_Data_Add(list, key, data2);
    ct_test(pTest, index == 1);
    test_key = Keylist_Key(list, index);
    ct_test(pTest, test_key == key);

    key = 3;
    index = Keylist_Data_Add(list, key, data3);
    ct_test(pTest, index == 2);
    test_key = Keylist_Key(list, index);
    ct_test(pTest, test_key == key);

    ct_test(pTest, Keylist_Count(list) == 3);

    /* look at the data */
    key = 2;
    data = Keylist_Data(list, key);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data2) == 0);

    key = 1;
    data = Keylist_Data(list, key);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);

    key = 3;
    data = Keylist_Data(list, key);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data3) == 0);

    /* work the data */
    key = 2;
    data = Keylist_Data_Delete(list, key);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data2) == 0);
    data = Keylist_Data_Delete(list, key);
    ct_test(pTest, data == NULL);
    ct_test(pTest, Keylist_Count(list) == 2);

    key = 1;
    data = Keylist_Data(list, key);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);

    key = 3;
    data = Keylist_Data(list, key);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data3) == 0);

    /* cleanup */
    do {
        data = Keylist_Data_Pop(list);
    }
    while (data);

    Keylist_Delete(list);

    return;
}

static void testKeyListDataIndex(
    Test * pTest)
{
    OS_Keylist list;
    KEY key;
    int index;
    char *data1 = "Joshua";
    char *data2 = "Anna";
    char *data3 = "Mary";
    char *data;

    list = Keylist_Create();
    ct_test(pTest, list != NULL);

    key = 0;
    index = Keylist_Data_Add(list, key, data1);
    ct_test(pTest, index == 0);
    index = Keylist_Data_Add(list, key, data2);
    ct_test(pTest, index == 0);
    index = Keylist_Data_Add(list, key, data3);
    ct_test(pTest, index == 0);


    ct_test(pTest, Keylist_Count(list) == 3);

    /* look at the data */
    data = Keylist_Data_Index(list, 0);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data3) == 0);

    data = Keylist_Data_Index(list, 1);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data2) == 0);

    data = Keylist_Data_Index(list, 2);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);

    /* work the data */
    data = Keylist_Data_Delete_By_Index(list, 1);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data2) == 0);

    ct_test(pTest, Keylist_Count(list) == 2);

    data = Keylist_Data_Index(list, 0);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data3) == 0);

    data = Keylist_Data_Index(list, 1);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);

    data = Keylist_Data_Delete_By_Index(list, 1);
    ct_test(pTest, data != NULL);
    ct_test(pTest, strcmp(data, data1) == 0);

    data = Keylist_Data_Delete_By_Index(list, 1);
    ct_test(pTest, data == NULL);

    /* cleanup */
    do {
        data = Keylist_Data_Pop(list);
    }
    while (data);

    Keylist_Delete(list);

    return;
}

/* test access of a lot of entries */
static void testKeyListLarge(
    Test * pTest)
{
    int data1 = 42;
    int *data;
    OS_Keylist list;
    KEY key;
    int index;
    const unsigned num_keys = 1024 * 16;

    list = Keylist_Create();
    if (!list)
        return;

    for (key = 0; key < num_keys; key++) {
        index = Keylist_Data_Add(list, key, &data1);

    }
    for (key = 0; key < num_keys; key++) {
        data = Keylist_Data(list, key);
        ct_test(pTest, *data == data1);
    }
    for (index = 0; index < num_keys; index++) {
        data = Keylist_Data_Index(list, index);
        ct_test(pTest, *data == data1);
    }
    Keylist_Delete(list);

    return;
}

/* test access of a lot of entries */
void testKeyList(
    Test * pTest)
{
    bool rc;

    /* individual tests */
    rc = ct_addTestFunction(pTest, testKeyListFIFO);
    assert(rc);
    rc = ct_addTestFunction(pTest, testKeyListFILO);
    assert(rc);
    rc = ct_addTestFunction(pTest, testKeyListDataKey);
    assert(rc);
    rc = ct_addTestFunction(pTest, testKeyListDataIndex);
    assert(rc);
    rc = ct_addTestFunction(pTest, testKeyListLarge);
    assert(rc);
}

#ifdef TEST_KEYLIST
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("keylist", NULL);
    testKeyList(pTest);
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_KEYLIST */
#endif /* TEST */
