/**************************************************************************/
/*                                                                        */
/*  File: NetCfg.h                               Copyright (c) 1996,2004  */
/*  Version: 4.0                                 On Time Informatik GmbH  */
/*                                                                        */
/*                                                                        */
/*                                      On Time        /////////////----- */
/*                                    Informatik GmbH /////////////       */
/* --------------------------------------------------/////////////        */
/*                                  Real-Time and System Software         */
/*                                                                        */
/**************************************************************************/

/* Network environment configuration file for the On Time RTOS-32 RTIP-32
   demos.

   By default, the RTIP-32 demos use static IP address assignment. If this is
   not what you want, uncomment either #define AUTO_IP or #define DHCP. In all
   cases, make sure the IP addresses (NetMask, TargetIP, DefaultGateway,
   DNSServer, etc) given in the respective section below are all correct for
   your select and LAN configuration. If you choose to use DHCP, the library
   Dhcpc.lib must also be linked.

   Please define symbol DEVICE_ID to match your target's ethernet
   card and make sure that the card's hardware resource assigments are
   correct (for PCI cards, the drivers will determine this information
   automatically).

*/

/* #define AUTO_IP    // use xn_autoip() to get an IP address */
/* #define DHCP       // if you enable this, you must also link library dhcpc.lib */

#if defined(AUTO_IP)    /* use xn_autoip() to get an IP address */
static BYTE TargetIP[] = { 0, 0, 0, 0 };        /* will be filled at run-time */
static BYTE NetMask[] = { 255, 255, 255, 0 };
static BYTE MinIP[] = { 192, 168, 0, 128 };
static BYTE MaxIP[] = { 192, 168, 0, 255 };
static BYTE DefaultGateway[] = { 192, 168, 0, 1 };      /* set to zero if not available or required */
static BYTE DNSServer[] = { 192, 168, 0, 1 };   /* ditto */
#elif defined(DHCP)     /* use DHCP */
#include <dhcpcapi.h>
static BYTE TargetIP[] = { 0, 0, 0, 0 };        /* will be filled at run-time */
#else /* static IP address assignment (default) */
static BYTE TargetIP[] = { 192, 168, 0, 50 };
static BYTE NetMask[] = { 255, 255, 255, 0 };
static BYTE DefaultGateway[] = { 192, 168, 0, 1 };      /* set to zero if not available or required */
static BYTE DNSServer[] = { 192, 168, 0, 1 };   /* ditto */
#endif

#define DEVICE_ID     DAVICOM_DEVICE    /* define your device type here */

#ifndef DEVICE_ID
#error You must define Ethernet driver/resources and IP address/net mask here
#endif

/* The following values are ignored for PCI devices (the BIOS supplies */
/* them), but they must be set correctly for ISA/PCMCIA systems and for */
/* PCI devices if you do not have a BIOS */

#define ED_IO_ADD     0x300     /* I/O address of the device */
#define ED_IRQ        5 /* IRQ         of the device */
#define ED_MEM_ADD    0 /* Memory Window (only some devices) */

/* Define function to pull in the required driver */

#if DEVICE_ID == NE2000_DEVICE
#define BIND_DRIVER   xn_bind_ne2000
#elif DEVICE_ID == N83815_DEVICE
#define BIND_DRIVER   xn_bind_n83815
#elif DEVICE_ID == TC90X_DEVICE
#define BIND_DRIVER   xn_bind_tc90x
#elif DEVICE_ID == SMC91C9X_DEVICE
#define BIND_DRIVER   xn_bind_smc91c9x
#elif DEVICE_ID == LANCE_DEVICE
#define BIND_DRIVER   xn_bind_rtlance
#elif DEVICE_ID == LANCE_ISA_DEVICE
#define BIND_DRIVER   xn_bind_lance_isa
#elif DEVICE_ID == LAN_CS89X0_DEVICE
#define BIND_DRIVER   xn_bind_cs
#elif DEVICE_ID == I82559_DEVICE
#define BIND_DRIVER   xn_bind_i82559
#elif DEVICE_ID == R8139_DEVICE
#define BIND_DRIVER   xn_bind_r8139
#elif DEVICE_ID == DAVICOM_DEVICE
#define BIND_DRIVER   xn_bind_davicom
#elif DEVICE_ID == RHINE_DEVICE
#define BIND_DRIVER   xn_bind_rhine
#elif DEVICE_ID == AX172_DEVICE
#include <rtusb.h>      /* must also link Rtusb.lib and UsbInit.cpp */
#define BIND_DRIVER   xn_bind_ax172
#elif DEVICE_ID == AX772_DEVICE
#include <rtusb.h>      /* must also link Rtusb.lib and UsbInit.cpp */
#define BIND_DRIVER   xn_bind_ax772
#elif DEVICE_ID == PRISM_DEVICE
#include <wlanapi.h>    /* must also link Wlan.lib */
#define BIND_DRIVER   xn_bind_prism
#elif DEVICE_ID == PRISM_PCMCIA_DEVICE
#include <rtpcmcia.h>
#include <wlanapi.h>    /* must also link Wlan.lib */
#define BIND_DRIVER   xn_bind_prism_pcmcia
#else
#error Invalid DEVICE_ID value
#endif
