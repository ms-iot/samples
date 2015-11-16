/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef BACDEF_H
#define BACDEF_H

#include <stddef.h>
#include <stdint.h>
#include "bacenum.h"
#include "config.h"

#if defined(_MSC_VER)
/* Silence the warnings about unsafe versions of library functions */
/* as we need to keep the code portable */
#pragma warning( disable : 4996)
#endif

/* This stack implements this version of BACnet */
#define BACNET_PROTOCOL_VERSION 1
/* Although this stack can implement a later revision,
 * sometimes another revision is desired */
#ifndef BACNET_PROTOCOL_REVISION
#define BACNET_PROTOCOL_REVISION 14
#endif

/* there are a few dependencies on the BACnet Protocol-Revision */
#if (BACNET_PROTOCOL_REVISION == 0)
#define MAX_ASHRAE_OBJECT_TYPE 18
#define MAX_BACNET_SERVICES_SUPPORTED 35
#elif (BACNET_PROTOCOL_REVISION == 1)
#define MAX_ASHRAE_OBJECT_TYPE 21
#define MAX_BACNET_SERVICES_SUPPORTED 37
#elif (BACNET_PROTOCOL_REVISION == 2)
    /* from 135-2001 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 23
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 3)
#define MAX_ASHRAE_OBJECT_TYPE 23
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 4)
    /* from 135-2004 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 25
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 5)
#define MAX_ASHRAE_OBJECT_TYPE 30
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 6)
#define MAX_ASHRAE_OBJECT_TYPE 31
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 7)
#define MAX_ASHRAE_OBJECT_TYPE 31
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 8)
#define MAX_ASHRAE_OBJECT_TYPE 31
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 9)
    /* from 135-2008 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 38
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 10)
#define MAX_ASHRAE_OBJECT_TYPE 51
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 11)
#define MAX_ASHRAE_OBJECT_TYPE 51
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 12)
    /* from 135-2010 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 51
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 13)
#define MAX_ASHRAE_OBJECT_TYPE 53
#define MAX_BACNET_SERVICES_SUPPORTED 40
#elif (BACNET_PROTOCOL_REVISION == 14)
    /* from 135-2012 version of the BACnet Standard */
#define MAX_ASHRAE_OBJECT_TYPE 55
#define MAX_BACNET_SERVICES_SUPPORTED 41
#elif (BACNET_PROTOCOL_REVISION == 15)
#define MAX_ASHRAE_OBJECT_TYPE 55
#define MAX_BACNET_SERVICES_SUPPORTED 41
#elif (BACNET_PROTOCOL_REVISION == 16)
    /* Addendum 135-2012an, 135-2012at, 135-2012au,
       135-2012av, 135-2012aw, 135-2012ax, 135-2012az */
#define MAX_ASHRAE_OBJECT_TYPE 56
#define MAX_BACNET_SERVICES_SUPPORTED 41
#elif (BACNET_PROTOCOL_REVISION == 17)
    /* Addendum 135-2012ai */
#define MAX_ASHRAE_OBJECT_TYPE 57
#define MAX_BACNET_SERVICES_SUPPORTED 41
#else
#error MAX_ASHRAE_OBJECT_TYPE and MAX_BACNET_SERVICES_SUPPORTED not defined!
#endif

/* largest BACnet Instance Number */
/* Also used as a device instance number wildcard address */
#define BACNET_MAX_INSTANCE (0x3FFFFF)
#define BACNET_INSTANCE_BITS 22
/* large BACnet Object Type */
#define BACNET_MAX_OBJECT (0x3FF)
/* Array index 0=size of array, n=array element n,  MAX=all array elements */
/* 32-bit MAX, to use with uint32_t */
#define BACNET_ARRAY_ALL 0xFFFFFFFFU
/* For device object property references with no device id defined */
#define BACNET_NO_DEV_ID   0xFFFFFFFFu
#define BACNET_NO_DEV_TYPE 0xFFFFu
/* Priority Array for commandable objects */
#define BACNET_NO_PRIORITY 0
#define BACNET_MIN_PRIORITY 1
#define BACNET_MAX_PRIORITY 16

#define BACNET_BROADCAST_NETWORK (0xFFFF)
/* Any size MAC address should be allowed which is less than or
   equal to 7 bytes.  The IPv6 addresses are planned to be handled
   outside this area. */
/* FIXME: mac[] only needs to be as big as our local datalink MAC */
#define MAX_MAC_LEN 7
struct BACnet_Device_Address {
    /* mac_len = 0 is a broadcast address */
    uint8_t mac_len;
    /* note: MAC for IP addresses uses 4 bytes for addr, 2 bytes for port */
    /* use de/encode_unsigned32/16 for re/storing the IP address */
    uint8_t mac[MAX_MAC_LEN];
    /* DNET,DLEN,DADR or SNET,SLEN,SADR */
    /* the following are used if the device is behind a router */
    /* net = 0 indicates local */
    uint16_t net;       /* BACnet network number */
    /* LEN = 0 denotes broadcast MAC ADR and ADR field is absent */
    /* LEN > 0 specifies length of ADR field */
    uint8_t len;        /* length of MAC address */
    uint8_t adr[MAX_MAC_LEN];   /* hwaddr (MAC) address */
};
typedef struct BACnet_Device_Address BACNET_ADDRESS;
/* define a MAC address for manipulation */
struct BACnet_MAC_Address {
    uint8_t len;        /* length of MAC address */
    uint8_t adr[MAX_MAC_LEN];
};
typedef struct BACnet_MAC_Address BACNET_MAC_ADDRESS;

/* note: with microprocessors having lots more code space than memory,
   it might be better to have a packed encoding with a library to
   easily access the data. */
typedef struct BACnet_Object_Id {
    uint16_t type;
    uint32_t instance;
} BACNET_OBJECT_ID;

#define MAX_NPDU (1+1+2+1+MAX_MAC_LEN+2+1+MAX_MAC_LEN+1+1+2)
#define MAX_PDU (MAX_APDU + MAX_NPDU)

#define BACNET_ID_VALUE(bacnet_object_instance, bacnet_object_type) ((((bacnet_object_type) & BACNET_MAX_OBJECT) << BACNET_INSTANCE_BITS) | ((bacnet_object_instance) & BACNET_MAX_INSTANCE))
#define BACNET_INSTANCE(bacnet_object_id_num) ((bacnet_object_id_num)&BACNET_MAX_INSTANCE)
#define BACNET_TYPE(bacnet_object_id_num) (((bacnet_object_id_num) >> BACNET_INSTANCE_BITS ) & BACNET_MAX_OBJECT)

#define BACNET_STATUS_OK (0)
#define BACNET_STATUS_ERROR (-1)
#define BACNET_STATUS_ABORT (-2)
#define BACNET_STATUS_REJECT (-3)

#endif
