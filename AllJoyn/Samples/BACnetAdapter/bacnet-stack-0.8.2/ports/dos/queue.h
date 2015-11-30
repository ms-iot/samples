/*
+----------------------------------------------------+
|                Thunderbird Software                |
+----------------------------------------------------+
| Filespec  :  QUEUE.H                               |
| Date      :  August 30, 1994                       |
| Time      :  5:40 PM                               |
| Revision  :  0.0                                   |
+----------------------------------------------------+
| Programmer:  Scott Andrews                         |
| Address   :  5358 Summit RD SW                     |
| City/State:  Pataskala, Ohio                       |
| Zip       :  43062                                 |
+----------------------------------------------------+
| Released to the Public Domain                      |
+----------------------------------------------------+
*/

#ifndef QUEUE__H
#define QUEUE__H

/* Needed by Serial.C */

typedef struct {
    int size;
    int head;
    int tail;
    int avail;
    char *buffer;
} QUEUE;

#define queue_empty(queue) (queue)->head == (queue)->tail
#define queue_avail(queue) (queue)->avail

QUEUE *alloc_queue(
    int size);
int en_queue(
    QUEUE * queue_ptr,
    char data);
int de_queue(
    QUEUE * queue_ptr);

/* End of Queue.H */

#endif /* QUEUE__H */
