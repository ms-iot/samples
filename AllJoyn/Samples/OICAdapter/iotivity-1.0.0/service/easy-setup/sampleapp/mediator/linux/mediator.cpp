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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <sstream>

#include "prov_adapter.h"
#include "logger.h"

#define ES_MEDIATOR_TAG "easysetupsample"

int quitFlag = 0;

/* SIGINT handler: set quitFlag to 1 for graceful termination */
void handleSigInt(int signum) {
    if (signum == SIGINT) {
        quitFlag = 1;
    }
}

/**
 * This callback function is used to receive the notifications about the provisioning status
 * In success or failure, ProvisioningInfo structure holds the current state of provisioning
 * and also holds the Enrollee information for which provisioning is requested
 * This function can be used to update the application about the current provisioning status of the Enrollee
 */
void ProvisioningStatusCallback(ProvisioningInfo * provInfo) {
    OIC_LOG_V(INFO, ES_MEDIATOR_TAG, "Enrollee connectivity: %d", provInfo->provDeviceInfo.connType);
    if (provInfo->provStatus == DEVICE_PROVISIONED) {
        OIC_LOG_V(INFO, ES_MEDIATOR_TAG, "Successfully provisioned the Enrollee with IP : %s ",
                  provInfo->provDeviceInfo.addr->addr);
    }
    else {
        OIC_LOG_V(INFO, ES_MEDIATOR_TAG, "Provisioing Failed for the Enrollee with IP : %s",
                  provInfo->provDeviceInfo.addr->addr);
    }
}

static void PrintUsage() {
    OIC_LOG(INFO, ES_MEDIATOR_TAG, "Usage : occlient -d \"192.168.0.20\"");
}

int main(int argc, char **argv) {
    int opt;
    EnrolleeNWProvInfo_t netInfo;
    PrintUsage();
    InitProvProcess();


    RegisterCallback(ProvisioningStatusCallback);

    while ((opt = getopt(argc, argv, "d:s:p:")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(netInfo.netAddressInfo.WIFI.ipAddress, optarg, IPV4_ADDR_SIZE - 1);
                break;
            case 's':
                strncpy(netInfo.netAddressInfo.WIFI.ssid, optarg, NET_WIFI_SSID_SIZE - 1);
                break;
            case 'p':
                strncpy(netInfo.netAddressInfo.WIFI.pwd, optarg, NET_WIFI_PWD_SIZE - 1);
                break;
            default:
                PrintUsage();
                return -1;
        }
    }

    netInfo.connType = CT_ADAPTER_IP;
    OIC_LOG_V(INFO, ES_MEDIATOR_TAG, "IP Address of the Provisioning device is =%s\n",
              netInfo.netAddressInfo.WIFI.ipAddress);
    OIC_LOG_V(INFO, ES_MEDIATOR_TAG, "SSID of the Enroller is =%s\n", netInfo.netAddressInfo.WIFI.ssid);
    OIC_LOG_V(INFO, ES_MEDIATOR_TAG, "Password of the Enroller is =%s\n", netInfo.netAddressInfo.WIFI.pwd);

    StartProvisioning(&netInfo);

    signal(SIGINT, handleSigInt);
    while (!quitFlag) {
        sleep(1);
    }

    ResetProvProcess();
    OIC_LOG(INFO, ES_MEDIATOR_TAG, "Exiting occlient main loop...");

    return 0;
}

