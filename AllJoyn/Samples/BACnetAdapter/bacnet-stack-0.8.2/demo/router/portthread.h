/**
* @file
* @author Andriy Sukhynyuk, Vasyl Tkhir, Andriy Ivasiv
* @date 2012
* @brief Network port storage and handling
*
* @section LICENSE
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*/
#ifndef PORTTHREAD_H
#define PORTTHREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "msgqueue.h"
#include "bacdef.h"
#include "npdu.h"

#define ERROR 1
#define INFO 2
#define DEBUG 3

#define DEBUG_LEVEL 3
#ifdef DEBUG_LEVEL
#define PRINT(debug_level, ...) if(debug_level <= DEBUG_LEVEL) fprintf(stderr, __VA_ARGS__)
#else
#define PRINT(...)
#endif

typedef enum {
    BIP = 1,
    MSTP = 2
} DL_TYPE;

typedef enum {
    INIT,
    INIT_FAILED,
    RUNNING,
    FINISHED
} PORT_STATE;

/* router port thread function */
typedef void *(
    *PORT_FUNC) (
    void *);

typedef enum {
    PARITY_NONE,
    PARITY_EVEN,
    PARITY_ODD
} PARITY;

/* port specific parameters */
typedef union _port_params {
    struct {
        uint16_t port;
    } bip_params;
    struct {
        uint32_t baudrate;
        PARITY parity;
        uint8_t databits;
        uint8_t stopbits;
        uint8_t max_master;
        uint8_t max_frames;
    } mstp_params;
} PORT_PARAMS;

/* list node for reacheble networks */
typedef struct _dnet {
    uint8_t mac[MAX_MAC_LEN];
    uint8_t mac_len;
    uint16_t net;
    bool state; /* enabled or disabled */
    struct _dnet *next;
} DNET;

/* information for routing table */
typedef struct _routing_table_entry {
    uint8_t mac[MAX_MAC_LEN];
    uint8_t mac_len;
    uint16_t net;
    DNET *dnets;
} RT_ENTRY;

typedef struct _port {
    DL_TYPE type;
    PORT_STATE state;
    MSGBOX_ID main_id;  /* same for every router port */
    MSGBOX_ID port_id;  /* different for every router port */
    char *iface;
    PORT_FUNC func;
    RT_ENTRY route_info;
    PORT_PARAMS params;
    struct _port *next; /* pointer to next list node */
} ROUTER_PORT;

extern ROUTER_PORT *head;
extern int port_count;

/* get recieving router port */
ROUTER_PORT *find_snet(
    MSGBOX_ID id);

/* get sending router port */
ROUTER_PORT *find_dnet(
    uint16_t net,
    BACNET_ADDRESS * addr);

/* add reacheble network for specified router port */
void add_dnet(
    RT_ENTRY * route_info,
    uint16_t net,
    BACNET_ADDRESS addr);

void cleanup_dnets(
    DNET * dnets);

#endif /* end of PORTTHREAD_H */
