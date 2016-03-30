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
 * This file contains SDK APIs for device operating in Enrollee Mode of EasySetup
 */

#ifndef EASYSETUP_ENROLLEE_H__
#define EASYSETUP_ENROLLEE_H__

#include "Arduino.h"
#include "escommon.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Callback function for updating the Enrollee OnBoarding and Provisioning status result to the application
 *
 * @param esResult ESResult provides the current state of the Enrollee Device
 */
typedef void (*EventCallback)(ESResult esResult, EnrolleeState enrolleeState);

/**
 * This function Initializes the EasySetup. This API must be called prior to invoking any other API
 *
 * @param networkType       NetworkType on which OnBoarding has to be performed.
 * @param ssid                   SSID of the target SoftAP network to which the Enrollee is connecting.
 * @param passwd              Password of the target SoftAP network to which the Enrollee is connecting
 * @param eventCallback     EventCallback for for updating the Enrollee OnBoarding and Provisioning status
 *                                    result to the application
 * @return ::ES_OK on success, some other value upon failure.
 */
ESResult InitEasySetup(OCConnectivityType networkType, const char *ssid,
                          const char *passwd,
                          EventCallback eventCallback);

/**
 * This function performs initialization of Provisioning and Network resources needed for EasySetup process.
 *
 * @return ::ES_OK on success, some other value upon failure.
 */
ESResult InitProvisioning();

/**
 * This function performs termination of Provisioning and Network resources.
 * Also terminates the IoTivity core stack.
 *
 * @return ::ES_OK on success, some other value upon failure.
 */
ESResult TerminateEasySetup();

#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* EASYSETUP_ENROLLEE_H__ */

