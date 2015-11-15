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
#ifndef WHOIS_H
#define WHOIS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* encode service  - use -1 for limit if you want unlimited */
    int whois_encode_apdu(
        uint8_t * apdu,
        int32_t low_limit,
        int32_t high_limit);

    int whois_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        int32_t * pLow_limit,
        int32_t * pHigh_limit);

#ifdef TEST
    int whois_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        int32_t * pLow_limit,
        int32_t * pHigh_limit);

    void testWhoIs(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup DMDDB Device Management-Dynamic Device Binding (DM-DDB)
 * @ingroup RDMS
 * 16.10 Who-Is and I-Am Services <br>
 * The Who-Is service is used by a sending BACnet-user to determine the device
 * object identifier, the network address, or both, of other BACnet devices
 * that share the same internetwork.
 * The Who-Is service is an unconfirmed service. The Who-Is service may be used
 * to determine the device object identifier and network addresses of all devices
 * on the network, or to determine the network address of a specific device whose
 * device object identifier is known, but whose address is not. <br>
 * The I-Am service is also an unconfirmed service. The I-Am service is used to
 * respond to Who-Is service requests. However, the I-Am service request may be
 * issued at any time. It does not need to be preceded by the receipt of a
 * Who-Is service request. In particular, a device may wish to broadcast an I-Am
 * service request when it powers up. The network address is derived either
 * from the MAC address associated with the I-Am service request, if the device
 * issuing the request is on the local network, or from the NPCI if the device
 * is on a remote network.
 */
#endif
