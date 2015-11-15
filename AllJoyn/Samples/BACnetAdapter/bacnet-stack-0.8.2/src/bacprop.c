/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 John Goulah

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to 
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330 
 Boston, MA  02111-1307, USA.

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
#include <stdbool.h>
#include <string.h>
#if PRINT_ENABLED
#include <stdio.h>
#endif
#include "bacprop.h"

/** @file bacprop.c  Lookup BACnet Property Tags */

PROP_TAG_DATA bacnet_object_device_property_tag_map[] = {
    {PROP_OBJECT_IDENTIFIER, BACNET_APPLICATION_TAG_OBJECT_ID}
    ,
    {PROP_OBJECT_NAME, BACNET_APPLICATION_TAG_CHARACTER_STRING}
    ,
    {PROP_OBJECT_TYPE, BACNET_APPLICATION_TAG_ENUMERATED}
    ,
    {PROP_SYSTEM_STATUS, BACNET_APPLICATION_TAG_ENUMERATED}
    ,
    {PROP_VENDOR_NAME, BACNET_APPLICATION_TAG_CHARACTER_STRING}
    ,
    {PROP_VENDOR_IDENTIFIER, BACNET_APPLICATION_TAG_UNSIGNED_INT}
    ,
    {PROP_MODEL_NAME, BACNET_APPLICATION_TAG_CHARACTER_STRING}
    ,
    {PROP_FIRMWARE_REVISION, BACNET_APPLICATION_TAG_CHARACTER_STRING}
    ,
    {PROP_APPLICATION_SOFTWARE_VERSION,
        BACNET_APPLICATION_TAG_CHARACTER_STRING}
    ,
    {PROP_PROTOCOL_VERSION, BACNET_APPLICATION_TAG_UNSIGNED_INT}
    ,
    {PROP_PROTOCOL_CONFORMANCE_CLASS, BACNET_APPLICATION_TAG_UNSIGNED_INT}
    ,
    {PROP_PROTOCOL_SERVICES_SUPPORTED, BACNET_APPLICATION_TAG_BIT_STRING}
    ,
    {PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED,
        BACNET_APPLICATION_TAG_BIT_STRING}
    ,
    {PROP_MAX_APDU_LENGTH_ACCEPTED, BACNET_APPLICATION_TAG_UNSIGNED_INT}
    ,
    {PROP_SEGMENTATION_SUPPORTED, BACNET_APPLICATION_TAG_ENUMERATED}
    ,
    {PROP_APDU_TIMEOUT, BACNET_APPLICATION_TAG_UNSIGNED_INT}
    ,
    {PROP_NUMBER_OF_APDU_RETRIES, BACNET_APPLICATION_TAG_UNSIGNED_INT}
    ,
    {-1, -1}
};

signed bacprop_tag_by_index_default(
    PROP_TAG_DATA * data_list,
    signed index,
    signed default_ret)
{
    signed pUnsigned = 0;

    if (data_list) {
        while (data_list->prop_id != -1) {
            if (data_list->prop_id == index) {
                pUnsigned = data_list->tag_id;
                break;
            }
            data_list++;
        }
    }

    return pUnsigned ? pUnsigned : default_ret;
}


signed bacprop_property_tag(
    BACNET_OBJECT_TYPE type,
    signed prop)
{
    switch (type) {
        case OBJECT_DEVICE:
            return
                bacprop_tag_by_index_default
                (bacnet_object_device_property_tag_map, prop, -1);
        default:
#if PRINT_ENABLED
            fprintf(stderr, "Unsupported object type");
#endif
            break;
    }

    return -1;
}
