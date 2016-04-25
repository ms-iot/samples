/******************************************************************
*
* Copyright 2014 Samsung Electronics All Rights Reserved.
*
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************/
#include "caipinterface.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <utility/server_drv.h>
#include <utility/wifi_drv.h>
#include <IPAddress.h>

#include "logger.h"
#include "cacommon.h"
#include "caadapterinterface.h"
#include "caipadapter.h"
#include "caadapterutils.h"

/// This is the max buffer size between Arduino and WiFi Shield
#define ARDUINO_IP_BUFFERSIZE (90)
#define TAG "IPC"

static WiFiUDP Udp;

void CAIPSetUnicastSocket(int socketID)
{

}

void CAIPSetUnicastPort(uint16_t port)
{

}

void CAIPSendData(CAEndpoint_t *endpoint,
                  const void *data, uint32_t dataLength, bool isMulticast)
{
    OIC_LOG(DEBUG, TAG, "IN");

    VERIFY_NON_NULL_VOID(data, TAG, "data");
    VERIFY_NON_NULL_VOID(endpoint, TAG, "endpoint");

    OIC_LOG_V(DEBUG, TAG, "remoteip: %s", endpoint->addr);
    OIC_LOG_V(DEBUG, TAG, "port: %d", endpoint->port);

    uint8_t ip[4] = {0};
    uint16_t parsedPort = 0;
    CAResult_t res = CAParseIPv4AddressInternal(endpoint->addr, ip, sizeof(ip),
                                                &parsedPort);
    if (res != CA_STATUS_OK)
    {
        OIC_LOG_V(ERROR, TAG, "Remote adrs parse fail %d", res);
        return;
    }

    IPAddress remoteIp(ip);
    Udp.beginPacket(remoteIp, endpoint->port);

    uint32_t bytesWritten = 0;
    while (bytesWritten < dataLength)
    {
        // get remaining bytes
        size_t writeCount = dataLength - bytesWritten;
        // write upto max ARDUINO_WIFI_BUFFERSIZE bytes
        writeCount = Udp.write((uint8_t *)data + bytesWritten,
                                (writeCount > ARDUINO_IP_BUFFERSIZE ?
                                 ARDUINO_IP_BUFFERSIZE:writeCount));
        if(writeCount == 0)
        {
            // write failed
            OIC_LOG_V(ERROR, TAG, "Failed after %u", bytesWritten);
            break;
        }
        bytesWritten += writeCount;
    }

    if (Udp.endPacket() == 0)
    {
        OIC_LOG(ERROR, TAG, "Failed to send");
        return;
    }
    OIC_LOG(DEBUG, TAG, "OUT");
    return;
}

