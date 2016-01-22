/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "bacdef.h"
#include "bacaddr.h"

/** @file bacaddr.c  BACnet Address structure utilities */

void bacnet_address_copy(
    BACNET_ADDRESS * dest,
    BACNET_ADDRESS * src)
{
    int i = 0;

    if (dest && src) {
        dest->mac_len = src->mac_len;
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->mac[i] = src->mac[i];
        }
        dest->net = src->net;
        dest->len = src->len;
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = src->adr[i];
        }
    }
}

bool bacnet_address_same(
    BACNET_ADDRESS * dest,
    BACNET_ADDRESS * src)
{
    uint8_t i = 0;      /* loop counter */
    uint8_t max_len = 0;        /* used for dynamic max */

    if (dest == src)    /* same ? */
        return true;

    if (dest->net != src->net)
        return false;

    if (dest->len != src->len)
        return false;

    max_len = dest->len;
    if (max_len > MAX_MAC_LEN)
        max_len = MAX_MAC_LEN;
    for (i = 0; i < max_len; i++) {
        if (dest->adr[i] != src->adr[i])
            return false;
    }
    if (dest->net == 0) {
        if (dest->mac_len != src->mac_len)
            return false;
        max_len = dest->mac_len;
        if (max_len > MAX_MAC_LEN)
            max_len = MAX_MAC_LEN;
        for (i = 0; i < max_len; i++) {
            if (dest->mac[i] != src->mac[i])
                return false;
        }
    }
    return true;
}
