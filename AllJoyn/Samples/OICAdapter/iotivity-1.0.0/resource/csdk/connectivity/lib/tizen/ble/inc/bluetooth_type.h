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


#ifndef __TIZEN_NETWORK_BLUETOOTH_TYPE_H__
#define __TIZEN_NETWORK_BLUETOOTH_TYPE_H__

 #ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @file bluetooth_type.h
 * @brief API to control the Bluetooth adapter, devices and communications.
 * @ingroup     CAPI_NETWORK_BLUETOOTH_TYPE_MODULE
 */


/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_MODULE
 * @brief Enumerations of Bluetooth error codes.
 */
typedef enum
{
	BT_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful*/
	BT_ERROR_CANCELLED = TIZEN_ERROR_CANCELED, /**< Operation cancelled */
	BT_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	BT_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	BT_ERROR_RESOURCE_BUSY = TIZEN_ERROR_RESOURCE_BUSY, /**< Device or resource busy */
	BT_ERROR_TIMED_OUT = TIZEN_ERROR_TIMED_OUT, /**< Timeout error */
	BT_ERROR_NOW_IN_PROGRESS = TIZEN_ERROR_NOW_IN_PROGRESS, /**< Operation now in progress */
	BT_ERROR_NOT_SUPPORTED = TIZEN_ERROR_NOT_SUPPORTED,
	BT_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED, /**< Permission denied */
	BT_ERROR_QUOTA_EXCEEDED = TIZEN_ERROR_QUOTA_EXCEEDED, /**< Quota exceeded */
	BT_ERROR_NOT_INITIALIZED = TIZEN_ERROR_BLUETOOTH|0x0101, /**< Local adapter not initialized */
	BT_ERROR_NOT_ENABLED = TIZEN_ERROR_BLUETOOTH|0x0102, /**< Local adapter not enabled */
	BT_ERROR_ALREADY_DONE = TIZEN_ERROR_BLUETOOTH|0x0103, /**< Operation already done  */
	BT_ERROR_OPERATION_FAILED = TIZEN_ERROR_BLUETOOTH|0x0104, /**< Operation failed */
	BT_ERROR_NOT_IN_PROGRESS = TIZEN_ERROR_BLUETOOTH|0x0105, /**< Operation not in progress */
	BT_ERROR_REMOTE_DEVICE_NOT_BONDED = TIZEN_ERROR_BLUETOOTH|0x0106, /**< Remote device not bonded */
	BT_ERROR_AUTH_REJECTED = TIZEN_ERROR_BLUETOOTH|0x0107, /**< Authentication rejected */
	BT_ERROR_AUTH_FAILED = TIZEN_ERROR_BLUETOOTH|0x0108, /**< Authentication failed */
	BT_ERROR_REMOTE_DEVICE_NOT_FOUND = TIZEN_ERROR_BLUETOOTH|0x0109, /**< Remote device not found */
	BT_ERROR_SERVICE_SEARCH_FAILED = TIZEN_ERROR_BLUETOOTH|0x010A, /**< Service search failed */
	BT_ERROR_REMOTE_DEVICE_NOT_CONNECTED = TIZEN_ERROR_BLUETOOTH|0x010B, /**< Remote device is not connected */
	BT_ERROR_AGAIN = TIZEN_ERROR_BLUETOOTH|0x010C, /**< Resource temporarily unavailable */
	BT_ERROR_SERVICE_NOT_FOUND = TIZEN_ERROR_BLUETOOTH|0x010D, /**< Service Not Found */
} bt_error_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Enumerations of the Bluetooth adapter state.
 */
typedef enum
{
	BT_ADAPTER_DISABLED = 0x00, /**< Bluetooth adapter is disabled */
	BT_ADAPTER_ENABLED, /**< Bluetooth adapter is enabled */
} bt_adapter_state_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth adapter le state.
 */
typedef enum
{
	BT_ADAPTER_LE_DISABLED = 0x00, /**< Bluetooth le is disabled */
	BT_ADAPTER_LE_ENABLED, /**< Bluetooth le is enabled */
} bt_adapter_le_state_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief Enumerations of the Bluetooth visibility mode.
 */
typedef enum
{
	BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE = 0x00,  /**< Other devices cannot find your device via discovery */
	BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE,  /**< Discoverable mode */
	BT_ADAPTER_VISIBILITY_MODE_LIMITED_DISCOVERABLE,  /**< Discoverable mode with time limit. After specific period,
							    it is changed to #BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE.*/
} bt_adapter_visibility_mode_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief Enumerations of the discovery state of Bluetooth device.
 *
 */
typedef enum
{
	BT_ADAPTER_DEVICE_DISCOVERY_STARTED, /**< Device discovery is started */
	BT_ADAPTER_DEVICE_DISCOVERY_FINISHED, /**< Device discovery is finished */
	BT_ADAPTER_DEVICE_DISCOVERY_FOUND, /**< The remote Bluetooth device is found */
} bt_adapter_device_discovery_state_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief Enumerations of the discovery state of Bluetooth LE device.
 *
 */
typedef enum
{
	BT_ADAPTER_LE_DEVICE_DISCOVERY_STARTED, /**< LE device discovery is started */
	BT_ADAPTER_LE_DEVICE_DISCOVERY_FINISHED, /**< LE device discovery is finished */
	BT_ADAPTER_LE_DEVICE_DISCOVERY_FOUND, /**< The remote Bluetooth LE device is found */
} bt_adapter_le_device_discovery_state_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth advertising state.
 */
typedef enum {
	BT_ADAPTER_LE_ADVERTISING_STOPPED = 0x00, /**< Bluetooth advertising is stopped */
	BT_ADAPTER_LE_ADVERTISING_STARTED, /**< Bluetooth advertising is started */
} bt_adapter_le_advertising_state_e;


/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth advertising filter policy.
 */
typedef enum {
	BT_ADAPTER_LE_ADVERTISING_FILTER_DEFAULT = 0x00, /**< White list is not in use */
	BT_ADAPTER_LE_ADVERTISING_FILTER_ALLOW_SCAN_WL = 0x01, /**< Allow the scan
					request that in the White list */
	BT_ADAPTER_LE_ADVERTISING_FILTER_ALLOW_CONN_WL = 0x02, /**< Allow the connectoin
					request that in the White list */
	BT_ADAPTER_LE_ADVERTISING_FILTER_ALLOW_SCAN_CONN_WL = 0x03, /**< Allow the
					scan and connectoin request that in the White list */
} bt_adapter_le_advertising_filter_policy_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth advertising type.
 */
typedef enum {
	BT_ADAPTER_LE_ADVERTISING_CONNECTABLE = 0x00, /**< Connectable undirected advertising (ADV_IND) */
	BT_ADAPTER_LE_ADVERTISING_CONNECTABLE_DIRECT_HIGH = 0x01, /**< Connectable high duty cycle directed advertising (ADV_DIRECT_IND) */
	BT_ADAPTER_LE_ADVERTISING_SCANNABLE = 0x02, /**< Scannable undirected advertising (ADV_SCAN_IND) */
	BT_ADAPTER_LE_ADVERTISING_NON_CONNECTABLE = 0x03, /**< Non connectable undirected advertising (ADV_NONCOND_IND) */
	BT_ADAPTER_LE_ADVERTISING_CONNECTABLE_DIRECT_LOW = 0x04, /**< Connectable low duty cycle directed advertising (ADV_DIRECT_IND) */
} bt_adapter_le_advertising_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth LE packet type.
 */
typedef enum {
	BT_ADAPTER_LE_PACKET_ADVERTISING, /**< Advertising packet */
	BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, /**< Sacn response packet */
} bt_adapter_le_packet_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Enumerations of the Bluetooth LE data type
 *         that can be included in LE packets.
 */
typedef enum {
	BT_ADAPTER_LE_PACKET_DATA_COMP_LIST_16_BIT_SERVICE_CLASS_UUIDS = 0x03, /**< 16 bit UUID */
	BT_ADAPTER_LE_PACKET_DATA_MANUFACTURER_SPECIFIC_DATA = 0xff, /**< Manufacturer data */
} bt_adapter_le_packet_data_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Enumerations of device disconnect reason.
 */
typedef enum
{
	BT_DEVICE_DISCONNECT_REASON_UNKNOWN, /**< Disconnected by unknown reason */
	BT_DEVICE_DISCONNECT_REASON_TIMEOUT, /**< Disconnected by timeout */
	BT_DEVICE_DISCONNECT_REASON_LOCAL_HOST, /**< Disconnected by local host */
	BT_DEVICE_DISCONNECT_REASON_REMOTE, /**< Disconnected by remote */
} bt_device_disconnect_reason_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Enumerations of connection link type.
 */
typedef enum
{
	BT_DEVICE_CONNECTION_LINK_BREDR, /**< BR/EDR link */
	BT_DEVICE_CONNECTION_LINK_LE, /**< LE link */
	BT_DEVICE_CONNECTION_LINK_DEFAULT = 0xFF, /**< The connection type defualt */
} bt_device_connection_link_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Enumerations of device authorization state.
 */
typedef enum
{
	BT_DEVICE_AUTHORIZED, /**< The remote Bluetooth device is authorized */
	BT_DEVICE_UNAUTHORIZED, /**< The remote Bluetooth device is unauthorized */
} bt_device_authorization_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Enumerations of Bluetooth profile.
 */
typedef enum
{
    BT_PROFILE_RFCOMM = 0x01, /**< RFCOMM Profile */
    BT_PROFILE_A2DP = 0x02, /**< Advanced Audio Distribution Profile */
    BT_PROFILE_HSP = 0x04, /**< Headset Profile */
    BT_PROFILE_HID = 0x08, /**< Human Interface Device Profile */
    BT_PROFILE_NAP = 0x10, /**< Network Access Point Profile */
    BT_PROFILE_AG = 0x20, /**< Audio Gateway Profile */
    BT_PROFILE_GATT = 0x40, /**< Generic Attribute Profile */
    BT_PROFILE_NAP_SERVER = 0x80, /**< NAP server Profile */
} bt_profile_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Enumerations of device address type.
 */
typedef enum
{
	BT_DEVICE_PUBLIC_ADDRESS = 0x00, /**< Public address */
	BT_DEVICE_RANDOM_ADDRESS, /**< Random address */
} bt_device_address_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Enumerations of service class.
 */
typedef enum {
	BT_SC_NONE = 0, /**< No service class */
	BT_SC_RES_SERVICE_MASK = 0x00000001, /**< RES service class */
	BT_SC_SPP_SERVICE_MASK = 0x00000002, /**< SPP service class */
	BT_SC_DUN_SERVICE_MASK = 0x00000004, /**< DUN service class */
	BT_SC_FAX_SERVICE_MASK = 0x00000008, /**< FAX service class */
	BT_SC_LAP_SERVICE_MASK = 0x00000010, /**< LAP service class */
	BT_SC_HSP_SERVICE_MASK = 0x00000020, /**< HSP service class */
	BT_SC_HFP_SERVICE_MASK = 0x00000040, /**< HFP service class */
	BT_SC_OPP_SERVICE_MASK = 0x00000080, /**< OPP service class */
	BT_SC_FTP_SERVICE_MASK = 0x00000100, /**< FTP service class */
	BT_SC_CTP_SERVICE_MASK = 0x00000200, /**< CTP service class */
	BT_SC_ICP_SERVICE_MASK = 0x00000400, /**< ICP service class */
	BT_SC_SYNC_SERVICE_MASK = 0x00000800, /**< SYNC service class */
	BT_SC_BPP_SERVICE_MASK = 0x00001000, /**< BPP service class */
	BT_SC_BIP_SERVICE_MASK = 0x00002000, /**< BIP service class */
	BT_SC_PANU_SERVICE_MASK = 0x00004000, /**< PANU service class */
	BT_SC_NAP_SERVICE_MASK = 0x00008000, /**< NAP service class */
	BT_SC_GN_SERVICE_MASK = 0x00010000, /**< GN service class */
	BT_SC_SAP_SERVICE_MASK = 0x00020000, /**< SAP service class */
	BT_SC_A2DP_SERVICE_MASK = 0x00040000, /**< A2DP service class */
	BT_SC_AVRCP_SERVICE_MASK = 0x00080000, /**< AVRCP service class */
	BT_SC_PBAP_SERVICE_MASK = 0x00100000, /**< PBAP service class */
	BT_SC_HID_SERVICE_MASK = 0x00200000, /**< HID service class */
	BT_SC_ALL_SERVICE_MASK = 0x00FFFFFF, /**< ALL service class */
	BT_SC_MAX /**< MAX service class */
} bt_service_class_t;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Enumerations of major service class.
 */
typedef enum
{
	BT_MAJOR_SERVICE_CLASS_LIMITED_DISCOVERABLE_MODE  = 0x002000, /**< Limited discoverable mode */
	BT_MAJOR_SERVICE_CLASS_POSITIONING                = 0x010000, /**< Positioning class */
	BT_MAJOR_SERVICE_CLASS_NETWORKING                 = 0x020000, /**< Networking class */
	BT_MAJOR_SERVICE_CLASS_RENDERING                  = 0x040000, /**< Rendering class */
	BT_MAJOR_SERVICE_CLASS_CAPTURING                  = 0x080000, /**< Capturing class */
	BT_MAJOR_SERVICE_CLASS_OBJECT_TRANSFER            = 0x100000, /**< Object transferring class */
	BT_MAJOR_SERVICE_CLASS_AUDIO                      = 0x200000, /**< Audio class*/
	BT_MAJOR_SERVICE_CLASS_TELEPHONY                  = 0x400000, /**< Telephony class */
	BT_MAJOR_SERVICE_CLASS_INFORMATION                = 0x800000, /**< Information class */
} bt_major_service_class_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Enumerations of major device class.
 */
typedef enum
{
	BT_MAJOR_DEVICE_CLASS_MISC = 0x00, /**< Miscellaneous major device class*/
	BT_MAJOR_DEVICE_CLASS_COMPUTER = 0x01, /**< Computer major device class */
	BT_MAJOR_DEVICE_CLASS_PHONE = 0x02, /**< Phone major device class */
	BT_MAJOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT = 0x03, /**< LAN/Network access point major device class */
	BT_MAJOR_DEVICE_CLASS_AUDIO_VIDEO = 0x04, /**< Audio/Video major device class */
	BT_MAJOR_DEVICE_CLASS_PERIPHERAL = 0x05, /**< Peripheral major device class */
	BT_MAJOR_DEVICE_CLASS_IMAGING = 0x06, /**< Imaging major device class */
	BT_MAJOR_DEVICE_CLASS_WEARABLE = 0x07, /**< Wearable device class */
	BT_MAJOR_DEVICE_CLASS_TOY = 0x08, /**< Toy device class */
	BT_MAJOR_DEVICE_CLASS_HEALTH = 0x09, /**< Health device class */
	BT_MAJOR_DEVICE_CLASS_UNCATEGORIZED = 0x1F, /**< Uncategorized major device class */
} bt_major_device_class_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Enumerations of minor device class.
 */
typedef enum
{
	BT_MINOR_DEVICE_CLASS_COMPUTER_UNCATEGORIZED = 0x00, /**< Uncategorized minor device class of computer */
	BT_MINOR_DEVICE_CLASS_COMPUTER_DESKTOP_WORKSTATION = 0x04, /**< Desktop workstation minor device class of computer */
	BT_MINOR_DEVICE_CLASS_COMPUTER_SERVER_CLASS = 0x08, /**< Server minor device class of computer */
	BT_MINOR_DEVICE_CLASS_COMPUTER_LAPTOP = 0x0C, /**< Laptop minor device class of computer */
	BT_MINOR_DEVICE_CLASS_COMPUTER_HANDHELD_PC_OR_PDA = 0x10, /**< Handheld PC/PDA minor device class of computer */
	BT_MINOR_DEVICE_CLASS_COMPUTER_PALM_SIZED_PC_OR_PDA = 0x14, /**< Palm sized PC/PDA minor device class of computer */
	BT_MINOR_DEVICE_CLASS_COMPUTER_WEARABLE_COMPUTER = 0x18, /**< Wearable(watch sized) minor device class of computer */

	BT_MINOR_DEVICE_CLASS_PHONE_UNCATEGORIZED = 0x00, /**< Uncategorized minor device class of phone */
	BT_MINOR_DEVICE_CLASS_PHONE_CELLULAR = 0x04, /**< Cellular minor device class of phone */
	BT_MINOR_DEVICE_CLASS_PHONE_CORDLESS = 0x08, /**< Cordless minor device class of phone */
	BT_MINOR_DEVICE_CLASS_PHONE_SMART_PHONE = 0x0C, /**< Smart phone minor device class of phone */
	BT_MINOR_DEVICE_CLASS_PHONE_WIRED_MODEM_OR_VOICE_GATEWAY = 0x10, /**< Wired modem or voice gateway minor device class of phone */
	BT_MINOR_DEVICE_CLASS_PHONE_COMMON_ISDN_ACCESS = 0x14, /**< Common ISDN minor device class of phone */

	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_FULLY_AVAILABLE = 0x04, /**< Fully available minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_1_TO_17_PERCENT_UTILIZED = 0x20, /**< 1-17% utilized minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_17_TO_33_PERCENT_UTILIZED = 0x40, /**< 17-33% utilized minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_33_TO_50_PERCENT_UTILIZED = 0x60, /**< 33-50% utilized minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_50_to_67_PERCENT_UTILIZED = 0x80, /**< 50-67% utilized minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_67_TO_83_PERCENT_UTILIZED = 0xA0, /**< 67-83% utilized minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_83_TO_99_PERCENT_UTILIZED = 0xC0, /**< 83-99% utilized minor device class of LAN/Network access point */
	BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_NO_SERVICE_AVAILABLE = 0xE0, /**< No service available minor device class of LAN/Network access point */

	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_UNCATEGORIZED = 0x00, /**< Uncategorized minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_WEARABLE_HEADSET = 0x04, /**< Wearable headset minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HANDS_FREE = 0x08, /**< Hands-free minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_MICROPHONE = 0x10, /**< Microphone minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_LOUDSPEAKER = 0x14, /**< Loudspeaker minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HEADPHONES = 0x18, /**< Headphones minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_PORTABLE_AUDIO = 0x1C, /**< Portable audio minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_CAR_AUDIO = 0x20, /**< Car audio minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_SET_TOP_BOX = 0x24, /**< Set-top box minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HIFI_AUDIO_DEVICE = 0x28, /**< Hifi audio minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VCR = 0x2C, /**< VCR minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_CAMERA = 0x30, /**< Video camera minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_CAMCORDER = 0x34, /**< Camcorder minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_MONITOR = 0x38, /**< Video monitor minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_DISPLAY_LOUDSPEAKER = 0x3C, /**< Video display and loudspeaker minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_CONFERENCING = 0x40, /**< Video conferencing minor device class of audio/video */
	BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_GAMING_TOY = 0x48, /**< Gaming/toy minor device class of audio/video */

	BT_MINOR_DEVICE_CLASS_PERIPHERA_UNCATEGORIZED = 0x00, /**< Uncategorized minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_KEY_BOARD = 0x40, /**< Key board minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_POINTING_DEVICE = 0x80, /**< Pointing device minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_COMBO_KEYBOARD_POINTING_DEVICE = 0xC0, /**< Combo keyboard or pointing device minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_JOYSTICK = 0x04, /**< Joystick minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_GAME_PAD = 0x08, /**< Game pad minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_REMOTE_CONTROL = 0x0C, /**< Remote control minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_SENSING_DEVICE = 0x10, /**< Sensing device minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_DIGITIZER_TABLET = 0x14, /**< Digitizer minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_CARD_READER = 0x18, /**< Card reader minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_DIGITAL_PEN = 0x1C, /**< Digital pen minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_HANDHELD_SCANNER = 0x20, /**< Handheld scanner minor device class of peripheral */
	BT_MINOR_DEVICE_CLASS_PERIPHERAL_HANDHELD_GESTURAL_INPUT_DEVICE = 0x24, /**< Handheld gestural input device minor device class of peripheral */

	BT_MINOR_DEVICE_CLASS_IMAGING_DISPLAY = 0x10, /**< Display minor device class of imaging */
	BT_MINOR_DEVICE_CLASS_IMAGING_CAMERA = 0x20, /**< Camera minor device class of imaging */
	BT_MINOR_DEVICE_CLASS_IMAGING_SCANNER = 0x40, /**< Scanner minor device class of imaging */
	BT_MINOR_DEVICE_CLASS_IMAGING_PRINTER = 0x80, /**< Printer minor device class of imaging */

	BT_MINOR_DEVICE_CLASS_WEARABLE_WRIST_WATCH = 0x04, /**< Wrist watch minor device class of wearable */
	BT_MINOR_DEVICE_CLASS_WEARABLE_PAGER = 0x08, /**< Pager minor device class of wearable */
	BT_MINOR_DEVICE_CLASS_WEARABLE_JACKET = 0x0C, /**< Jacket minor device class of wearable */
	BT_MINOR_DEVICE_CLASS_WEARABLE_HELMET = 0x10, /**< Helmet minor device class of wearable */
	BT_MINOR_DEVICE_CLASS_WEARABLE_GLASSES = 0x14, /**< Glasses minor device class of wearable */

	BT_MINOR_DEVICE_CLASS_TOY_ROBOT = 0x04, /**< Robot minor device class of toy */
	BT_MINOR_DEVICE_CLASS_TOY_VEHICLE = 0x08, /**< Vehicle minor device class of toy */
	BT_MINOR_DEVICE_CLASS_TOY_DOLL_ACTION = 0x0C, /**< Doll/Action minor device class of toy */
	BT_MINOR_DEVICE_CLASS_TOY_CONTROLLER = 0x10, /**< Controller minor device class of toy */
	BT_MINOR_DEVICE_CLASS_TOY_GAME = 0x14, /**< Game minor device class of toy */

	BT_MINOR_DEVICE_CLASS_HEALTH_UNCATEGORIZED = 0x00, /**< Uncategorized minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_BLOOD_PRESSURE_MONITOR = 0x04, /**< Blood pressure monitor minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_THERMOMETER = 0x08, /**< Thermometer minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_WEIGHING_SCALE = 0x0C, /**< Weighing scale minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_GLUCOSE_METER = 0x10, /**< Glucose minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_PULSE_OXIMETER = 0x14, /**< Pulse oximeter minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_HEART_PULSE_RATE_MONITOR = 0x18, /**< Heart/Pulse rate monitor minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_DATA_DISPLAY = 0x1C, /**< Health data display minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_STEP_COUNTER = 0x20, /**< Step counter minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_BODY_COMPOSITION_ANALYZER = 0x24, /**< Body composition analyzer minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_PEAK_FLOW_MONITOR = 0x28, /**< Peak flow monitor minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_MEDICATION_MONITOR = 0x2C, /**< Medication monitor minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_KNEE_PROSTHESIS = 0x30, /**< Knee prosthesis minor device class of health */
	BT_MINOR_DEVICE_CLASS_HEALTH_ANKLE_PROSTHESIS = 0x34, /**< Ankle prosthesis minor device class of health */
} bt_minor_device_class_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Enumerations of gap appearance type.
 */
typedef enum
{
	BT_APPEARANCE_TYPE_UNKNOWN = 0x00, /**< Unknown appearance type */
	BT_APPEARANCE_TYPE_GENERIC_PHONE = 0x40, /**< Generic Phone type - Generic category */
	BT_APPEARANCE_TYPE_GENERIC_COMPUTER = 0x80, /**< Generic Computer type - Generic category */
	BT_APPEARANCE_TYPE_GENERIC_WATCH = 0xC0, /**< Generic Watch type - Generic category */
} bt_appearance_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 * @brief  Enumerations of connected Bluetooth device event role.
 */
typedef enum
{
	BT_SOCKET_UNKNOWN = 0x00, /**< Unknown role*/
	BT_SOCKET_SERVER , /**< Server role*/
	BT_SOCKET_CLIENT, /**< Client role*/
} bt_socket_role_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 * @brief  Enumerations of Bluetooth socket connection state.
 */
typedef enum
{
	BT_SOCKET_CONNECTED, /**< RFCOMM is connected */
	BT_SOCKET_DISCONNECTED, /**< RFCOMM is disconnected */
} bt_socket_connection_state_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_MODULE
 * @brief  Enumerations for the types of profiles related with audio
 */
typedef enum {
    BT_AUDIO_PROFILE_TYPE_ALL = 0,  /**< All supported profiles related with audio */
    BT_AUDIO_PROFILE_TYPE_HSP_HFP,  /**< HSP(Headset Profile) and HFP(Hands-Free Profile) */
    BT_AUDIO_PROFILE_TYPE_A2DP,  /**< A2DP(Advanced Audio Distribution Profile) */
    BT_AUDIO_PROFILE_TYPE_AG,  /**< AG(Audio Gateway) */
} bt_audio_profile_type_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Enumerations for the call handling event
 */
typedef enum {
    BT_AG_CALL_HANDLING_EVENT_ANSWER = 0x00,  /**< Request to answer an incoming call */
    BT_AG_CALL_HANDLING_EVENT_RELEASE,  /**< Request to release a call */
    BT_AG_CALL_HANDLING_EVENT_REJECT,  /**< Request to reject an incoming call */
} bt_ag_call_handling_event_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Enumerations for the multi call handling event
 */
typedef enum {
    BT_AG_MULTI_CALL_HANDLING_EVENT_RELEASE_HELD_CALLS = 0x00,  /**< Request to release held calls */
    BT_AG_MULTI_CALL_HANDLING_EVENT_RELEASE_ACTIVE_CALLS,  /**< Request to release active calls */
    BT_AG_MULTI_CALL_HANDLING_EVENT_ACTIVATE_HELD_CALL,  /**< Request to put active calls into hold state and activate another (held or waiting) call */
    BT_AG_MULTI_CALL_HANDLING_EVENT_MERGE_CALLS,  /**< Request to add a held call to the conversation */
    BT_AG_MULTI_CALL_HANDLING_EVENT_EXPLICIT_CALL_TRANSFER,  /**< Request to let a user who has two calls to connect these two calls together and release its connections to both other parties */
} bt_ag_multi_call_handling_event_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Enumerations for the call state
 */
typedef enum {
    BT_AG_CALL_EVENT_IDLE = 0x00,  /**< Idle */
    BT_AG_CALL_EVENT_ANSWERED,  /**< Answered */
    BT_AG_CALL_EVENT_HELD,  /**< Held */
    BT_AG_CALL_EVENT_RETRIEVED,  /**< Retrieved */
    BT_AG_CALL_EVENT_DIALING,  /**< Dialing */
    BT_AG_CALL_EVENT_ALERTING,  /**< Alerting */
    BT_AG_CALL_EVENT_INCOMING,  /**< Incoming */
} bt_ag_call_event_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Enumerations for the call state
 */
typedef enum {
    BT_AG_CALL_STATE_IDLE = 0x00,  /**< Idle state */
    BT_AG_CALL_STATE_ACTIVE,  /**< Active state */
    BT_AG_CALL_STATE_HELD,  /**< Held state */
    BT_AG_CALL_STATE_DIALING,  /**< Dialing state */
    BT_AG_CALL_STATE_ALERTING,  /**< Alerting state */
    BT_AG_CALL_STATE_INCOMING,  /**< Incoming state */
    BT_AG_CALL_STATE_WAITING,  /**< Waiting for connected indication event after answering an incoming call*/
} bt_ag_call_state_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Enumerations for the equalizer state
 */
typedef enum {
    BT_AVRCP_EQUALIZER_STATE_OFF = 0x01,  /**< Equalizer Off */
    BT_AVRCP_EQUALIZER_STATE_ON,  /**< Equalizer On */
} bt_avrcp_equalizer_state_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Enumerations for the repeat mode
 */
typedef enum {
    BT_AVRCP_REPEAT_MODE_OFF = 0x01,  /**< Repeat Off */
    BT_AVRCP_REPEAT_MODE_SINGLE_TRACK,  /**< Single track repeat */
    BT_AVRCP_REPEAT_MODE_ALL_TRACK,  /**< All track repeat */
    BT_AVRCP_REPEAT_MODE_GROUP,  /**< Group repeat */
} bt_avrcp_repeat_mode_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Enumerations for the shuffle mode
 */
typedef enum {
    BT_AVRCP_SHUFFLE_MODE_OFF = 0x01,  /**< Shuffle Off */
    BT_AVRCP_SHUFFLE_MODE_ALL_TRACK,  /**< All tracks shuffle */
    BT_AVRCP_SHUFFLE_MODE_GROUP,  /**< Group shuffle */
} bt_avrcp_shuffle_mode_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Enumerations for the scan mode
 */
typedef enum {
    BT_AVRCP_SCAN_MODE_OFF = 0x01,  /**< Scan Off */
    BT_AVRCP_SCAN_MODE_ALL_TRACK,  /**< All tracks scan */
    BT_AVRCP_SCAN_MODE_GROUP,  /**< Group scan */
} bt_avrcp_scan_mode_e;

/**
 * @internal
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Enumerations for the player state
 */
typedef enum {
    BT_AVRCP_PLAYER_STATE_STOPPED = 0x00,  /**< Stopped */
    BT_AVRCP_PLAYER_STATE_PLAYING,  /**< Playing */
    BT_AVRCP_PLAYER_STATE_PAUSED,  /**< Paused */
    BT_AVRCP_PLAYER_STATE_FORWARD_SEEK,  /**< Seek Forward */
    BT_AVRCP_PLAYER_STATE_REWIND_SEEK,  /**< Seek Rewind */
} bt_avrcp_player_state_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_HDP_MODULE
 * @brief  Enumerations for the data channel type
 */
typedef enum {
    BT_HDP_CHANNEL_TYPE_RELIABLE  = 0x01,  /**< Reliable Data Channel */
    BT_HDP_CHANNEL_TYPE_STREAMING,  /**< Streaming Data Channel */
} bt_hdp_channel_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_PAN_PANU_MODULE
 * @brief  Enumerations for the types of PAN(Personal Area Networking) service
 */
typedef enum {
    BT_PANU_SERVICE_TYPE_NAP = 0,  /**< Network Access Point */
} bt_panu_service_type_e;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief The handle to control Bluetooth LE advertising
 */
typedef void* bt_advertiser_h;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  The attribute handle of GATT(Generic Attribute Profile)
 */
typedef void* bt_gatt_attribute_h;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  The handle of calls state
 */
typedef void* bt_call_list_h;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Class structure of device and service.
 *
 * @see #bt_device_info_s
 * @see #bt_adapter_device_discovery_info_s
 * @see bt_device_bond_created_cb()
 * @see bt_adapter_device_discovery_state_changed_cb()
 */
typedef struct
{
	bt_major_device_class_e major_device_class;	/**< Major device class. */
	bt_minor_device_class_e minor_device_class;	/**< Minor device class. */
	int major_service_class_mask;	/**< Major service class mask.
	This value can be a combination of #bt_major_service_class_e like #BT_MAJOR_SERVICE_CLASS_RENDERING | #BT_MAJOR_SERVICE_CLASS_AUDIO */
} bt_class_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief Structure of device discovery information.
 *
 * @see #bt_class_s
 * @see bt_adapter_device_discovery_state_changed_cb()
 */
typedef struct
{
	char *remote_address;	/**< The address of remote device */
	char *remote_name;	/**< The name of remote device */
	bt_class_s bt_class;	/**< The Bluetooth classes */
	int rssi;	/**< The strength indicator of received signal  */
	bool is_bonded;	/**< The bonding state */
	char **service_uuid;  /**< The UUID list of service */
	int service_count;	/**< The number of services */
	bt_appearance_type_e appearance;	/**< The Bluetooth appearance */
	int manufacturer_data_len;	/**< manufacturer specific data length */
	char *manufacturer_data;		/**< manufacturer specific data */
} bt_adapter_device_discovery_info_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief Structure of le device discovery information.
 *
 * @see #bt_class_s
 * @see bt_adapter_le_device_discovery_state_changed_cb()
 */
typedef struct
{
	char *remote_address;	/**< The address of remote device */
	int address_type;	/**< The address type of remote device */
	int rssi;	/**< The strength indicator of received signal  */
	char **service_uuid;  /**< The UUID list of service */
	int service_count;	/**< The number of services */
	int adv_data_len;		/**< advertising indication data length */
	char *adv_data;			/**< advertising indication data */
	int scan_data_len;		/**< scan response data length */
	char *scan_data;		/**< scan response data */
} bt_adapter_le_device_discovery_info_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief Structure of advertising parameters
 *
 * @see #bt_class_s
 * @see bt_adapter_le_advertising_state_changed_cb()
 * @see bt_adapter_le_start_advertising()
 */
typedef struct {
	float interval_min; /**< Minimum advertising interval for non-directed advertising.
			      A multiple of 0.625ms is only allowed (Time range : 20ms to 10.24sec). */
	float interval_max; /**< Maximum advertising interval for non-directed advertising.
			      A multiple of 0.625ms is only allowed (Time range : 20ms to 10.24sec). */
	char filter_policy; /* Advertising filter policy */
	char type; /* Advertising type */
} bt_adapter_le_advertising_params_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Device information structure used for identifying pear device.
 *
 * @see #bt_class_s
 * @see bt_device_bond_created_cb()
 */
typedef struct
{
	char *remote_address;	/**< The address of remote device */
	char *remote_name;	/**< The name of remote device */
	bt_class_s bt_class;	/**< The Bluetooth classes */
	char **service_uuid;  /**< The UUID list of service */
	int service_count;	/**< The number of services */
	bool is_bonded;	/**< The bonding state */
	bool is_connected;	/**< The connection state */
	bool is_authorized;	/**< The authorization state */
	int manufacturer_data_len;	/**< manufacturer specific data length */
	char *manufacturer_data;		/**< manufacturer specific data */
} bt_device_info_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Service Discovery Protocol (SDP) data structure.
 *
 * @details This protocol is used for discovering available services or pear device,
 * and finding one to connect with.
 *
 * @see bt_device_service_searched_cb()
 */
typedef struct
{
	char *remote_address;   /**< The address of remote device */
	char **service_uuid;  /**< The UUID list of service */
	int service_count;    /**< The number of services. */
} bt_device_sdp_info_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Device connection information structure.
 *
 * @see bt_device_connection_state_changed_cb()
 */
typedef struct
{
	char *remote_address;   /**< The address of remote device */
	bt_device_connection_link_type_e link;  /**< Link type */
	bt_device_disconnect_reason_e disconn_reason;  /**< Disconnection reason */
} bt_device_connection_info_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Device LE connection update structure.
 *
 * @see bt_device_le_conn_update()
 */
typedef struct
{
       unsigned int interval_min;   /**< Minimum value for the connection event interval (msec) */
       unsigned int interval_max;   /**< Maximum value for the connection event interval (msec) */
       unsigned int latency;   /**< Slave latency (msec) */
       unsigned int time_out;   /**< Supervision timeout (msec) */
} bt_le_conn_update_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 *
 * @brief Rfcomm connection data used for exchanging data between Bluetooth devices.
 *
 * @see bt_socket_connection_state_changed_cb()
 */
typedef struct
{
	int socket_fd;	/**< The file descriptor of connected socket */
	int server_fd;	/**< The file descriptor of the server socket or -1 for client connection */
	bt_socket_role_e local_role;	/**< The local device role in this connection */
	char *remote_address;	/**< The remote device address */
	char *service_uuid;	/**< The service UUId */
} bt_socket_connection_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 *
 * @brief Structure of RFCOMM received data.
 *
 * @remarks User can use standard linux functions for reading/writing
 * data from/to sockets.
 *
 * @see bt_socket_data_received_cb()
 */
typedef struct
{
	int socket_fd;	/**< The socket fd */
	int data_size;	/**< The length of the received data */
	char *data;	/**< The received data */
} bt_socket_received_data_s;

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when the Bluetooth adapter state changes.
 * @param[in]   result  The result of the adapter state changing
 * @param[in]   adapter_state  The adapter state to be changed
 * @param[in]   user_data  The user data passed from the callback registration function
 * @see bt_adapter_set_state_changed_cb()
 * @see bt_adapter_unset_state_changed_cb()
 */
typedef void (*bt_adapter_state_changed_cb)(int result, bt_adapter_state_e adapter_state, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when adapter name changes.
 * @param[in]   device_name	The name of the Bluetooth device to be changed
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre This function will be invoked when the name of Bluetooth adapter changes
 * if you register this callback using bt_adapter_set_name_changed_cb().
 * @see bt_adapter_set_name()
 * @see bt_adapter_set_name_changed_cb()
 * @see bt_adapter_unset_name_changed_cb()
 */
typedef void (*bt_adapter_name_changed_cb)(char *device_name, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when the visibility mode changes.
 * @param[in] result The result of the visibility mode changing
 * @param[in] visibility_mode The visibility mode to be changed
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre This function will be invoked when the visibility of Bluetooth adapter changes
 * if you register this callback using bt_adapter_set_visibility_mode_changed_cb().
 *
 * @see bt_adapter_set_visibility_mode_changed_cb()
 * @see bt_adapter_unset_visibility_mode_changed_cb()
 */
typedef void (*bt_adapter_visibility_mode_changed_cb)
	(int result, bt_adapter_visibility_mode_e visibility_mode, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called every second until the visibility mode is changed to #BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE.
 * @remarks  This callback function is called only if visibility mode is #BT_ADAPTER_VISIBILITY_MODE_LIMITED_DISCOVERABLE.
 * @param[in]  duration  The duration until the visibility mode is changed to #BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE (in seconds)
 * @param[in]  user_data  The user data passed from the callback registration function
 * @pre  This function will be invoked if you register this callback using bt_adapter_set_visibility_duration_changed_cb().
 * @see  bt_adapter_set_visibility_duration_changed_cb()
 * @see  bt_adapter_unset_visibility_duration_changed_cb()
 */
typedef void (*bt_adapter_visibility_duration_changed_cb)(int duration, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when the state of device discovery changes.
 *
 * @remarks If \a discovery_state is #BT_ADAPTER_DEVICE_DISCOVERY_FOUND,
 * then you can get some information, such as remote device address, remote device name, rssi, and bonding state.
 *
 * @param[in] result The result of the device discovery
 * @param[in] discovery_state The discovery state to be changed
 * @param[in] discovery_info The information of the discovered device \n
 *					If \a discovery_state is #BT_ADAPTER_DEVICE_DISCOVERY_STARTED or
 * #BT_ADAPTER_DEVICE_DISCOVERY_FINISHED, then \a discovery_info is NULL.
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre Either bt_adapter_start_device_discovery() or bt_adapter_stop_device_discovery() will invoke this function
 * if you register this callback using bt_adapter_set_device_discovery_state_changed_cb().
 *
 * @see bt_adapter_start_device_discovery()
 * @see bt_adapter_stop_device_discovery()
 * @see bt_adapter_set_device_discovery_state_changed_cb()
 * @see bt_adapter_unset_device_discovery_state_changed_cb()
 *
 */
typedef void (*bt_adapter_device_discovery_state_changed_cb)
	(int result, bt_adapter_device_discovery_state_e discovery_state, bt_adapter_device_discovery_info_s *discovery_info, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when you get bonded devices repeatedly.
 *
 * @param[in] device_info The bonded device information
 * @param[in] user_data The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop,
 * \n @c false to break out of the loop.
 * @pre bt_adapter_foreach_bonded_device() will invoke this function.
 *
 * @see bt_adapter_foreach_bonded_device()
 *
 */
typedef bool (*bt_adapter_bonded_device_cb)(bt_device_info_s *device_info, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_MODULE
 * @brief  Called when the connectable state changes.
 * @param[in] result The result of the connectable state changing
 * @param[in] connectable The connectable to be changed
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre This function will be invoked when the connectable state of local Bluetooth adapter changes
 * if you register this callback using bt_adapter_set_connectable_changed_cb().
 *
 * @see bt_adapter_set_connectable()
 * @see bt_adapter_set_connectable_changed_cb()
 * @see bt_adapter_unset_connectable_changed_cb()
 */
typedef void (*bt_adapter_connectable_changed_cb)
	(int result, bool connectable, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Called when the state of LE device discovery changes.
 *
 * @remarks If \a discovery_state is #BT_ADAPTER_LE_DEVICE_DISCOVERY_FOUND,
 * then you can get some information, such as remote LE device address, remote device name, rssi, and bonding state.
 *
 * @param[in] result The result of the LE device discovery
 * @param[in] discovery_state The discovery state to be changed
 * @param[in] discovery_info The information of the discovered LE device \n
 *					If \a discovery_state is #BT_ADAPTER_LE_DEVICE_DISCOVERY_STARTED or
 * #BT_ADAPTER_LE_DEVICE_DISCOVERY_FINISHED, then \a discovery_info is NULL.
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre Either bt_adapter_start_device_discovery() or bt_adapter_stop_device_discovery() will invoke this function
 * if you register this callback using bt_adapter_set_device_discovery_state_changed_cb().
 *
 * @see bt_adapter_le_start_device_discovery()
 * @see bt_adapter_le_stop_device_discovery()
 * @see bt_adapter_le_set_device_discovery_state_changed_cb()
 * @see bt_adapter_le_unset_device_discovery_state_changed_cb()
 *
 */
typedef void (*bt_adapter_le_device_discovery_state_changed_cb)
	(int result, bt_adapter_le_device_discovery_state_e discovery_state, bt_adapter_le_device_discovery_info_s *discovery_info, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Called when the state of advertiser changes.
 *
 * @param[out] result The result of the requested state change of advertiser
 * @param[out] advertiser The handle of the state changed advertiser
 * @param[out] adv_state The advertiser state to be changed
 * @param[out] user_data The user data passed from the start function
 *
 * @see bt_adapter_le_start_advertiser()
 * @see bt_adapter_le_stop_advertiser()
 */
typedef void (*bt_adapter_le_advertising_state_changed_cb)(int result,
		bt_advertiser_h advertiser, bt_adapter_le_advertising_state_e adv_state, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_ADAPTER_LE_MODULE
 * @brief  Called when the Bluetooth adapter le state changes.
 * @param[in]   result  The result of the adapter state changing
 * @param[in]   adapter_le_state  The adapter le state to be changed
 * @param[in]   user_data  The user data passed from the callback registration function
 * @pre Either bt_adapter_le_enable() or bt_adapter_le_disable() will invoke this callback if you register this callback using bt_adapter_le_set_state_changed_cb().
 * @see bt_adapter_le_enable()
 * @see bt_adapter_le_disable()
 * @see bt_adapter_le_set_state_changed_cb()
 * @see bt_adapter_le_unset_state_changed_cb()
 */
typedef void (*bt_adapter_le_state_changed_cb)(int result, bt_adapter_le_state_e adapter_le_state, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief Called when the process of creating bond finishes.
 * @remarks If the remote user does not respond within 60 seconds, a time out will happen with #BT_ERROR_TIMED_OUT result code.\n
 * If bt_device_cancel_bonding() is called and it returns #BT_ERROR_NONE, then this callback function will be called
 * with #BT_ERROR_CANCELLED result. \n
 * If creating a bond succeeds but service search fails, then this callback will be called with #BT_ERROR_SERVICE_SEARCH_FAILED.
 * In this case, you should try service search again by bt_device_start_service_search() to get the supported service list.
 *
 * @param[in] result The result of the bonding device
 * @param[in] device_info The device information which you creates bond with
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre Either bt_device_create_bond() will invoke this function
 * if you register this callback using bt_device_set_bond_created_cb().
 *
 * @see bt_device_create_bond()
 * @see bt_device_cancel_bonding()
 * @see bt_device_set_bond_created_cb()
 * @see bt_device_unset_bond_created_cb()
 */
typedef void (*bt_device_bond_created_cb)(int result, bt_device_info_s *device_info, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when you get connected profiles repeatedly.
 * @param[in] profile The connected Bluetooth profile
 * @param[in] user_data The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop,
 * \n @c false to break out of the loop.
 * @pre bt_device_foreach_connected_profiles() will invoke this function.
 * @see bt_device_foreach_connected_profiles()
 */
typedef bool (*bt_device_connected_profile)(bt_profile_e profile, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when the bond destroys.
 * @param[in] result The result that a bond is destroyed
 * @param[in] remote_address The address of the remote Bluetooth device to destroy bond with
 * @param[in] user_data The user data passed from the callback registration function
 * @pre bt_device_destroy_bond() will invoke this function
 * if you register this callback using bt_device_set_bond_destroyed_cb().
 *
 * @see bt_device_destroy_bond()
 * @see bt_device_set_bond_destroyed_cb()
 * @see bt_device_unset_bond_destroyed_cb()
 */
typedef void (*bt_device_bond_destroyed_cb)(int result, char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when the authorization of device changes.
 * @param[in] authorization The authorization of device
 * @param[in] remote_address The address of the remote Bluetooth device which is (un)authorized
 * @param[in] user_data The user data passed from the callback registration function
 * @pre bt_device_set_authorization() will invoke this function if you register this callback using bt_device_set_authorization_changed_cb().
 *
 * @see bt_device_set_authorization()
 * @see #bt_device_set_authorization_changed_cb()
 * @see #bt_device_unset_authorization_changed_cb()
 */
typedef void (*bt_device_authorization_changed_cb)
	(bt_device_authorization_e authorization, char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when the process of service search finishes.
 * @remark If bt_device_cancel_service_search() is called and it returns #BT_ERROR_NONE,
 * then this callback function will be called with #BT_ERROR_CANCELLED result.
 *
 * @param[in] result The result of the service searching
 * @param[in] sdp_info The structure of service lists found on a device
 * @param[in] user_data The user data passed from the callback registration function
 * @pre Either bt_device_start_service_search() will invoke this function
 * if you register this callback using  bt_device_set_service_searched_cb().
 *
 * @see bt_device_start_service_search()
 * @see bt_device_cancel_service_search()
 * @see bt_device_set_service_searched_cb()
 * @see bt_device_unset_service_searched_cb()
 *
 */
typedef void (*bt_device_service_searched_cb)(int result, bt_device_sdp_info_s *sdp_info, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_DEVICE_MODULE
 * @brief  Called when the connection state is changed.
 * @param[in] connected The connection status: (@c true = connected, @c false = disconnected)
 * @param[in] conn_info The connection information
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_device_set_connection_state_changed_cb()
 * @see bt_device_unset_connection_state_changed_cb()
 */
typedef void (*bt_device_connection_state_changed_cb)(bool connected, bt_device_connection_info_s *conn_info, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 * @brief Called when you receive data.
 *
 * @param[in] data The received data from the remote device
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre When the connected remote Bluetooth device invokes bt_socket_send_data(),
 *		this function will be invoked if you register this function using bt_socket_set_data_received_cb().
 *
 * @see bt_socket_set_data_received_cb()
 * @see bt_socket_unset_data_received_cb()
 * @see bt_socket_send_data()
 */
typedef void (*bt_socket_data_received_cb)(bt_socket_received_data_s *data, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 * @brief  Called when the socket connection state changes.
 * @param[in] result The result of connection state changing
 * @param[in] connection_state The connection state
 * @param[in] connection The connection information which is established or disconnected
 * @param[in] user_data The user data passed from the callback registration function
 * @pre Either bt_socket_connect_rfcomm() will invoke this function.
 * In addtion, bt_socket_connection_state_changed_cb() will be invoked when the socket connection state is changed.
 * @see bt_socket_listen_and_accept_rfcomm()
 * @see bt_socket_connect_rfcomm()
 * @see bt_socket_set_connection_state_changed_cb()
 * @see bt_socket_unset_connection_state_changed_cb()
 */
typedef void (*bt_socket_connection_state_changed_cb)
	(int result, bt_socket_connection_state_e connection_state, bt_socket_connection_s *connection, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_SOCKET_MODULE
 * @brief  Called when a RFCOMM connection is requested.
 * @details You must call bt_socket_accept() if you want to accept. Otherwise, you must call bt_socket_reject().
 * @param[in] socket_fd  The file descriptor of socket on which a connection is requested
 * @param[in] remote_address  The address of remote device
 * @param[in] user_data The user data passed from the callback registration function
 * @pre If you register this callback function by bt_socket_set_connection_requested_cb() and listen a socket by bt_socket_listen(),
 * bt_socket_connection_requested_cb() will be invoked.
 * @see bt_socket_listen()
 * @see bt_socket_accept()
 * @see bt_socket_reject()
 */
typedef void (*bt_socket_connection_requested_cb) (int socket_fd, const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_SERVER_MODULE
 * @brief  Called when the push is requested.
 * @details You must call bt_opp_server_accept() if you want to accept.
 * Otherwise, you must call bt_opp_server_reject().
 * @param[in] file  The path of file to be pushed
 * @param[in] size The file size (bytes)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_opp_server_initialize()
 */
typedef void (*bt_opp_server_push_requested_cb)(const char *file, int size, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_SERVER_MODULE
 * @brief  Called when an OPP connection is requested.
 * @details You must call bt_opp_server_accept_connection() if you want to accept.
 * Otherwise, you must call bt_opp_server_reject_connection().
 * @param[in] remote_address  The address of remote device
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_opp_server_initialize()
 * @see bt_opp_server_accept_connection()
 * @see bt_opp_server_reject_connection()
 */
typedef void (*bt_opp_server_connection_requested_cb)(const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_SERVER_MODULE
 * @brief  Called when a file is being transfered.
 * @param[in] file  The path of file to be pushed
 * @param[in] size The file size (bytes)
 * @param[in] percent The progress in percentage (1 ~ 100)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_opp_server_accept()
 * @see bt_opp_server_accept_connection()
 */
typedef void (*bt_opp_server_transfer_progress_cb) (const char *file, long long size, int percent, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_SERVER_MODULE
 * @brief  Called when a transfer is finished.
 * @param[in] error_code  The result of push
 * @param[in] file  The path of file to be pushed
 * @param[in] size The file size (bytes)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_opp_server_accept()
 * @see bt_opp_server_accept_connection()
 */
typedef void (*bt_opp_server_transfer_finished_cb) (int result, const char *file, long long size, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_CLIENT_MODULE
 * @brief  Called when OPP server responds to the push request.
 * @param[in] result  The result of OPP server response
 * @param[in] remote_address  The remote address
 * @param[in] user_data  The user data passed from the callback registration function
 * @pre bt_opp_client_push_files() will invoke this function.
 * @see bt_opp_client_push_files()
 */
typedef void (*bt_opp_client_push_responded_cb)(int result, const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_CLIENT_MODULE
 * @brief  Called when each file is being transfered.
 * @param[in] file  The path of file to be pushed
 * @param[in] size The file size (bytes)
 * @param[in] percent The progress in percentage (1 ~ 100). 100 means that a file is transfered completely.
 * @param[in] user_data The user data passed from the callback registration function
 * @pre bt_opp_client_push_files() will invoke this function.
 * @see bt_opp_client_push_files()
 */
typedef void (*bt_opp_client_push_progress_cb)(const char *file, long long size, int percent, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_OPP_CLIENT_MODULE
 * @brief  Called when the push request is finished.
 * @param[in] result  The result of the push request
 * @param[in] remote_address  The remote address
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_opp_client_push_files()
 */
typedef void (*bt_opp_client_push_finished_cb)(int result, const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_MODULE
 * @brief  Called when the connection state is changed.
 * @details  This callback is called when the connection state is changed.
 * When you call bt_audio_connect() or bt_audio_disconnect(), this callback is also called with error result even though these functions fail.
 * @param[in] result  The result of changing the connection state
 * @param[in] connected  The state to be changed. @a true means connected state, Otherwise, @a false.
 * @param[in] remote_address  The remote address
 * @param[in] type  The type of audio profile except #BT_AUDIO_PROFILE_TYPE_ALL
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_audio_set_connection_state_changed_cb()
 * @see bt_audio_unset_connection_state_changed_cb()
 */
typedef void (*bt_audio_connection_state_changed_cb) (int result, bool connected, const char *remote_address, bt_audio_profile_type_e type, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when the SCO(Synchronous Connection Oriented link) state is changed.
 * @details  This callback is called when the SCO state is changed.
 * When you call bt_ag_open_sco() or bt_ag_close_sco(), this callback is also called with error result even though these functions failed.
 * @param[in] result  The result of changing the connection state
 * @param[in] opened  The state to be changed: (@c true = opened, @c  false = not opened)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_sco_state_changed_cb()
 * @see bt_ag_unset_sco_state_changed_cb()
 * @see bt_ag_open_sco()
 * @see bt_ag_close_sco()
 */
typedef void (*bt_ag_sco_state_changed_cb) (int result, bool opened, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when a call handling event happened from Hands-Free.
 * @param[in] event  The call handling event happened from Hands-Free
 * @param[in] call_id  The call ID
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_call_handling_event_cb()
 * @see bt_ag_unset_call_handling_event_cb()
 */
typedef void (*bt_ag_call_handling_event_cb) (bt_ag_call_handling_event_e event, unsigned int call_id, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when a multi call handling event happened from Hands-Free.
 * @param[in] event  The call handling event happened from Hands-Free
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_multi_call_handling_event_cb()
 * @see bt_ag_unset_multi_call_handling_event_cb()
 */
typedef void (*bt_ag_multi_call_handling_event_cb) (bt_ag_multi_call_handling_event_e event, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when a DTMF(Dual Tone Multi Frequency) is transmitted from Hands-Free.
 * @param[in] dtmf  The DTMF transmitted from Hands-Free
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_dtmf_transmitted_cb()
 * @see bt_ag_unset_dtmf_transmitted_cb()
 */
typedef void (*bt_ag_dtmf_transmitted_cb) (const char *dtmf, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when the speaker gain of the remote device is changed.
 * @param[in] gain The gain of speaker (0 ~ 15)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_speaker_gain_changed_cb()
 * @see bt_ag_unset_speaker_gain_changed_cb()
 */
typedef void (*bt_ag_speaker_gain_changed_cb) (int gain, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AUDIO_AG_MODULE
 * @brief  Called when the microphone gain of the remote device is changed.
 * @param[in] gain The gain of microphone (0 ~ 15)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_ag_set_microphone_gain_changed_cb()
 * @see bt_ag_unset_microphone_gain_changed_cb()
 */
typedef void (*bt_ag_microphone_gain_changed_cb) (int gain, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Called when the connection state is changed.
 * @param[in] connected  The state to be changed. @a true means connected state, Otherwise, @a false.
 * @param[in] remote_address  The remote address
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_avrcp_target_initialize()
 * @see bt_avrcp_target_deinitialize()
 */
typedef void (*bt_avrcp_target_connection_state_changed_cb) (bool connected, const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Called when the equalizer state is changed by the remote control device.
 * @param[in] equalizer The equalizer state
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_avrcp_set_equalizer_state_changed_cb()
 * @see bt_avrcp_unset_equalizer_state_changed_cb()
 */
typedef void (*bt_avrcp_equalizer_state_changed_cb) (bt_avrcp_equalizer_state_e equalizer, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Called when the repeat mode is changed by the remote control device.
 * @param[in] repeat The repeat mode
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_avrcp_set_repeat_mode_changed_cb()
 * @see bt_avrcp_unset_repeat_mode_changed_cb()
 */
typedef void (*bt_avrcp_repeat_mode_changed_cb) (bt_avrcp_repeat_mode_e repeat, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Called when the shuffle mode is changed by the remote control device.
 * @param[in] shuffle The shuffle mode
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_avrcp_set_shuffle_mode_changed_cb()
 * @see bt_avrcp_unset_shuffle_mode_changed_cb()
 */
typedef void (*bt_avrcp_shuffle_mode_changed_cb) (bt_avrcp_shuffle_mode_e shuffle, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_AVRCP_MODULE
 * @brief  Called when the scan mode is changed by the remote control device.
 * @param[in] shuffle The shuffle mode
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_avrcp_set_scan_mode_changed_cb()
 * @see bt_avrcp_unset_scan_mode_changed_cb()
 */
typedef void (*bt_avrcp_scan_mode_changed_cb) (bt_avrcp_scan_mode_e scan, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_HID_MODULE
 * @brief  Called when the connection state is changed.
 * @details  This callback is called when the connection state is changed.
 * When you call bt_hid_host_connect() or bt_hid_host_disconnect(), this callback is also called with error result even though these functions fail.
 * @param[in] result  The result of changing the connection state
 * @param[in] connected  The state to be changed. @a true means connected state, Otherwise, @a false.
 * @param[in] remote_address  The remote address
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_hid_host_connect()
 * @see bt_hid_host_disconnect()
 */
typedef void (*bt_hid_host_connection_state_changed_cb) (int result, bool connected, const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_HDP_MODULE
 * @brief  Called when the connection is established.
 * @param[in] result  The result of connecting to the remote device
 * @param[in] remote_address  The address of connected remote device
 * @param[in] app_id  The ID of application
 * @param[in] type  The type of HDP(Health Device Profile) channel
 * @param[in] channel  The connected data channel
 * @param[in] user_data The user data passed from the callback registration function
 * @see  bt_hdp_disconnected_cb
 * @see bt_hdp_set_connection_state_changed_cb()
 * @see bt_hdp_unset_connection_state_changed_cb()
 */
typedef void (*bt_hdp_connected_cb) (int result, const char *remote_address, const char *app_id,
    bt_hdp_channel_type_e type, unsigned int channel, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_HDP_MODULE
 * @brief  Called when the connection is disconnected.
 * @param[in] result  The result of disconnecting from the remote device
 * @param[in] remote_address  The address of disconnected remote device
 * @param[in] channel  The connected data channel
 * @param[in] user_data The user data passed from the callback registration function
 * @see  bt_hdp_connected_cb
 * @see bt_hdp_set_connection_state_changed_cb()
 * @see bt_hdp_unset_connection_state_changed_cb()
 */
typedef void (*bt_hdp_disconnected_cb) (int result, const char *remote_address, unsigned int channel, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_HDP_MODULE
 * @brief  Called when the you receive the data.
 * @param[in] channel  The connected data channel
 * @param[in] data  The received data
 * @param[in] size  The size of received data (byte)
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_hdp_set_data_received_cb()
 * @see bt_hdp_unset_data_received_cb()
 */
typedef void (*bt_hdp_data_received_cb) (unsigned int channel, const char *data, unsigned int size, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called when you get the primary services repeatedly.
 * @param[in]  service  The attribute handle of service
 * @param[in]  index  The index of a service, starts from 0
 * @param[in]  total  The total number of services
 * @param[in]  user_data  The user data passed from the foreach function
 * @return  @c true to continue with the next iteration of the loop,
 * \n @c false to break out of the loop.
 * @pre  bt_gatt_foreach_primary_services() will invoke this function.
 * @see  bt_gatt_foreach_primary_services()
 */
typedef bool (*bt_gatt_primary_service_cb) (bt_gatt_attribute_h service, int index, int total, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called after the characteristics are discovered by bt_gatt_discover_characteristics().
 * @remarks  If bt_gatt_discover_characteristics() failed, then this callback function is called only once with 0 totla and NULL characteristic_handle.
 * @param[in]  result  The result of discovering
 * @param[in]  index  The index of characteristics in a service, starts from 0
 * @param[in]  total  The total number of characteristics in a service
 * @param[in]  characteristic  The attribute handle of characteristic
 * @param[in]  user_data  The user data passed from the request function
 * @return  @c true to continue with the next iteration of the loop,
 * \n @c false to break out of the loop.
 * @pre  bt_gatt_discover_characteristics() will invoke this callback.
 * @see  bt_gatt_discover_characteristics()
 */
typedef bool (*bt_gatt_characteristics_discovered_cb) (int result, int index, int total, bt_gatt_attribute_h characteristic, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called when you get the included services repeatedly.
 * @param[in]  service  The attribute handle of service
 * @param[in]  user_data  The user data passed from the foreach function
 * @return  @c true to continue with the next iteration of the loop,
 * \n @c false to break out of the loop.
 * @pre  bt_gatt_foreach_included_services() will invoke this function.
 * @see  bt_gatt_foreach_included_services()
 */
typedef bool (*bt_gatt_included_service_cb) (bt_gatt_attribute_h service, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called when a characteristic in service is changed.
 * @param[in]  characteristic  The attribute handle of characteristic
 * @param[in]  value  The value of characteristic (byte array)
 * @param[in]  value_length  The length of value
 * @param[in]  user_data  The user data passed from the callback registration function
 * @see bt_gatt_set_characteristic_changed_cb()
 * @see bt_gatt_unset_characteristic_changed_cb()
 */
typedef void (*bt_gatt_characteristic_changed_cb) (bt_gatt_attribute_h characteristic, unsigned char *value, int value_length, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called when a characteristic value is written.
 * @see bt_gatt_set_characteristic_value()
 */
typedef void (*bt_gatt_characteristic_write_cb) (int result, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called when a characteristic value is read.
 * @param[in]  value  The value of characteristic (byte array)
 * @param[in]  value_length  The length of value
 * @param[in]  user_data  The user data passed from the foreach function
 * @see bt_gatt_read_characteristic_value()
 */
typedef void (*bt_gatt_characteristic_read_cb) (unsigned char *value,
			int value_length, void *user_data);

/**
 * @ingroup  CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief  Called after the characteristics descriptors are discovered by bt_gatt_discover_characteristic_descriptor().
 * @param[in]  result  The result of discovering
 * @param[in]  characteristic_format  The format of the information data.
 *                        characteristic_format = 0x01 indicates UUIDs are 16-bits
 *                        characteristic_format = 0x02 indicates UUIDs are 128-bits
 * @param[in]  total  The total number of elements in characteristic_descriptor
 * @param[in]  characteristic descriptor  The attribute handle and the UUID of characteristic descriptor
 * @param[in]  characteristic  The attribute handle and the UUID of characteristic
 * @param[in]  user_data  The user data passed from the request function
 * @see  bt_gatt_discover_characteristic_descriptor()
 */
typedef void (*bt_gatt_characteristic_descriptor_discovered_cb) (int result,
		unsigned char characteristic_format, int total,
		bt_gatt_attribute_h characteristic_descriptor,
		bt_gatt_attribute_h characteristic, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief Called when the connection state is changed.
 * @details This callback is called when the connection state is changed.
 * When you called bt_gatt_connect() or bt_gatt_disconnect(), this callback is also called with error result even though these functions fail.
 *
 * @param[in] result The result of changing the connection state.
 * @param[in] connected The state to be changed, @a true means connected state, Otherwise, @a false.
 * @param[in] remote_address The remote_address
 * @param[in] user_data The user data passed from the callback registration function.
 *
 * @see bt_gatt_connect()
 * @see bt_gatt_disconnect()
 * @see bt_gatt_set_connection_state_changed_cb()
 * @see bt_gatt_unset_connection_state_changed_cb()
 */
typedef void (*bt_gatt_connection_state_changed_cb)(int result, bool connected, const char *remote_address, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_GATT_MODULE
 * @brief Called when the remote devices writes a characteristic to the server db.
 * @details This callback is called when the remote client device writes a characteristics
 * 			to the server attribute database.
 *
 * @param[in] char_path Characteristic path for which the value is writtten.
 * @param[in] char_value The characteristic value
 * @param[in] value_length The length of the characteristic value written.
 * @param[in] remote_address remote client device bd address.
 *
 * @see bt_gatt_connect()
 */
typedef void (*bt_gatt_remote_characteristic_write_cb) (char *char_path,
					unsigned char* char_value, int value_length,
					const char *remote_address,
					void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_PAN_NAP_MODULE
 * @brief  Called when the connection state is changed.
 * @param[in] connected  Indicates whether a client is connected or disconnected
 * @param[in] remote_address  The remote address
 * @param[in] interface_name  The interface name. For example, bnep0, bnep1.
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_nap_set_connection_state_changed_cb()
 * @see bt_nap_unset_connection_state_changed_cb()
 */
typedef void (*bt_nap_connection_state_changed_cb) (bool connected, const char *remote_address, const char *interface_name, void *user_data);

/**
 * @ingroup CAPI_NETWORK_BLUETOOTH_PAN_PANU_MODULE
 * @brief  Called when the connection state is changed.
 * @details  This callback is called when the connection state is changed.
 * When you call bt_panu_connect() or bt_panu_disconnect(), this callback is also called with error result even though these functions fail.
 * @param[in] result  The result of changing the connection state
 * @param[in] connected  The state to be changed. @a true means connected state, Otherwise, @a false.
 * @param[in] remote_address  The remote address
 * @param[in] type  The type of PAN service
 * @param[in] user_data The user data passed from the callback registration function
 * @see bt_nap_set_connection_state_changed_cb()
 * @see bt_nap_unset_connection_state_changed_cb()
 */
typedef void (*bt_panu_connection_state_changed_cb) (int result, bool connected, const char *remote_address, bt_panu_service_type_e type, void *user_data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TIZEN_NETWORK_BLUETOOTH_TYPE_H__
