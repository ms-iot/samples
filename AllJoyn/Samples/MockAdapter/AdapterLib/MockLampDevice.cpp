#include "pch.h"
#include "MockLampDevice.h"
#include "MockLampConsts.h"

using namespace AdapterLib;
using namespace Windows::Foundation;


namespace AdapterLib
{
    Platform::String^ DEVICE_NAME           = L"MockLampDevice";
    Platform::String^ VERSION_VALUE_NAME    = L"Version";

    Platform::String^ LAMP_STATE_CHANGED_SIGNAL_NAME    = L"LampStateChanged";
    Platform::String^ SIGNAL_PARAMETER__LAMP_ID__NAME   = L"LampID";

    Platform::String^ LAMP_ID               = L"333-333-333";
    Platform::String^ LAMP_MODEL            = "0.0.0.1";
    Platform::String^ LAMP_MANUFACTURER     = "Microsoft";

    bool LAMP_STATE_ON_OFF          = false;
    uint32 LAMP_STATE_BRIGHTNESS    = 0;
    uint32 LAMP_STATE_HUE           = 0;
    uint32 LAMP_STATE_SATURATION    = 0;
    uint32 LAMP_STATE_COLORTEMP     = 0;
}


MockLampDevice::MockLampDevice(
    Platform::String^ Name, 
    Platform::Object^ ParentObject
    )
    : MockAdapterDevice(Name, ParentObject)
    , m_lampState(nullptr)
{   
    SetSerialNumber(LAMP_ID);
    SetModel(LAMP_MODEL);
    SetVendor(LAMP_MANUFACTURER);
}

uint32 
MockLampDevice::Initialize()
{
    uint32 status = ERROR_SUCCESS;
    
    status = setLampStateChangedSignal();
    if (ERROR_SUCCESS != status)
    {
        goto leave;
    }

    try
    {
        m_lampState = ref new BridgeRT::State();
        m_lampState->IsOn       = LAMP_STATE_ON_OFF;
        m_lampState->Brightness = LAMP_STATE_BRIGHTNESS;
        m_lampState->Hue        = LAMP_STATE_HUE;
        m_lampState->Saturation = LAMP_STATE_SATURATION;
        m_lampState->ColorTemp  = LAMP_STATE_COLORTEMP;
    }
    catch (Platform::OutOfMemoryException^ ex)
    {
        status = ERROR_OUTOFMEMORY;
    }

leave:

    return status;
}

uint32 
MockLampDevice::setLampStateChangedSignal()
{
    uint32 status = ERROR_SUCCESS;
    Platform::Object^ object = nullptr;
    MockAdapterSignal^ lampStateChangedSignal = nullptr;

    try
    {
        lampStateChangedSignal = ref new MockAdapterSignal(LAMP_STATE_CHANGED_SIGNAL_NAME, this);

        // "LampID" signal parameter (string)
        object = PropertyValue::CreateString(LAMP_ID);
        if (object == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto leave;
        }

        MockAdapterValue^ lampIdSignalParameter = ref new MockAdapterValue(
            SIGNAL_PARAMETER__LAMP_ID__NAME,
            lampStateChangedSignal,
            object
            );
        lampStateChangedSignal += lampIdSignalParameter;
    }
    catch (Platform::OutOfMemoryException^ ex)
    {
        status = ERROR_OUTOFMEMORY;
        goto leave;
    }

    // Add signal to the device
    this->AddSignal(lampStateChangedSignal);

leave:

    return status;
}