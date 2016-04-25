#include "pch.h"
#include "LampParameters.h"
#include "LSFConsts.h"
#include "Bridge.h"
#include "AllJoynHelper.h"
#include "LSF.h"

using namespace BridgeRT;

LampParameters::LampParameters(LSF* pLightingService)
    : m_pLightingService (pLightingService)
    , m_lampParametersInterface (nullptr)
{
}

LampParameters::~LampParameters()
{
    if (m_lampParametersInterface != nullptr)
    {
        alljoyn_busattachment_deleteinterface(m_pLightingService->GetBus(), m_lampParametersInterface);
        m_lampParametersInterface = nullptr;
    }

    m_pLightingService = nullptr;
}

QStatus
LampParameters::Initialize()
{
    QStatus status = ER_OK;

    // Create LampParameters interface
    CHK_AJSTATUS(alljoyn_busattachment_createinterface(m_pLightingService->GetBus(), LAMP_PARAMETERS_INTERFACE_NAME, &m_lampParametersInterface));

    // Add "Version" Property to LampParameters Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampParametersInterface,
        PROPERTY_VERSION_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Energy_Usage_Milliwatts" Property to LampParameters Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampParametersInterface,
        PROPERTY_ENERGY_USAGE_MILLIWATTS_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Brightness_Lumens" Property to LampParameters Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampParametersInterface,
        PROPERTY_BRIGHTNESS_LUMENS_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));


    // Activate the LampParameters Interface
    alljoyn_interfacedescription_activate(m_lampParametersInterface);

    // add LampParameters interface to bus object
    status = alljoyn_busobject_addinterface_announced(m_pLightingService->GetBusObject(), m_lampParametersInterface);
    if (ER_BUS_IFACE_ALREADY_EXISTS == status)
    {
        // this is OK
        status = ER_OK;
    }
    CHK_AJSTATUS(status);

leave:

    return status;
}

QStatus
LampParameters::Get(_In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;

    if (strcmp(PROPERTY_VERSION_STR, propName) == 0)
    {
        uint32 version = m_pLightingService->GetLSFHandler()->LampParameters_Version;
        status = alljoyn_msgarg_set(val, "u", version);
    }
    else if (strcmp(PROPERTY_ENERGY_USAGE_MILLIWATTS_STR, propName) == 0)
    {
        uint32 energyUsageMilliwatts = m_pLightingService->GetLSFHandler()->LampParameters_EnergyUsageMilliwatts;
        status = alljoyn_msgarg_set(val, "u", energyUsageMilliwatts);
    }
    else if (strcmp(PROPERTY_BRIGHTNESS_LUMENS_STR, propName) == 0)
    {
        uint32 brightnessLumens = m_pLightingService->GetLSFHandler()->LampParameters_BrightnessLumens;
        status = alljoyn_msgarg_set(val, "u", brightnessLumens);
    }

    return status;
}