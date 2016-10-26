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
#ifndef DCC_H
#define DCC_H

#include <stdint.h>
#include <stdbool.h>
#include "bacenum.h"
#include "bacstr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* return the status */
    BACNET_COMMUNICATION_ENABLE_DISABLE dcc_enable_status(
        void);
    bool dcc_communication_enabled(
        void);
    bool dcc_communication_disabled(
        void);
    bool dcc_communication_initiation_disabled(
        void);
/* return the time */
    uint32_t dcc_duration_seconds(
        void);
/* called every second or so.  If more than one second,
  then seconds should be the number of seconds to tick away */
    void dcc_timer_seconds(
        uint32_t seconds);
/* setup the communication values */
    bool dcc_set_status_duration(
        BACNET_COMMUNICATION_ENABLE_DISABLE status,
        uint16_t minutes);

/* encode service */
    int dcc_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        uint16_t timeDuration,  /* 0=optional */
        BACNET_COMMUNICATION_ENABLE_DISABLE enable_disable,
        BACNET_CHARACTER_STRING * password);    /* NULL=optional */

/* decode the service request only */
    int dcc_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        uint16_t * timeDuration,
        BACNET_COMMUNICATION_ENABLE_DISABLE * enable_disable,
        BACNET_CHARACTER_STRING * password);

#ifdef TEST
#include "ctest.h"
    int dcc_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        uint8_t * invoke_id,
        uint16_t * timeDuration,
        BACNET_COMMUNICATION_ENABLE_DISABLE * enable_disable,
        BACNET_CHARACTER_STRING * password);

    void test_DeviceCommunicationControl(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup RDMS  Device and Network Management Service BIBBs
 * These device management BIBBs prescribe the BACnet capabilities required
 * to interoperably perform the device management functions enumerated in
 * 22.2.1.5 for the BACnet devices defined therein.
 *
 * The network management BIBBs prescribe the BACnet capabilities required to
 * interoperably perform network management functions.
          *//** @defgroup DMDCC Device Management-Device Communication Control (DM-DCC)
 * @ingroup RDMS
 * 16.1 DeviceCommunicationControl Service <br>
 * The DeviceCommunicationControl service is used by a client BACnet-user to
 * instruct a remote device to stop initiating and optionally stop responding
 * to all APDUs (except DeviceCommunicationControl or, if supported,
 * ReinitializeDevice) on the communication network or internetwork for a
 * specified duration of time. This service is primarily used by a human operator
 * for diagnostic purposes. A password may be required from the client
 * BACnet-user prior to executing the service. The time duration may be set to
 * "indefinite," meaning communication must be re-enabled by a
 * DeviceCommunicationControl or, if supported, ReinitializeDevice service,
 * not by time.
          *//** @defgroup NMRC Network Management-Router Configuration (NM-RC)
 * @ingroup RDMS
 * The A device may query and change the configuration of routers and
 * half-routers.
 * The B device responds to router management commands and must meet the
 * requirements for BACnet Routers as stated in Clause 6.
 *
 * 6.4 Network Layer Protocol Messages <br>
 * This subclause describes the format and purpose of the ten BACnet network
 * layer protocol messages. These messages provide the basis for router
 * auto-configuration, router table maintenance, and network layer congestion
 * control.
 */
#endif
