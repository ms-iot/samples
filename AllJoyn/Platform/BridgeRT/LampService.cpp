#include "pch.h"
#include "LampService.h"
#include "LSFConsts.h"
#include "Bridge.h"
#include "AllJoynHelper.h"
#include "LSF.h"
#include "BridgeDevice.h"

using namespace Windows::Foundation;
using namespace BridgeRT;

//**************************************************************************************************************************************
//
//  Lamp Service Constructor
//
//**************************************************************************************************************************************
LampService::LampService(LSF* pLightingService)
    : m_pLightingService (pLightingService)
    , m_lampServiceInterface(nullptr)
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
LampService::~LampService()
{
    if (m_lampServiceInterface != nullptr)
    {
        alljoyn_busattachment_deleteinterface(m_pLightingService->GetBus(), m_lampServiceInterface);
        m_lampServiceInterface = nullptr;
    }
    m_pLightingService = nullptr;
}

QStatus 
LampService::Initialize()
{
    QStatus status = ER_OK;

    // Create LampService interface
    CHK_AJSTATUS(alljoyn_busattachment_createinterface(m_pLightingService->GetBus(), LAMP_SERVICE_INTERFACE_NAME, &m_lampServiceInterface));

    // Add "Version" Property to LampService Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampServiceInterface,
        PROPERTY_VERSION_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "LampServiceVersion" Property to LampService Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampServiceInterface,
        PROPERTY_LAMP_SERVICE_VERSION_STR,
        ARG_UINT32_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "LampFaults" Property to LampService Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(
        m_lampServiceInterface,
        PROPERTY_LAMP_FAULTS_STR,
        ARG_UINT32_ARRY_STR,
        ALLJOYN_PROP_ACCESS_READ
        ));

    // Add "ClearLampFault" Method to LampService Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addmethod(
        m_lampServiceInterface,
        METHOD_CLEAR_LAMP_FAULT_STR,
        METHOD_CLEAR_LAMP_FAULT_INPUT_SIGNATURE_STR,
        METHOD_CLEAR_LAMP_FAULT_OUTPUT_SIGNATURE_STR,
        METHOD_CLEAR_LAMP_FAULT_ARG_NAMES_STR,
        0,
        nullptr
        ));


    // Activate the LampService Interface
    alljoyn_interfacedescription_activate(m_lampServiceInterface);

    // add LampService interface to bus object
    status = alljoyn_busobject_addinterface_announced(m_pLightingService->GetBusObject(), m_lampServiceInterface);
    if (ER_BUS_IFACE_ALREADY_EXISTS == status)
    {
        // this is OK
        status = ER_OK;
    }
    CHK_AJSTATUS(status);


    // add ClearLampFault method to the bus object
    QCC_BOOL found = false;
    found = alljoyn_interfacedescription_getmember(m_lampServiceInterface, METHOD_CLEAR_LAMP_FAULT_STR, &m_methodClearLampFault_Member);
    if (!found)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    status = alljoyn_busobject_addmethodhandler(m_pLightingService->GetBusObject(), m_methodClearLampFault_Member, AJMethod, NULL);
    if (ER_OK != status)
    {
        goto leave;
    }

leave:

    return status;
}

QStatus 
LampService::Get(_In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    ILSFHandler^ lightingServiceHandler = m_pLightingService->GetLSFHandler();

    if (strcmp(PROPERTY_VERSION_STR, propName) == 0)
    {
        uint32 version = lightingServiceHandler->LampService_Version;
        status = alljoyn_msgarg_set(val, "u", version);
    }
    else if (strcmp(PROPERTY_LAMP_SERVICE_VERSION_STR, propName) == 0)
    {
        uint32 lampServiceVersion = lightingServiceHandler->LampService_LampServiceVersion;
        status = alljoyn_msgarg_set(val, "u", lampServiceVersion);
    }
    else if (strcmp(PROPERTY_LAMP_FAULTS_STR, propName) == 0)
    {
        Platform::Array<uint32>^ lampFaults = lightingServiceHandler->LampService_LampFaults;
        status = setAllJoynArrayArgument(lampFaults, val);
    }

    return status;
}

void
AJ_CALL LampService::AJMethod(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    alljoyn_msgarg outArgs = NULL;
    size_t nbOfOutArgs = METHOD_CLEAR_LAMP_FAULT_OUTPUT_COUNT;

    if (member == nullptr ||
        strcmp(METHOD_CLEAR_LAMP_FAULT_STR, member->name) != 0)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    LampService* pLampService = LampService::GetInstance(busObject);
    if (pLampService == nullptr)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    status = pLampService->invokeClearLampFault(msg, &outArgs);

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
LampService::invokeClearLampFault(_In_ alljoyn_message msg, _Out_ alljoyn_msgarg *outArgs)
{
    QStatus status = ER_OK;
    uint32 lampServiceStatus = ERROR_SUCCESS;
    size_t inArgIndex = 0;

    // Input Argument
    alljoyn_msgarg inMsgArg_LampFaultCode = NULL;
    uint32 inLampFaultCode = 0;

    // Output Arguments
    alljoyn_msgarg outMsgArg_lampResponseCode = NULL;
    uint32 LampResponseCode = 0;

    alljoyn_msgarg outMsgArg_outLampFaultCode = NULL;
    uint32 outLampFaultCode = 0;

    if (outArgs == nullptr)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    // create output arguments if necessary
    *outArgs = alljoyn_msgarg_array_create(METHOD_CLEAR_LAMP_FAULT_OUTPUT_COUNT);
    if (*outArgs == NULL)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // get the only input parameter "LampFaultCode" from the message
    inMsgArg_LampFaultCode = alljoyn_message_getarg(msg, inArgIndex);
    if (inMsgArg_LampFaultCode == NULL)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    status = alljoyn_msgarg_get(inMsgArg_LampFaultCode, "u", &inLampFaultCode);
    if (ER_OK != status)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }


    // invoke the method
    lampServiceStatus = m_pLightingService->GetLSFHandler()->ClearLampFault(inLampFaultCode, &LampResponseCode, &outLampFaultCode);
    if (ERROR_SUCCESS != lampServiceStatus)
    {
        status = ER_OS_ERROR;
        goto leave;
    }


    // set output arguments - "LampResponseCode", "LampFaultCode"
    outMsgArg_lampResponseCode = alljoyn_msgarg_array_element(*outArgs, 0);
    outMsgArg_outLampFaultCode = alljoyn_msgarg_array_element(*outArgs, 1);

    status = alljoyn_msgarg_set(outMsgArg_lampResponseCode, "u", LampResponseCode);
    if (ER_OK != status)
    {
        goto leave;
    }

    status = alljoyn_msgarg_set(outMsgArg_outLampFaultCode, "u", outLampFaultCode);

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

LampService*
LampService::GetInstance(_In_ alljoyn_busobject busObject)
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
        return pLightingService->GetLampServiceInterfacePtr();
    }
    else
    {
        return nullptr;
    }
}

QStatus 
LampService::setAllJoynArrayArgument(_In_ Platform::Array<uint32>^ lampFaults, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    std::vector<uint32> lampFaultVector;

    if (lampFaults == nullptr)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    if (lampFaults->Length <= 0)
    {
        status = alljoyn_msgarg_set(val, "au", 0, lampFaultVector.data());
    }
    else
    {
        for (auto faultCode : lampFaults)
        {
            lampFaultVector.push_back(faultCode);
        }

        status = alljoyn_msgarg_set(val, "au", lampFaultVector.size(), lampFaultVector.data());
    }

leave:
    
    return status;
}