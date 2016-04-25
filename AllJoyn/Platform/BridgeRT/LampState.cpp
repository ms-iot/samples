#include "pch.h"
#include "LampState.h"
#include "LSFConsts.h"
#include "Bridge.h"
#include "AllJoynHelper.h"
#include "LSF.h"
#include "BridgeDevice.h"
#include "BridgeUtils.h"

using namespace BridgeRT;
using namespace Windows::Foundation;

LampState::LampState(LSF* pLightingService)
    : m_pLightingService(pLightingService)
    , m_lampStateInterface (nullptr)
{
    m_pLSFSignalHandler = ref new LSFSignalHandler(this);
}

LampState::~LampState()
{
    if (m_lampStateInterface != nullptr)
    {
        alljoyn_busattachment_deleteinterface(m_pLightingService->GetBus(), m_lampStateInterface);
        m_lampStateInterface = nullptr;
    }
    m_pLSFSignalHandler = nullptr;
    m_pLightingService = nullptr;
}

QStatus
LampState::Initialize()
{
    QStatus status = ER_OK;
    uint32 lampStateStatus = ERROR_SUCCESS;
    QCC_BOOL found = false;

    // Create LampService interface
    CHK_AJSTATUS(alljoyn_busattachment_createinterface(m_pLightingService->GetBus(), LAMP_STATE_INTERFACE_NAME, &m_lampStateInterface));

    // Add "Version" Property to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampStateInterface,
        PROPERTY_VERSION_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "OnOff" Property to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampStateInterface,
        PROPERTY_ON_OFF_STR,
        ARG_BOOLEAN_STR,
        ALLJOYN_PROP_ACCESS_RW
        ));

    // Add "Hue" Property to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampStateInterface,
        PROPERTY_HUE_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_RW
        ));

    // Add "Saturation" Property to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampStateInterface,
        PROPERTY_SATURATION_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_RW
        ));

    // Add "ColorTemp" Property to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampStateInterface,
        PROPERTY_COLOR_TEMP_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_RW
        ));

    // Add "Brightness" Property to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampStateInterface,
        PROPERTY_BRIGHTNESS_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_RW
        ));



    // Add "TransitionLampState" method to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addmethod(
        m_lampStateInterface,
        METHOD_TRANSITION_LAMP_STATE_STR,
        METHOD_TRANSITION_LAMP_STATE_INPUT_SIGNATURE_STR,
        METHOD_TRANSITION_LAMP_STATE_OUTPUT_SIGNATURE_STR,
        METHOD_TRANSITION_LAMP_STATE_ARG_NAMES_STR,
        0,
        nullptr
        ));

    // Add "ApplyPulseEffect" method to LampState Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addmethod(
        m_lampStateInterface,
        METHOD_APPLY_PULSE_EFFECT_STR,
        METHOD_APPLY_PULSE_EFFECT_INPUT_SIGNATURE_STR,
        METHOD_APPLY_PULSE_EFFECT_OUTPUT_SIGNATURE_STR,
        METHOD_APPLY_PULSE_EFFECT_ARG_NAMES_STR,
        0,
        nullptr
        ));



    // Add "LampStateChanged" signal to LampState Interface
    alljoyn_interfacedescription_addsignal(
        m_lampStateInterface,
        SIGNAL_LAMP_STATE_CHANGED_STR,
        ARG_STRING_STR,
        SIGNAL_PARAMETER_LAMP_ID_STR,
        0,
        nullptr
        );


    // Activate the LampService Interface
    alljoyn_interfacedescription_activate(m_lampStateInterface);

    // add LampService interface to bus object
    status = alljoyn_busobject_addinterface_announced(m_pLightingService->GetBusObject(), m_lampStateInterface);
    if (ER_BUS_IFACE_ALREADY_EXISTS == status)
    {
        // this is OK
        status = ER_OK;
    }
    CHK_AJSTATUS(status);


    // add TransitionLampState method to the bus object
    found = alljoyn_interfacedescription_getmember(m_lampStateInterface, METHOD_TRANSITION_LAMP_STATE_STR, &m_methodTransitionLampState_Member);
    if (!found)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    status = alljoyn_busobject_addmethodhandler(m_pLightingService->GetBusObject(), m_methodTransitionLampState_Member, AJMethod, NULL);
    if (ER_OK != status)
    {
        goto leave;
    }


    // add ApplyPulseEffect method to the bus object
    found = alljoyn_interfacedescription_getmember(m_lampStateInterface, METHOD_APPLY_PULSE_EFFECT_STR, &m_methodApplyPulseEffect_Member);
    if (!found)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    status = alljoyn_busobject_addmethodhandler(m_pLightingService->GetBusObject(), m_methodApplyPulseEffect_Member, AJMethod, NULL);
    if (ER_OK != status)
    {
        goto leave;
    }


    //
    // Register "LampStateChanged" Signal Listener with the Adapter
    // If the signal is null, then just continue.
    //
    m_pLampState_LampStateChanged = m_pLightingService->GetLSFHandler()->LampState_LampStateChanged;
    if (m_pLampState_LampStateChanged == nullptr)
    {
        goto leave;
    }

    lampStateStatus = DsbBridge::SingleInstance()->GetAdapter()->RegisterSignalListener(
        m_pLampState_LampStateChanged,
        m_pLSFSignalHandler,
        nullptr
        );

    if (ERROR_SUCCESS != lampStateStatus)
    {
        status = ER_OS_ERROR;
        goto leave;
    }
    
leave:

    return status;
}

QStatus
LampState::Get(_In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;

    if (strcmp(PROPERTY_VERSION_STR, propName) == 0)
    {
        uint32 version = m_pLightingService->GetLSFHandler()->LampState_Version;
        status = alljoyn_msgarg_set(val, "u", version);
    }
    else if (strcmp(PROPERTY_ON_OFF_STR, propName) == 0)
    {
        bool isOn = m_pLightingService->GetLSFHandler()->LampState_OnOff;
        status = alljoyn_msgarg_set(val, "b", isOn);
    }
    else if (strcmp(PROPERTY_HUE_STR, propName) == 0)
    {
        uint32 hue = m_pLightingService->GetLSFHandler()->LampState_Hue;
        status = alljoyn_msgarg_set(val, "u", hue);
    }
    else if (strcmp(PROPERTY_SATURATION_STR, propName) == 0)
    {
        uint32 saturation = m_pLightingService->GetLSFHandler()->LampState_Saturation;
        status = alljoyn_msgarg_set(val, "u", saturation);
    }
    else if (strcmp(PROPERTY_COLOR_TEMP_STR, propName) == 0)
    {
        uint32 colorTemp = m_pLightingService->GetLSFHandler()->LampState_ColorTemp;
        status = alljoyn_msgarg_set(val, "u", colorTemp);
    }
    else if (strcmp(PROPERTY_BRIGHTNESS_STR, propName) == 0)
    {
        uint32 brightness = m_pLightingService->GetLSFHandler()->LampState_Brightness;
        status = alljoyn_msgarg_set(val, "u", brightness);
    }   

    return status;
}

QStatus
LampState::Set(_In_z_ const char* propName, _In_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;

    // The property "Version" is read-only
    if (strcmp(PROPERTY_VERSION_STR, propName) == 0)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    else if (strcmp(PROPERTY_ON_OFF_STR, propName) == 0)
    {
        bool isOn = false;
        status = alljoyn_msgarg_get(val, "b", &isOn);
        if (ER_OK != status)
        {
            goto leave;
        }

        m_pLightingService->GetLSFHandler()->LampState_OnOff = isOn;
    }
    else if (strcmp(PROPERTY_HUE_STR, propName) == 0)
    {
        uint32 hue = 0;
        status = alljoyn_msgarg_get(val, "u", &hue);
        if (ER_OK != status)
        {
            goto leave;
        }

        m_pLightingService->GetLSFHandler()->LampState_Hue = hue;
    }
    else if (strcmp(PROPERTY_SATURATION_STR, propName) == 0)
    {
        uint32 saturation = 0;
        status = alljoyn_msgarg_get(val, "u", &saturation);
        if (ER_OK != status)
        {
            goto leave;
        }

        m_pLightingService->GetLSFHandler()->LampState_Saturation = saturation;
    }
    else if (strcmp(PROPERTY_COLOR_TEMP_STR, propName) == 0)
    {
        uint32 colorTemp = 0;
        status = alljoyn_msgarg_get(val, "u", &colorTemp);
        if (ER_OK != status)
        {
            goto leave;
        }
            
        m_pLightingService->GetLSFHandler()->LampState_ColorTemp = colorTemp;
    }
    else if (strcmp(PROPERTY_BRIGHTNESS_STR, propName) == 0)
    {
        uint32 brightness = 0;
        status = alljoyn_msgarg_get(val, "u", &brightness);
        if (ER_OK != status)
        {
            goto leave;
        }

        m_pLightingService->GetLSFHandler()->LampState_Brightness = brightness;
    }

leave:

    return status;
}

void
AJ_CALL LampState::AJMethod(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    alljoyn_msgarg outArgs = NULL;
    size_t nbOfOutArgs = 0;

    if (member == nullptr)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    LampState* pLampService = LampState::GetInstance(busObject);
    if (pLampService == nullptr)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    if (strcmp(METHOD_TRANSITION_LAMP_STATE_STR, member->name) == 0)
    {
        nbOfOutArgs = METHOD_TRANSITION_LAMP_STATE_OUTPUT_COUNT;
        status = pLampService->invokeTransitionLampState(msg, &outArgs);
    }
    else if (strcmp(METHOD_APPLY_PULSE_EFFECT_STR, member->name) == 0)
    {
        nbOfOutArgs = METHOD_APPLY_PULSE_EFFECT_OUTPUT_COUNT;
        status = pLampService->invokeApplyPulseEffect(msg, &outArgs);
    }
    else
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

leave:
    if (ER_OK != status)
    {
        alljoyn_busobject_methodreply_status(busObject, msg, status);
    }
    else
    {
        alljoyn_busobject_methodreply_args(busObject, msg, outArgs, nbOfOutArgs);
    }
    if (outArgs != NULL)
    {
        alljoyn_msgarg_destroy(outArgs);
    }
}

QStatus
LampState::invokeTransitionLampState(_In_ alljoyn_message msg, _Out_ alljoyn_msgarg *outArgs)
{
    QStatus status = ER_OK;
    uint32 lampStateStatus = ERROR_SUCCESS;
    size_t inArgIndex = 0;

    // Input Argument "Timestamp"
    alljoyn_msgarg inMsgArg_Timestamp = NULL;
    uint64 inTimestamp = 0;

    // Input Argument "NewState"
    alljoyn_msgarg inMsgArg_NewState = NULL;
    State^ inNewState = nullptr;
    try
    {
        inNewState = ref new State();
    }
    catch (OutOfMemoryException^ ex)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // Input Argument "TransitionPeriod"
    alljoyn_msgarg inMsgArg_TransitionPeriod = NULL;
    uint32 inTransitionPeriod = 0;

    // Output Argument "LampResponseCode"
    uint32 outLampResponseCode = 0;

    if (outArgs == nullptr)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }


    // Create output arguments if necessary
    *outArgs = alljoyn_msgarg_create();
    if (*outArgs == NULL)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }


    // Get input argument "Timestamp" from the message
    inMsgArg_Timestamp = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_Timestamp == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = alljoyn_msgarg_get(inMsgArg_Timestamp, "t", &inTimestamp);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "NewState" from the message
    inMsgArg_NewState = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_NewState == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = setStateParameter(inMsgArg_NewState, inNewState);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "TransitionPeriod" from the message
    inMsgArg_TransitionPeriod = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_TransitionPeriod == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = alljoyn_msgarg_get(inMsgArg_TransitionPeriod, "u", &inTransitionPeriod);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Invoke the method
    lampStateStatus = m_pLightingService->GetLSFHandler()->TransitionLampState(
        inTimestamp,
        inNewState,
        inTransitionPeriod,
        &outLampResponseCode
        );
    if (ERROR_SUCCESS != lampStateStatus)
    {
        status = ER_OS_ERROR;
        goto leave;
    }


    // Set output argument "LampResponseCode"
    status = alljoyn_msgarg_set(*outArgs, "u", outLampResponseCode);

leave:
    if (ER_OK != status &&
        outArgs != nullptr &&
        *outArgs != NULL)
    {
        alljoyn_msgarg_destroy(*outArgs);
        *outArgs = NULL;
    }

    return status;
}

QStatus 
LampState::invokeApplyPulseEffect(_In_ alljoyn_message msg, _Out_ alljoyn_msgarg *outArgs)
{
    QStatus status = ER_OK;
    uint32 lampStateStatus = ERROR_SUCCESS;
    size_t inArgIndex = 0;

    // Input Arguments "FromState" and "ToState"
    alljoyn_msgarg inMsgArg_FromState = NULL;
    State^ inFromState = nullptr;
    alljoyn_msgarg inMsgArg_ToState = NULL;
    State^ inToState = nullptr;
    try 
    {
        inFromState = ref new State();
        inToState = ref new State();
    }
    catch (OutOfMemoryException^ ex)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // Input Argument "Period"
    alljoyn_msgarg inMsgArg_Period = NULL;
    uint32 inPeriod = 0;

    // Input Argument "Duration"
    alljoyn_msgarg inMsgArg_Duration = NULL;
    uint32 inDuration = 0;

    // Input Argument "NumPulses"
    alljoyn_msgarg inMsgArg_NumPulses = NULL;
    uint32 inNumPulses = 0;

    // Input Argument "Timestamp"
    alljoyn_msgarg inMsgArg_Timestamp = NULL;
    uint64 inTimestamp = 0;

    // Output Argument "LampResponseCode"
    uint32 outLampResponseCode = 0;

    // Sanity check
    if (outArgs == nullptr)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }


    // Create output arguments if necessary
    *outArgs = alljoyn_msgarg_create();
    if (*outArgs == NULL)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }


    // Get input argument "FromState" from the message
    inMsgArg_FromState = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_FromState == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = setStateParameter(inMsgArg_FromState, inFromState);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "ToState" from the message
    inMsgArg_ToState = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_ToState == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = setStateParameter(inMsgArg_ToState, inToState);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "Period" from the message
    inMsgArg_Period = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_Period == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = alljoyn_msgarg_get(inMsgArg_Period, "u", &inPeriod);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "Duration" from the message
    inMsgArg_Duration = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_Duration == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = alljoyn_msgarg_get(inMsgArg_Duration, "u", &inDuration);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "NumPulses" from the message
    inMsgArg_NumPulses = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_NumPulses == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = alljoyn_msgarg_get(inMsgArg_NumPulses, "u", &inNumPulses);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Get input argument "Timestamp" from the message
    inMsgArg_Timestamp = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_Timestamp == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    status = alljoyn_msgarg_get(inMsgArg_Timestamp, "t", &inTimestamp);
    if (ER_OK != status)
    {
        goto leave;
    }
    inArgIndex++;


    // Invoke the method
    lampStateStatus = m_pLightingService->GetLSFHandler()->LampState_ApplyPulseEffect(
        inFromState,
        inToState,
        inPeriod,
        inDuration,
        inNumPulses,
        inTimestamp,
        &outLampResponseCode
        );
    if (ERROR_SUCCESS != lampStateStatus)
    {
        status = ER_OS_ERROR;
        goto leave;
    }


    // Set output argument "LampResponseCode"
    status = alljoyn_msgarg_set(*outArgs, "u", outLampResponseCode);
    
leave:
    if (ER_OK != status &&
        outArgs != nullptr &&
        *outArgs != NULL)
    {
        alljoyn_msgarg_destroy(*outArgs);
        *outArgs = NULL;
    }

    return status;
}

void 
LampState::RaiseStateChangedSignal()
{
    QStatus status = ER_OK;
    alljoyn_interfacedescription_member signalDescription = { 0 };
    alljoyn_msgarg str_arg = nullptr;

    // send signal on AllJoyn
    QCC_BOOL signalFound = alljoyn_interfacedescription_getsignal(m_lampStateInterface, SIGNAL_LAMP_STATE_CHANGED_STR, &signalDescription);
    if (QCC_TRUE == signalFound)
    {
        IAdapterValue^ pLampIDParam = m_pLampState_LampStateChanged->Params->GetAt(0);
        std::string srcContent = ConvertTo<std::string>(pLampIDParam->Data->ToString());
        str_arg = alljoyn_msgarg_create();
        CHK_POINTER(str_arg);
        CHK_AJSTATUS(alljoyn_msgarg_set(str_arg, ARG_STRING_STR, srcContent.c_str()));

        CHK_AJSTATUS(alljoyn_busobject_signal(m_pLightingService->GetBusObject(), nullptr, ALLJOYN_SESSION_ID_ALL_HOSTED, signalDescription, str_arg, 1, 0, 0, NULL));
    }

leave:
    if (str_arg != nullptr)
    {
        alljoyn_msgarg_destroy(str_arg);
        str_arg = nullptr;
    }

    return;
}

QStatus 
LampState::setStateParameter(_In_ alljoyn_msgarg msgArg, _Out_ State^ newState)
{
    QStatus status = ER_OK;
    alljoyn_msgarg entries;
    size_t numVals = 0;
    size_t stateArgIndex = 1;

    char* key = "";
    bool val_OnOff = false;
    uint32 val = 0;

    if (newState == nullptr)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    status = alljoyn_msgarg_get(msgArg, "a{sv}", &numVals, &entries);
    if (ER_OK != status)
    {
        goto leave;
    }

    for (size_t i = 0; i < numVals; i++)
    {
        status = alljoyn_msgarg_get(alljoyn_msgarg_array_element(entries, i), "{sb}", &key, &val_OnOff);
        if (status == ER_BUS_SIGNATURE_MISMATCH)
        {
            status = alljoyn_msgarg_get(alljoyn_msgarg_array_element(entries, i), "{su}", &key, &val);
            if (status == ER_BUS_SIGNATURE_MISMATCH)
            {
                goto leave;
            }
            else
            {
                if (strcmp(PROPERTY_BRIGHTNESS_STR, key)==0)
                {
                    newState->Brightness = val;
                }
                else if (strcmp(PROPERTY_HUE_STR, key)==0)
                {
                    newState->Hue = val;
                }
                else if (strcmp(PROPERTY_SATURATION_STR, key)==0)
                {
                    newState->Saturation = val;
                }
                else if (strcmp(PROPERTY_COLOR_TEMP_STR, key) == 0)
                {
                    newState->ColorTemp = val;
                }
                stateArgIndex++;
            }
        }
        else
        {
            newState->IsOn = val_OnOff;
        }
    }

leave:

    return status;
}

LampState*
LampState::GetInstance(_In_ alljoyn_busobject busObject)
{
    LSF* pLightingService = nullptr;

    auto deviceList = DsbBridge::SingleInstance()->GetDeviceList();
    for (auto device : deviceList)
    {
        pLightingService = device.second->GetLightingService();
        if (pLightingService != nullptr &&
            pLightingService->GetBusObject() == busObject)
        {
            break;
        }
        else
        {
            pLightingService = nullptr;
        }
    }

    if (pLightingService != nullptr)
    {
        return pLightingService->GetLampStateInterfacePtr();
    }
    else
    {
        return nullptr;
    }
}