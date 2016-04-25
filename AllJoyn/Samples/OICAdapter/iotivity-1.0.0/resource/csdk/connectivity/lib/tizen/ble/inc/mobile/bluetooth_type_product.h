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

#ifndef __TIZEN_NETWORK_BLUETOOTH_TYPE_PRODUCT_H__
#define __TIZEN_NETWORK_BLUETOOTH_TYPE_PRODUCT_H__

 #ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth adapter le scan type.
 */
typedef enum
{
	BT_ADAPTER_LE_PASSIVE_SCAN = 0x00,
	BT_ADAPTER_LE_ACTIVE_SCAN
} bt_adapter_le_scan_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief Structure of le scan parameters
 *
 * @see bt_adapter_le_set_scan_parameter()
 * @see bt_adapter_le_start_device_discovery()
 */
typedef struct {
	bt_adapter_le_scan_type_e type;  /**< LE scan type */
	float interval;  /**< LE scan interval */
	float window;  /**< LE scan window */
} bt_adapter_le_scan_params_s;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when the manufacturer dat changes.
 * @param[in]   data		The manufacurer data of the Bluetooth device to be changed
 * @param[in]   len			The length of @a data
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre This function will be invoked when the manufacturer data of Bluetooth adapter changes
 * if callback is registered using bt_adapter_set_manufacturer_data_changed_cb().
 * @see bt_adapter_set_manufacturer_data()
 * @see bt_adapter_set_manufacturer_data_changed_cb()
 * @see bt_adapter_unset_manufacturer_data_changed_cb()
 */
typedef void (*bt_adapter_manufacturer_data_changed_cb) (char *data,
		int len, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when RSSI monitoring is enabled.
 * @param[in] remote_address Remote Device address
 * @param[in] link_type Link type for the connection (@c 0 = BR/EDR link, @c 1 = LE link).
 * @param[in] rssi_enabled RSSI monitoring status (@c 1 = enabled, @c 0 = disabled)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_device_enable_rssi_monitor()
 * @see bt_device_disable_rssi_monitor()
 */
typedef void (*bt_rssi_monitor_enabled_cb)(const char *remote_address,
		bt_device_connection_link_type_e link_type,
		int rssi_enabled, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when RSSI Alert is received.
 * @param[in] remote_address Remote Device address
 * @param[in] link_type Link type for the connection (@c 0 = BR/EDR link, @c 1 = LE link).
 * @param[in] rssi_alert_type RSSI Alert type (@c 1 = High Alert (In-Range Alert), @c 2 = Low Alert)
 * @param[in] rssi_alert_dbm RSSI Alert signal strength value
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_device_enable_rssi_monitor()
 * @see bt_device_disable_rssi_monitor()
 */
typedef void (*bt_rssi_alert_cb)(char *bt_address,
		bt_device_connection_link_type_e link_type,
		int rssi_alert_type, int rssi_alert_dbm, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when Raw RSSI signal strength is received.
 * @param[in] remote_address Remote Device address
 * @param[in] link_type Link type for the connection (@c 0 = BR/EDR link, @c 1 = LE link).
 * @param[in] rssi_dbm Raw RSSI signal strength value
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_device_get_rssi_strength()
 */
typedef void (*bt_rssi_strength_cb)(char *bt_address,
		bt_device_connection_link_type_e link_type,
		int rssi_dbm, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when a XSAT vendor command is transmitted from Hands-Free.
 * @param[in] command  The XSAT vendor command transmitted from Hands-Free
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_vendor_cmd_cb()
 * @see bt_ag_unset_vendor_cmd_cb()
 */
typedef void (*bt_ag_vendor_cmd_cb) (char *command, void *user_data);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TIZEN_NETWORK_BLUETOOTH_TYPE_PRODUCT_H__
