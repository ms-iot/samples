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

#include "pch.h"
#include "ZWaveAdapter.h"
#include "ZWaveAdapterDevice.h"
#include "ZWaveAdapterProperty.h"
#include "ZWaveAdapterSignal.h"
#include "ZWaveAdapterValue.h"
#include "ZWaveAdapterMethod.h"
#include "SwitchControlPanelHandler.h"
#include "UniversalControlPanelHandler.h"
#include "LSFHandler.h"
#include "Misc.h"

//openzwave
#include "Options.h"
#include "Manager.h"

#include <ppltasks.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Devices::Enumeration;
using namespace Concurrency;

using namespace BridgeRT;
using namespace DsbCommon;
using namespace OpenZWave;
using namespace std;
using namespace concurrency;
using namespace Windows::System::Threading;

namespace AdapterLib
{
    // {7C78ED73-E66D-4DE0-AE67-085443E1941D}
    static const GUID DSB_ZWAVE_APPLICATION_GUID = { 0x7c78ed73, 0xe66d, 0x4de0,{ 0xae, 0x67, 0x8, 0x54, 0x43, 0xe1, 0x94, 0x1d } };

    // Light Bulb (from Dimmable Switch)
    // Manufacturer: Linear
    static const std::string LINEAR__LIGHT_BULB__PRODUCT_TYPE = "4754";
    static const std::string LINEAR__LIGHT_BULB__PRODUCT_ID = "3038";

    // Light Bulb (from Switch)
    // Manufacturer: Aeon switch
    static const std::string AEON__LIGHT_BULB__PRODUCT_TYPE = "0003";
    static const std::string AEON__LIGHT_BULB__PRODUCT_ID = "0018";

    //
    // ZWaveAdapter class.
    // Description:
    //  The class that implements the ZWave Adapter as IAdapter.
    //
    ZWaveAdapter::ZWaveAdapter()
        : m_pMgr(nullptr)
        , m_bShutdown(false)
    {
        Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
        Windows::ApplicationModel::PackageId^ packageId = package->Id;
        Windows::ApplicationModel::PackageVersion versionFromPkg = packageId->Version;

        this->m_vendor = ref new String(cVendor.c_str());
        this->m_adapterName = ref new String(cAdapterName.c_str());
        // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
        // it is used by the Device System Bridge as root string for all services and interfaces it exposes
        this->m_exposedAdapterPrefix = ref new String(cAdapterPrefix.c_str());
        this->m_exposedApplicationGuid = Platform::Guid(DSB_ZWAVE_APPLICATION_GUID);

        if (nullptr != package &&
            nullptr != packageId)
        {
            this->m_exposedApplicationName = packageId->Name;
            this->m_version = versionFromPkg.Major.ToString() + L"." + versionFromPkg.Minor.ToString() + L"." + versionFromPkg.Revision.ToString() + L"." + versionFromPkg.Build.ToString();
        }
        else
        {
            this->m_exposedApplicationName = L"DeviceSystemBridge";
            this->m_version = L"0.0.0.0";
        }
    }


    ZWaveAdapter::~ZWaveAdapter()
    {
        Shutdown();
    }

    _Use_decl_annotations_
    uint32 ZWaveAdapter::GetConfiguration(Platform::Array<byte>^* ConfigurationDataPtr)
    {
        uint32 status = ERROR_SUCCESS;

        // Sync access to configuration parameters
        AutoLock sync(&m_configLock, true);

        String^ configurationXml;
        status = WIN32_FROM_HRESULT(m_adapterConfig.GetConfig(&configurationXml));
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        status = StringToArray(configurationXml, ConfigurationDataPtr);

    done:
        return status;
    }

    _Use_decl_annotations_
    uint32 ZWaveAdapter::SetConfiguration(const Platform::Array<byte>^ ConfigurationData)
    {
        uint32 status = ERROR_SUCCESS;

        // Sync access to configuration parameters
        AutoLock sync(&m_configLock, true);

        try
        {
            String^ configurationXml = ref new String((wchar_t*)ConfigurationData->Data);
            status = WIN32_FROM_HRESULT(m_adapterConfig.SetConfig(configurationXml));
        }
        catch (Platform::OutOfMemoryException^ ex)
        {
            status = WIN32_FROM_HRESULT(ex->HResult);
            goto done;
        }

    done:
        return status;
    }

    uint32 ZWaveAdapter::Initialize()
    {
        uint32 status = ERROR_SUCCESS;

        status = WIN32_FROM_HRESULT(m_adapterConfig.Init());
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        m_bShutdown = false;

        //configurations
        Options::Create(ConvertTo<string>(AdapterConfig::GetConfigPath()), ConvertTo<string>(AdapterConfig::GetUserPath()), "");

        Options::Get()->AddOptionBool("Logging", false);    //Disable logging
        Options::Get()->AddOptionInt("PollInterval", 500);
        Options::Get()->AddOptionBool("IntervalBetweenPolls", true);
        Options::Get()->AddOptionBool("ConsoleOutput", false);
        Options::Get()->AddOptionBool("SaveConfiguration", false);

        Options::Get()->AddOptionString("ControllerPath", "", false);
        Options::Get()->AddOptionInt("ControllerInterface", (int)(Driver::ControllerInterface_Serial));
        Options::Get()->AddOptionInt("NetworkMonitorInterval", 30000);  //30 seconds


        Options::Get()->Lock();

        //instantiate the Manager object
        m_pMgr = Manager::Create();

        //add awatcher for notification
        m_pMgr->AddWatcher(OnNotification, reinterpret_cast<void*>(this));

        //create signals
        status = CreateSignals();
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        //Get the network monitor interval
        int32 nInterval;
        Options::Get()->GetOptionAsInt("NetworkMonitorInterval", &nInterval);
        m_networkMonitorTimeout = nInterval * 10000; //in 100 nano second interval

        StartDeviceDiscovery();

    done:
        return status;
    }


    uint32 ZWaveAdapter::Shutdown()
    {
        uint32 status = ERROR_SUCCESS;

        m_bShutdown = true;

        if (m_DiscoveryTimer)
        {
            m_DiscoveryTimer->Cancel();
            m_DiscoveryTimer = nullptr;
        }

        if (m_MonitorTimer)
        {
            m_MonitorTimer->Cancel();
            m_MonitorTimer = nullptr;
        }

        Manager::Destroy();
        m_pMgr = nullptr;

        Options::Destroy();

        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::EnumDevices(ENUM_DEVICES_OPTIONS Options, IAdapterDeviceVector^* DeviceListPtr, IAdapterIoRequest^* RequestPtr)
    {
        uint32 status = ERROR_SUCCESS;

        UNREFERENCED_PARAMETER(Options);

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        if (DeviceListPtr == nullptr)
        {
            status = ERROR_INVALID_PARAMETER;
            goto done;
        }

        {
            AutoLock sync(&m_deviceListLock, true);

            //just return whatever list we have. the device will be notified as they arrive
            *DeviceListPtr = ref new AdapterDeviceVector(m_devices);
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::GetProperty(IAdapterProperty^ Property, IAdapterIoRequest^* RequestPtr)
    {
        uint32 status = ERROR_SUCCESS;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        ZWaveAdapterProperty^ adapterProperty = dynamic_cast<ZWaveAdapterProperty^>(Property);
        ZWaveAdapterDevice^ device = nullptr;

        if (adapterProperty == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto done;
        }

        {
            //get the device first
            auto iter = FindDevice(m_devices, adapterProperty->m_valueId.GetHomeId(), adapterProperty->m_valueId.GetNodeId());
            if (iter == m_devices.end())
            {
                status = ERROR_INVALID_HANDLE;
                goto done;
            }

            device = dynamic_cast<ZWaveAdapterDevice^>(*iter);
        }

        //get the property object from the internal device list
        {
            auto iter = device->GetProperty(adapterProperty->m_valueId);
            if (iter == device->m_properties.end())
            {
                status = ERROR_INVALID_HANDLE;
                goto done;
            }

            //refresh value
            dynamic_cast<ZWaveAdapterProperty^>(*iter)->UpdateValue();

            auto attributes = dynamic_cast<ZWaveAdapterProperty^>(*iter)->m_attributes;
            adapterProperty->m_attributes = attributes;
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::SetProperty(IAdapterProperty^ Property, IAdapterIoRequest^* RequestPtr)
    {
        uint32 status = ERROR_SUCCESS;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        ZWaveAdapterProperty^ adapterProperty = dynamic_cast<ZWaveAdapterProperty^>(Property);
        ZWaveAdapterValue^ attribute = nullptr;

        if (adapterProperty == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto done;
        }

        //only the value field is settable
        attribute = adapterProperty->GetAttributeByName(ref new String(ValueName.c_str()));
        if (attribute == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto done;
        }

        status = adapterProperty->SetValue(attribute->Data);

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::GetPropertyValue(
                                            IAdapterProperty^ Property,
                                            String^ AttributeName,
                                            IAdapterValue^* ValuePtr,
                                            IAdapterIoRequest^* RequestPtr
                                             )
    {
        uint32 status = ERROR_SUCCESS;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        ZWaveAdapterProperty^ adapterProperty = dynamic_cast<ZWaveAdapterProperty^>(Property);
        ZWaveAdapterDevice^ device = nullptr;
        ZWaveAdapterValue^ attribute = nullptr;

        if (adapterProperty == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto done;
        }

        {
            //get the device first
            auto iter = FindDevice(m_devices, adapterProperty->m_valueId.GetHomeId(), adapterProperty->m_valueId.GetNodeId());
            if (iter == m_devices.end())
            {
                status = ERROR_INVALID_HANDLE;
                goto done;
            }

            device = dynamic_cast<ZWaveAdapterDevice^>(*iter);
        }

        //get the property object from the internal device list
        {
            auto iter = device->GetProperty(adapterProperty->m_valueId);
            if (iter == device->m_properties.end())
            {
                status = ERROR_INVALID_HANDLE;
                goto done;
            }

            //refresh value
            dynamic_cast<ZWaveAdapterProperty^>(*iter)->UpdateValue();

            attribute = dynamic_cast<ZWaveAdapterProperty^>(*iter)->GetAttributeByName(AttributeName);;
            if (attribute == nullptr)
            {
                status = ERROR_NOT_FOUND;
                goto done;
            }

            try
            {
                *ValuePtr = ref new ZWaveAdapterValue(attribute);
            }
            catch (OutOfMemoryException^)
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::SetPropertyValue(
                                            IAdapterProperty^ Property,
                                            IAdapterValue^ Value,
                                            IAdapterIoRequest^* RequestPtr
                                            )
    {
        uint32 status = ERROR_SUCCESS;

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        ZWaveAdapterProperty^ adapterProperty = dynamic_cast<ZWaveAdapterProperty^>(Property);

        if (adapterProperty == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto done;
        }

        //only setting to value attribute permitted
        if (ValueName != Value->Name->Data())
        {
            status = ERROR_NOT_SUPPORTED;
            goto done;
        }

        status = adapterProperty->SetValue(Value->Data);

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::CallMethod(IAdapterMethod^ Method, IAdapterIoRequest^* RequestPtr)
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        ZWaveAdapterMethod^ adapterMethod = dynamic_cast<ZWaveAdapterMethod^>(Method);
        if (adapterMethod == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        adapterMethod->Execute();

        return adapterMethod->HResult;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::RegisterSignalListener(
                                                    IAdapterSignal^ Signal,
                                                    IAdapterSignalListener^ Listener,
                                                    Object^ ListenerContext
                                                    )
    {
        uint32 status = ERROR_SUCCESS;

        try
        {
            // Sync access to listeners list
            AutoLock sync(&m_signalLock, true);

            // We use the hash code as the signal key
            int mmapkey = Signal->GetHashCode();

            // check if the listener is already registered for the signal
            auto handlers = m_signalListeners.equal_range(mmapkey);

            for (auto iter = handlers.first; iter != handlers.second; ++iter)
            {
                if (iter->second.Listener == Listener)
                {
                    goto done;
                }
            }

            // add it to the map.
            m_signalListeners.insert({ mmapkey, SIGNAL_LISTENER_ENTRY(Signal, Listener, ListenerContext) });
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

   done:
        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::UnregisterSignalListener(IAdapterSignal^ Signal, IAdapterSignalListener^ Listener)
    {
        uint32 status = ERROR_NOT_FOUND;

        // Sync access to listeners list
        AutoLock sync(&m_signalLock, true);

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // get all the listeners for the SignalHandle
        auto handlers = m_signalListeners.equal_range(mmapkey);

        for (auto iter = handlers.first; iter != handlers.second; ++iter)
        {
            if (iter->second.Listener == Listener)
            {
                // found it remove it
                m_signalListeners.erase(iter);

                status = ERROR_SUCCESS;
                break;
            }
        }

        return status;
    }


    _Use_decl_annotations_
    uint32 ZWaveAdapter::NotifySignalListener(IAdapterSignal^ Signal)
    {
        uint32 status = ERROR_INVALID_HANDLE;

        // Sync access to listeners list
        AutoLock sync(&m_signalLock, true);

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // Iterate through listeners
        auto handlerRange = m_signalListeners.equal_range(mmapkey);

        vector<pair<int, SIGNAL_LISTENER_ENTRY>> handlers(handlerRange.first, handlerRange.second);
        for (auto iter = handlers.begin(); iter != handlers.end(); ++iter)
        {
            // Notify the listener
            IAdapterSignalListener^ listener = iter->second.Listener;
            Object^ listenerContext = iter->second.Context;

            listener->AdapterSignalHandler(Signal, listenerContext);

            status = ERROR_SUCCESS;
        }

        return status;
    }

    uint32 ZWaveAdapter::CreateSignals()
    {
        uint32  status = ERROR_SUCCESS;

        try
        {
            // Device arrival signal
            ZWaveAdapterSignal^ signal = ref new ZWaveAdapterSignal(Constants::DEVICE_ARRIVAL_SIGNAL);

            signal->AddParam(ref new ZWaveAdapterValue(
                                    Constants::DEVICE_ARRIVAL__DEVICE_HANDLE,
                                    ref new ZWaveAdapterDevice(0, 0) //place holder
                                    )
                             );
            m_signals.push_back(signal);

            //Device removal signal
            signal = ref new ZWaveAdapterSignal(Constants::DEVICE_REMOVAL_SIGNAL);

            signal->AddParam(ref new ZWaveAdapterValue(
                                    Constants::DEVICE_REMOVAL__DEVICE_HANDLE,
                                    ref new ZWaveAdapterDevice(0, 0) //place holder
                                    )
                            );
            m_signals.push_back(signal);

        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

        return status;
    }

    vector<IAdapterDevice^>::iterator ZWaveAdapter::FindDevice(vector<IAdapterDevice^>& deviceList, uint32 homeId, uint32 nodeId)
    {
        return find_if(deviceList.begin(), deviceList.end(), [=](IAdapterDevice^ device)
        {
            return ((dynamic_cast<ZWaveAdapterDevice^>(device))->m_homeId == homeId) && ((dynamic_cast<ZWaveAdapterDevice^>(device))->m_nodeId == nodeId);
        });
    }

    IAdapterSignal^ ZWaveAdapter::GetSignal(String^ name)
    {
        for (auto signal : m_signals)
        {
            if (signal->Name == name)
            {
                return signal;
            }
        }
        return nullptr;
    }

    void ZWaveAdapter::StartDeviceDiscovery()
    {
        string path;
        int inf = static_cast<int>(Driver::ControllerInterface_Serial);

        if (Options::Get())
        {
            Options::Get()->GetOptionAsString("ControllerPath", &path);
            Options::Get()->GetOptionAsInt("ControllerInterface", &inf);

            auto selector = SerialDevice::GetDeviceSelector();

            create_task(DeviceInformation::FindAllAsync(selector))
                .then([path, inf, this](DeviceInformationCollection ^ devices)
            {
                for (auto iterator = devices->First(); iterator->HasCurrent; iterator->MoveNext())
                {
                    string currentId = ConvertTo<string>(iterator->Current->Id);
                    if (currentId.find(path) != string::npos && m_pMgr)
                    {
                        m_pMgr->AddDriver(currentId, (Driver::ControllerInterface)inf);
                    }
                }
            });
        }

        //Set the time for next discovery
        TimeSpan ts;
        ts.Duration = m_networkMonitorTimeout;
        m_DiscoveryTimer = ThreadPoolTimer::CreateTimer(ref new TimerElapsedHandler([this](ThreadPoolTimer^ timer)
        {
            StartDeviceDiscovery();
        }), ts);
    }

    void ZWaveAdapter::AddDevice(const uint32 homeId, const uint8 nodeId, bool bPending)
    {
        AutoLock sync(&m_deviceListLock, true);

        if(bPending)
        {
            ZWaveAdapterDevice^ device = ref new ZWaveAdapterDevice(homeId, nodeId);
            m_pendingDevices.push_back(device);
        }
        else
        {
            auto iter = FindDevice(m_pendingDevices, homeId, nodeId);
            if (iter != m_pendingDevices.end())
            {
                ZWaveAdapterDevice^ currDevice = dynamic_cast<ZWaveAdapterDevice^>(*iter);
                currDevice->Initialize();

                m_devices.push_back(currDevice);
                m_pendingDevices.erase(iter);

                // Create a Control Panel for *any* device
                UniversalControlPanelHandler^ myControlPanel = ref new UniversalControlPanelHandler(currDevice);
                currDevice->ControlPanelHandler = myControlPanel;

                // Create a Lighting Service Handler if the device is a lighting device
                std::string deviceProductType = m_pMgr->GetNodeProductType(homeId, nodeId);
                std::string deviceProductId = m_pMgr->GetNodeProductId(homeId, nodeId);

                if ((LINEAR__LIGHT_BULB__PRODUCT_TYPE == deviceProductType &&
                    LINEAR__LIGHT_BULB__PRODUCT_ID == deviceProductId)
                    // ||
                    // (AEON__LIGHT_BULB__PRODUCT_TYPE == deviceProductType &&
                    //  AEON__LIGHT_BULB__PRODUCT_ID == deviceProductId)
                    )
                {
                    currDevice->SetParent(this);
                    currDevice->AddLampStateChangedSignal();
                    currDevice->LightingServiceHandler = ref new LSFHandler(currDevice);
                }

                //notify the signal
                AutoLock sync2(&m_signalLock, true);

                IAdapterSignal^ signal = GetSignal(Constants::DEVICE_ARRIVAL_SIGNAL);
                if (signal != nullptr)
                {
                    // Set the 'Device_Handle' signal parameter
                    signal->Params->GetAt(0)->Data = m_devices.back();

                    NotifySignalListener(signal);
                }
            }
        }
    }

    void ZWaveAdapter::RemoveDevice(const uint32 homeId, const uint8 nodeId, bool bMoveToPending)
    {
        AutoLock sync(&m_deviceListLock, true);

        // Remove the node from our list
        auto iter = FindDevice(m_devices, homeId, nodeId);

        if (iter != m_devices.end())
        {
            //notify the signal
            AutoLock sync2(&m_signalLock, true);

            IAdapterSignal^ signal = GetSignal(Constants::DEVICE_REMOVAL_SIGNAL);
            if (signal != nullptr)
            {
                // Set the 'Device_Handle' signal parameter
                signal->Params->GetAt(0)->Data = *iter;

                NotifySignalListener(signal);
            }

            if (bMoveToPending)
            {
                m_pendingDevices.push_back(*iter);
            }

            //now erase it
            m_devices.erase(iter);
        }
        else if(!bMoveToPending)
        {
            //check if the device is in pending list and remove it
            iter = FindDevice(m_pendingDevices, homeId, nodeId);
            if (iter != m_pendingDevices.end())
            {
                m_pendingDevices.erase(iter);  //no need to notify the signal as it was never announced
            }
        }
    }

    void ZWaveAdapter::RemoveAllDevices(uint32 homeId)
    {
        AutoLock sync(&m_deviceListLock, true);
        AutoLock sync2(&m_signalLock, true);

        //remove devices from m_devices list
        auto iter = m_devices.begin();
        while(iter != m_devices.end())
        {
            auto device = dynamic_cast<ZWaveAdapterDevice^>(*iter);
            if (device->m_homeId == homeId)
            {
                //notify
                IAdapterSignal^ signal = GetSignal(Constants::DEVICE_REMOVAL_SIGNAL);
                if (signal != nullptr)
                {
                    // Set the 'Device_Handle' signal parameter
                    signal->Params->GetAt(0)->Data = device;

                    NotifySignalListener(signal);
                }
                iter = m_devices.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        //also remove devices from m_pendingDevices list
        iter = m_pendingDevices.begin();
        while (iter != m_pendingDevices.end())
        {
            auto device = dynamic_cast<ZWaveAdapterDevice^>(*iter);
            if (device->m_homeId == homeId)
            {
                iter = m_pendingDevices.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    void ZWaveAdapter::OnNotification(Notification const* _notification, void* _context)
    {
        ZWaveAdapter^ adapter = reinterpret_cast<ZWaveAdapter^>(_context);

        uint32 const homeId = _notification->GetHomeId();
        uint8 const nodeId = _notification->GetNodeId();

        try
        {
            switch (_notification->GetType())
            {
            case Notification::Type_Notification:
            {
                uint8 notifCode = _notification->GetNotification();
                switch (notifCode)
                {
                case Notification::NotificationCode::Code_Dead:
                {
                    //The device is dead, move the device to pending devices
                    adapter->RemoveDevice(nodeId, true);
                    break;
                }
                case Notification::NotificationCode::Code_Alive:
                {
                    //if we had already received the info before, move the device to m_devices
                    //and notify Device arrival.
                    //If the info has not been received yet, leave it in m_pendindDevices as more info will follow.
                    if (Manager::Get()->IsNodeInfoReceived(homeId, nodeId))
                    {
                        adapter->AddDevice(homeId, nodeId, false);
                    }

                    break;
                }
                }   //switch
                break;
            }
            case Notification::Type_DriverReady:
            {
                //Start the DeviceMonitor timer
                TimeSpan ts;
                ts.Duration = adapter->m_networkMonitorTimeout;
                adapter->m_MonitorTimer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler([homeId](ThreadPoolTimer^ timer)
                {
                    if (Manager::Get())
                    {
                       Manager::Get()->TestNetwork(homeId, 1);
                    }
                }), ts);

                break;
            }
            case Notification::Type_DriverRemoved:
            case Notification::Type_DriverFailed:
            {
                if (adapter->m_MonitorTimer)
                {
                    adapter->m_MonitorTimer->Cancel();
                    adapter->m_MonitorTimer = nullptr;
                }
                adapter->RemoveAllDevices(homeId);

                if ((_notification->GetType() == Notification::Type_DriverFailed) && !(adapter->m_bShutdown))
                {
                    //remove the driver
                    if (homeId != 0)
                    {
                        create_task([homeId]()
                        {
                            Manager::Get()->RemoveDriver(Manager::Get()->GetControllerPath(homeId));
                        });
                    }
                }

                break;
            }
            case Notification::Type_NodeAdded:
            {
                // Add the new node to our pending device list as we dont yet have all the values for the node
                adapter->AddDevice(homeId, nodeId, true);
                break;
            }
            case Notification::Type_NodeRemoved:
            {
                // Remove the node from our list
                adapter->RemoveDevice(homeId, nodeId);
                break;
            }
            case Notification::Type_NodeQueriesComplete:
            {
                //move the device from the pending list to actual list and notify the signal
                adapter->AddDevice(homeId, nodeId, false);
                break;
            }
            case Notification::Type_ValueAdded:
            {
                //Value added should be received only during the addition of node. Hence look in the pending device list
                auto iter = adapter->FindDevice(adapter->m_pendingDevices, homeId, nodeId);
                if (iter != adapter->m_pendingDevices.end())
                {
                    //add the value
                    ValueID value = _notification->GetValueID();
                    dynamic_cast<ZWaveAdapterDevice^>(*iter)->AddPropertyValue(value);
                }
                break;
            }
            case Notification::Type_ValueRemoved:
            {
                //Value removed should be received only during the addition of node. Hence look in the pending device list
                auto iter = adapter->FindDevice(adapter->m_pendingDevices, homeId, nodeId);
                if (iter != adapter->m_pendingDevices.end())
                {
                    //remove the value
                    dynamic_cast<ZWaveAdapterDevice^>(*iter)->RemovePropertyValue(_notification->GetValueID());
                }
                break;
            }
            case Notification::Type_ValueChanged:
            {
                //look into device list
                auto iter = adapter->FindDevice(adapter->m_devices, homeId, nodeId);
                if (iter != adapter->m_devices.end())
                {
                    ZWaveAdapterDevice^ device = dynamic_cast<ZWaveAdapterDevice^>(*iter);

                    //add the value
                    device->UpdatePropertyValue(_notification->GetValueID());

                    //notify the signal
                    AutoLock sync(&adapter->m_signalLock, true);

                    IAdapterSignal^ signal = device->GetSignal(Constants::CHANGE_OF_VALUE_SIGNAL);
                    if (signal != nullptr)
                    {
                        ZWaveAdapterProperty^ adapterProperty = nullptr;
                        ZWaveAdapterValue^ adapterValue = nullptr;

                        auto adapterPropertyIter = device->GetProperty(_notification->GetValueID());

                        if (adapterPropertyIter != device->m_properties.end())
                        {
                            adapterProperty = dynamic_cast<ZWaveAdapterProperty^>(*adapterPropertyIter);

                            //get the AdapterValue
                            adapterValue = adapterProperty->GetAttributeByName(ref new String(ValueName.c_str()));

                            // Set the 'Property_Handle' signal parameter
                            signal->Params->GetAt(0)->Data = adapterProperty;

                            // Set the 'Property_Handle' signal parameter
                            signal->Params->GetAt(1)->Data = adapterValue;

                            adapter->NotifySignalListener(signal);
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        catch (Exception^)
        {
            //just ignore
        }
    }

} // namespace AdapterLib
