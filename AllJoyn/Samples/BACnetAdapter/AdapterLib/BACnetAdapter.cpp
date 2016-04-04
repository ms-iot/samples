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
#include "BACnetNotification.h"
#include "BACnetInterface.h"
#include "BACnetAdapter.h"
#include "BACnetAdapterDevice.h"
#include "BfiDefinitions.h"
#include "BACnetDef.h"
#include "BridgeUtils.h"
#include "AdapterUtils.h"
#include <cctype>
#include <functional>


using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation::Collections;

using namespace BridgeRT;

// minutes to mSec
static DWORD MIN_TO_MSEC = (60 * 1000);


namespace AdapterLib
{
    // {96600ACA-8BF8-4735-8377-60D1C56ACB88}
    static const GUID DSB_BACNET_APPLICATION_GUID = { 0x96600aca, 0x8bf8, 0x4735,{ 0x83, 0x77, 0x60, 0xd1, 0xc5, 0x6a, 0xcb, 0x88 } };

    //
    // DsbBACnetAdapter class.
    // Description:
    //  The class that implements the BACnet Adapter as IAdapter.
    //
    BACnetAdapter::BACnetAdapter()
        : deviceDiscoveryThread(this, &BACnetAdapter::deviceDiscoveryThreadEntry)
    {
        Windows::ApplicationModel::Package^ package = Windows::ApplicationModel::Package::Current;
        Windows::ApplicationModel::PackageId^ packageId = package->Id;
        Windows::ApplicationModel::PackageVersion versionFromPkg = packageId->Version;

        this->vendor = ref new String(cVendor.c_str());
        this->adapterName = ref new String(cAdapterName.c_str());
        // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
        // it is used by the Device System Bridge as root string for all services and interfaces it exposes
        std::wstring adapterPrefix = cDomainPrefix + L"." + cVendor;
        std::transform(adapterPrefix.begin(), adapterPrefix.end(), adapterPrefix.begin(), std::tolower);
        this->exposedAdapterPrefix = ref new String(adapterPrefix.c_str());

        this->exposedApplicationGuid = Platform::Guid(DSB_BACNET_APPLICATION_GUID);

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


    BACnetAdapter::~BACnetAdapter()
    {
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::SetConfiguration(const Platform::Array<byte>^ ConfigurationData)
    {
        uint32 status = ERROR_SUCCESS;

        // Sync access to configuration parameters
        AutoLock sync(this->lock);

        try
        {
            String^ configurationXml = ref new String((wchar_t*)ConfigurationData->Data);
            status = WIN32_FROM_HRESULT(adapterConfig.SetConfig(configurationXml));
        }
        catch (Platform::OutOfMemoryException^ ex)
        {
            status = WIN32_FROM_HRESULT(ex->HResult);
            goto done;
        }

    done:
        return status;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::GetConfiguration(Platform::Array<byte>^* ConfigurationDataPtr)
    {
        uint32 status = ERROR_SUCCESS;

        // Sync access to configuration parameters
        AutoLock sync(this->lock);

        String^ configurationXml;
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
    BACnetAdapter::Initialize()
    {
        uint32 status = ERROR_SUCCESS;

        status = WIN32_FROM_HRESULT(this->adapterConfig.Init());
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        try
        {
            this->stackInterface = ref new BACnetInterface();
        }
        catch (OutOfMemoryException^)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        status = this->stackInterface->Initialize(adapterConfig, this);
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        status = this->createSignals();
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        if (adapterConfig.DeviceDiscoveryIntervalMin != 0)
        {
            status = this->deviceDiscoveryThread.Start(DEF_THREAD_START_TIMEOUT_MSEC);
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }
        }

    done:

        return status;
    }


    uint32
    BACnetAdapter::Shutdown()
    {
        uint32 status = ERROR_SUCCESS;

        (void)this->deviceDiscoveryThread.Stop();

        for (auto device : this->devices)
        {
            BACnetAdapterDevice^ bacnetDevice = dynamic_cast<BACnetAdapterDevice^>(device);
            DSB_ASSERT(bacnetDevice != nullptr);

            bacnetDevice->Shutdown();
        }

        if (this->stackInterface != nullptr)
        {
            status = this->stackInterface->Shutdown();
        }
        delete this->stackInterface; this->stackInterface = nullptr;

        return status;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::EnumDevices(
        ENUM_DEVICES_OPTIONS Options,
        IAdapterDeviceVector^* DeviceListPtr,
        IAdapterIoRequest^* RequestPtr
        )
    {
        uint32 status = ERROR_SUCCESS;

        UNREFERENCED_PARAMETER(Options);

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        if (DeviceListPtr == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        if (Options == ENUM_DEVICES_OPTIONS::FORCE_REFRESH)
        {
            status = this->stackInterface->EnumDevices(nullptr, nullptr, RequestPtr);
        }

        *DeviceListPtr = ref new AdapterDeviceVector(this->devices);

        return status;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::GetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        BACnetAdapterProperty^ adapterProperty = dynamic_cast<BACnetAdapterProperty^>(Property);
        if (adapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(adapterProperty->Parent);
        if (device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        return device->ReadProperty(adapterProperty->GetBACnetObjectId(), adapterProperty, RequestPtr);
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::SetProperty(
        IAdapterProperty^ Property,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        BACnetAdapterProperty^ adapterProperty = dynamic_cast<BACnetAdapterProperty^>(Property);
        if (adapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(adapterProperty->Parent);
        if (device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        //
        // With the current implementation, when setting an entire property,
        // we only set the present value.
        //

        IAdapterValue^ presentValue = adapterProperty->GetPresentValue();
        if (presentValue == nullptr)
        {
            return ERROR_INVALID_PARAMETER;
        }

        return device->WritePropertyAttribute(adapterProperty, presentValue, false, RequestPtr);
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::GetPropertyValue(
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

        BACnetAdapterProperty^ adapterProperty = dynamic_cast<BACnetAdapterProperty^>(Property);
        if (adapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(adapterProperty->Parent);
        if (device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        auto attribute = adapterProperty->GetAttributeByName(AttributeName);
        if (attribute == nullptr)
        {
            return ERROR_NOT_FOUND;
        }

        try
        {
            *ValuePtr = ref new BACnetAdapterValue(dynamic_cast<BACnetAdapterValue^>(attribute->Value));

            return device->ReadPropertyAttribute(adapterProperty, ValuePtr, RequestPtr);
        }
        catch (OutOfMemoryException^)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::SetPropertyValue(
        IAdapterProperty^ Property,
        IAdapterValue^ Value,
        IAdapterIoRequest^* RequestPtr
        )
    {
        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        BACnetAdapterProperty^ adapterProperty = dynamic_cast<BACnetAdapterProperty^>(Property);
        if (adapterProperty == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(adapterProperty->Parent);
        if (device == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        auto attribute = adapterProperty->GetAttributeByName(Value->Name);
        if (attribute == nullptr)
        {
            return ERROR_NOT_FOUND;
        }

        return device->WritePropertyAttribute(adapterProperty, Value, false, RequestPtr);
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::CallMethod(
        IAdapterMethod^ Method,
        IAdapterIoRequest^* RequestPtr
        )
    {
        UNREFERENCED_PARAMETER(Method);

        if (RequestPtr != nullptr)
        {
            *RequestPtr = nullptr;
        }

        return ERROR_NOT_SUPPORTED;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::RegisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener,
        Object^ ListenerContext
        )
    {
        uint32 status = ERROR_SUCCESS;

        BACnetAdapterSignal^ adapterSignal = dynamic_cast<BACnetAdapterSignal^>(Signal);
        if (adapterSignal == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterDevice^ bacnetDevice = dynamic_cast<BACnetAdapterDevice^>(adapterSignal->Parent);

        try
        {
            // We use the hash code as the signal key
            int mmapkey = Signal->GetHashCode();

            // Sync access to listeners list
            AutoLock sync(this->lock);

            // check if the listener is already registered for the signal
            auto handlers = this->signalListeners.equal_range(mmapkey);

            for (auto iter = handlers.first; iter != handlers.second; ++iter)
            {
                if (iter->second.Listener == Listener)
                {
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
            goto done;
        }

        //
        // A device level signal ?
        //
        if (bacnetDevice != nullptr)
        {
            status = bacnetDevice->Subscribe(Signal, true);
            if (status != ERROR_SUCCESS)
            {
                goto done;
            }
        }
        else
        {
            DSB_ASSERT(adapterSignal->Parent == this);
        }

    done:

        if (status != ERROR_SUCCESS)
        {
            (void)this->UnregisterSignalListener(Signal, Listener);
        }

        return status;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::UnregisterSignalListener(
        IAdapterSignal^ Signal,
        IAdapterSignalListener^ Listener
        )
    {
        uint32 status = ERROR_NOT_FOUND;

        BACnetAdapterSignal^ adapterSignal = dynamic_cast<BACnetAdapterSignal^>(Signal);
        if (adapterSignal == nullptr)
        {
            return ERROR_INVALID_HANDLE;
        }

        BACnetAdapterDevice^ bacnetDevice = dynamic_cast<BACnetAdapterDevice^>(adapterSignal->Parent);

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // Sync access to listeners list
        AutoLock sync(this->lock);

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

        //
        // Device level signal ?
        //
        if (bacnetDevice != nullptr)
        {
            status = bacnetDevice->Subscribe(Signal, false);
        }
        else
        {
            DSB_ASSERT(adapterSignal->Parent == this);
        }

        return status;
    }


    _Use_decl_annotations_
    uint32
    BACnetAdapter::NotifySignalListener(
        IAdapterSignal^ Signal
        )
    {
        uint32 status = ERROR_INVALID_HANDLE;

        // We use the hash code as the signal key
        int mmapkey = Signal->GetHashCode();

        // Sync access to listeners list
        AutoLock sync(this->lock);

        // Iterate through listeners
        auto handlers = signalListeners.equal_range(mmapkey);

        for (auto iter = handlers.first; iter != handlers.second; ++iter)
        {
            // Notify the listener
            IAdapterSignalListener^ listener = iter->second.Listener;
            Object^ listenerContex = iter->second.Context;

            listener->AdapterSignalHandler(Signal, listenerContex);

            status = ERROR_SUCCESS;
        }

        return status;
    }


    bool
    BACnetAdapter::IsAllowedDevice(String^ DeviceModelName)
    {
        if (this->adapterConfig.AllowedDeviceList.size() == 0)
        {
            return true;
        }

        std::wstring deviceModelName(DeviceModelName->Data());
        std::transform(deviceModelName.begin(), deviceModelName.end(), deviceModelName.begin(), tolower);
        String^ modelName = ref new String(deviceModelName.c_str());

        for (String^ devToken : this->adapterConfig.AllowedDeviceList)
        {
            if (wcsstr(modelName->Data(), devToken->Data()) != nullptr)
            {
                return true;
            }
        }

        return false;
    }


    uint32
    BACnetAdapter::createSignals()
    {
        uint32  status = ERROR_SUCCESS;

        try
        {
            // Device arrival signal
            {
                BACnetAdapterSignal^ signal = ref new BACnetAdapterSignal(Constants::DEVICE_ARRIVAL_SIGNAL, this);

                //
                // Signal parameters
                //
                signal += ref new BACnetAdapterValue(
                                    Constants::DEVICE_ARRIVAL__DEVICE_HANDLE,
                                    signal,
                                    ref new BACnetAdapterDevice(L"DsbDevice", this) // For signature spec
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


    _Use_decl_annotations_
    IAdapterSignal^
    BACnetAdapter::getSignalByName(String^ SignalName)
    {
        AutoLock sync(this->lock);

        for (UINT signalInx = 0; signalInx < this->signals.size(); ++signalInx)
        {
            if (_wcsicmp(this->signals[signalInx]->Name->Data(), SignalName->Data()) == 0)
            {
                return this->signals[signalInx];
            }
        }

        return nullptr;
    }


    DWORD
    BACnetAdapter::deviceDiscoveryThreadEntry()
    {
        DWORD pollIntervalmSec = adapterConfig.DeviceDiscoveryIntervalMin * MIN_TO_MSEC;
        bool isDone = false;

        this->deviceDiscoveryThread.SetStartStatus(ERROR_SUCCESS);

        while (!isDone)
        {
            DWORD waitStatus = ::WaitForSingleObjectEx(this->deviceDiscoveryThread.GetStopEvent(), pollIntervalmSec, FALSE);

            switch (waitStatus)
            {
            case WAIT_OBJECT_0:
                isDone = true;
                break;

            case WAIT_TIMEOUT:
                // Refresh our cached devices
                (void)this->stackInterface->EnumDevices(nullptr, nullptr, nullptr);
                break;

            default:
                DSB_ASSERT(FALSE);
                break;
            }

        } // not done

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    void
    BACnetAdapter::OnEvent(BACNET_EVENT_PARAMETERS^ EventParameters)
    {
        switch (EventParameters->Type)
        {
        case BACNET_EVENT_PARAMETERS::EventType_NewDevice:
            this->onNewDeviceEvent(EventParameters);
            break;

        case BACNET_EVENT_PARAMETERS::EventType_ChangeOfValue:
            this->onChangeOfValue(EventParameters);
            break;

        case BACNET_EVENT_PARAMETERS::EventType_SetValueAck:
            this->onSetValueAck(EventParameters);
            break;
        }
    }


    _Use_decl_annotations_
    void
    BACnetAdapter::onNewDeviceEvent(BACNET_EVENT_PARAMETERS^ EventParameters)
    {
        BACnetAdapterDevice^ newDevice = ref new BACnetAdapterDevice(this);

        DWORD status = newDevice->Init(
                                    EventParameters->AsNewDevice.DeviceId,
                                    EventParameters->AsNewDevice.VendorId,
                                    this->stackInterface
                                    );
        if (status != ERROR_SUCCESS)
        {
            if (status != ERROR_MOD_NOT_FOUND) { DSB_ASSERT(FALSE); }
            // Log and leave
            return;
        }

        //
        // Add the device to device db.
        //

        try
        {
            AutoLock sync(this->lock);

            auto insertRes = this->deviceLookup.insert(std::pair<ULONG, BACnetAdapterDevice^>(EventParameters->AsNewDevice.DeviceId, newDevice));
            if (insertRes.second)
            {
                this->devices.push_back(std::move(newDevice));
            }
            else
            {
                DSB_ASSERT(FALSE);
                return;
            }

            //
            // Send 'Device Arrival' notification...
            //

            IAdapterSignal^ deviceArrivalSignal = this->getSignalByName(Constants::DEVICE_ARRIVAL_SIGNAL);
            if (deviceArrivalSignal == nullptr)
            {
                DSB_ASSERT(FALSE);
                // Log and leave
                return;
            }

            // Set the 'Device_Handle' signal parameter
            DSB_ASSERT(deviceArrivalSignal->Params->Size == DeviceArrivalSignal_MAX);
            deviceArrivalSignal->Params->GetAt(DeviceArrivalSignal_DeviceHandle)->Data = insertRes.first->second;

            (void)this->NotifySignalListener(deviceArrivalSignal);
        }
        catch (std::bad_alloc)
        {
            DSB_ASSERT(FALSE);
            // Log and leave
            return;
        }
    }


    _Use_decl_annotations_
    void
    BACnetAdapter::onChangeOfValue(BACNET_EVENT_PARAMETERS^ EventParameters)
    {
        auto deviceLookupIter = this->deviceLookup.find(EventParameters->AsCOV.DeviceId);
        if (deviceLookupIter == this->deviceLookup.end())
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(deviceLookupIter->second);
        if (device == nullptr)
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetAdapterProperty^ adapterProperty = device->GetPropertyByObjectId(EventParameters->AsCOV.PropertyObectId);
        if (adapterProperty == nullptr)
        {
            DSB_ASSERT(FALSE);
            return;
        }

        if (adapterProperty->SetPresentValue(EventParameters->AsCOV.PresentValue) != ERROR_SUCCESS)
        {
            DSB_ASSERT(FALSE);
        }

        adapterProperty->NotifyCovSignal();
    }


    _Use_decl_annotations_
    void
    BACnetAdapter::onSetValueAck(BACNET_EVENT_PARAMETERS^ EventParameters)
    {
        //
        // Update the cache...
        //

        auto deviceLookupIter = this->deviceLookup.find(EventParameters->AsSetValueAck.DeviceId);
        if (deviceLookupIter == this->deviceLookup.end())
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetAdapterDevice^ device = dynamic_cast<BACnetAdapterDevice^>(deviceLookupIter->second);
        if (device == nullptr)
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetAdapterProperty^ adapterProperty = device->GetPropertyByObjectId(EventParameters->AsSetValueAck.PropertyObectId);
        if (adapterProperty == nullptr)
        {
            DSB_ASSERT(FALSE);
            return;
        }

        (void)adapterProperty->SetAttributeByPropertyId(
                            BACNET_PROPERTY_ID(EventParameters->AsSetValueAck.BACnetPorpertyId),
                            EventParameters->AsSetValueAck.CurrentValue
                            );
    }

} // namespace AdapterLib