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
#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bacdef.h"
#include "bacstr.h"
#include "bacenum.h"

typedef union BACnetScale_t {
    float Float;
    int32_t Integer;
} BACNET_SCALE_T;

/* structures to hold the data gotten by ReadProperty from the device */
typedef struct object_accumulator_t {
    BACNET_OBJECT_ID Object_Identifier;
    BACNET_CHARACTER_STRING Object_Name;
    BACNET_OBJECT_TYPE Object_Type;
    uint32_t Present_Value;
    BACNET_STATUS_FLAGS Status_Flags;
    BACNET_EVENT_STATE Event_State;
    bool Out_Of_Service;
    BACNET_SCALE_T Scale;
    BACNET_ENGINEERING_UNITS Units;
    uint32_t Max_Pres_Value;
} OBJECT_ACCUMULATOR_T;

typedef struct object_device_t {
    BACNET_OBJECT_ID Object_Identifier;
    BACNET_CHARACTER_STRING Object_Name;
    BACNET_OBJECT_TYPE Object_Type;
    BACNET_DEVICE_STATUS System_Status;
    BACNET_CHARACTER_STRING Vendor_Name;
    uint16_t Vendor_Identifier;
    BACNET_CHARACTER_STRING Model_Name;
    BACNET_CHARACTER_STRING Firmware_Revision;
    BACNET_CHARACTER_STRING Application_Software_Version;
    BACNET_CHARACTER_STRING Location;
    BACNET_CHARACTER_STRING Description;
    uint8_t Protocol_Version;
    uint8_t Protocol_Revision;
    BACNET_BIT_STRING Protocol_Services_Supported;
    BACNET_BIT_STRING Protocol_Object_Types_Supported;
    OS_Keylist Object_List;
    uint32_t Max_APDU_Length_Accepted;
    BACNET_SEGMENTATION Segmentation_Supported;
    uint32_t APDU_Timeout;
    uint8_t Number_Of_APDU_Retries;
    uint32_t Database_Revision;
} OBJECT_DEVICE_T;


#endif
