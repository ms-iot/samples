/**************************************************************************
*
* Copyright (C) 2010 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef AUTOMAC_H
#define AUTOMAC_H

#include <stdbool.h>
#include <stdint.h>

/* MS/TP Auto MAC address functionality */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void automac_init(
        void);

    bool automac_free_address_valid(
        uint8_t mac);
    uint8_t automac_free_address_count(
        void);
    uint8_t automac_free_address_mac(
        uint8_t count);
    uint8_t automac_free_address_random(
        void);
    void automac_pfm_set(
        uint8_t mac);
    void automac_token_set(
        uint8_t mac);
    void automac_emitter_set(
        uint8_t mac);
    uint8_t automac_next_station(
        uint8_t mac);
    uint8_t automac_address(
        void);
    void automac_address_set(
        uint8_t mac);
    void automac_address_init(
        void);
    uint16_t automac_time_slot(
        void);
    bool automac_pfm_cycle_complete(
        void);
    bool automac_enabled(
        void);
    void automac_enabled_set(
        bool status);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
