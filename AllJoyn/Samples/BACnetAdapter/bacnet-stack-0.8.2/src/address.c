/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2004 Steve Karg

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
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "bacaddr.h"
#include "address.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "readrange.h"

/** @file address.c  Handle address binding */

/* This module is used to handle the address binding that */
/* occurs in BACnet.  A device id is bound to a MAC address. */
/* The normal method is using Who-Is, and using the data from I-Am */

static struct Address_Cache_Entry {
    uint8_t Flags;
    uint32_t device_id;
    unsigned max_apdu;
    BACNET_ADDRESS address;
    uint32_t TimeToLive;
} Address_Cache[MAX_ADDRESS_CACHE];

/* State flags for cache entries */

#define BAC_ADDR_IN_USE    1    /* Address cache entry in use */
#define BAC_ADDR_BIND_REQ  2    /* Bind request outstanding for entry */
#define BAC_ADDR_STATIC    4    /* Static address mapping - does not expire */
#define BAC_ADDR_SHORT_TTL 8    /* Oppertunistaclly added address with short TTL */
#define BAC_ADDR_RESERVED  128  /* Freed up but held for caller to fill */

#define BAC_ADDR_SECS_1HOUR 3600        /* 60x60 */
#define BAC_ADDR_SECS_1DAY  86400       /* 60x60x24 */

#define BAC_ADDR_LONG_TIME  BAC_ADDR_SECS_1DAY
#define BAC_ADDR_SHORT_TIME BAC_ADDR_SECS_1HOUR
#define BAC_ADDR_FOREVER    0xFFFFFFFF  /* Permenant entry */

bool address_match(
    BACNET_ADDRESS * dest,
    BACNET_ADDRESS * src)
{
    uint8_t i = 0;
    uint8_t max_len = 0;

    if (dest->mac_len != src->mac_len)
        return false;
    max_len = dest->mac_len;
    if (max_len > MAX_MAC_LEN)
        max_len = MAX_MAC_LEN;
    for (i = 0; i < max_len; i++) {
        if (dest->mac[i] != src->mac[i])
            return false;
    }
    if (dest->net != src->net)
        return false;

    /* if local, ignore remaining fields */
    if (dest->net == 0)
        return true;

    if (dest->len != src->len)
        return false;
    max_len = dest->len;
    if (max_len > MAX_MAC_LEN)
        max_len = MAX_MAC_LEN;
    for (i = 0; i < max_len; i++) {
        if (dest->adr[i] != src->adr[i])
            return false;
    }

    return true;
}

void address_remove_device(
    uint32_t device_id)
{
    struct Address_Cache_Entry *pMatch;

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & BAC_ADDR_IN_USE) != 0) &&
            (pMatch->device_id == device_id)) {
            pMatch->Flags = 0;
            break;
        }
        pMatch++;
    }

    return;
}

/*****************************************************************************
 * Search the cache for the entry nearest expiry and delete it. Mark the     *
 * entry as reserved with a 1 hour TTL and return a pointer to the reserved  *
 * entry. Will not delete a static entry and returns NULL pointer if no      *
 * entry available to free up. Does not check for free entries as it is      *
 * assumed we are calling this due to the lack of those.                     *
 *****************************************************************************/


static struct Address_Cache_Entry *address_remove_oldest(
    void)
{
    struct Address_Cache_Entry *pMatch;
    struct Address_Cache_Entry *pCandidate;
    uint32_t ulTime;

    pCandidate = NULL;
    ulTime = BAC_ADDR_FOREVER - 1;      /* Longest possible non static time to live */

    /* First pass - try only in use and bound entries */

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if ((pMatch->
                Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ |
                    BAC_ADDR_STATIC)) == BAC_ADDR_IN_USE) {
            if (pMatch->TimeToLive <= ulTime) { /* Shorter lived entry found */
                ulTime = pMatch->TimeToLive;
                pCandidate = pMatch;
            }
        }
        pMatch++;
    }

    if (pCandidate != NULL) {   /* Found something to free up */
        pCandidate->Flags = BAC_ADDR_RESERVED;
        pCandidate->TimeToLive = BAC_ADDR_SHORT_TIME;   /* only reserve it for a short while */
        return (pCandidate);
    }

    /* Second pass - try in use and un bound as last resort */
    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if ((pMatch->
                Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ |
                    BAC_ADDR_STATIC)) ==
            ((uint8_t) (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ))) {
            if (pMatch->TimeToLive <= ulTime) { /* Shorter lived entry found */
                ulTime = pMatch->TimeToLive;
                pCandidate = pMatch;
            }
        }
        pMatch++;
    }

    if (pCandidate != NULL) {   /* Found something to free up */
        pCandidate->Flags = BAC_ADDR_RESERVED;
        pCandidate->TimeToLive = BAC_ADDR_SHORT_TIME;   /* only reserve it for a short while */
    }

    return (pCandidate);
}

/** Initialize a BACNET_MAC_ADDRESS
 *
 * @param mac [out] BACNET_MAC_ADDRESS structure
 * @param adr [in] address to initialize, null if empty
 * @param len [in] length of address in bytes
 */
void address_mac_init(
    BACNET_MAC_ADDRESS *mac,
    uint8_t *adr,
    uint8_t len)
{
    uint8_t i = 0;

    if (mac && adr && (len <= sizeof(mac->adr))) {
        for (i = 0; i < len; i++) {
            mac->adr[i] = adr[i];
        }
        mac->len = len;
    } else {
        mac->len = 0;
    }
}

/** Parse an ASCII string for a bacnet-address
 *
 * @param mac [out] BACNET_MAC_ADDRESS structure to store the results
 * @param arg [in] nul terminated ASCII string to parse
 *
 * @return true if the address was parsed
 */
bool address_mac_from_ascii(
    BACNET_MAC_ADDRESS *mac,
    char *arg)
{
    unsigned a[6] = {0}, p = 0;
    uint16_t port = 0;
    int c;
    bool status = false;

    if (!(mac && arg)) {
        return false;
    }
    c = sscanf(arg, "%3u.%3u.%3u.%3u:%5u", &a[0],&a[1],&a[2],&a[3],&p);
    if ((c == 4) || (c == 5)) {
        mac->adr[0] = a[0];
        mac->adr[1] = a[1];
        mac->adr[2] = a[2];
        mac->adr[3] = a[3];
        if (c == 4) {
            port = 0xBAC0;
        } else {
            port = (uint16_t)p;
        }
        encode_unsigned16(&mac->adr[4], port);
        mac->len = 6;
        status = true;
    } else {
        c = sscanf(arg, "%2x:%2x:%2x:%2x:%2x:%2x",
            &a[0],&a[1],&a[2],&a[3],&a[4],&a[5]);
        if (c == 6) {
            mac->adr[0] = a[0];
            mac->adr[1] = a[1];
            mac->adr[2] = a[2];
            mac->adr[3] = a[3];
            mac->adr[4] = a[4];
            mac->adr[5] = a[5];
            mac->len = 6;
            status = true;
        } else if (c == 1) {
            if (a[0] <= 255) {
                mac->adr[0] = a[0];
                mac->len = 1;
                status = true;
            }
        }
    }

    return status;
}

/* File format:
DeviceID MAC SNET SADR MAX-APDU
4194303 05 0 0 50
55555 C0:A8:00:18:BA:C0 26001 19 50
note: useful for MS/TP Slave static binding
*/
static const char *Address_Cache_Filename = "address_cache";

static void address_file_init(
    const char *pFilename)
{
    FILE *pFile = NULL; /* stream pointer */
    char line[256] = { "" };    /* holds line from file */
    long device_id = 0;
    unsigned snet = 0;
    unsigned max_apdu = 0;
    char mac_string[80] = { "" }, sadr_string[80] = {
    ""};
    BACNET_ADDRESS src = { 0 };
    BACNET_MAC_ADDRESS mac = { 0 };
    int index = 0;

    pFile = fopen(pFilename, "r");
    if (pFile) {
        while (fgets(line, (int) sizeof(line), pFile) != NULL) {
            /* ignore comments */
            if (line[0] != ';') {
                if (sscanf(line, "%7ld %79s %5u %79s %4u", &device_id,
                        &mac_string[0], &snet, &sadr_string[0],
                        &max_apdu) == 5) {
                    if (address_mac_from_ascii(&mac, mac_string)) {
                        src.mac_len = mac.len;
                        for (index = 0; index < MAX_MAC_LEN; index++) {
                            src.mac[index] = mac.adr[index];
                        }
                    }
                    src.net = (uint16_t) snet;
                    if (snet) {
                        if (address_mac_from_ascii(&mac, sadr_string)) {
                            src.len = mac.len;
                            for (index = 0; index < MAX_MAC_LEN; index++) {
                                src.adr[index] = mac.adr[index];
                            }
                        }
                    } else {
                        src.len = 0;
                        for (index = 0; index < MAX_MAC_LEN; index++) {
                            src.adr[index] = 0;
                        }
                    }
                    address_add((uint32_t) device_id, max_apdu, &src);
                    address_set_device_TTL((uint32_t) device_id, 0, true);      /* Mark as static entry */
                }
            }
        }
        fclose(pFile);
    }

    return;
}


/****************************************************************************
 * Clear down the cache and make sure the full complement of entries are    *
 * available. Assume no persistance of memory.                              *
 ****************************************************************************/

void address_init(
    void)
{
    struct Address_Cache_Entry *pMatch;

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        pMatch->Flags = 0;
        pMatch++;
    }
    address_file_init(Address_Cache_Filename);

    return;
}

/****************************************************************************
 * Clear down the cache of any non bound, expired  or reserved entries.     *
 * Leave static and unexpired bound entries alone. For use where the cache  *
 * is held in persistant memory which can survive a reset or power cycle.   *
 * This reduces the network traffic on restarts as the cache will have much *
 * of its entries intact.                                                   *
 ****************************************************************************/

void address_init_partial(
    void)
{
    struct Address_Cache_Entry *pMatch;

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if ((pMatch->Flags & BAC_ADDR_IN_USE) != 0) {   /* It's in use so let's check further */
            if (((pMatch->Flags & BAC_ADDR_BIND_REQ) != 0) ||
                (pMatch->TimeToLive == 0))
                pMatch->Flags = 0;
        }

        if ((pMatch->Flags & BAC_ADDR_RESERVED) != 0) { /* Reserved entries should be cleared */
            pMatch->Flags = 0;
        }

        pMatch++;
    }
    address_file_init(Address_Cache_Filename);

    return;
}


/****************************************************************************
 * Set the TTL info for the given device entry. If it is a bound entry we   *
 * set it to static or normal and can change the TTL. If it is unbound we   *
 * can only set the TTL. This is done as a seperate function at the moment  *
 * to avoid breaking the current API.                                       *
 ****************************************************************************/

void address_set_device_TTL(
    uint32_t device_id,
    uint32_t TimeOut,
    bool StaticFlag)
{
    struct Address_Cache_Entry *pMatch;

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & BAC_ADDR_IN_USE) != 0) &&
            (pMatch->device_id == device_id)) {
            if ((pMatch->Flags & BAC_ADDR_BIND_REQ) == 0) {     /* If bound then we have either static or normaal */
                if (StaticFlag) {
                    pMatch->Flags |= BAC_ADDR_STATIC;
                    pMatch->TimeToLive = BAC_ADDR_FOREVER;
                } else {
                    pMatch->Flags &= ~BAC_ADDR_STATIC;
                    pMatch->TimeToLive = TimeOut;
                }
            } else {
                pMatch->TimeToLive = TimeOut;   /* For unbound we can only set the time to live */
            }
            break;      /* Exit now if found at all - bound or unbound */
        }
        pMatch++;
    }
}


bool address_get_by_device(
    uint32_t device_id,
    unsigned *max_apdu,
    BACNET_ADDRESS * src)
{
    struct Address_Cache_Entry *pMatch;
    bool found = false; /* return value */

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & BAC_ADDR_IN_USE) != 0) &&
            (pMatch->device_id == device_id)) {
            if ((pMatch->Flags & BAC_ADDR_BIND_REQ) == 0) {     /* If bound then fetch data */
                *src = pMatch->address;
                *max_apdu = pMatch->max_apdu;
                found = true;   /* Prove we found it */
            }
            break;      /* Exit now if found at all - bound or unbound */
        }
        pMatch++;
    }

    return found;
}

/* find a device id from a given MAC address */

bool address_get_device_id(
    BACNET_ADDRESS * src,
    uint32_t * device_id)
{
    struct Address_Cache_Entry *pMatch;
    bool found = false; /* return value */

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) == BAC_ADDR_IN_USE) {       /* If bound */
            if (bacnet_address_same(&pMatch->address, src)) {
                if (device_id) {
                    *device_id = pMatch->device_id;
                }
                found = true;
                break;
            }
        }
        pMatch++;
    }

    return found;
}

void address_add(
    uint32_t device_id,
    unsigned max_apdu,
    BACNET_ADDRESS * src)
{
    bool found = false; /* return value */
    struct Address_Cache_Entry *pMatch;

    /* Note: Previously this function would ignore bind request
       marked entries and in fact would probably overwrite the first
       bind request entry blindly with the device info which may
       have nothing to do with that bind request. Now it honours the
       bind request if it exists */

    /* existing device or bind request outstanding - update address */
    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & BAC_ADDR_IN_USE) != 0) &&
            (pMatch->device_id == device_id)) {
            pMatch->address = *src;
            pMatch->max_apdu = max_apdu;

            /* Pick the right time to live */

            if ((pMatch->Flags & BAC_ADDR_BIND_REQ) != 0)       /* Bind requested so long time */
                pMatch->TimeToLive = BAC_ADDR_LONG_TIME;
            else if ((pMatch->Flags & BAC_ADDR_STATIC) != 0)    /* Static already so make sure it never expires */
                pMatch->TimeToLive = BAC_ADDR_FOREVER;
            else if ((pMatch->Flags & BAC_ADDR_SHORT_TTL) != 0) /* Opportunistic entry so leave on short fuse */
                pMatch->TimeToLive = BAC_ADDR_SHORT_TIME;
            else
                pMatch->TimeToLive = BAC_ADDR_LONG_TIME;        /* Renewing existing entry */

            pMatch->Flags &= ~BAC_ADDR_BIND_REQ;        /* Clear bind request flag just in case */
            found = true;
            break;
        }
        pMatch++;
    }

    /* new device - add to cache if there is room */
    if (!found) {
        pMatch = Address_Cache;
        while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
            if ((pMatch->Flags & BAC_ADDR_IN_USE) == 0) {
                pMatch->Flags = BAC_ADDR_IN_USE;
                pMatch->device_id = device_id;
                pMatch->max_apdu = max_apdu;
                pMatch->address = *src;
                pMatch->TimeToLive = BAC_ADDR_SHORT_TIME;       /* Opportunistic entry so leave on short fuse */
                found = true;
                break;
            }
            pMatch++;
        }
    }

    /* See if we can squeeze it in */
    if (!found) {
        pMatch = address_remove_oldest();
        if (pMatch != NULL) {
            pMatch->Flags = BAC_ADDR_IN_USE;
            pMatch->device_id = device_id;
            pMatch->max_apdu = max_apdu;
            pMatch->address = *src;
            pMatch->TimeToLive = BAC_ADDR_SHORT_TIME;   /* Opportunistic entry so leave on short fuse */
        }
    }
    return;
}

/* returns true if device is already bound */
/* also returns the address and max apdu if already bound */
bool address_bind_request(
    uint32_t device_id,
    unsigned *max_apdu,
    BACNET_ADDRESS * src)
{
    bool found = false; /* return value */
    struct Address_Cache_Entry *pMatch;

    /* existing device - update address info if currently bound */
    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & BAC_ADDR_IN_USE) != 0) &&
            (pMatch->device_id == device_id)) {
            if ((pMatch->Flags & BAC_ADDR_BIND_REQ) == 0) {     /* Already bound */
                found = true;
                *src = pMatch->address;
                *max_apdu = pMatch->max_apdu;
                if ((pMatch->Flags & BAC_ADDR_SHORT_TTL) != 0) {        /* Was picked up opportunistacilly */
                    pMatch->Flags &= ~BAC_ADDR_SHORT_TTL;       /* Convert to normal entry  */
                    pMatch->TimeToLive = BAC_ADDR_LONG_TIME;    /* And give it a decent time to live */
                }
            }
            return (found);     /* True if bound, false if bind request outstanding */
        }
        pMatch++;
    }

    /* Not there already so look for a free entry to put it in */
    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_RESERVED)) == 0) {
            /* In use and awaiting binding */
            pMatch->Flags = (uint8_t) (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ);
            pMatch->device_id = device_id;
            /* No point in leaving bind requests in for long haul */
            pMatch->TimeToLive = BAC_ADDR_SHORT_TIME;
            /* now would be a good time to do a Who-Is request */
            return (false);
        }
        pMatch++;
    }

    /* No free entries, See if we can squeeze it in by dropping an existing one */
    pMatch = address_remove_oldest();
    if (pMatch != NULL) {
        pMatch->Flags = (uint8_t) (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ);
        pMatch->device_id = device_id;
        /* No point in leaving bind requests in for long haul */
        pMatch->TimeToLive = BAC_ADDR_SHORT_TIME;
    }
    return (false);
}


void address_add_binding(
    uint32_t device_id,
    unsigned max_apdu,
    BACNET_ADDRESS * src)
{
    struct Address_Cache_Entry *pMatch;

    /* existing device or bind request - update address */
    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & BAC_ADDR_IN_USE) != 0) &&
            (pMatch->device_id == device_id)) {
            pMatch->address = *src;
            pMatch->max_apdu = max_apdu;
            /* Clear bind request flag in case it was set */
            pMatch->Flags &= ~BAC_ADDR_BIND_REQ;
            /* Only update TTL if not static */
            if ((pMatch->Flags & BAC_ADDR_STATIC) == 0) {
                /* and set it on a long fuse */
                pMatch->TimeToLive = BAC_ADDR_LONG_TIME;
            }
            break;
        }
        pMatch++;
    }
    return;
}

bool address_get_by_index(
    unsigned index,
    uint32_t * device_id,
    unsigned *max_apdu,
    BACNET_ADDRESS * src)
{
    struct Address_Cache_Entry *pMatch;
    bool found = false; /* return value */

    if (index < MAX_ADDRESS_CACHE) {
        pMatch = &Address_Cache[index];
        if ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) ==
            BAC_ADDR_IN_USE) {
            *src = pMatch->address;
            *device_id = pMatch->device_id;
            *max_apdu = pMatch->max_apdu;
            found = true;
        }
    }

    return found;
}

unsigned address_count(
    void)
{
    struct Address_Cache_Entry *pMatch;
    unsigned count = 0; /* return value */

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        /* Only count bound entries */
        if ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) ==
            BAC_ADDR_IN_USE)
            count++;

        pMatch++;
    }

    return count;
}

/****************************************************************************
 * Build a list of the current bindings for the device address binding      *
 * property.                                                                *
 ****************************************************************************/

int address_list_encode(
    uint8_t * apdu,
    unsigned apdu_len)
{
    int iLen = 0;
    struct Address_Cache_Entry *pMatch;
    BACNET_OCTET_STRING MAC_Address;

    /* FIXME: I really shouild check the length remaining here but it is
       fairly pointless until we have the true length remaining in
       the packet to work with as at the moment it is just MAX_APDU */
    apdu_len = apdu_len;
    /* look for matching address */
    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) ==
            BAC_ADDR_IN_USE) {
            iLen +=
                encode_application_object_id(&apdu[iLen], OBJECT_DEVICE,
                pMatch->device_id);
            iLen +=
                encode_application_unsigned(&apdu[iLen], pMatch->address.net);

            /* pick the appropriate type of entry from the cache */

            if (pMatch->address.len != 0) {
                octetstring_init(&MAC_Address, pMatch->address.adr,
                    pMatch->address.len);
                iLen +=
                    encode_application_octet_string(&apdu[iLen], &MAC_Address);
            } else {
                octetstring_init(&MAC_Address, pMatch->address.mac,
                    pMatch->address.mac_len);
                iLen +=
                    encode_application_octet_string(&apdu[iLen], &MAC_Address);
            }
        }
        pMatch++;
    }

    return (iLen);
}

/****************************************************************************
 * Build a list of the current bindings for the device address binding      *
 * property as required for the ReadsRange functionality.                   *
 * We assume we only get called for "Read All" or "By Position" requests.   *
 *                                                                          *
 * We need to treat the address cache as a contiguous array but in reality  *
 * it could be sparsely populated. We can get the count but we can only     *
 * extract entries by doing a linear scan starting from the first entry in  *
 * the cache and picking them off one by one.                               *
 *                                                                          *
 * We do assume the list cannot change whilst we are accessing it so would  *
 * not be multithread safe if there are other tasks that change the cache.  *
 *                                                                          *
 * We take the simple approach here to filling the buffer by taking a max   *
 * size for a single entry and then stopping if there is less than that     *
 * left in the buffer. You could build each entry in a seperate buffer and  *
 * determine the exact length before copying but this is time consuming,    *
 * requires more memory and would probably only let you sqeeeze one more    *
 * entry in on occasion. The value is calculated as 5 bytes for the device  *
 * ID + 3 bytes for the network number and nine bytes for the MAC address   *
 * oct string to give 17 bytes (the minimum possible is 5 + 2 + 3 = 10).    *
 ****************************************************************************/

#define ACACHE_MAX_ENC 17       /* Maximum size of encoded cache entry, see above */

int rr_address_list_encode(
    uint8_t * apdu,
    BACNET_READ_RANGE_DATA * pRequest)
{
    int iLen = 0;
    int32_t iTemp = 0;
    struct Address_Cache_Entry *pMatch = NULL;
    BACNET_OCTET_STRING MAC_Address;
    uint32_t uiTotal = 0;       /* Number of bound entries in the cache */
    uint32_t uiIndex = 0;       /* Current entry number */
    uint32_t uiFirst = 0;       /* Entry number we started encoding from */
    uint32_t uiLast = 0;        /* Entry number we finished encoding on */
    uint32_t uiTarget = 0;      /* Last entry we are required to encode */
    uint32_t uiRemaining = 0;   /* Amount of unused space in packet */

    /* Initialise result flags to all false */
    bitstring_init(&pRequest->ResultFlags);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_FIRST_ITEM, false);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_LAST_ITEM, false);
    bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_MORE_ITEMS, false);
    /* See how much space we have */
    uiRemaining = (uint32_t) (MAX_APDU - pRequest->Overhead);

    pRequest->ItemCount = 0;    /* Start out with nothing */
    uiTotal = address_count();  /* What do we have to work with here ? */
    if (uiTotal == 0)   /* Bail out now if nowt */
        return (0);

    if (pRequest->RequestType == RR_READ_ALL) {
        /*
         * Read all the array or as much as will fit in the buffer by selecting
         * a range that covers the whole list and falling through to the next
         * section of code
         */
        pRequest->Count = uiTotal;      /* Full list */
        pRequest->Range.RefIndex = 1;   /* Starting at the beginning */
    }

    if (pRequest->Count < 0) {  /* negative count means work from index backwards */
        /*
         * Convert from end index/negative count to
         * start index/positive count and then process as
         * normal. This assumes that the order to return items
         * is always first to last, if this is not true we will
         * have to handle this differently.
         *
         * Note: We need to be careful about how we convert these
         * values due to the mix of signed and unsigned types - don't
         * try to optimise the code unless you understand all the
         * implications of the data type conversions!
         */

        iTemp = pRequest->Range.RefIndex;       /* pull out and convert to signed */
        iTemp += pRequest->Count + 1;   /* Adjust backwards, remember count is -ve */
        if (iTemp < 1) {        /* if count is too much, return from 1 to start index */
            pRequest->Count = pRequest->Range.RefIndex;
            pRequest->Range.RefIndex = 1;
        } else {        /* Otherwise adjust the start index and make count +ve */
            pRequest->Range.RefIndex = iTemp;
            pRequest->Count = -pRequest->Count;
        }
    }

    /* From here on in we only have a starting point and a positive count */

    if (pRequest->Range.RefIndex > uiTotal)     /* Nothing to return as we are past the end of the list */
        return (0);

    uiTarget = pRequest->Range.RefIndex + pRequest->Count - 1;  /* Index of last required entry */
    if (uiTarget > uiTotal)     /* Capped at end of list if necessary */
        uiTarget = uiTotal;

    pMatch = Address_Cache;
    uiIndex = 1;
    while ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) != BAC_ADDR_IN_USE)  /* Find first bound entry */
        pMatch++;

    /* Seek to start position */
    while (uiIndex != pRequest->Range.RefIndex) {
        if ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) == BAC_ADDR_IN_USE) {       /* Only count bound entries */
            pMatch++;
            uiIndex++;
        } else
            pMatch++;
    }

    uiFirst = uiIndex;  /* Record where we started from */
    while (uiIndex <= uiTarget) {
        if (uiRemaining < ACACHE_MAX_ENC) {
            /*
             * Can't fit any more in! We just set the result flag to say there
             * was more and drop out of the loop early
             */
            bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_MORE_ITEMS,
                true);
            break;
        }

        iTemp =
            (int32_t) encode_application_object_id(&apdu[iLen], OBJECT_DEVICE,
            pMatch->device_id);
        iTemp +=
            encode_application_unsigned(&apdu[iLen + iTemp],
            pMatch->address.net);

        /* pick the appropriate type of entry from the cache */

        if (pMatch->address.len != 0) {
            octetstring_init(&MAC_Address, pMatch->address.adr,
                pMatch->address.len);
            iTemp +=
                encode_application_octet_string(&apdu[iLen + iTemp],
                &MAC_Address);
        } else {
            octetstring_init(&MAC_Address, pMatch->address.mac,
                pMatch->address.mac_len);
            iTemp +=
                encode_application_octet_string(&apdu[iLen + iTemp],
                &MAC_Address);
        }

        uiRemaining -= iTemp;   /* Reduce the remaining space */
        iLen += iTemp;  /* and increase the length consumed */

        uiLast = uiIndex;       /* Record the last entry encoded */
        uiIndex++;      /* and get ready for next one */
        pMatch++;
        pRequest->ItemCount++;  /* Chalk up another one for the response count */

        while ((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_BIND_REQ)) != BAC_ADDR_IN_USE)      /* Find next bound entry */
            pMatch++;
    }

    /* Set remaining result flags if necessary */
    if (uiFirst == 1)
        bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_FIRST_ITEM,
            true);

    if (uiLast == uiTotal)
        bitstring_set_bit(&pRequest->ResultFlags, RESULT_FLAG_LAST_ITEM, true);

    return (iLen);
}

/****************************************************************************
 * Scan the cache and eliminate any expired entries. Should be called       *
 * periodically to ensure the cache is managed correctly. If this function  *
 * is never called at all the whole cache is effectivly rendered static and *
 * entries never expire unless explictely deleted.                          *
 ****************************************************************************/

void address_cache_timer(
    uint16_t uSeconds)
{       /* Approximate number of seconds since last call to this function */
    struct Address_Cache_Entry *pMatch;

    pMatch = Address_Cache;
    while (pMatch <= &Address_Cache[MAX_ADDRESS_CACHE - 1]) {
        if (((pMatch->Flags & (BAC_ADDR_IN_USE | BAC_ADDR_RESERVED)) != 0)
            && ((pMatch->Flags & BAC_ADDR_STATIC) == 0)) {      /* Check all entries holding a slot except statics */
            if (pMatch->TimeToLive >= uSeconds)
                pMatch->TimeToLive -= uSeconds;
            else
                pMatch->Flags = 0;
        }

        pMatch++;
    }
}



#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

static void set_address(
    unsigned index,
    BACNET_ADDRESS * dest)
{
    unsigned i;

    for (i = 0; i < MAX_MAC_LEN; i++) {
        dest->mac[i] = index;
    }
    dest->mac_len = MAX_MAC_LEN;
    dest->net = 7;
    dest->len = MAX_MAC_LEN;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        dest->adr[i] = index;
    }
}

static void set_file_address(
    const char *pFilename,
    uint32_t device_id,
    BACNET_ADDRESS * dest,
    uint16_t max_apdu)
{
    unsigned i;
    FILE *pFile = NULL;

    pFile = fopen(pFilename, "w");

    if (pFile) {
        fprintf(pFile, "%lu ", (long unsigned int) device_id);
        for (i = 0; i < dest->mac_len; i++) {
            fprintf(pFile, "%02x", dest->mac[i]);
            if ((i + 1) < dest->mac_len) {
                fprintf(pFile, ":");
            }
        }
        fprintf(pFile, " %hu ", dest->net);
        if (dest->net) {
            for (i = 0; i < dest->len; i++) {
                fprintf(pFile, "%02x", dest->adr[i]);
                if ((i + 1) < dest->len) {
                    fprintf(pFile, ":");
                }
            }
        } else {
            fprintf(pFile, "0");
        }
        fprintf(pFile, " %hu\n", max_apdu);
        fclose(pFile);
    }
}

void testAddressFile(
    Test * pTest)
{
    BACNET_ADDRESS src = { 0 };
    uint32_t device_id = 0;
    unsigned max_apdu = 480;
    BACNET_ADDRESS test_address = { 0 };
    unsigned test_max_apdu = 0;

    /* create a fake address */
    device_id = 55555;
    src.mac_len = 1;
    src.mac[0] = 25;
    src.net = 0;
    src.adr[0] = 0;
    max_apdu = 50;
    set_file_address(Address_Cache_Filename, device_id, &src, max_apdu);
    /* retrieve it from the file, and see if we can find it */
    address_file_init(Address_Cache_Filename);
    ct_test(pTest, address_get_by_device(device_id, &test_max_apdu,
            &test_address));
    ct_test(pTest, test_max_apdu == max_apdu);
    ct_test(pTest, bacnet_address_same(&test_address, &src));

    /* create a fake address */
    device_id = 55555;
    src.mac_len = 6;
    src.mac[0] = 0xC0;
    src.mac[1] = 0xA8;
    src.mac[2] = 0x00;
    src.mac[3] = 0x18;
    src.mac[4] = 0xBA;
    src.mac[5] = 0xC0;
    src.net = 26001;
    src.len = 1;
    src.adr[0] = 25;
    max_apdu = 50;
    set_file_address(Address_Cache_Filename, device_id, &src, max_apdu);
    /* retrieve it from the file, and see if we can find it */
    address_file_init(Address_Cache_Filename);
    ct_test(pTest, address_get_by_device(device_id, &test_max_apdu,
            &test_address));
    ct_test(pTest, test_max_apdu == max_apdu);
    ct_test(pTest, bacnet_address_same(&test_address, &src));

}

void testAddress(
    Test * pTest)
{
    unsigned i, count;
    BACNET_ADDRESS src;
    uint32_t device_id = 0;
    unsigned max_apdu = 480;
    BACNET_ADDRESS test_address;
    uint32_t test_device_id = 0;
    unsigned test_max_apdu = 0;

    /* create a fake address database */
    for (i = 0; i < MAX_ADDRESS_CACHE; i++) {
        set_address(i, &src);
        device_id = i * 255;
        address_add(device_id, max_apdu, &src);
        count = address_count();
        ct_test(pTest, count == (i + 1));
    }

    for (i = 0; i < MAX_ADDRESS_CACHE; i++) {
        device_id = i * 255;
        set_address(i, &src);
        /* test the lookup by device id */
        ct_test(pTest, address_get_by_device(device_id, &test_max_apdu,
                &test_address));
        ct_test(pTest, test_max_apdu == max_apdu);
        ct_test(pTest, bacnet_address_same(&test_address, &src));
        ct_test(pTest, address_get_by_index(i, &test_device_id, &test_max_apdu,
                &test_address));
        ct_test(pTest, test_device_id == device_id);
        ct_test(pTest, test_max_apdu == max_apdu);
        ct_test(pTest, bacnet_address_same(&test_address, &src));
        ct_test(pTest, address_count() == MAX_ADDRESS_CACHE);
        /* test the lookup by MAC */
        ct_test(pTest, address_get_device_id(&src, &test_device_id));
        ct_test(pTest, test_device_id == device_id);
    }

    for (i = 0; i < MAX_ADDRESS_CACHE; i++) {
        device_id = i * 255;
        address_remove_device(device_id);
        ct_test(pTest, !address_get_by_device(device_id, &test_max_apdu,
                &test_address));
        count = address_count();
        ct_test(pTest, count == (MAX_ADDRESS_CACHE - i - 1));
    }
}

#ifdef TEST_ADDRESS
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Address", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAddress);
    assert(rc);
    rc = ct_addTestFunction(pTest, testAddressFile);
    assert(rc);


    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ADDRESS */
#endif /* TEST */
