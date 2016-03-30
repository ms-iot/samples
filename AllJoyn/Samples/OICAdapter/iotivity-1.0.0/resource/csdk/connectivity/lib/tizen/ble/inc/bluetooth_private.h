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


#ifndef __TIZEN_NETWORK_BLUETOOTH_PRIVATE_H__
#define __TIZEN_NETWORK_BLUETOOTH_PRIVATE_H__

#include <dlog.h>
#include <stdbool.h>
#include <bluetooth-api.h>
#include <bluetooth-audio-api.h>
#include <bluetooth-telephony-api.h>
#include <bluetooth-media-control.h>
#include <bluetooth-hid-api.h>

#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef LOG_TAG
#define LOG_TAG "CAPI_NETWORK_BLUETOOTH"

#define BT_INFO(fmt, args...) SLOGI(fmt, ##args)
#define BT_DBG(fmt, args...) SLOGD(fmt, ##args)
#define BT_ERR(fmt, args...) SLOGE(fmt, ##args)

#define OPP_UUID "00001105-0000-1000-8000-00805f9b34fb"

/**
 * @internal
 * @brief Bluetooth callback.
 */
typedef enum
{
	BT_EVENT_STATE_CHANGED = 0x00, /**< Adapter state is changed */
	BT_EVENT_LE_STATE_CHANGED, /**< Adapter le state is changed */
	BT_EVENT_NAME_CHANGED, /**< Adapter name is changed */
	BT_EVENT_VISIBILITY_MODE_CHANGED, /**< Adapter visibility mode is changed */
	BT_EVENT_VISIBILITY_DURATION_CHANGED, /**< Adapter visibility duration is changed */
	BT_EVENT_DEVICE_DISCOVERY_STATE_CHANGED, /**< Device discovery state is changed */
	BT_EVENT_LE_DEVICE_DISCOVERY_STATE_CHANGED, /**< LE Device discovery state is changed */
	BT_EVENT_BOND_CREATED, /**< A bond is created */
	BT_EVENT_BOND_DESTROYED, /**< A bond is destroyed */
	BT_EVENT_AUTHORIZATION_CHANGED, /**< Authorization is changed */
	BT_EVENT_SERVICE_SEARCHED, /**< Service search finish */
	BT_EVENT_DATA_RECEIVED, /**< Data is received */
	BT_EVENT_CONNECTION_STATE_CHANGED, /**< Connection state is changed */
	BT_EVENT_RFCOMM_CONNECTION_REQUESTED, /**< RFCOMM connection is requested */
	BT_EVENT_OPP_CONNECTION_REQUESTED, /**< OPP connection is requested */
	BT_EVENT_OPP_PUSH_REQUESTED, /**< OPP push is requested */
	BT_EVENT_OPP_SERVER_TRANSFER_PROGRESS, /**< OPP transfer progress */
	BT_EVENT_OPP_SERVER_TRANSFER_FINISHED, /**< OPP transfer is completed */
	BT_EVENT_OPP_CLIENT_PUSH_RESPONSED, /**< OPP client connection is reponsed */
	BT_EVENT_OPP_CLIENT_PUSH_PROGRESS, /**< OPP client push progress */
	BT_EVENT_OPP_CLIENT_PUSH_FINISHED, /**< OPP client push is finished */
	BT_EVENT_PAN_CONNECTION_STATE_CHANGED, /**< PAN connection change */
	BT_EVENT_NAP_CONNECTION_STATE_CHANGED, /**< NAP connection change */
	BT_EVENT_HDP_CONNECTED, /**< HDP connection change */
	BT_EVENT_HDP_DISCONNECTED, /**< HDP disconnection change */
	BT_EVENT_HDP_DATA_RECEIVED, /**< HDP Data receive Callabck */
	BT_EVENT_AUDIO_CONNECTION_STATUS, /**< Audio Connection change callback */
	BT_EVENT_AG_SCO_CONNECTION_STATUS, /**< Audio - AG SCO Connection state change callback */
	BT_EVENT_AG_CALL_HANDLING_EVENT, /**< Audio - AG call event callback */
	BT_EVENT_AG_MULTI_CALL_HANDLING_EVENT, /**< Audio - AG 3-way call event callback */
	BT_EVENT_AG_DTMF_TRANSMITTED, /**< Audio - DTMF tone sending request */
	BT_EVENT_AG_MICROPHONE_GAIN_CHANGE, /**< Audio Microphone change callback */
	BT_EVENT_AG_SPEAKER_GAIN_CHANGE, /**< Audio Speaker gain change callback */
	BT_EVENT_AG_VENDOR_CMD, /**< Audio - XSAT Vendor cmd */
	BT_EVENT_AVRCP_CONNECTION_STATUS, /**< AVRCP connection change callback */
	BT_EVENT_AVRCP_EQUALIZER_STATE_CHANGED, /**< AVRCP equalizer state change callback */
	BT_EVENT_AVRCP_REPEAT_MODE_CHANGED, /**< AVRCP repeat mode change callback */
	BT_EVENT_AVRCP_SHUFFLE_MODE_CHANGED, /**< AVRCP equalizer mode change callback */
	BT_EVENT_AVRCP_SCAN_MODE_CHANGED, /**< AVRCP scan mode change callback */
	BT_EVENT_HID_CONNECTION_STATUS, /**< HID connection status callback */
	BT_EVENT_DEVICE_CONNECTION_STATUS, /**< Device connection status callback */
	BT_EVENT_GATT_CONNECTION_STATUS, /** < GATT connection status callback */
	BT_EVENT_GATT_PRIM_SVC_DISCOVERED, /**< GATT Primary Service discovered callback */
	BT_EVENT_GATT_CHARACTERISTIC_DISCOVERED, /**< GATT characteristic discovered callback */
	BT_EVENT_GATT_CHARACTERISTIC_DESCRIPTOR_DISCOVERED, /**< GATT characteristic descriptor discovered callback */
	BT_EVENT_GATT_VALUE_CHANGED, /**< GATT characteristic value changed callback */
	BT_EVENT_GATT_READ_CHARACTERISTIC, /**< GATT characteristic value read callback */
	BT_EVENT_GATT_WRITE_CHARACTERISTIC, /**< GATT characteristic value write callback */
	BT_EVENT_GATT_SERVER_ON_WRITE_CHAR, /**< GATT characteristic value write callback */
	BT_EVENT_ADVERTISING_STATE_CHANGED, /**< Advertising state changed callback */
	BT_EVENT_MANUFACTURER_DATA_CHANGED, /**< Manufacturer data changed callback */
	BT_EVENT_CONNECTABLE_CHANGED_EVENT, /**< Adapter connectable changed callback */
	BT_EVENT_RSSI_ENABLED_EVENT, /**< RSSI Enabled callback */
	BT_EVENT_RSSI_ALERT_EVENT, /**< RSSI Alert callback */
	BT_EVENT_GET_RSSI_EVENT, /**< Get RSSI Strength callback */
#ifdef TIZEN_WEARABLE
	BT_EVENT_PBAP_CONNECTION_STATUS, /**< PBAP connection status callback */
	BT_EVENT_PBAP_PHONEBOOK_SIZE, /**< PBAP Phonebook Size status callback */
	BT_EVENT_PBAP_PHONEBOOK_PULL, /**< PBAP Phonebook Pull status callback */
	BT_EVENT_PBAP_VCARD_LIST, /**< PBAP vCard List status callback */
	BT_EVENT_PBAP_VCARD_PULL, /**< PBAP vCard Pull status callback */
	BT_EVENT_PBAP_PHONEBOOK_SEARCH, /**< PBAP Phonebook Search status callback */
	BT_EVENT_HF_SCO_CONNECTION_STATUS, /**< Audio - HF SCO Connection state change callback */
	BT_EVENT_HF_SPEAKER_GAIN_CHANGE, /**< Audio - HF Speaker gain change callback */
	BT_EVENT_HF_CALL_HANDLING_EVENT, /**< Audio - HF call event callback */
	BT_EVENT_HF_VENDOR_DEP_CMD_EVENT, /**< Audio - HF Vendor Command callback */
	BT_EVENT_HF_MULTI_CALL_HANDLING_EVENT, /**< Audio - HF 3-way call event callback */
#endif
} bt_event_e;

/**
 * @internal
 */
typedef struct {
	int handle;

	bt_adapter_le_advertising_state_changed_cb cb;
	void *cb_data;

	bt_adapter_le_advertising_params_s adv_params;

	unsigned int adv_data_len;
	char *adv_data;

	unsigned int scan_rsp_data_len;
	char *scan_rsp_data;

	void *user_data;
} bt_advertiser_s;

/**
 * @internal
 */
typedef struct bt_event_sig_event_slot_s
{
    int event_type;
    const void *callback;
    void *user_data;
} bt_event_sig_event_slot_s;


#define BT_CHECK_INPUT_PARAMETER(arg) \
	if (arg == NULL) \
	{ \
		LOGE("[%s] INVALID_PARAMETER(0x%08x)", __FUNCTION__, BT_ERROR_INVALID_PARAMETER); \
		return BT_ERROR_INVALID_PARAMETER; \
	}

/**
 * @internal
 * @brief Check the initialzating status
 */
int _bt_check_init_status(void);

#define BT_CHECK_INIT_STATUS() \
	if (_bt_check_init_status() == BT_ERROR_NOT_INITIALIZED) \
	{ \
		LOGE("[%s] NOT_INITIALIZED(0x%08x)", __FUNCTION__, BT_ERROR_NOT_INITIALIZED); \
		return BT_ERROR_NOT_INITIALIZED; \
	}

/**
 * @internal
 * @brief Initialize Bluetooth LE adapter
 */
int _bt_le_adapter_init(void);

/**
 * @internal
 * @brief Deinitialize Bluetooth LE adapter
 */
int _bt_le_adapter_deinit(void);

/**
 * @internal
 * @brief Set the event callback.
 */
void _bt_set_cb(int events, void *callback, void *user_data);

/**
 * @internal
 * @brief Unset the event callback.
 */
void _bt_unset_cb(int events);

/**
 * @internal
 * @brief Check if the event callback exist or not.
 */
bool _bt_check_cb(int events);

/**
 * @internal
 * @brief Convert Bluetooth F/W error codes to capi Bluetooth error codes.
 */
int _bt_get_error_code(int origin_error);


/**
 * @internal
 * @brief Convert Bluetooth F/W bluetooth_device_info_t to capi bt_device_info_s.
 */
int _bt_get_bt_device_info_s(bt_device_info_s **dest_dev, bluetooth_device_info_t *source_dev);


/**
 * @internal
 * @brief Free bt_device_info_s.
 */
void _bt_free_bt_device_info_s(bt_device_info_s *device_info);

/**
 * @internal
 * @brief Convert Bluetooth F/W bluetooth_device_address_t to string.
 */
int _bt_convert_address_to_string(char **addr_str, bluetooth_device_address_t *addr_hex);


/**
 * @internal
 * @brief Convert string to Bluetooth F/W bluetooth_device_address_t.
 */
void _bt_convert_address_to_hex(bluetooth_device_address_t *addr_hex, const char *addr_str);


/**
 * @internal
 * @brief Convert error code to string.
 */
char* _bt_convert_error_to_string(int error);

/**
 * @internal
 * @brief Convert the visibility mode
 */
bt_adapter_visibility_mode_e _bt_get_bt_visibility_mode_e(bluetooth_discoverable_mode_t mode);

/**
 * @internal
 * @brief Since the Audio call back and event proxy call backs have different prototype it is wrapper function.
 */
void _bt_audio_event_proxy(int event, bt_audio_event_param_t *param, void *user_data);

#ifdef TIZEN_WEARABLE
/**
 * @internal
 * @brief Since the HF call back and event proxy call backs have different prototype it is wrapper function.
 */
void _bt_hf_event_proxy(int event, bt_hf_event_param_t *param, void *user_data);
#endif

/**
 * @internal
 * @brief Since the Telephony call back and event proxy call backs have different prototype it is wrapper function.
 */
void _bt_telephony_event_proxy(int event, telephony_event_param_t *param, void *user_data);

/**
 * @internal
 * @brief Since the AVRCP call back and event proxy call backs have different prototype it is wrapper function.
 */
void _bt_avrcp_event_proxy(int event, media_event_param_t *param, void *user_data);

/**
 * @internal
 * @brief Since the HID call back and event proxy call backs have different prototype it is wrapper function.
 */
void _bt_hid_event_proxy(int event, hid_event_param_t *param, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_NETWORK_BLUETOOTH_PRIVATE_H__ */
