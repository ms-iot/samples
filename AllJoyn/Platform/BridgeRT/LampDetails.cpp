#include "pch.h"
#include "LampDetails.h"
#include "LSFConsts.h"
#include "Bridge.h"
#include "AllJoynHelper.h"
#include "LSF.h"

using namespace BridgeRT;
using namespace DsbCommon;
using namespace std;

LampDetails::LampDetails(LSF* pLightingService)
    : m_pLightingService (pLightingService)
    , m_lampDetailsInterface (nullptr)
{
}

LampDetails::~LampDetails()
{
    if (m_pLightingService != nullptr)
    {
        delete m_pLightingService;
        m_pLightingService = nullptr;
    }
}

QStatus
LampDetails::Initialize()
{
    QStatus status = ER_OK;

    // Create LampDetails interface
    CHK_AJSTATUS(alljoyn_busattachment_createinterface(m_pLightingService->GetBus(), LAMP_DETAILS_INTERFACE_NAME, &m_lampDetailsInterface));

    // Add "Version" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_VERSION_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Make" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MAKE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Model" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MODEL_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Type" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_TYPE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "LampType" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_LAMP_TYPE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "LampBaseType" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_LAMP_BASE_TYPE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "LampBeamAngle" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_LAMP_BEAM_ANGLE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Dimmable" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_DIMMABLE_STR,
        ARG_BOOLEAN_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Color" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_COLOR_STR,
        ARG_BOOLEAN_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "VariableColorTemp" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_VARIABLE_COLOR_TEMP_STR,
        ARG_BOOLEAN_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "HasEffects" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_HAS_EFFECTS_STR,
        ARG_BOOLEAN_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "MinVoltage" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MIN_VOLTAGE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "MaxVoltage" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MAX_VOLTAGE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "Wattage" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_WATTAGE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "IncandescentEquivalent" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_INCANDESCENT_EQUIVALENT_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "MaxLumens" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MAX_LUMENS_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "MinTemperature" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MIN_TEMPERATURE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "MaxTemperature" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_MAX_TEMPERATURE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "ColorRenderingIndex" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_COLOR_RENDERING_INDEX_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "LampID" Property to LampDetails Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampDetailsInterface,
        PROPERTY_LAMP_ID_STR,
        ARG_STRING_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));


    // Activate the LampDetails Interface
    alljoyn_interfacedescription_activate(m_lampDetailsInterface);

    // add LampDetails interface to bus object
    status = alljoyn_busobject_addinterface_announced(m_pLightingService->GetBusObject(), m_lampDetailsInterface);
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
LampDetails::Get(_In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;

    if (strcmp(PROPERTY_VERSION_STR, propName) == 0)
    {
        uint32 version = m_pLightingService->GetLSFHandler()->LampDetails_Version;
        status = alljoyn_msgarg_set(val, "u", version);
    }
    else if (strcmp(PROPERTY_MAKE_STR, propName) == 0)
    {
        uint32 make = m_pLightingService->GetLSFHandler()->LampDetails_Make;
        status = alljoyn_msgarg_set(val, "u", make);
    }
    else if (strcmp(PROPERTY_MODEL_STR, propName) == 0)
    {
        uint32 model = m_pLightingService->GetLSFHandler()->LampDetails_Model;
        status = alljoyn_msgarg_set(val, "u", model);
    }
    else if (strcmp(PROPERTY_TYPE_STR, propName) == 0)
    {
        uint32 type = m_pLightingService->GetLSFHandler()->LampDetails_Type;
        status = alljoyn_msgarg_set(val, "u", type);
    }
    else if (strcmp(PROPERTY_LAMP_TYPE_STR, propName) == 0)
    {
        uint32 lampType = m_pLightingService->GetLSFHandler()->LampDetails_LampType;
        status = alljoyn_msgarg_set(val, "u", lampType);
    }
    else if (strcmp(PROPERTY_LAMP_BASE_TYPE_STR, propName) == 0)
    {
        uint32 lampBaseType = m_pLightingService->GetLSFHandler()->LampDetails_LampBaseType;
        status = alljoyn_msgarg_set(val, "u", lampBaseType);
    }
    else if (strcmp(PROPERTY_LAMP_BEAM_ANGLE_STR, propName) == 0)
    {
        uint32 lampBeamAngle = m_pLightingService->GetLSFHandler()->LampDetails_LampBeamAngle;
        status = alljoyn_msgarg_set(val, "u", lampBeamAngle);
    }
    else if (strcmp(PROPERTY_DIMMABLE_STR, propName) == 0)
    {
        bool dimmable = m_pLightingService->GetLSFHandler()->LampDetails_Dimmable;
        status = alljoyn_msgarg_set(val, "b", dimmable);
    }
    else if (strcmp(PROPERTY_COLOR_STR, propName) == 0)
    {
        bool color = m_pLightingService->GetLSFHandler()->LampDetails_Color;
        status = alljoyn_msgarg_set(val, "b", color);
    }
    else if (strcmp(PROPERTY_VARIABLE_COLOR_TEMP_STR, propName) == 0)
    {
        bool variableColorTemp = m_pLightingService->GetLSFHandler()->LampDetails_VariableColorTemp;
        status = alljoyn_msgarg_set(val, "b", variableColorTemp);
    }
    else if (strcmp(PROPERTY_HAS_EFFECTS_STR, propName) == 0)
    {
        bool hasEffects = m_pLightingService->GetLSFHandler()->LampDetails_HasEffects;
        status = alljoyn_msgarg_set(val, "b", hasEffects);
    }
    else if (strcmp(PROPERTY_MIN_VOLTAGE_STR, propName) == 0)
    {
        uint32 minVoltage = m_pLightingService->GetLSFHandler()->LampDetails_MinVoltage;
        status = alljoyn_msgarg_set(val, "u", minVoltage);
    }
    else if (strcmp(PROPERTY_MAX_VOLTAGE_STR, propName) == 0)
    {
        uint32 maxVoltage= m_pLightingService->GetLSFHandler()->LampDetails_MaxVoltage;
        status = alljoyn_msgarg_set(val, "u", maxVoltage);
    }
    else if (strcmp(PROPERTY_WATTAGE_STR, propName) == 0)
    {
        uint32 wattage = m_pLightingService->GetLSFHandler()->LampDetails_Wattage;
        status = alljoyn_msgarg_set(val, "u", wattage);
    }
    else if (strcmp(PROPERTY_INCANDESCENT_EQUIVALENT_STR, propName) == 0)
    {
        uint32 incandescentEquivalent = m_pLightingService->GetLSFHandler()->LampDetails_IncandescentEquivalent;
        status = alljoyn_msgarg_set(val, "u", incandescentEquivalent);
    }
    else if (strcmp(PROPERTY_MAX_LUMENS_STR, propName) == 0)
    {
        uint32 maxLumens = m_pLightingService->GetLSFHandler()->LampDetails_MaxLumens;
        status = alljoyn_msgarg_set(val, "u", maxLumens);
    }
    else if (strcmp(PROPERTY_MIN_TEMPERATURE_STR, propName) == 0)
    {
        uint32 minTemperature = m_pLightingService->GetLSFHandler()->LampDetails_MinTemperature;
        status = alljoyn_msgarg_set(val, "u", minTemperature);
    }
    else if (strcmp(PROPERTY_MAX_TEMPERATURE_STR, propName) == 0)
    {
        uint32 maxTemperature = m_pLightingService->GetLSFHandler()->LampDetails_MaxTemperature;
        status = alljoyn_msgarg_set(val, "u", maxTemperature);
    }
    else if (strcmp(PROPERTY_COLOR_RENDERING_INDEX_STR, propName) == 0)
    {
        uint32 colorRenderingIndex = m_pLightingService->GetLSFHandler()->LampDetails_ColorRenderingIndex;
        status = alljoyn_msgarg_set(val, "u", colorRenderingIndex);
    }
    else if (strcmp(PROPERTY_LAMP_ID_STR, propName) == 0)
    {
        std::string lampID = ConvertTo<string>(m_pLightingService->GetLSFHandler()->LampDetails_LampID);
        status = alljoyn_msgarg_set_and_stabilize(val, "s", lampID.c_str());
    }

    return status;
}