//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "networkHandler.h"

// Arduino WiFi Shield includes
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include <string.h>

#include "logger.h"

/**
 * @var ES_NH_TAG
 * @brief Logging tag for module name.
 */
#define ES_NH_TAG "ES_NH"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
/**
 *  ES_MAX_NETWORK_RETRY sets the default number of retry count for network connection.
 */
#define ES_MAX_NETWORK_RETRY (5)

//-----------------------------------------------------------------------------
// Private variables
//-----------------------------------------------------------------------------
static IPAddress enrolleeIP;

/**
 * @var g_retryCounter
 * @brief Retry counter for cancelling network retry. Currently network retry is limited to 5 attempts
 */
static uint16_t g_retryCounter = 0;



//-----------------------------------------------------------------------------
// Private internal function prototypes
//-----------------------------------------------------------------------------
int findNetwork(const char *ssid);
int ConnectToNetwork(const char *ssid, const char *pass);
void printEncryptionType(int thisType);

// Arduino WiFi Shield
// Note : Arduino WiFi Shield currently does NOT support multicast and therefore
// this server will NOT be listening on 224.0.1.187 multicast address.
static const char ARDUINO_WIFI_SHIELD_UDP_FW_VER[] = "1.1.0";

ESResult ConnectToWiFiNetwork(const char *ssid, const char *pass, NetworkEventCallback cb)
{
    char *fwVersion;
    int status = WL_IDLE_STATUS;
    int res;

    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD)
    {
        OC_LOG(ERROR, ES_NH_TAG, "WiFi shield not present");
        return ES_ERROR;
    }

    // Verify that WiFi Shield is running the firmware with all UDP fixes
    fwVersion = WiFi.firmwareVersion();
    OC_LOG_V(INFO, ES_NH_TAG, "WiFi Shield Firmware version %s", fwVersion);
    if (strncmp(fwVersion, ARDUINO_WIFI_SHIELD_UDP_FW_VER, sizeof(ARDUINO_WIFI_SHIELD_UDP_FW_VER))
            != 0)
    {
        OC_LOG(DEBUG, ES_NH_TAG, "!!!!! Upgrade WiFi Shield Firmware version !!!!!!");
        return ES_ERROR;
    }

    //Retry counter is reset everytime the ConnectToWiFiNetwork is invoked
    g_retryCounter = 0;

    OC_LOG_V(INFO, ES_NH_TAG, "Finding SSID: %s", ssid);

    while ((findNetwork(ssid) == 0) && g_retryCounter < ES_MAX_NETWORK_RETRY) // found
    {
        delay(1000);
        g_retryCounter++;
    }

    if(g_retryCounter == ES_MAX_NETWORK_RETRY){
        OC_LOG_V(ERROR, ES_NH_TAG, "Connection to network failed after %d attempts",
                                                                    g_retryCounter);
        return ES_ERROR;
    }

    if (cb != NULL)
    {
        cb(ES_OK);
    }

    if (WiFi.status() == WL_CONNECTED)
        WiFi.disconnect();

    //Retry counter is reset everytime the ConnectToWiFiNetwork is invoked
    g_retryCounter = 0;

    res = ConnectToNetwork(ssid, pass);

    if (res == 0)
    {
        return ES_NETWORKCONNECTED;
    }
    else
    {
        return ES_NETWORKNOTCONNECTED;
    }
}

int findNetwork(const char *ssid)
{
    int res = 0;
    // scan for nearby networks:
    Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1)
    {
        Serial.println("Couldn't get a wifi connection");

        return res;
    }

    // print the list of networks seen:
    Serial.print("number of available networks:");
    Serial.println(numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++)
    {
        Serial.print(thisNet);
        Serial.print(") ");
        Serial.print(WiFi.SSID(thisNet));
        Serial.print("\tEncryption: ");
        printEncryptionType(WiFi.encryptionType(thisNet));

        if (strcmp(WiFi.SSID(thisNet), ssid) == 0)
        {
            res = 1;
        }
    }

    return res;
}

int ConnectToNetwork(const char *ssid, const char *pass)
{
    int status = WL_IDLE_STATUS;

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED && g_retryCounter < ES_MAX_NETWORK_RETRY)
    {
        OC_LOG_V(INFO, ES_NH_TAG, "Attempting to connect to SSID: %s", ssid);

        status = WiFi.begin((char *) ssid, (char *) pass);

        // wait 10 seconds for connection:
        delay(10000);

        g_retryCounter++;
    }

    if(g_retryCounter == ES_MAX_NETWORK_RETRY){
        OC_LOG_V(ERROR, ES_NH_TAG, "Connection to network failed after %d attempts",
                                                                    g_retryCounter);
        return ES_ERROR;
    }

    OC_LOG(DEBUG, ES_NH_TAG, "Connected to wifi");

    enrolleeIP = WiFi.localIP();
    OC_LOG_V(INFO, ES_NH_TAG, "IP Address:  %d.%d.%d.%d", enrolleeIP[0], enrolleeIP[1],
                                                          enrolleeIP[2], enrolleeIP[3]);

    char buf[50];
    sprintf(buf, "IP Address:  %d.%d.%d.%d", enrolleeIP[0], enrolleeIP[1],
                                             enrolleeIP[2], enrolleeIP[3]);
    Serial.println(buf);

    return 0;
}

ESResult getCurrentNetworkInfo(OCConnectivityType targetType, NetworkInfo *info)
{
    if (targetType == CT_ADAPTER_IP && WiFi.status() == WL_CONNECTED)
    {
        info->type = CT_ADAPTER_IP;
        info->ipaddr = WiFi.localIP();
        if(strlen(WiFi.SSID())<=MAXSSIDLEN)
        {
            strcpy(info->ssid, WiFi.SSID());
            return ES_OK;
        }
        else
        {
            return ES_ERROR;
        }
    }

    return ES_ERROR;
}

void printEncryptionType(int thisType)
{
    // read the encryption type and print out the name:
    switch (thisType)
    {
        case ENC_TYPE_WEP:
            Serial.println("WEP");
            break;
        case ENC_TYPE_TKIP:
            Serial.println("WPA");
            break;
        case ENC_TYPE_CCMP:
            Serial.println("WPA2");
            break;
        case ENC_TYPE_NONE:
            Serial.println("None");
            break;
        case ENC_TYPE_AUTO:
            Serial.println("Auto");
            break;
    }
}
