/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 *
 *
 */

#ifndef __TIZEN_NETWORK_BLUETOOTH_PRODUCT_H__
#define __TIZEN_NETWORK_BLUETOOTH_PRODUCT_H__

#include "bluetooth_type.h"
#include "bluetooth_type_product.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief Sets the manufacturer data of local Bluetooth adapter.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 *
 * @param[in]   data	The manufacturer specific data of the Bluetooth device.
 * @param[in]   len	The length of @a data.Maximaum length is 240 bytes.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #BT_ERROR_NOT_ENABLED  Not enabled
 * @retval #BT_ERROR_OPERATION_FAILED Operation failed
 *
 * @pre The state of local Bluetooth must be #BT_ADAPTER_ENABLED.
 * @post bt_adapter_manufacturer_data_changed_cb() will be invoked
 * if this function returns #BT_ERROR_NONE.
 *
 * @see bt_adapter_manufacturer_data_changed_cb
 * @see bt_adapter_set_manufacturer_data_changed_cb()
 * @see bt_adapter_unset_manufacturer_data_changed_cb()
 */
int bt_adapter_set_manufacturer_data(char *data, int len);

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Registers a callback function to be invoked
 * when the manufacturer data of Bluetooth adapter changes.
 * @since_tizen 2.3
 *
 * @param[in]   callback	The callback function to invoke
 * @param[in]   user_data	The user data to be passed to the callback function
 *
 * @return   0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @pre The Bluetooth service must be initialized with bt_initialize().
 * @post  bt_adapter_manufacturer_data_changed_cb() will be invoked.
 *
 * @see bt_initialize()
 * @see bt_adapter_unset_manufacturer_data_changed_cb()
 */
int bt_adapter_set_manufacturer_data_changed_cb(
		bt_adapter_manufacturer_data_changed_cb callback,
		void *user_data);

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief	Unregisters the callback function.
 * @since_tizen 2.3
 *
 * @return	0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 *
 * @pre The Bluetooth service must be initialized with bt_initialize().
 *
 * @see bt_initialize()
 * @see bt_adapter_set_manufacturer_data_changed_cb()
 */
int bt_adapter_unset_manufacturer_data_changed_cb(void);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief Sets the scan interval and widow, synchronously.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 * @param[in]  scan_params  The parameters of le scanning \n
 *				If NULL is passed, default values which are defined in driver / controller are used.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE Successful
 * @retval #BT_ERROR_NOT_ENABLED Adapter is not enabled
 * @retval #BT_ERROR_INVALID_PARAM Parameter is invalid
 * @pre The state of local Bluetooth must be #BT_ADAPTER_ENABLED.
 * @pre The Bluetooth service must be initialized with bt_initialize().
 * @see bt_initialize()
 */
int bt_adapter_le_set_scan_parameter(bt_adapter_le_scan_params_s *scan_params);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Enables RSSI monitoring and sets threshold for LE/ACL link present with the remote device.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 *
 * @param[in] remote_address The address of the remote Bluetooth device for which RSSI is to be monitored
 * @param[in] link_type Link type for the connection (@c 0 = BR/EDR link, @c 1 = LE link).
 * @param[in] low_threshold Lower threshold value for the LE Link in dBm.
 * @param[in] in_range_threshold In-Range threshold value for the LE Link in dBm.
 * @param[in] cb_enable Callback to be called when RSSI monitoring is enabled.
 * @param[in] user_data_enable Data to be passed to RSSI enable callback.
 * @param[in] cb_alert Callback to receive RSSI Alert values.
 * @param[in] user_data_alert Data to be passed to RSSI Alert callback.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #BT_ERROR_NOT_ENABLED  Not enabled
 * @retval #BT_ERROR_RESOURCE_BUSY  Device or resource busy
 * @retval #BT_ERROR_OPERATION_FAILED  Operation failed
 *
 * @remarks High Threshold value is set to 127.
 *
 * @see bt_device_unset_rssi_alert_cb()
 * @see bt_device_disable_rssi_monitor()
 */
int bt_device_enable_rssi_monitor(const char *remote_address,
		bt_device_connection_link_type_e link_type,
		int low_threshold, int in_range_threshold,
		bt_rssi_monitor_enabled_cb cb_enable, void *user_data_enable,
		bt_rssi_alert_cb cb_alert, void *user_data_alert);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Disables RSSI monitoring for LE/ACL link present with the remote device.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 *
 * @param[in] remote_address The address of the remote Bluetooth device for which RSSI monitoring is to be disabled
 * @param[in] link_type Link type for the connection (@c 0 = BR/EDR link, @c 1 = LE link).
 * @param[in] cb_disable Callback to be called when RSSI monitoring is disabled.
 * @param[in] user_data_enable Data to be passed to RSSI enable callback.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #BT_ERROR_NOT_ENABLED  Not enabled
 * @retval #BT_ERROR_RESOURCE_BUSY  Device or resource busy
 * @retval #BT_ERROR_OPERATION_FAILED  Operation failed
 *
 * @remarks Low Threshold, In-range Threshold and High Threshold value are set to 0 to disable RSSI monitoring.
 * @remarks This also calls bt_device_unset_rssi_alert_cb()
 *
 * @see bt_device_unset_rssi_alert_cb()
 * @see bt_device_enable_rssi_monitor()
 */
int bt_device_disable_rssi_monitor(const char *remote_address,
		bt_device_connection_link_type_e link_type,
		bt_rssi_monitor_enabled_cb cb_disable, void *user_data_disable);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Get Raw RSSI for LE link present with the remote device.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 *
 * @param[in] remote_address The address of the remote Bluetooth device for which RSSI is to be monitored
 * @param[in] link_type Link type for the connection (@c 0 = BR/EDR link, @c 1 = LE link).
 * @param[in] callback Callback to receive Raw RSSI values.
 * @param[in] user_data Data to be passed to Raw RSSI callback.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #BT_ERROR_NOT_ENABLED  Not enabled
 * @retval #BT_ERROR_RESOURCE_BUSY  Device or resource busy
 * @retval #BT_ERROR_OPERATION_FAILED  Operation failed
 *
 * @see bt_device_le_unset_rssi_strength_cb()
 */
int bt_device_get_rssi_strength(const char *remote_address,
		bt_device_connection_link_type_e link_type,
		bt_rssi_strength_cb callback, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Unset the callback to receive RSSI Alert values.
 * @since_tizen 2.3
 *
 * @return 0 on success, otherwise a negative error value.
 *
 * @see bt_device_le_enable_rssi()
 */
int bt_device_unset_rssi_alert_cb(void);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief Notifies the XSAT vendor command to the remote.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 * @param[in] state  The XSAT vendor dependent command string. Ex: "AT+SAT= 00,TY,WA"
 * @return 0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #BT_ERROR_NOT_ENABLED  Not enabled
 * @retval #BT_ERROR_REMOTE_DEVICE_NOT_BONDED  Remote device is not bonded
 * @retval #BT_ERROR_REMOTE_DEVICE_NOT_CONNECTED  Remote device is not connected
 * @retval #BT_ERROR_OPERATION_FAILED  Operation failed
 * @pre The Bluetooth audio device must be connected with bt_audio_connect().
 * @see bt_audio_connect()
 */
int bt_ag_notify_vendor_cmd(const char *command);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Registers a callback function that will be invoked when a XSATvendor AT command is transmitted from Hands-Free.
 * @since_tizen 2.3
 * @param[in] callback The callback function to register
 * @param[in] user_data The user data to be passed to the callback function
 * @return   0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @pre The Bluetooth audio service must be initialized with bt_audio_initialize().
 * @see bt_audio_initialize()
 * @see bt_ag_vendor_cmd_cb()
 * @see bt_ag_unset_vendor_cmd_cb()
 */
int bt_ag_set_vendor_cmd_cb(bt_ag_vendor_cmd_cb callback, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Unregisters a callback function that will be invoked when a XSATvendor AT command is transmitted from Hands-Free
 * @since_tizen 2.3
 * @return   0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @pre The Bluetooth audio service must be initialized with bt_audio_initialize().
 * @see bt_audio_initialize()
 * @see bt_ag_vendor_cmd_cb()
 * @see bt_ag_set_vendor_cmd_cb()
 */
int bt_ag_unset_vendor_cmd_cb(void);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief Checks whether the remoted device is wbs (Wide Band Speech) mode or not.
 * @since_tizen 2.3
 * @param[out] wbs_mode The wbs status: (@c true = wide band speech, @c  false = narrow band speech)
 * @return   0 on success, otherwise a negative error value.
 * @retval #BT_ERROR_NONE  Successful
 * @retval #BT_ERROR_NOT_INITIALIZED  Not initialized
 * @retval #BT_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #BT_ERROR_NOT_ENABLED  Not enabled
 * @retval #BT_ERROR_REMOTE_DEVICE_NOT_CONNECTED  Remote device is not connected
 * @pre The remote device is connected by bt_audio_connect() with #BT_AUDIO_PROFILE_TYPE_HSP_HFP service.
 * @see bt_audio_connect()
 */
int bt_ag_is_wbs_mode(bool *wbs_mode);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_AUDIO_A2DP_MODULE
 * @brief  Sets copy protection. streaming application that needs to have the copy protection for the streaming data, shall invoke this API.
 * @since_tizen 2.3
 * @privlevel platform
 * @privilege %http://tizen.org/privilege/bluetooth.admin
 * @param[in] status - TRUE/FALSE
 * @return  0 on success, otherwise negative error value.
 * @retval  #BT_ERROR_NONE  Successful
 * @retval  #BT_ERROR_OPERATION_FAILED  on failure
 */
int bt_a2dp_set_content_protection(bool status);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TIZEN_NETWORK_BLUETOOTH_PRODUCT_H__
