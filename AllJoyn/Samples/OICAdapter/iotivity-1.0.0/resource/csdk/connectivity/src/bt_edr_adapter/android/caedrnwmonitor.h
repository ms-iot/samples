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
#ifndef CA_EDR_NW_MONITOR_H_
#define CA_EDR_NW_MONITOR_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Set context of application.
 */
void CAEDRNetworkMonitorJNISetContext();

/**
 * Initialize JNI object.
 */
void CAEDRNetworkMonitorJniInit();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_NW_MONITOR_H_ */

