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

//
// Module Name:
//
//      Adapter.h
//
// Abstract:
//
//      Adapter class declaration.
//
//      Adapter class implements the IAdapter interface.
//      When the DSB bridge component uses the adapter it instantiates an Adapter object
//      and uses it as the IAdapter.
//
//
#pragma once

#include "AdapterDefinitions.h"
#include "PlatformInfo.h"
#include "BridgeUtils.h"

#include "ocstack.h"

namespace AdapterLib
{
    //const definitions
    static const std::wstring cVendor = L"Microsoft";
    static const std::wstring cAdapterName = L"OIC Bridge";
    static const std::wstring cDomainPrefix = L"com";

    ref class AdapterProperty;

    typedef struct
    {
        HANDLE hdl;
        OCStackResult result;
        AdapterProperty^ pProperty;
    }ResourceContext;


    //
    // Adapter class.
    // Description:
    // The class that implements the IAdapter.
    //
    public ref class Adapter sealed : public BridgeRT::IAdapter
    {
    public:
        Adapter();
        virtual ~Adapter();

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

        // Adapter signals
        virtual property BridgeRT::IAdapterSignalVector^ Signals
        {
            BridgeRT::IAdapterSignalVector^ get() { return ref new BridgeRT::AdapterSignalVector(m_signals); }
        }

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
        //      NotifySignalListener is called by the Adapter to notify a registered
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
        //  Routine Description:
        //      NotifyDeviceArrival is called by the Adapter to notify arrival
        //      of a new device.
        //
        //  Arguments:
        //
        //      Device - The object for the device that recently arrived.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid device object
        //
        uint32 NotifyDeviceArrival(
            _In_ BridgeRT::IAdapterDevice^ Device
            );

        //
        //  Routine Description:
        //      NotifyDeviceRemoval is called by the Adapter to notify removal
        //      of a device.
        //
        //  Arguments:
        //
        //      Device - The object for the device that is recently removed.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid device object
        //
        uint32 NotifyDeviceRemoval(
            _In_ BridgeRT::IAdapterDevice^ Device
            );

        internal:
            static OCStackApplicationResult OnNotifyPlatform(void* ctx, OCDoHandle handle, OCClientResponse * response);
            static OCStackApplicationResult OnNotifyDevice(void* ctx, OCDoHandle handle, OCClientResponse * response);
            static OCStackApplicationResult OnNotifyResource(void* ctx, OCDoHandle handle, OCClientResponse * response);
            static OCStackApplicationResult OnNotifyRepresentation(void* ctx, OCDoHandle handle, OCClientResponse * response);
            static OCStackApplicationResult OnNotifyObserve(void* ctx, OCDoHandle handle, OCClientResponse * response);
            static OCStackApplicationResult OnNotifyPresence(void* ctx, OCDoHandle handle, OCClientResponse * response);

    private:
        void CreateSignals();

        OCStackResult InitPlatformDiscovery();
        OCStackResult InitDeviceDiscovery(PlatformInfo* pInfo);
        OCStackResult InitResourceDiscovery(AdapterDevice^ pDevice);
        OCStackResult InitGetRequest(ResourceContext* context, const std::string& query);
        OCStackResult InitPostRequest(ResourceContext* context, const std::string& uri, OCPayload* payload);
        OCStackResult InitPutRequest(ResourceContext* context, const std::string& uri, OCPayload* payload);
        OCStackResult InitObserveRequest(AdapterProperty^ pProperty);
        OCStackResult InitPresenceRequest();

        uint32 AddToPayload(OCRepPayload*& payload, const std::string& name, Windows::Foundation::IPropertyValue ^value, const size_t dimensions[MAX_REP_ARRAY_DEPTH] = {});
        OCStackResult AddResources(OCRepPayload* payload, AdapterProperty^ pProperty, const std::wstring& namePrefix = L"");

        bool DoDeviceResourcesMatch(AdapterDevice^ pDevice, OCResourcePayload* resource);
        void MonitorDevices();

    private:
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

        // Signals
        std::vector<BridgeRT::IAdapterSignal^> m_signals;

        // Sync object
        std::recursive_mutex m_lock;

        //Sync Object for accessing OIC stack
        std::recursive_mutex m_ocStackLock;

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

        //platform list
        std::vector<std::shared_ptr<PlatformInfo>> m_platforms;

        //OIC stack processing Thread
        Windows::Foundation::IAsyncAction^ m_OICProcessThreadAction{ nullptr };

        //Device Discovery Timer
        Windows::System::Threading::ThreadPoolTimer^ m_DeviceDiscoveryTimer{ nullptr };

        //Device Monitor Timer
        Windows::System::Threading::ThreadPoolTimer^ m_DeviceMonitorTimer{ nullptr };

        static Adapter^ adapterInstance;

        // Device Arrival and Device Removal Signal Indices
        static const int DEVICE_ARRIVAL_SIGNAL_INDEX = 0;
        static const int DEVICE_ARRIVAL_SIGNAL_PARAM_INDEX = 0;
        static const int DEVICE_REMOVAL_SIGNAL_INDEX = 1;
        static const int DEVICE_REMOVAL_SIGNAL_PARAM_INDEX = 0;
    };
} // namespace AdapterLib