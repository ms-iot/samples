#include "pch.h"
#include "LSFHandler.h"
#include "ZWaveAdapter.h"
#include "ZWaveAdapterDevice.h"
#include "ZWaveAdapterProperty.h"
#include "ZWaveAdapterValue.h"

using namespace AdapterLib;
using namespace BridgeRT;
using namespace Windows::Foundation;

#define PROPERTY_SWITCH_MULTILEVEL  L"SwitchMultilevel.Level"
#define PROPERTY_SWITCH  L"SwitchBinary.Switch"
#define ATTRIBUTE_VALUE  L"value"

LSFHandler::LSFHandler(ZWaveAdapterDevice^ parentDevice)
    : m_parentDevice(parentDevice)
{
}


LSFHandler::~LSFHandler()
{
}


// LampService Interface
uint32
LSFHandler::LampService_Version::get()
{
    return 1;
}


uint32
LSFHandler::LampService_LampServiceVersion::get()
{
    return 1;
}


Platform::Array<uint32>^
LSFHandler::LampService_LampFaults::get()
{
    Platform::Array<uint32>^ lampFaults = ref new Platform::Array<uint32>(1);
    lampFaults[0] = 0;
    return lampFaults;
}


uint32
LSFHandler::ClearLampFault(
    _In_ uint32 InLampFaultCode,
    _Out_ uint32 *LampResponseCode,
    _Out_ uint32 *OutLampFaultCode
    )
{
    *LampResponseCode = 0;
    *OutLampFaultCode = InLampFaultCode;
    return ERROR_SUCCESS;
}

// LampParameters Interface
uint32
LSFHandler::LampParameters_Version::get()
{
    return 1;
}


uint32
LSFHandler::LampParameters_EnergyUsageMilliwatts::get()
{
    return 600;
}


uint32
LSFHandler::LampParameters_BrightnessLumens::get()
{
    return 1000;
}


// LampDetails Interface
uint32
LSFHandler::LampDetails_Version::get()
{
    return 1;
}


uint32
LSFHandler::LampDetails_Make::get()
{
    return static_cast<uint32>(BridgeRT::LSFLampMake::MAKE_OEM1);
}


uint32
LSFHandler::LampDetails_Model::get()
{
    return static_cast<uint32>(BridgeRT::LSFLampModel::MODEL_LED);
}


uint32
LSFHandler::LampDetails_Type::get()
{
    return static_cast<uint32>(BridgeRT::LSFLampType::LAMPTYPE_A15);
}


uint32
LSFHandler::LampDetails_LampType::get()
{
    return static_cast<uint32>(BridgeRT::LSFLampType::LAMPTYPE_A19);
}


uint32
LSFHandler::LampDetails_LampBaseType::get()
{
    return static_cast<uint32>(BridgeRT::LSFLampBaseType::BASETYPE_E26);
}


uint32
LSFHandler::LampDetails_LampBeamAngle::get()
{
    return 160;
}


bool
LSFHandler::LampDetails_Dimmable::get()
{
    return false;
}


bool
LSFHandler::LampDetails_Color::get()
{
    return false;
}


bool
LSFHandler::LampDetails_VariableColorTemp::get()
{
    return false;
}


bool
LSFHandler::LampDetails_HasEffects::get()
{
    return false;
}


uint32
LSFHandler::LampDetails_MinVoltage::get()
{
    return 100;
}


uint32
LSFHandler::LampDetails_MaxVoltage::get()
{
    return 120;
}


uint32
LSFHandler::LampDetails_Wattage::get()
{
    return 9;
}


uint32
LSFHandler::LampDetails_IncandescentEquivalent::get()
{
    return 60;
}


uint32
LSFHandler::LampDetails_MaxLumens::get()
{
    return 620;
}


uint32
LSFHandler::LampDetails_MinTemperature::get()
{
    return 2700;
}


uint32
LSFHandler::LampDetails_MaxTemperature::get()
{
    return 9000;
}


uint32
LSFHandler::LampDetails_ColorRenderingIndex::get()
{
    return 80;
}


Platform::String^
LSFHandler::LampDetails_LampID::get()
{
    return  m_parentDevice->SerialNumber;
}




// LampState Interface
uint32
LSFHandler::LampState_Version::get()
{
    return 1;
}


bool
LSFHandler::LampState_OnOff::get()
{
    ZWaveAdapterProperty^ adapterProperty = m_parentDevice->GetPropertyByName(PROPERTY_SWITCH);
    if (adapterProperty == nullptr)
    {
        return 0;
    }

    ZWaveAdapterValue^ adapterValue = adapterProperty->GetAttributeByName(ATTRIBUTE_VALUE);
    if (adapterValue == nullptr)
    {
        return 0;
    }

    IPropertyValue^ propertyValue = dynamic_cast<IPropertyValue ^>(adapterValue->Data);
    if (propertyValue == nullptr)
    {
        return 0;
    }

    return propertyValue->GetBoolean();
}

void
LSFHandler::LampState_OnOff::set(bool isOn)
{
    ZWaveAdapterProperty^ adapterProperty = m_parentDevice->GetPropertyByName(PROPERTY_SWITCH);
    if (adapterProperty == nullptr)
    {
        return;
    }

    ZWaveAdapterValue^ adapterValue = adapterProperty->GetAttributeByName(ATTRIBUTE_VALUE);
    if (adapterValue == nullptr)
    {
        return;
    }

    IPropertyValue^ propertyValue = dynamic_cast<IPropertyValue ^>(adapterValue->Data);
    if (propertyValue == nullptr)
    {
        return;
    }

    if (propertyValue->GetBoolean() != isOn)
    {

        Platform::Object^ object = PropertyValue::CreateBoolean(isOn);
        if (object == nullptr)
        {
            return;
        }

        adapterProperty->SetValue(object);
        notifyLampStateChange();
    }
}


uint32
LSFHandler::LampState_Brightness::get()
{
    ZWaveAdapterProperty^ adapterProperty = m_parentDevice->GetPropertyByName(PROPERTY_SWITCH_MULTILEVEL);
    if (adapterProperty == nullptr)
    {
        return 0;
    }

    ZWaveAdapterValue^ adapterValue = adapterProperty->GetAttributeByName(ATTRIBUTE_VALUE);
    if (adapterValue == nullptr)
    {
        return 0;
    }

    IPropertyValue^ propertyValue = dynamic_cast<IPropertyValue ^>(adapterValue->Data);
    if (propertyValue == nullptr)
    {
        return 0;
    }

    return propertyValue->GetUInt32();
}

void
LSFHandler::LampState_Brightness::set(uint32 brightness)
{
    ZWaveAdapterProperty^ adapterProperty = m_parentDevice->GetPropertyByName(PROPERTY_SWITCH_MULTILEVEL);
    if(adapterProperty == nullptr)
    {
        return;
    }

    if (isSameUInt32Value(adapterProperty, brightness))
    {
        return;
    }

    Platform::Object^ object = PropertyValue::CreateUInt32(brightness);
    if (object == nullptr)
    {
        return;
    }

    adapterProperty->SetValue(object);
    notifyLampStateChange();
}


uint32
LSFHandler::LampState_Hue::get()
{
    return 0;
}

void
LSFHandler::LampState_Hue::set(uint32 hue)
{
    return;
}


uint32
LSFHandler::LampState_Saturation::get()
{
    return 0;
}

void
LSFHandler::LampState_Saturation::set(uint32 saturation)
{
    return;
}


uint32
LSFHandler::LampState_ColorTemp::get()
{
    return 0;
}

void
LSFHandler::LampState_ColorTemp::set(uint32 colorTemp)
{
    return;
}


uint32
LSFHandler::TransitionLampState(
    _In_ uint64 Timestamp,
    _In_ BridgeRT::State^ NewState,
    _In_ uint32 TransitionPeriod,
    _Out_ uint32 *LampResponseCode
    )
{
    UNREFERENCED_PARAMETER(Timestamp);
    UNREFERENCED_PARAMETER(NewState);
    UNREFERENCED_PARAMETER(TransitionPeriod);

    *LampResponseCode = 0;
    return ERROR_SUCCESS;
}


uint32
LSFHandler::LampState_ApplyPulseEffect(
    _In_ BridgeRT::State^ FromState,
    _In_ BridgeRT::State^ ToState,
    _In_ uint32 Period,
    _In_ uint32 Duration,
    _In_ uint32 NumPulses,
    _In_ uint64 Timestamp,
    _Out_ uint32 *LampResponseCode
    )
{
    UNREFERENCED_PARAMETER(FromState);
    UNREFERENCED_PARAMETER(ToState);
    UNREFERENCED_PARAMETER(Period);
    UNREFERENCED_PARAMETER(Duration);
    UNREFERENCED_PARAMETER(NumPulses);
    UNREFERENCED_PARAMETER(Timestamp);

    *LampResponseCode = 0;
    return ERROR_SUCCESS;
}


BridgeRT::IAdapterSignal^
LSFHandler::LampState_LampStateChanged::get()
{
    return m_parentDevice->GetSignal(Constants::LAMP_STATE_CHANGED_SIGNAL_NAME);
}

void 
LSFHandler::notifyLampStateChange()
{
    ZWaveAdapter^ adapter = dynamic_cast<ZWaveAdapter^>(m_parentDevice->GetParent());
    if (adapter == nullptr)
    {
        return;
    }

    IAdapterSignal^ lampStateChangedSignal = m_parentDevice->GetSignal(Constants::LAMP_STATE_CHANGED_SIGNAL_NAME);
    if (lampStateChangedSignal == nullptr)
    {
        return;
    }

    adapter->NotifySignalListener(lampStateChangedSignal);
}

bool
LSFHandler::isSameUInt32Value(ZWaveAdapterProperty^ adapterProperty, uint32 value)
{
    Platform::Object^ data = adapterProperty->GetAttributeByName(ATTRIBUTE_VALUE)->Data;

    IPropertyValue^ propertyValue = dynamic_cast<IPropertyValue ^>(data);
    if (propertyValue == nullptr)
    {
        return 0;
    }

    if (propertyValue->Type != PropertyType::UInt32)
    {
        return false;
    }

    if (value == propertyValue->GetUInt32())
    {
        return true;
    }
    else
    {
        return false;
    }
}