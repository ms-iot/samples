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

/**
 * @file
 *
 * This file contains IP network handling functionality for Enrollee device
 */

#ifndef ES_NETWORK_HANDLER_H_
#define ES_NETWORK_HANDLER_H_

// Do not remove the include below
#include "Arduino.h"

// Arduino WiFi Shield includes
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include <string.h>
#include "logger.h"
#include "escommon.h"

#define MAXSSIDLEN 33
#define MAXNETCREDLEN 20
#define MAXNUMTYPE 5
#define MAXADDRLEN 15

/*
 * Callback function for updating the Network status to the subscribers
 *
 * @param esResult ESResult provides the current state of the network connection status
 */
typedef void (*NetworkEventCallback)(ESResult esResult);

typedef struct
{
    OCConnectivityType type;
    // for WiFI
    IPAddress ipaddr;
    char ssid[MAXSSIDLEN];
    // for BT, BLE
    byte mac[6];
} NetworkInfo;

ESResult ConnectToWiFiNetwork(const char *ssid, const char *pass, NetworkEventCallback);
ESResult getCurrentNetworkInfo(OCConnectivityType targetType, NetworkInfo *info);

#endif
