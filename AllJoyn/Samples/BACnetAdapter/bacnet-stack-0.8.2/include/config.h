/**************************************************************************
*
* Copyright (C) 2004 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

/* Note: these defines can be defined in your makefile or project
   or here or not defined and defaults will be used */

/* declare a single physical layer using your compiler define.
   see datalink.h for possible defines. */
#if !(defined(BACDL_ETHERNET) || defined(BACDL_ARCNET) || defined(BACDL_MSTP) || defined(BACDL_BIP) || defined(BACDL_TEST) || defined(BACDL_ALL))
#define BACDL_BIP
#endif

/* optional configuration for BACnet/IP datalink layers */
#if (defined(BACDL_BIP) || defined(BACDL_ALL))
/* other BIP defines (define as 1 to enable):
    USE_INADDR - uses INADDR_BROADCAST for broadcast and binds using INADDR_ANY
    USE_CLASSADDR = uses IN_CLASSx_HOST where x=A,B,C or D for broadcast
*/
#if !defined(BBMD_ENABLED)
#define BBMD_ENABLED 1
#endif
#endif

/* Enable the Gateway (Routing) functionality here, if desired. */
#if !defined(MAX_NUM_DEVICES)
#ifdef BAC_ROUTING
#define MAX_NUM_DEVICES 3       /* Eg, Gateway + two remote devices */
#else
#define MAX_NUM_DEVICES 1       /* Just the one normal BACnet Device Object */
#endif
#endif


/* Define your processor architecture as
   Big Endian (PowerPC,68K,Sparc) or Little Endian (Intel,AVR)
   ARM and MIPS can be either - what is your setup? */
#if !defined(BIG_ENDIAN)
#define BIG_ENDIAN 0
#endif

/* Define your Vendor Identifier assigned by ASHRAE */
#if !defined(BACNET_VENDOR_ID)
#define BACNET_VENDOR_ID 260
#endif
#if !defined(BACNET_VENDOR_NAME)
#define BACNET_VENDOR_NAME "BACnet Stack at SourceForge"
#endif

/* Max number of bytes in an APDU. */
/* Typical sizes are 50, 128, 206, 480, 1024, and 1476 octets */
/* This is used in constructing messages and to tell others our limits */
/* 50 is the minimum; adjust to your memory and physical layer constraints */
/* Lon=206, MS/TP=480, ARCNET=480, Ethernet=1476, BACnet/IP=1476 */
#if !defined(MAX_APDU)
    /* #define MAX_APDU 50 */
    /* #define MAX_APDU 1476 */
#if defined(BACDL_BIP)
#define MAX_APDU 1476
/* #define MAX_APDU 128 enable this IP for testing readrange so you get the More Follows flag set */
#elif defined (BACDL_ETHERNET)
#define MAX_APDU 1476
#else
#define MAX_APDU 480
#endif
#endif

/* for confirmed messages, this is the number of transactions */
/* that we hold in a queue waiting for timeout. */
/* Configure to zero if you don't want any confirmed messages */
/* Configure from 1..255 for number of outstanding confirmed */
/* requests available. */
#if !defined(MAX_TSM_TRANSACTIONS)
#define MAX_TSM_TRANSACTIONS 255
#endif
/* The address cache is used for binding to BACnet devices */
/* The number of entries corresponds to the number of */
/* devices that might respond to an I-Am on the network. */
/* If your device is a simple server and does not need to bind, */
/* then you don't need to use this. */
#if !defined(MAX_ADDRESS_CACHE)
#define MAX_ADDRESS_CACHE 255
#endif

/* some modules have debugging enabled using PRINT_ENABLED */
#if !defined(PRINT_ENABLED)
#define PRINT_ENABLED 0
#endif

/* BACAPP decodes WriteProperty service requests
   Choose the datatypes that your application supports */
#if !(defined(BACAPP_ALL) || \
    defined(BACAPP_MINIMAL) || \
    defined(BACAPP_NULL) || \
    defined(BACAPP_BOOLEAN) || \
    defined(BACAPP_UNSIGNED) || \
    defined(BACAPP_SIGNED) || \
    defined(BACAPP_REAL) || \
    defined(BACAPP_DOUBLE) || \
    defined(BACAPP_OCTET_STRING) || \
    defined(BACAPP_CHARACTER_STRING) || \
    defined(BACAPP_BIT_STRING) || \
    defined(BACAPP_ENUMERATED) || \
    defined(BACAPP_DATE) || \
    defined(BACAPP_TIME) || \
    defined(BACAPP_LIGHTING_COMMAND) || \
    defined(BACAPP_DEVICE_OBJECT_PROP_REF) || \
    defined(BACAPP_OBJECT_ID))
#endif

#if defined (BACAPP_ALL)
    #define BACAPP_NULL
    #define BACAPP_BOOLEAN
    #define BACAPP_UNSIGNED
    #define BACAPP_SIGNED
    #define BACAPP_REAL
    #define BACAPP_DOUBLE
    #define BACAPP_OCTET_STRING
    #define BACAPP_CHARACTER_STRING
    #define BACAPP_BIT_STRING
    #define BACAPP_ENUMERATED
    #define BACAPP_DATE
    #define BACAPP_TIME
    #define BACAPP_OBJECT_ID
    #define BACAPP_DEVICE_OBJECT_PROP_REF
    #define BACAPP_LIGHTING_COMMAND
#elif defined (BACAPP_MINIMAL)
    #define BACAPP_NULL
    #define BACAPP_BOOLEAN
    #define BACAPP_UNSIGNED
    #define BACAPP_SIGNED
    #define BACAPP_REAL
    #define BACAPP_CHARACTER_STRING
    #define BACAPP_ENUMERATED
    #define BACAPP_DATE
    #define BACAPP_TIME
    #define BACAPP_OBJECT_ID
#endif

/*
** Set the maximum vector type sizes
*/
#ifndef MAX_BITSTRING_BYTES
#define MAX_BITSTRING_BYTES (15)
#endif

#ifndef MAX_CHARACTER_STRING_BYTES
#define MAX_CHARACTER_STRING_BYTES (MAX_APDU-6)
#endif

#ifndef MAX_OCTET_STRING_BYTES
#define MAX_OCTET_STRING_BYTES (MAX_APDU-6)
#endif

/*
** Control the selection of services etc to enable code size reduction for those
** compiler suites which do not handle removing of unused functions in modules
** so well.
**
** We will start with the A type services code first as these are least likely
** to be required in embedded systems using the stack.
*/

/*
** First we see if this is a test build and enable all the services as they
** may be required.
**
** Note: I've left everything enabled here in the main config.h. You should
** use a local copy of config.h with settings configured for your needs to
** make use of the code space reductions.
**/

#ifdef TEST
#define BACNET_SVC_I_HAVE_A    1
#define BACNET_SVC_WP_A        1
#define BACNET_SVC_RP_A        1
#define BACNET_SVC_RPM_A       1
#define BACNET_SVC_DCC_A       1
#define BACNET_SVC_RD_A        1
#define BACNET_SVC_TS_A        1
#define BACNET_SVC_SERVER      0
#define BACNET_USE_OCTETSTRING 1
#define BACNET_USE_DOUBLE      1
#define BACNET_USE_SIGNED      1
#else /* Otherwise define our working set - all enabled here to avoid breaking things */
#define BACNET_SVC_I_HAVE_A    1
#define BACNET_SVC_WP_A        1
#define BACNET_SVC_RP_A        1
#define BACNET_SVC_RPM_A       1
#define BACNET_SVC_DCC_A       1
#define BACNET_SVC_RD_A        1
#define BACNET_SVC_TS_A        1
#define BACNET_SVC_SERVER      0
#define BACNET_USE_OCTETSTRING 1
#define BACNET_USE_DOUBLE      1
#define BACNET_USE_SIGNED      1
#endif

/* Do them one by one */
#ifndef BACNET_SVC_I_HAVE_A     /* Do we send I_Have requests? */
#define BACNET_SVC_I_HAVE_A 0
#endif

#ifndef BACNET_SVC_WP_A /* Do we send WriteProperty requests? */
#define BACNET_SVC_WP_A 0
#endif

#ifndef BACNET_SVC_RP_A /* Do we send ReadProperty requests? */
#define BACNET_SVC_RP_A 0
#endif

#ifndef BACNET_SVC_RPM_A        /* Do we send ReadPropertyMultiple requests? */
#define BACNET_SVC_RPM_A 0
#endif

#ifndef BACNET_SVC_DCC_A        /* Do we send DeviceCommunicationControl requests? */
#define BACNET_SVC_DCC_A 0
#endif

#ifndef BACNET_SVC_RD_A /* Do we send ReinitialiseDevice requests? */
#define BACNET_SVC_RD_A 0
#endif

#ifndef BACNET_SVC_SERVER       /* Are we a pure server type device? */
#define BACNET_SVC_SERVER 1
#endif

#ifndef BACNET_USE_OCTETSTRING  /* Do we need any octet strings? */
#define BACNET_USE_OCTETSTRING 0
#endif

#ifndef BACNET_USE_DOUBLE       /* Do we need any doubles? */
#define BACNET_USE_DOUBLE 0
#endif

#ifndef BACNET_USE_SIGNED       /* Do we need any signed integers */
#define BACNET_USE_SIGNED 0
#endif

#endif
