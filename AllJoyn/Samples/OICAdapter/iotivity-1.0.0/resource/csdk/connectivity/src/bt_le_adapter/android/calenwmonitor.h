/* ****************************************************************
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

/**
* @file
* This file contains the APIs for BT LE communications.
*/
#ifndef CA_LENWMONITOR_H_
#define CA_LENWMONITOR_H_

#include "cacommon.h"
#include "cathreadpool.h"
#include "uarraylist.h"
#include "caleinterface.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * set context of application.
 */
void CALENetworkMonitorJNISetContext();

/**
 * initialize JNI object.
 */
void CALENetworkMonitorJniInit();

/**
 * Set this callback for receiving network information from BT stack.
 * @param[in]  callback    Callback to be notified on reception of BT state information.

 */
void CALESetNetStateCallback(CALEDeviceStateChangedCallback callback);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_LENWMONITOR_H_ */

