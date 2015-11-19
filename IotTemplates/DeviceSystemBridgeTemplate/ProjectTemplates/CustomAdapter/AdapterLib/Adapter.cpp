//
// Module Name:
//
//      Adapter.cpp
//
// Abstract:
//
//      Adapter class implementation.
//
//      Adapter class implements the IAdapter interface.
//      When the DSB bridge component uses the adapter it instantiates an Adapter object
//      and uses it as the IAdapter.
//
//
#include "pch.h"
#include "Adapter.h"
#include "AdapterDevice.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;

using namespace BridgeRT;
using namespace DsbCommon;

namespace AdapterLib
{

    static const GUID APPLICATION_GUID = $appguid$;

    Adapter::Adapter()
    {
        Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
        Windows::ApplicationModel::PackageId^ packageId = package->Id;
        Windows::ApplicationModel::PackageVersion versionFromPkg = packageId->Version;

        this->vendor = L"$safeusername$";
        this->adapterName = L"$saferootprojectname$";
        // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
        // it is used by the Device System Bridge as root string for all services and interfaces it exposes
        this->exposedAdapterPrefix = L"com." + DsbCommon::ToLower(this->vendor->Data());
        this->exposedApplicationGuid = Platform::Guid(APPLICATION_GUID);

        if (nullptr != package &&
            nullptr != packageId )
        {
            this->exposedApplicationName = packageId->Name;
            this->version = versionFromPkg.Major.ToString() + L"." + versionFromPkg.Minor.ToString() + L"." + versionFromPkg.Revision.ToString() + L"." + versionFromPkg.Build.ToString();
        }
        else
        {
            this->exposedApplicationName = L"DeviceSystemBridge";
            this->version = L"0.0.0.0";
        }

        // Create Adapter Signals
        this->createSignals();
    }


    Adapter::~Adapter()
    {
    }


    _Use_decl_annotations_
    uint32
    Adapter::SetConfiguration(const Platform::Array<byte>^ ConfigurationData)
    {
        UNREFERENCED_PARAMETER(ConfigurationData);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::GetConfiguration(Platform::Array<byte>^* ConfigurationDataPtr)
    {
        UNREFERENCED_PARAMETER(ConfigurationDataPtr);

        return ERROR_SUCCESS;
    }


    uint32
    Adapter::Initialize()
    {
        return ERROR_SUCCESS;
    }


    uint32
    Adapter::Shutdown()
    {
        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::EnumDevices(
        ENUM_DEVICES_OPTIONS Options,
        IAdapterDeviceVector^* DeviceListPtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Options);
        UNREFERENCED_PARAMETER(RequestPtr);

        try
        {
            *DeviceListPtr = ref new BridgeRT::AdapterDeviceVector(this->devices);
        }
        catch (OutOfMemoryException^ ex)
        {
            throw;
        }

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::GetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Property);
        UNREFERENCED_PARAMETER(RequestPtr);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::SetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Property);
        UNREFERENCED_PARAMETER(RequestPtr);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::GetPropertyValue(
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

        // sanity check
        AdapterProperty^ tempProperty = dynamic_cast<AdapterProperty^>(Property);
        if (ValuePtr == nullptr ||
            tempProperty == nullptr ||
            tempProperty->Attributes == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        // find corresponding attribute
        *ValuePtr = nullptr;
        for (auto attribute : tempProperty->Attributes)
        {
            if (attribute->Value != nullptr &&
                attribute->Value->Name == AttributeName)
            {
                *ValuePtr = attribute->Value;
                return ERROR_SUCCESS;
            }
        }

        return ERROR_INVALID_HANDLE;
    }


    _Use_decl_annotations_
    uint32
    Adapter::SetPropertyValue(
        IAdapterProperty^ Property,
        IAdapterValue^ Value,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        // sanity check
        AdapterProperty^ tempProperty = dynamic_cast<AdapterProperty^>(Property);
        AdapterValue^ tempValue = dynamic_cast<AdapterValue^>(Value);
        if (tempValue == nullptr ||
            tempProperty == nullptr ||
            tempProperty->Attributes == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        // find corresponding attribute
        for (auto attribute : tempProperty->Attributes)
        {
            if (attribute->Value != nullptr &&
                attribute->Value->Name == tempValue->Name)
            {
                attribute->Value->Data = tempValue->Data;
                return ERROR_SUCCESS;
            }
        }

        return ERROR_INVALID_HANDLE;
    }


    _Use_decl_annotations_
    uint32
    Adapter::CallMethod(
        IAdapterMethod^ Method,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Method);
        UNREFERENCED_PARAMETER(RequestPtr);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::RegisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener,
        Object^ ListenerContext
        )
    {
        if (Signal == nullptr || Listener == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        AutoLock sync(&this->lock, true);

        this->signalListeners.insert(
            {
                Signal->GetHashCode(),
                SIGNAL_LISTENER_ENTRY(Signal, Listener, ListenerContext)
            }
        );

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::UnregisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener
        )
    {
        UNREFERENCED_PARAMETER(Signal);
        UNREFERENCED_PARAMETER(Listener);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::NotifySignalListener(
        IAdapterSignal^ Signal
        )
    {
        if (Signal == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        AutoLock sync(&this->lock, true);

        auto handlerRange = this->signalListeners.equal_range(Signal->GetHashCode());

        std::vector<std::pair<int, SIGNAL_LISTENER_ENTRY>> handlers(
            handlerRange.first,
            handlerRange.second);

        for (auto iter = handlers.begin(); iter != handlers.end(); ++iter)
        {
            IAdapterSignalListener^ listener = iter->second.Listener;
            Object^ listenerContext = iter->second.Context;
            listener->AdapterSignalHandler(Signal, listenerContext);
        }

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::NotifyDeviceArrival(
        IAdapterDevice^ Device
        )
    {
        if (Device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        IAdapterSignal^ deviceArrivalSignal = this->signals.at(DEVICE_ARRIVAL_SIGNAL_INDEX);
        deviceArrivalSignal->Params->GetAt(DEVICE_ARRIVAL_SIGNAL_PARAM_INDEX)->Data = Device;
        this->NotifySignalListener(deviceArrivalSignal);

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    uint32
    Adapter::NotifyDeviceRemoval(
        IAdapterDevice^ Device
        )
    {
        if (Device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        IAdapterSignal^ deviceRemovalSignal = this->signals.at(DEVICE_REMOVAL_SIGNAL_INDEX);
        deviceRemovalSignal->Params->GetAt(DEVICE_REMOVAL_SIGNAL_PARAM_INDEX)->Data = Device;
        this->NotifySignalListener(deviceRemovalSignal);

        return ERROR_SUCCESS;
    }


    void
    Adapter::createSignals()
    {
        try
        {
            // Device Arrival Signal
            AdapterSignal^ deviceArrivalSignal = ref new AdapterSignal(Constants::DEVICE_ARRIVAL_SIGNAL);
            deviceArrivalSignal += ref new AdapterValue(
                Constants::DEVICE_ARRIVAL__DEVICE_HANDLE,
                nullptr
                );

            // Device Removal Signal
            AdapterSignal^ deviceRemovalSignal = ref new AdapterSignal(Constants::DEVICE_REMOVAL_SIGNAL);
            deviceRemovalSignal += ref new AdapterValue(
                Constants::DEVICE_REMOVAL__DEVICE_HANDLE,
                nullptr
                );

            // Add Signals to Adapter Signals
            this->signals.push_back(deviceArrivalSignal);
            this->signals.push_back(deviceRemovalSignal);
        }
        catch (OutOfMemoryException^ ex)
        {
            throw;
        }
    }
} // namespace AdapterLib