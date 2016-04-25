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

#include "AdapterDefinitions.h"
#include "AdapterConfig.h"

#include "Manager.h"
#include "Notification.h"

#include <string>
#include <map>

namespace AdapterLib
{
    //const definitions
    static const std::wstring cVendor = L"Microsoft";
    static const std::wstring cAdapterName = L"ZWave Bridge";
    static const std::wstring cDomainPrefix = L"com";

    //
    // ZWaveAdapter class.
    // Description:
    //  The class that implements the ZWave Adapter as IAdapter.
    //
    public ref class ZWaveAdapter sealed
        : public BridgeRT::IAdapter
    {
    public:
        //
        // Adapter information
        //
        virtual property Platform::String^ Vendor
        {
            Platform::String^ get() { return m_vendor; }
        }
        virtual property Platform::String^ AdapterName
        {
            Platform::String^ get() { return m_adapterName; }
        }
        virtual property Platform::String^ Version
        {
            Platform::String^ get() { return m_version; }
        }
        virtual property Platform::String^ ExposedAdapterPrefix
        {
            Platform::String^ get() { return this->m_exposedAdapterPrefix; }
        }
        virtual property Platform::String^ ExposedApplicationName
        {
            Platform::String^ get() { return this->m_exposedApplicationName; }
        }
        virtual property Platform::Guid ExposedApplicationGuid
        {
            Platform::Guid get() { return this->m_exposedApplicationGuid; }
        }

        //
        // Adapter signals
        //
        virtual property BridgeRT::IAdapterSignalVector^ Signals
        {
            BridgeRT::IAdapterSignalVector^ get() { return ref new BridgeRT::AdapterSignalVector(m_signals); }
        }

        //
        // IAdapter interface
        //
        virtual uint32 GetConfiguration(_Out_ Platform::Array<byte>^* ConfigurationDataPtr);
        virtual uint32 SetConfiguration(_In_ const Platform::Array<byte>^ ConfigurationData);

        virtual uint32 Initialize();
        virtual uint32 Shutdown();

        virtual uint32 EnumDevices(
            _In_ BridgeRT::ENUM_DEVICES_OPTIONS Options,
            _Out_ BridgeRT::IAdapterDeviceVector^* DeviceListPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
            );

        virtual uint32 GetProperty(
            _Inout_ BridgeRT::IAdapterProperty^ Property,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
            );
        virtual uint32 SetProperty(
            _In_ BridgeRT::IAdapterProperty^ Property,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
            );

        virtual uint32 GetPropertyValue(
            _In_ BridgeRT::IAdapterProperty^ Property,
            _In_ Platform::String^ AttributeName,
            _Out_ BridgeRT::IAdapterValue^* ValuePtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
            );
        virtual uint32 SetPropertyValue(
            _In_ BridgeRT::IAdapterProperty^ Property,
            _In_ BridgeRT::IAdapterValue^ Value,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
            );

        virtual uint32 CallMethod(
            _Inout_ BridgeRT::IAdapterMethod^ Method,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
            );

        virtual uint32 RegisterSignalListener(
            _In_ BridgeRT::IAdapterSignal^ Signal,
            _In_ BridgeRT::IAdapterSignalListener^ Listener,
            _In_opt_ Platform::Object^ ListenerContext
            );
        virtual uint32 UnregisterSignalListener(
            _In_ BridgeRT::IAdapterSignal^ Signal,
            _In_ BridgeRT::IAdapterSignalListener^ Listener
            );

        //
        //  Routine Description:
        //      NotifySignalListener is called by the Adapter to notify and registered
        //      signal listener of an intercepted signal.
        //
        //  Arguments:
        //
        //      Signal - The signal object to notify listeners.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid signal object.
        //
        uint32 NotifySignalListener(
            _In_ BridgeRT::IAdapterSignal^ Signal
            );

        //
        // Construction
        //
        ZWaveAdapter();
        virtual ~ZWaveAdapter();

    internal:
        static void OnNotification(OpenZWave::Notification const * _notification, void * _context);

    private:
        std::vector<BridgeRT::IAdapterDevice^>::iterator ZWaveAdapter::FindDevice(
            std::vector<BridgeRT::IAdapterDevice^>& deviceList,
            uint32 homeId,
            uint32 nodeId
            );

        uint32 CreateSignals();
        BridgeRT::IAdapterSignal^ GetSignal(Platform::String^ name);
        void StartDeviceDiscovery();
        void AddDevice(const uint32 homeId, const uint8 nodeId, bool bPending);
        void RemoveDevice(const uint32 homeId, const uint8 nodeId, bool bMoveToPending = false);
        void RemoveAllDevices(uint32 homeId);

    private:

        // Adapter information
        Platform::String^ m_vendor;
        Platform::String^ m_adapterName;
        Platform::String^ m_version;
        // the prefix for AllJoyn service should be something like
        // com.mycompany (only alpha num and dots) and is used by the Device System Bridge
        // as root string for all services and interfaces it exposes
        Platform::String^ m_exposedAdapterPrefix;

        // name and GUID of the DSB/Adapter application that will be published on AllJoyn
        Platform::String^ m_exposedApplicationName;
        Platform::Guid m_exposedApplicationGuid;

        // Devices
        std::vector<BridgeRT::IAdapterDevice^> m_devices;
        std::vector<BridgeRT::IAdapterDevice^> m_pendingDevices;

        // Signals
        std::vector<BridgeRT::IAdapterSignal^> m_signals;

        // Sync object
        std::recursive_mutex m_configLock;
        std::recursive_mutex m_deviceListLock;
        std::recursive_mutex m_signalLock;

        //Adapter Config
        AdapterConfig m_adapterConfig;

        //
        // Signal listener entry
        //
        struct SIGNAL_LISTENER_ENTRY
        {
            SIGNAL_LISTENER_ENTRY(
                BridgeRT::IAdapterSignal^ SignalToRegisterTo,
                BridgeRT::IAdapterSignalListener^ ListenerObject,
                Platform::Object^ ListenerContext
                )
                : Signal(SignalToRegisterTo)
                , Listener(ListenerObject)
                , Context(ListenerContext)
            {
            }

            // The  signal object
            BridgeRT::IAdapterSignal^ Signal;

            // The listener object
            BridgeRT::IAdapterSignalListener^ Listener;

            //
            // The listener context that will be
            // passed to the signal handler
            //
            Platform::Object^ Context;
        };

        // A map of signal handle (object's hash code) and related listener entry
        std::multimap<int, SIGNAL_LISTENER_ENTRY> m_signalListeners;

        OpenZWave::Manager *m_pMgr;
        Windows::System::Threading::ThreadPoolTimer^ m_MonitorTimer{ nullptr };
        Windows::System::Threading::ThreadPoolTimer^ m_DiscoveryTimer{ nullptr };

        bool m_bShutdown;

       int64 m_networkMonitorTimeout;
    };

} // namespace AdapterLib
