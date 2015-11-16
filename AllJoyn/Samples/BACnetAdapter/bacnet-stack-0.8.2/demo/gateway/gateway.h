/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

#ifndef GATEWAY_H_
#define GATEWAY_H_

/** @file gateway/gateway.h  Header for example gateway (ie, BACnet Router 
 *        and Devices) using the BACnet Stack. */

/** @defgroup GatewayDemo Demo of a BACnet virtual gateway (multiple Device).
 * @ingroup Demos
 * This is a basic demonstration of a BACnet Router with child devices (ie, 
 * gateway) appearing on a virtual BACnet network behind the Router.
 * This is an extension of the ServerDemo project.
 */

/* Device configuration definitions. */
#define FIRST_DEVICE_NUMBER 260001
#define VIRTUAL_DNET  2709      /* your choice of number here */
#define DEV_NAME_BASE "Gateway Demo Device"
#define DEV_DESCR_GATEWAY "Gateway Device and Router"
#define DEV_DESCR_REMOTE  "Routed Remote Device"



#endif /* GATEWAY_H_ */
