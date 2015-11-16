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
#include "ethernet.h"
#include "bip.h"
#include "bvlc.h"
#include "arcnet.h"
#include "dlmstp.h"
#include "datalink.h"
#include <string.h>

/** @file datalink.c  Optional run-time assignment of datalink transport */

#if defined(BACDL_ALL) || defined FOR_DOXYGEN
/* Function pointers - point to your datalink */

/** Function template to Initialize the DataLink services at the given interface.
 * @ingroup DLTemplates
 *
 * @note For Linux, ifname is eth0, ath0, arc0, ttyS0, and others.
         For Windows, ifname is the COM port or dotted ip address of the interface.

 * @param ifname [in] The named interface to use for the network layer.
 * @return True if the interface is successfully initialized,
 *         else False if the initialization fails.
 */
bool(*datalink_init) (char *ifname);

/** Function template to send a packet via the DataLink.
 * @ingroup DLTemplates
 *
 * @param dest [in] Destination address.
 * @param npdu_data [in] The NPDU header (Network) information.
 * @param pdu [in] Buffer of data to be sent - may be null.
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */
int (
    *datalink_send_pdu) (
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * npdu_data,
    uint8_t * pdu,
    unsigned pdu_len);

uint16_t(*datalink_receive) (BACNET_ADDRESS * src, uint8_t * pdu,
    uint16_t max_pdu, unsigned timeout);

/** Function template to close the DataLink services and perform any cleanup.
 * @ingroup DLTemplates
 */
void (
    *datalink_cleanup) (
    void);

void (
    *datalink_get_broadcast_address) (
    BACNET_ADDRESS * dest);

void (
    *datalink_get_my_address) (
    BACNET_ADDRESS * my_address);

void datalink_set(
    char *datalink_string)
{
    if (strcasecmp("bip", datalink_string) == 0) {
        datalink_init = bip_init;
        datalink_send_pdu = bip_send_pdu;
        datalink_receive = bip_receive;
        datalink_cleanup = bip_cleanup;
        datalink_get_broadcast_address = bip_get_broadcast_address;
        datalink_get_my_address = bip_get_my_address;
    } else if (strcasecmp("bvlc", datalink_string) == 0) {
        datalink_init = bip_init;
        datalink_send_pdu = bvlc_send_pdu;
        datalink_receive = bvlc_receive;
        datalink_cleanup = bip_cleanup;
        datalink_get_broadcast_address = bip_get_broadcast_address;
        datalink_get_my_address = bip_get_my_address;
    } else if (strcasecmp("ethernet", datalink_string) == 0) {
        datalink_init = ethernet_init;
        datalink_send_pdu = ethernet_send_pdu;
        datalink_receive = ethernet_receive;
        datalink_cleanup = ethernet_cleanup;
        datalink_get_broadcast_address = ethernet_get_broadcast_address;
        datalink_get_my_address = ethernet_get_my_address;
    } else if (strcasecmp("arcnet", datalink_string) == 0) {
        datalink_init = arcnet_init;
        datalink_send_pdu = arcnet_send_pdu;
        datalink_receive = arcnet_receive;
        datalink_cleanup = arcnet_cleanup;
        datalink_get_broadcast_address = arcnet_get_broadcast_address;
        datalink_get_my_address = arcnet_get_my_address;
    } else if (strcasecmp("mstp", datalink_string) == 0) {
        datalink_init = dlmstp_init;
        datalink_send_pdu = dlmstp_send_pdu;
        datalink_receive = dlmstp_receive;
        datalink_cleanup = dlmstp_cleanup;
        datalink_get_broadcast_address = dlmstp_get_broadcast_address;
        datalink_get_my_address = dlmstp_get_my_address;
    }
}
#endif
