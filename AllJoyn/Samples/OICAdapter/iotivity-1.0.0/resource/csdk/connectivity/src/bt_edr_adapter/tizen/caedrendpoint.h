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
 *
 * This file provides the APIs to send data on established RFCOMM connections.
 */

#ifndef CA_EDR_ENDPOINT_H_
#define CA_EDR_ENDPOINT_H_

#include <bluetooth.h>

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Send data over RFCOMM connection.
 *
 * @param[in]  serverFD          The RFCOMM connection socket file descriptor.
 * @param[in]  data              The data needs to be sent.
 * @param[in]  dataLength        The length of data.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRSendData(int serverFD, const void *data, uint32_t dataLength);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_ENDPOINT_H_ */


