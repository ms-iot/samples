//
// Module Name:
//
//      AdapterDevice.cpp
//
// Abstract:
//
//      AdapterValue, AdapterProperty, AdapterSignal, and AdapterDevice classes implementation.
//
//      Together with Adapter class, the classes declared in this file implement the IAdapter interface.
//
//
#include "pch.h"
#include "Adapter.h"
#include "AdapterDevice.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace BridgeRT;
using namespace DsbCommon;

namespace AdapterLib
{
    //
    // AdapterValue.
    // Description:
    // The class that implements BridgeRT::IAdapterValue.
    //
    AdapterValue::AdapterValue(
        String^ ObjectName,
        Object^ DefaultData // = nullptr
        )
        : name(ObjectName)
        , data(DefaultData)
    {
    }


    AdapterValue::AdapterValue(const AdapterValue^ Other)
        : name(Other->name)
        , data(Other->data)
    {
    }


    uint32 AdapterValue::Set(IAdapterValue^ Other)
    {
        this->name = Other->Name;
        this->data = Other->Data;

        return ERROR_SUCCESS;
    }


    //
    // AdapterProperty.
    // Description:
    // The class that implements BridgeRT::IAdapterProperty.
    //
    AdapterProperty::AdapterProperty(
        String^ ObjectName,
        String^ IfHint
        )
        : name(ObjectName)
        , interfaceHint(IfHint)
    {
    }


    AdapterProperty::AdapterProperty(const AdapterProperty^ Other)
        : name(Other->name)
        , attributes(Other->attributes)
        , interfaceHint(Other->interfaceHint)
    {
    }


    uint32
    AdapterProperty::Set(IAdapterProperty^ Other)
    {
        this->name = Other->Name;

        IAdapterAttributeVector^ otherAttributes = Other->Attributes;

        if (this->attributes.size() != otherAttributes->Size)
        {
            throw ref new InvalidArgumentException(L"Incompatible Adapter Properties");
        }

        for (uint32 attrInx = 0; attrInx < otherAttributes->Size; ++attrInx)
        {
            AdapterValue^ attr = static_cast<AdapterValue^>(this->attributes[attrInx]->Value);

            attr->Set(otherAttributes->GetAt(attrInx)->Value);
        }

        return ERROR_SUCCESS;
    }

    //
    // AdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    AdapterAttribute::AdapterAttribute(String^ ObjectName, E_ACCESS_TYPE accessType, Object^ DefaultData)
    {
        try
        {
            value = ref new AdapterValue(ObjectName, DefaultData);
            Access = accessType;
        }
        catch (OutOfMemoryException^ ex)
        {
            throw;
        }
    }

    AdapterAttribute::AdapterAttribute(const AdapterAttribute^ other)
        : value(other->value)
        , annotations(other->annotations)
        , covBehavior(other->covBehavior)
        , access(other->access)
    {

    }

    //
    // AdapterMethod.
    // Description:
    // The class that implements BridgeRT::IAdapterMethod.
    //
    AdapterMethod::AdapterMethod(
        String^ ObjectName
        )
        : name(ObjectName)
        , result(HRESULT_FROM_WIN32(ERROR_NOT_READY))
    {
    }


    AdapterMethod::AdapterMethod(const AdapterMethod^ Other)
        : name(Other->name)
        , description(Other->description)
        , inParams(Other->inParams)
        , outParams(Other->outParams)
        , result(Other->result)
    {
    }


    void
    AdapterMethod::InputParams::set(IAdapterValueVector^ Params)
    {
        if (Params->Size != this->inParams.size())
        {
            throw ref new InvalidArgumentException(L"Incompatible method input parameters");
        }

        for (uint32 paramInx = 0; paramInx < Params->Size; ++paramInx)
        {
            dynamic_cast<AdapterValue^>(this->inParams[paramInx])->Set(Params->GetAt(paramInx));
        }
    }


    void
    AdapterMethod::SetResult(HRESULT Hr)
    {
        this->result = Hr;
    }


    //
    // AdapterSignal.
    // Description:
    // The class that implements BridgeRT::IAdapterSignal.
    //
    AdapterSignal::AdapterSignal(
        String^ ObjectName
        )
        : name(ObjectName)
    {
    }


    AdapterSignal::AdapterSignal(const AdapterSignal^ Other)
        : name(Other->name)
        , params(Other->params)
    {
    }


    //
    // AdapterDevice.
    // Description:
    // The class that implements BridgeRT::IAdapterDevice.
    //
    AdapterDevice::AdapterDevice(
        String^ ObjectName
        )
        : name(ObjectName)
    {
    }


    AdapterDevice::AdapterDevice(
        const DEVICE_DESCRIPTOR* DeviceDescPtr
        )
        : name(DeviceDescPtr->Name)
        , vendor(DeviceDescPtr->VendorName)
        , model(DeviceDescPtr->Model)
        , firmwareVersion(DeviceDescPtr->Version)
        , serialNumber(DeviceDescPtr->SerialNumer)
        , description(DeviceDescPtr->Description)
    {
    }


    AdapterDevice::AdapterDevice(const AdapterDevice^ Other)
        : name(Other->name)
        , vendor(Other->vendor)
        , model(Other->model)
        , firmwareVersion(Other->firmwareVersion)
        , serialNumber(Other->serialNumber)
        , description(Other->description)
    {
    }


    void
    AdapterDevice::AddChangeOfValueSignal(
        IAdapterProperty^ Property,
        IAdapterValue^ Attribute)
    {
        try
        {
            AdapterSignal^ covSignal = ref new AdapterSignal(Constants::CHANGE_OF_VALUE_SIGNAL);
            covSignal += ref new AdapterValue(Constants::COV__PROPERTY_HANDLE, Property);
            covSignal += ref new AdapterValue(Constants::COV__ATTRIBUTE_HANDLE, Attribute);

            this->signals.push_back(covSignal);
        }
        catch (OutOfMemoryException^ ex)
        {
            throw;
        }
    }
} // namespace AdapterLib