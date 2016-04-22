#include "pch.h"
#include "LSFHandler.h"
#include "MockLampConsts.h"
#include "MockLampDevice.h"
#include "MockAdapter.h"

using namespace AdapterLib;
using namespace BridgeRT;

LSFHandler::LSFHandler(MockLampDevice^ parentDevice)
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
    return m_parentDevice->SerialNumber;
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
    // IsOn type is Platform::IBox<bool>^ which means that it can be equal to nullptr, true or false
    // unlike bool which only can be true or false
    if (m_parentDevice->LampState->IsOn != nullptr)
    {
        return m_parentDevice->LampState->IsOn->Value;
    }
    else
    {
        // assume false if nullptr
        return false;
    }
}

void 
LSFHandler::LampState_OnOff::set(bool isOn)
{
    // IsOn type is Platform::IBox<bool>^ which means that it can be equal to nullptr, true or false
    // unlike bool which only can be true or false
    if (m_parentDevice->LampState->IsOn != nullptr &&
        m_parentDevice->LampState->IsOn->Value == isOn)
    {
        return;
    }

    m_parentDevice->LampState->IsOn = isOn;
    notifyLampStateChange();
}


uint32 
LSFHandler::LampState_Brightness::get()
{
    // Brightness type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->Brightness != nullptr)
    {
        return m_parentDevice->LampState->Brightness->Value;
    }
    else
    {
        // assume 0 if nullptr
        return 0;
    }
}

void
LSFHandler::LampState_Brightness::set(uint32 brightness)
{
    // Brightness type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->Brightness != nullptr &&
        m_parentDevice->LampState->Brightness->Value == brightness)
    {
        return;
    }

    m_parentDevice->LampState->Brightness = brightness;
    notifyLampStateChange();
}


uint32 
LSFHandler::LampState_Hue::get()
{
    // Hue type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->Hue != nullptr)
    {
        return m_parentDevice->LampState->Hue->Value;
    }
    else
    {
        // assume 0 if nullptr
        return 0;
    }
}

void 
LSFHandler::LampState_Hue::set(uint32 hue)
{
    // Hue type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->Hue != nullptr &&
        m_parentDevice->LampState->Hue->Value == hue)
    {
        return;
    }

    m_parentDevice->LampState->Hue = hue;
    notifyLampStateChange();
}


uint32 
LSFHandler::LampState_Saturation::get()
{
    // Saturation type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->Saturation != nullptr)
    {
        return m_parentDevice->LampState->Saturation->Value;
    }
    else
    {
        // assume 0 if nullptr
        return 0;
    }
}

void
LSFHandler::LampState_Saturation::set(uint32 saturation)
{
    // Saturation type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->Saturation != nullptr &&
        m_parentDevice->LampState->Saturation->Value == saturation)
    {
        return;
    }

    m_parentDevice->LampState->Saturation = saturation;
    notifyLampStateChange();
}


uint32 
LSFHandler::LampState_ColorTemp::get()
{
    // ColorTemp type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->ColorTemp != nullptr)
    {
        return m_parentDevice->LampState->ColorTemp->Value;
    }
    else
    {
        // assume 0 if nullptr
        return 0;
    }
}

void
LSFHandler::LampState_ColorTemp::set(uint32 colorTemp)
{
    // ColorTemp type is Platform::IBox<uint32>^ which means that unlike uint32 it can be equal to nullptr
    if (m_parentDevice->LampState->ColorTemp != nullptr &&
        m_parentDevice->LampState->ColorTemp->Value == colorTemp)
    {
        return;
    }

    m_parentDevice->LampState->ColorTemp = colorTemp;
    notifyLampStateChange();
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
    UNREFERENCED_PARAMETER(TransitionPeriod);

    if (!isSameState(m_parentDevice->LampState, NewState))
    {
        m_parentDevice->LampState = NewState;
        notifyLampStateChange();
    }

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
    UNREFERENCED_PARAMETER(Period);
    UNREFERENCED_PARAMETER(Duration);
    UNREFERENCED_PARAMETER(NumPulses);
    UNREFERENCED_PARAMETER(Timestamp);

    if (!isSameState(m_parentDevice->LampState, ToState))
    {
        m_parentDevice->LampState = ToState;
        notifyLampStateChange();
    }

    *LampResponseCode = 0;

    return ERROR_SUCCESS;
}


BridgeRT::IAdapterSignal^ 
LSFHandler::LampState_LampStateChanged::get()
{
    return m_parentDevice->GetSignalByName(LAMP_STATE_CHANGED_SIGNAL_NAME);
}


void 
LSFHandler::notifyLampStateChange()
{
    MockAdapter^ adapter = dynamic_cast<MockAdapter^>(m_parentDevice->Parent);
    if (adapter == nullptr)
    {
        return;
    }

    MockAdapterSignal^ signal = m_parentDevice->GetSignalByName(LAMP_STATE_CHANGED_SIGNAL_NAME);
    if (signal == nullptr)
    {
        return;
    }

    adapter->NotifySignalListener(signal);
}


bool 
LSFHandler::isSameState(
    _In_ BridgeRT::State^ currentState,
    _In_ BridgeRT::State^ newState
    )
{
    if (currentState->IsOn != newState->IsOn ||
        currentState->Brightness != newState->Brightness ||
        currentState->Hue != newState->Hue ||
        currentState->Saturation != newState->Saturation ||
        currentState->ColorTemp != newState->ColorTemp
        )
    {
        return false;
    }

    return true;
}