// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#pragma once

//
// Default configuration values
//

// BACnet stack configuration defaults
#define BACNET_DEF_NETWORK_ADDRESS      UINT16(65535)
#define BACNET_DEF_BBMD_IP_PORT         UINT16(47808)
#define BACNET_DEF_BBMD_IP_ADDR         L"127.0.0.1"
#define BACNET_DEF_BBMD_TTL             UINT32(60000)
#define BACNET_PACKET_TIMEOUT_MSEC      unsigned(500)
#define BACNET_DEF_MIN_DEVICE_INSTANCE  INT32(-1)
#define BACNET_DEF_MAX_DEVICE_INSTANCE  INT32(-1)
#define BACNET_DEF_REQUEST_PRIORITY     BACNET_REQUEST_PRIORITY_MANUAL_OPERATOR

// Valid priority value range (1..16)
#define BACNET_REQUEST_PRIORITY_HIGHEST         UINT8(1)    // Highest request priority
#define BACNET_REQUEST_PRIORITY_LOWEST          UINT8(16)   // Lowest request priority
#define BACNET_REQUEST_PRIORITY_MANUAL_OPERATOR UINT8(8)    // Manual operator request priority

// Default device discovery interval (min)
#define DEVICE_DISCOVERY_DEF_INTERVAL_MIN   DWORD(24 * 60)

// Default device discovery idle time (mSec)
#define DEVICE_DISCOVERY_DEF_IDLE_MSEC      DWORD(1000)

// Default request timeout
#if _DEBUG || DBG
    #define DEF_IO_REQ_TIMEOUT_MSEC     DWORD(20000)
#else
    #define DEF_IO_REQ_TIMEOUT_MSEC     DWORD(1000)
#endif


//
// AdapterConfig.
// Description:
//  The class that includes:
//  - BACnet adapter configuration parameters.
//  - BACnet interface (stack) parameters.
//
class AdapterConfig
{
public:

    //
    // BACnet stack parameters
    // -----------------------
    //

    //
    // BACnet network number for directed requests.
    // Valid range is from 0 to 65535 where 0 is the local
    // connection and 65535 is network broadcast.
    //
    uint16              NetworkNumber;

    //
    // BACnet mac address number.
    // Valid ranges are from 0 to 255 or a IP connection
    // string including port number like 10.1.2.3:47808.
    //
    Platform::String^   NetworkAddress;

    //
    // BACnet Device Object Instance number that you are trying to access.
    // The value should be in  the range of 0 to 4194303.
    // A range of values can also be specified by
    // using a minimum value and a maximum value.
    //
    int32               DeviceInstanceMin;
    int32               DeviceInstanceMax;

    //
    // BACnet Broadcast Management Device (BBMD)
    // server address
    //
    Platform::String^   BbmdIpAddress;

    //
    // The priority value used when setting
    // a property value.
    //
    uint8               RequestPriority;

    //
    // UDP/IP port number (0..65534) used for Foreign
    // Device Registration.  Defaults to 47808 (0xBAC0).
    //
    // Event though port number is a 16 bit, we use UINT32
    // to make it more compatible for other services.
    //
    uint32              BbmdIpPort;

    //
    // BBMD registration.
    //
    uint32              BbmdTimetoliveSeconds;

    //
    // The network interface (IP V4 address) to use
    // for BACnet server communication.
    //
    Platform::String^   NetworkInterface;

    //
    // RX packet timeout in mSec
    //
    uint32              RxPacketTimeoutMsec;

    //
    // Device discovery interval in mSec
    //
    uint32              DeviceDiscoveryIntervalMin;

    //
    // Device discovery idle time in mSec
    //
    uint32              DeviceDiscoveryIdleTimeMsec;

    //
    // BACnet adapter general configuration parameters
    // -----------------------------------------------
    //

    //
    // The list of device (partial) model names, that are allowed to be exposed.
    // - If AllowedDeviceList is empty all devices will be exposed.
    // - If AllowedDeviceList is not empty, for each discovered device, the adapter checks if the device model name
    //    contains one of the tokens, listed in AllowedDeviceList. If it does not the device is ignored.
    //
    std::vector<Platform::String^> AllowedDeviceList;


    //******************************************************************************************************
    //
    //  If the configuration file exists, then load it to the memory. Otherwise, create a configuration file
    //	with the default settings.
    //
    //  returns: S_OK on success,
    //           HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if the file is missing.
    //           Other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT Init();

    //******************************************************************************************************
    //
    //  Get the validity status of the current configuration parameters.
    //
    //  returns: true if configuration parameters are valid, otherwise false.
    //
    //******************************************************************************************************
    bool IsValid() const
    {
        return this->isValid;
    }

    //*****************************************************************************************************
    //
    //	Loads new configuration parameters from an XML string, and saves them
    //  to the adapter configuration file.
    //
    //	XmlString: A complete XML configuration
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //*****************************************************************************************************
    HRESULT SetConfig(_In_ Platform::String^ XmlString);

    //*****************************************************************************************************
    //
    //	Reads the adapter configurations into an XML string.
    //
    //	XmlStringPtr: Returned adapter configuration in string format.
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //*****************************************************************************************************
    HRESULT GetConfig(_Out_ Platform::String^* XmlStringPtr);

    // Construction
    AdapterConfig();
    virtual ~AdapterConfig();

private:

    //******************************************************************************************************
    //
    //  Loads BACnet adapter configurations from the XML disk file, with a pre-defined name.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT fromFile();

    //******************************************************************************************************
    //
    //  Saves the current configuration parameters to XML file on disk, with a pre-defined name.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT toFile();

    //******************************************************************************************************
    //
    //	Checks if the configuration file exists.
    //
    //******************************************************************************************************
    bool isConfigFilePresent();

    //******************************************************************************************************
    //
    //  Loads BACnet adapter configuration from an XmlDocument object
    //
    //  XmlDoc:  The XML document.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT fromXml(_In_ Windows::Data::Xml::Dom::XmlDocument^ XmlDoc);

    //******************************************************************************************************
    //
    //  Saves BACnet adapter configuration to an XmlDocument object
    //
    //  XmlDoc:  The XML document.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT toXml(_Inout_ Windows::Data::Xml::Dom::XmlDocument^& XmlDoc);

    // If current configuration is valid
    bool isValid;
};