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
#ifndef ADDRESS_H
#define ADDRESS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"
#include "readrange.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void address_init(
        void);

    void address_init_partial(
        void);

    void address_add(
        uint32_t device_id,
        unsigned max_apdu,
        BACNET_ADDRESS * src);

    void address_remove_device(
        uint32_t device_id);

    bool address_get_by_device(
        uint32_t device_id,
        unsigned *max_apdu,
        BACNET_ADDRESS * src);

    bool address_get_by_index(
        unsigned index,
        uint32_t * device_id,
        unsigned *max_apdu,
        BACNET_ADDRESS * src);

    bool address_get_device_id(
        BACNET_ADDRESS * src,
        uint32_t * device_id);

    unsigned address_count(
        void);

    bool address_match(
        BACNET_ADDRESS * dest,
        BACNET_ADDRESS * src);

    bool address_bind_request(
        uint32_t device_id,
        unsigned *max_apdu,
        BACNET_ADDRESS * src);

    void address_add_binding(
        uint32_t device_id,
        unsigned max_apdu,
        BACNET_ADDRESS * src);

    int address_list_encode(
        uint8_t * apdu,
        unsigned apdu_len);

    int rr_address_list_encode(
        uint8_t * apdu,
        BACNET_READ_RANGE_DATA * pRequest);

    void address_set_device_TTL(
        uint32_t device_id,
        uint32_t TimeOut,
        bool StaticFlag);

    void address_cache_timer(
        uint16_t uSeconds);

    void address_mac_init(
        BACNET_MAC_ADDRESS *mac,
        uint8_t *adr,
        uint8_t len);

    bool address_mac_from_ascii(
        BACNET_MAC_ADDRESS *mac,
        char *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
