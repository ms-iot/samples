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
#ifndef VERSION_H
#define VERSION_H

/* This BACnet protocol stack version 0.0.0 - FF.FF.FF */
#ifndef BACNET_VERSION
#define BACNET_VERSION(x,y,z) (((x)<<16)+((y)<<8)+(z))
#endif

#define BACNET_VERSION_TEXT "0.9.0"
#define BACNET_VERSION_CODE BACNET_VERSION(0,9,0)
#define BACNET_VERSION_MAJOR ((BACNET_VERSION_CODE>>16)&0xFF)
#define BACNET_VERSION_MINOR ((BACNET_VERSION_CODE>>8)&0xFF)
#define BACNET_VERSION_MAINTENANCE (BACNET_VERSION_CODE&0xFF)
extern char *BACnet_Version;

#endif
