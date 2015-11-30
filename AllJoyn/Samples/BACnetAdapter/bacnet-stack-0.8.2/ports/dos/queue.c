/*
+----------------------------------------------------+
|               Thunderbird Software                 |
+----------------------------------------------------+
| Filespec  :  Queue.c                               |
| Date      :  September 29, 1994                    |
| Time      :  10:16AM                               |
| Revision  :  1.0                                   |
+----------------------------------------------------+
| Programmer:  Scott Andrews                         |
| Address   :  5358 Summit RD SW                     |
| City/State:  Pataskala, Ohio                       |
| Zip       :  43062                                 |
+----------------------------------------------------+
| Released to the Public Domain                      |
+----------------------------------------------------+
*/

#include <stdlib.h>

#include "queue.h"

QUEUE *alloc_queue(
    int size)
{
    QUEUE *retval;
    retval = (QUEUE *) malloc(sizeof(QUEUE) + (size_t) size);
    if ((QUEUE *) 0 != retval) {
        retval->size = size;
        retval->head = 0;
        retval->tail = 0;
        retval->avail = size;
        retval->buffer = ((char *) retval) + sizeof(QUEUE);
    }
    return retval;
}

int en_queue(
    QUEUE * queue,
    char data)
{
    int retval = -1;
    if (0 != queue->avail) {
        *(queue->buffer + queue->head) = data;
        queue->head += 1;
        if (queue->head == queue->size)
            queue->head = 0;
        queue->avail -= 1;
        retval = queue->avail;
    }
    return retval;
}

int de_queue(
    QUEUE * queue)
{
    int retval = -1;
    if (queue->avail != queue->size) {
        retval = *(queue->buffer + queue->tail);
        queue->tail += 1;
        if (queue->tail == queue->size)
            queue->tail = 0;
        queue->avail += 1;
    }
    return retval;
}

/* End of Queue.c */
