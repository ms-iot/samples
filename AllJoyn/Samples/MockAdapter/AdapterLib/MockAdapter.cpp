//
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
#include "MockAdapter.h"
#include "MockAdapterDevice.h"
#include "MockDevices.h"
#include "BfiDefinitions.h"
#include "ControlPanelHandlers.h"
#include "LSFHandler.h"
#include "MockLampDevice.h"
#include "MockLampConsts.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;

using namespace BridgeRT;
using namespace DsbCommon;

namespace AdapterLib
{

    // {EF116A26-9888-47C2-AE85-B77142F24EFA}
    static const GUID APPLICATION_GUID =
    { 0xef116a26, 0x9888, 0x47c2,{ 0xae, 0x85, 0xb7, 0x71, 0x42, 0xf2, 0x4e, 0xfa } };

    MockAdapter::MockAdapter()
    {
        Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
        Windows::ApplicationModel::PackageId^ packageId = package->Id;
        Windows::ApplicationModel::PackageVersion versionFromPkg = packageId->Version;

        this->vendor = ref new String(cVendor.c_str());
        this->adapterName = ref new String(cAdapterName.c_str());
        // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
        // it is used by the Device System Bridge as root string for all services and interfaces it exposes
        this->exposedAdapterPrefix = ref new String(cAdapterPrefix.c_str());
        this->exposedApplicationGuid = Platform::Guid(APPLICATION_GUID);

        if (nullptr != package &&
            nullptr != packageId)
        {
            this->exposedApplicationName = packageId->Name;
            this->version = versionFromPkg.Major.ToString() + L"." + versionFromPkg.Minor.ToString() + L"." + versionFromPkg.Revision.ToString() + L"." + versionFromPkg.Build.ToString();
        }
        else
        {
            this->exposedApplicationName = L"DeviceSystemBridge";
            this->version = L"0.0.0.0";
        }
    }


    MockAdapter::~MockAdapter()
    {
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::SetConfiguration(const Platform::Array<byte>^ ConfigurationData)
    {
        uint32 status = ERROR_SUCCESS;

        try
        {
            String^ configurationXml = ref new String((wchar_t*)ConfigurationData->Data);
            status = WIN32_FROM_HRESULT(adapterConfig.SetConfig(configurationXml));
        }
        catch (Platform::OutOfMemoryException^ ex)
        {
            status = WIN32_FROM_HRESULT(ex->HResult);
        }

        return status;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::GetConfiguration(Platform::Array<byte>^* ConfigurationDataPtr)
    {
        uint32 status = ERROR_SUCCESS;
        String^ configurationXml = nullptr;

        status = WIN32_FROM_HRESULT(adapterConfig.GetConfig(&configurationXml));
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        status = StringToArray(configurationXml, ConfigurationDataPtr);

    done:
        return status;
    }


    uint32
    MockAdapter::Initialize()
    {
        uint32 status = WIN32_FROM_HRESULT(adapterConfig.Init());
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        status = this->createMockDevices();
        if (status != ERROR_SUCCESS)
        {
            return status;
        }

        return this->createSignals();
    }


    uint32
    MockAdapter::Shutdown()
    {
        signals.clear();
        devices.clear();
        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::EnumDevices(
        ENUM_DEVICES_OPTIONS Options,
        IAdapterDeviceVector^* DeviceListPtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Options);

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        if (DeviceListPtr == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        *DeviceListPtr = ref new BridgeRT::AdapterDeviceVector(this->devices);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::GetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        //
        // Mock adapter GetProperty() does not do much, it only
        // validates the given property.
        // In a real adapter, all property's attributes are re-read
        // from the device.
        //

        MockAdapterProperty^ mockAdapterProperty = dynamic_cast<MockAdapterProperty^>(Property);
        if (mockAdapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        MockAdapterDevice^ mockDevice = dynamic_cast<MockAdapterDevice^>(mockAdapterProperty->Parent);
        if (mockDevice == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        MockAdapter^ mockAdapter = dynamic_cast<MockAdapter^>(mockDevice->Parent);
        if (mockDevice == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        if (mockAdapter != this)
        {
            return ERROR_INVALID_HANDLE;
        }

        //
        // At this point a real adapter will re-read all property's attributes
        // from the device.
        //

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::SetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        //
        // Mock adapter SetProperty() does not do much, it only
        // validates the given property.
        // In a real adapter, all property's attributes are written
        // to the device.
        //

        MockAdapterProperty^ mockAdapterProperty = dynamic_cast<MockAdapterProperty^>(Property);
        if (mockAdapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        MockAdapterDevice^ mockDevice = dynamic_cast<MockAdapterDevice^>(mockAdapterProperty->Parent);
        if (mockDevice == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        MockAdapter^ mockAdapter = dynamic_cast<MockAdapter^>(mockDevice->Parent);
        if (mockAdapter == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        if (mockAdapter != this)
        {
            return ERROR_INVALID_HANDLE;
        }

        //
        // At this point a real adapter will write all property's attributes
        // to the device.
        //

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::GetPropertyValue(
        IAdapterProperty^ Property,
        String^ AttributeName,
        IAdapterValue^* ValuePtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        MockAdapterProperty^ mockAdapterProperty = dynamic_cast<MockAdapterProperty^>(Property);
        if (mockAdapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        auto attribute = mockAdapterProperty->GetAttributeByName(AttributeName);
        if (attribute == nullptr)
        {
            return ERROR_NOT_FOUND;
        }

        try
        {
            *ValuePtr = ref new MockAdapterValue(dynamic_cast<MockAdapterValue^>(attribute->Value));
        }
        catch (OutOfMemoryException^)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::SetPropertyValue(
        IAdapterProperty^ Property,
        IAdapterValue^ Value,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        MockAdapterProperty^ mockAdapterProperty = dynamic_cast<MockAdapterProperty^>(Property);
        if (mockAdapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        auto attribute = mockAdapterProperty->GetAttributeByName(Value->Name);
        if (attribute == nullptr)
        {
            return ERROR_NOT_FOUND;
        }

        return dynamic_cast<MockAdapterValue^>(attribute->Value)->Set(Value);
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::CallMethod(
        IAdapterMethod^ Method,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        MockAdapterMethod^ mockMethod = dynamic_cast<MockAdapterMethod^>(Method);
        if (mockMethod == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        MockAdapterDevice^ mockDevice = dynamic_cast<MockAdapterDevice^>(mockMethod->Parent);
        if (mockDevice == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        return mockDevice->MethodDispatch(Method, RequestPtr);
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::RegisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener,
        Object^ ListenerContext
        )
    {
        uint32 status = ERROR_SUCCESS;

        try
        {
            // We use the hash code as the signal key
            int mmapkey = Signal->GetHashCode();

            // Sync access to listeners list
            AutoLock sync(&this->lock, true);

            // check if the listener is already registered for the signal
            auto handlers = this->signalListeners.equal_range(mmapkey);

            for (auto iter = handlers.first; iter != handlers.second; ++iter)
            {
                if (iter->second.Listener == Listener)
                {
                    status = ERROR_ALREADY_EXISTS;
                    goto done;
                }
            }

            // add it to the map.
            this->signalListeners.insert(
            { mmapkey, SIGNAL_LISTENER_ENTRY(Signal, Listener, ListenerContext) }
            );
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

    done:

        return status;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::UnregisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener
        )
    {
        uint32 status = ERROR_NOT_FOUND;

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // Sync access to listeners list
        AutoLock sync(&this->lock, true);

        // get all the listeners for the SignalHandle
        auto handlers = this->signalListeners.equal_range(mmapkey);

        for (auto iter = handlers.first; iter != handlers.second; ++iter)
        {
            if (iter->second.Listener == Listener)
            {
                // found it remove it
                this->signalListeners.erase(iter);

                status = ERROR_SUCCESS;
                break;
            }
        }

        return status;
    }


    _Use_decl_annotations_
    uint32
    MockAdapter::NotifySignalListener(
        IAdapterSignal^ Signal
        )
    {
        uint32 status = ERROR_INVALID_HANDLE;

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // Sync access to listeners list
        AutoLock sync(&this->lock, true);

        //search for the listeners
        auto handlers = signalListeners.equal_range(mmapkey);

        for (auto iter = handlers.first; iter != handlers.second; ++iter)
        {
            //notify the listener
            IAdapterSignalListener^ listener = iter->second.Listener;
            Object^ listenerContex = iter->second.Context;

            listener->AdapterSignalHandler(Signal, listenerContex);

            status = ERROR_SUCCESS;
        }

        return status;
    }


    uint32
    MockAdapter::createMockDevices()
    {
        uint32  status = ERROR_SUCCESS;

        if (devices.size() > 0)
        {
            return ERROR_ALREADY_INITIALIZED;
        }

        for (ULONG devDescInx = 0; devDescInx < MockDevicesCount; ++devDescInx)
        {
            const MOCK_DEVICE_DESCRIPTOR* mockDeviceDescPtr = &MockDevices[devDescInx];

            try
            {
                MockAdapterDevice^ newDevice = ref new MockAdapterDevice(mockDeviceDescPtr, this);

                // Attach a Simple Control Panel Temperature Handler for the Temperature Sensor
                if (mockDeviceDescPtr->Name==L"Mock BACnet Temperature Sensor")
                {
                    newDevice->ControlPanelHandler = ref new ControlPanelHandlerTempSensor(newDevice);
                }
                else if (mockDeviceDescPtr->Name == L"Mock BACnet Dimmable Switch")
                {
                    newDevice->ControlPanelHandler = ref new ControlPanelHandlerDimmerSwitch(newDevice);
                }

                this->devices.push_back(std::move(newDevice));
            }
            catch (OutOfMemoryException^)
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto leave;
            }
        }

        try
        {
            MockLampDevice^ mockLamp = ref new MockLampDevice(DEVICE_NAME, this);
            mockLamp->Initialize();
            
            // LSF Handler for the Mock Lamp Device
            mockLamp->LightingServiceHandler = ref new LSFHandler(mockLamp);

            this->devices.push_back(std::move(mockLamp));
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto leave;
        }
    
    leave:
        return status;
    }


    uint32
    MockAdapter::createSignals()
    {
        uint32  status = ERROR_SUCCESS;

        if (signals.size() > 0)
        {
            return ERROR_ALREADY_INITIALIZED;
        }

        try
        {
            // Device arrival signal
            {
                MockAdapterSignal^ signal = ref new MockAdapterSignal(Constants::DEVICE_ARRIVAL_SIGNAL, this);

                //
                // Signal parameters
                //
                signal += ref new MockAdapterValue(
                                    Constants::DEVICE_ARRIVAL__DEVICE_HANDLE,
                                    signal,
                                    ref new MockAdapterDevice(L"DsbDevice", this) // For signature spec
                                    );

                this->signals.push_back(std::move(signal));
            }
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

        return status;
    }
} // namespace AdapterLib