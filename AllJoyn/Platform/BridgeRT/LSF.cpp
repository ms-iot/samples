#include "pch.h"
#include "LSF.h"
#include "LSFConsts.h"
#include "LampService.h"
#include "LampParameters.h"
#include "LampDetails.h"
#include "LampState.h"

using namespace BridgeRT;

LSF::LSF(_In_ IAdapterDevice^ pLightingDevice)
    : m_bRegistered(false)
    , m_busObject(nullptr)
    , m_bus(nullptr)
    , m_pLampServiceInterface(nullptr)
    , m_pLampParametersInterface(nullptr)
    , m_pLampDetailsInterface(nullptr)
    , m_pLampStateInterface(nullptr)
    , m_pLightingDevice(pLightingDevice)
{
    if (pLightingDevice != nullptr)
    {
        m_pLightingServiceHandler = dynamic_cast<IAdapterDeviceLightingService^>(pLightingDevice)->LightingServiceHandler;
    }
}

LSF::~LSF()
{
    if (m_busObject != nullptr)
    {
        if (m_bRegistered)
        {
            alljoyn_busattachment_unregisterbusobject(m_bus, m_busObject);
        }
        alljoyn_busobject_destroy(m_busObject);
        m_busObject = nullptr;
    }
}

QStatus 
LSF::Initialize(_In_ alljoyn_busattachment bus)
{
    QStatus status = ER_OK;

    alljoyn_busobject_callbacks lightingServiceCallbacks =
    {
        LSF::getProperty,
        LSF::setProperty,
        nullptr,
        nullptr
    };

    if (bus == nullptr)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    // Bus attachment
    m_bus = bus;

    // Create Lighting Service Framework Bus Object
    m_busObject = alljoyn_busobject_create(LSF_BUS_OBJECT_PATH, QCC_FALSE, &lightingServiceCallbacks, this);
    CHK_POINTER(m_busObject);

    try
    {
        m_pLampServiceInterface = new LampService(this);
        CHK_POINTER(m_pLampServiceInterface);

        m_pLampParametersInterface = new LampParameters(this);
        CHK_POINTER(m_pLampParametersInterface);

        m_pLampDetailsInterface = new LampDetails(this);
        CHK_POINTER(m_pLampDetailsInterface);

        m_pLampStateInterface = new LampState(this);
        CHK_POINTER(m_pLampStateInterface);
    }
    catch (std::bad_alloc ex)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    status = m_pLampServiceInterface->Initialize();
    if (ER_OK != status)
    {
        goto leave;
    }

    m_pLampParametersInterface->Initialize();
    if (ER_OK != status)
    {
        goto leave;
    }

    m_pLampDetailsInterface->Initialize();
    if (ER_OK != status)
    {
        goto leave;
    }

    m_pLampStateInterface->Initialize();
    if (ER_OK != status)
    {
        goto leave;
    }

    // register Lighting Service Framework Bus Object on Bus attachment
    CHK_AJSTATUS(alljoyn_busattachment_registerbusobject(m_bus, m_busObject));
    m_bRegistered = true;

leave:

    return status;
}

QStatus 
AJ_CALL LSF::getProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    LSF* pLightingService = nullptr;

    pLightingService = (LSF *)context;
    if (nullptr == pLightingService)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    if (strcmp(LAMP_SERVICE_INTERFACE_NAME, ifcName) == 0)
    {
        status = pLightingService->m_pLampServiceInterface->Get(propName, val);
    }
    else if (strcmp(LAMP_PARAMETERS_INTERFACE_NAME, ifcName) == 0)
    {
        status = pLightingService->m_pLampParametersInterface->Get(propName, val);
    }
    else if (strcmp(LAMP_DETAILS_INTERFACE_NAME, ifcName) == 0)
    {
        status = pLightingService->m_pLampDetailsInterface->Get(propName, val);
    }
    else if (strcmp(LAMP_STATE_INTERFACE_NAME, ifcName) == 0)
    {
        status = pLightingService->m_pLampStateInterface->Get(propName, val);
    }
    else
    {
        status = ER_BAD_ARG_1;
    }

leave:

    return status;
}

QStatus
AJ_CALL LSF::setProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _In_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    LSF* pLightingService = nullptr;

    pLightingService = (LSF *)context;
    if (nullptr == pLightingService)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }

    // Other interfaces has just read-only properties
    if (strcmp(LAMP_STATE_INTERFACE_NAME, ifcName) == 0)
    {
        status = pLightingService->m_pLampStateInterface->Set(propName, val);
    }
    else
    {
        status = ER_BAD_ARG_1;
    }

leave:

    return status;
}