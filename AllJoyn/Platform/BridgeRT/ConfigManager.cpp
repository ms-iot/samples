//
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

#include "Bridge.h"
#include "BridgeLog.h"
#include "DsbServiceNames.h"
#include "CspAdapter.h"
#include "CspBridge.h"
#include "AllJoynAbout.h"
#include "AllJoynFileTransfer.h"
#include "BridgeConfig.h"
#include "AllJoynHelper.h"

#include "ConfigManager.h"

using namespace BridgeRT;
using namespace DsbCommon;

static const uint32_t SESSION_LINK_TIMEOUT = 30;        // seconds
const StringReference BRIDGE_CONFIG_FILE(L"BridgeConfig.xml");

ConfigManager::ConfigManager()
    : m_adapter(nullptr),
    m_AJBusAttachment(NULL),
    m_AJBusListener(NULL),
    m_parent(nullptr)
{
}

ConfigManager::~ConfigManager()
{
    Shutdown();
}

int32 ConfigManager::Initialize(DsbBridge ^bridge)
{
    int32 hr = S_OK;

    if (nullptr == bridge)
    {
        hr = E_INVALIDARG;
        goto Leave;
    }
    m_parent = bridge;

    hr = m_bridgeConfig.Init(BRIDGE_CONFIG_FILE);
    if (FAILED(hr))
    {
        goto Leave;
    }

Leave:
    return hr;
}

int32 ConfigManager::Shutdown()
{
    int32 hr = S_OK;

    ShutdownAllJoyn();
    m_adapter = nullptr;
    m_parent = nullptr;

    return hr;
}

QStatus ConfigManager::ConnectToAllJoyn(_In_ IAdapter^ adapter)
{
    const int MAX_ATTEMPTS = 60;
    const int ATTEMPT_DELAY_MS = 500;

    BridgeLog::Instance()->LogEnter(__FUNCTIONW__);
    QStatus status = ER_OK;
    alljoyn_buslistener_callbacks busListenerCallbacks = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    alljoyn_sessionportlistener_callbacks sessionPortListenerCallbacks = { ConfigManager::AcceptSessionJoinerCallback, ConfigManager::SessionJoined };
    alljoyn_sessionlistener_callbacks sessionListenerCallbacks = { NULL, NULL, (alljoyn_sessionlistener_sessionmemberremoved_ptr)ConfigManager::MemberRemoved };
    alljoyn_sessionopts opts = NULL;
    alljoyn_sessionport sp = DSB_SERVICE_PORT;
    std::string appName;

    // sanity check
    if (nullptr == adapter)
    {
        status = ER_BAD_ARG_1;
        goto Leave;
    }

    // verify if already connected
    if (NULL != m_AJBusAttachment)
    {
        goto Leave;
    }

    // save away adapter
    m_adapter = adapter;

    // build service name
    status = BuildServiceName();
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Could not build service name.");
        goto Leave;
    }

    // create the bus attachment
    AllJoynHelper::EncodeStringForAppName(m_adapter->ExposedApplicationName, appName);
    m_AJBusAttachment = alljoyn_busattachment_create(appName.c_str(), QCC_TRUE);
    if (NULL == m_AJBusAttachment)
    {
        BridgeLog::Instance()->LogInfo(L"Could not create bus attachment.");
        status = ER_OUT_OF_MEMORY;
        goto Leave;
    }

    // create the bus listener
    m_AJBusListener = alljoyn_buslistener_create(&busListenerCallbacks, NULL);
    if (NULL == m_AJBusListener)
    {
        BridgeLog::Instance()->LogInfo(L"Could not create bus listener.");
        status = ER_OUT_OF_MEMORY;
        goto Leave;
    }

    // introduce the bus attachment and the listener
    alljoyn_busattachment_registerbuslistener(m_AJBusAttachment, m_AJBusListener);

    // start the bus attachment
    status = alljoyn_busattachment_start(m_AJBusAttachment);
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Could not start bus attachment.");
        goto Leave;
    }

    // initialize about service
    status = m_about.Initialize(m_AJBusAttachment);
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Could not initialize.");
        goto Leave;
    }

    // set adapter info in about
    m_about.SetApplicationName(m_adapter->ExposedApplicationName->Data());
    m_about.SetApplicationGuid(m_adapter->ExposedApplicationGuid);
    m_about.SetManufacturer(m_adapter->Vendor->Data());
    m_about.SetDeviceName(Windows::Networking::Proximity::PeerFinder::DisplayName->Data());
    m_about.SetSWVersion(m_adapter->Version->Data());
    m_about.SetModel(m_adapter->AdapterName->Data());

    status = InitializeCSPBusObjects();
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Problem initializing CSP Bus Objects.");
        goto Leave;
    }

    //
    // Due to a legacy issue with the FltMgr Driver, it is possible that the NamedPipeTrigger (NpSvcTrigger) filter driver will not
    // be attached to the Named Pipe Device Driver.  Normally when a client tries to connect to alljoyn, MSAJAPI (the alljoyn library)
    // will open a pipe connection with the AllJoyn Router Service.  The AllJoyn Router Service is configured as demand start.  The
    // act of connecting to the router should trigger the AllJoyn Router Service to start through the NpSvcTrigger filter drive.  On
    // boot however, the NpSvcTrigger filter driver isn't properly attached to the Named Pipe Driver for up to 15 seconds.  During this
    // time, if a client tries to connect to AllJoyn an ER_TRANSPORT_NOT_AVAILABLE error is returned from AllJoyn instead.
    //
    // To mitigate this issue, the following loop attempts to reconnect for up to 30 seconds.
    //
    int attempt = 0;
    for (;;)
    {
        // connect the bus attachment
        status = alljoyn_busattachment_connect(m_AJBusAttachment, NULL);
        if (ER_OK == status)
        {
            break;
        }

        if (attempt >= MAX_ATTEMPTS)
        {
            goto Leave;
        }

        BridgeLog::Instance()->LogInfo(L"Retrying bus attachment.");
        ++attempt;
        Sleep(ATTEMPT_DELAY_MS);
    }


    /*
    * Advertise this service on the bus.
    * There are three steps to advertising this service on the bus.
    * 1) Request a well-known name that will be used by the client to discover
    *    this service.
    * 2) Create a session.
    * 3) Advertise the well-known name.
    */
    status = alljoyn_busattachment_requestname(m_AJBusAttachment, m_serviceName.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE);
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Failed to request bus attachment name.");
        goto Leave;
    }

    // callback will get this class as context
    m_sessionListener = alljoyn_sessionlistener_create(&sessionListenerCallbacks, this);
    if (NULL == m_sessionListener)
    {
        BridgeLog::Instance()->LogInfo(L"Failed to create session listener.");
        status = ER_OUT_OF_MEMORY;
        goto Leave;
    }

    m_sessionPortListener = alljoyn_sessionportlistener_create(&sessionPortListenerCallbacks, this);
    if (NULL == m_sessionPortListener)
    {
        BridgeLog::Instance()->LogInfo(L"Failed to create session port listener.");
        status = ER_OUT_OF_MEMORY;
        goto Leave;
    }

    opts = alljoyn_sessionopts_create(ALLJOYN_TRAFFIC_TYPE_MESSAGES, QCC_TRUE, ALLJOYN_PROXIMITY_ANY, ALLJOYN_TRANSPORT_ANY);
    if (NULL == opts)
    {
        BridgeLog::Instance()->LogInfo(L"Failed to create session options.");
        status = ER_OUT_OF_MEMORY;
        goto Leave;
    }

    status = alljoyn_busattachment_bindsessionport(m_AJBusAttachment, &sp, opts, m_sessionPortListener);
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Failed to bind session port.");
        goto Leave;
    }

    status = alljoyn_busattachment_advertisename(m_AJBusAttachment, m_serviceName.c_str(), alljoyn_sessionopts_get_transports(opts));
    if (ER_OK != status)
    {
        BridgeLog::Instance()->LogInfo(L"Failed to advertise service name");
        goto Leave;
    }

    // announce
    m_about.Announce();

Leave:
    if (NULL != opts)
    {
        alljoyn_sessionopts_destroy(opts);
    }

    if (ER_OK != status)
    {
        ShutdownAllJoyn();
    }

    BridgeLog::Instance()->LogLeave(__FUNCTIONW__, status);
    return status;
}

QStatus ConfigManager::ShutdownAllJoyn()
{
    QStatus status = ER_OK;

    // verify if already shutdown
    if (NULL == m_AJBusAttachment)
    {
        goto Leave;
    }

    // note that destruction of all alljoyn bus objects (CSP, about and device related)
    // and interfaces must be performed before bus attachment destruction
    // cancel advertised name and session port binding
    if (!m_serviceName.empty())
    {
        alljoyn_busattachment_canceladvertisename(this->m_AJBusAttachment, m_serviceName.c_str(), ALLJOYN_TRANSPORT_ANY);
    }
    alljoyn_busattachment_unbindsessionport(this->m_AJBusAttachment, DSB_SERVICE_PORT);

    if (NULL != this->m_sessionPortListener)
    {
        alljoyn_sessionportlistener_destroy(this->m_sessionPortListener);
        this->m_sessionPortListener = NULL;
    }

    if (!m_serviceName.empty())
    {
        alljoyn_busattachment_releasename(this->m_AJBusAttachment, m_serviceName.c_str());
        m_serviceName.clear();
    }
    alljoyn_busattachment_disconnect(this->m_AJBusAttachment, nullptr);

    // destroy CSP interfaces
    m_adapterCSP.Destroy();
    m_bridgeCSP.Destroy();

    // remove authentication handler
    m_authHandler.ShutDown();

    // shutdown about
    m_about.ShutDown();

    alljoyn_busattachment_stop(this->m_AJBusAttachment);

    // destroy bus listener and session port listener
    if (NULL != this->m_AJBusListener)
    {
        alljoyn_busattachment_unregisterbuslistener(this->m_AJBusAttachment, this->m_AJBusListener);
        alljoyn_buslistener_destroy(this->m_AJBusListener);
        this->m_AJBusListener = NULL;
    }

    alljoyn_busattachment_destroy(this->m_AJBusAttachment);
    this->m_AJBusAttachment = NULL;

Leave:
    return status;
}

QStatus ConfigManager::InitializeCSPBusObjects()
{
    QStatus status = ER_OK;

    // init CSP related bus objects if this isn't an update
    //-------------------------------------------------------
    if (IsConfigurationAccessSecured())
    {
		status = m_authHandler.InitializeWithKeyXAuthentication(m_AJBusAttachment, m_bridgeConfig.BridgeKeyX());
        if (ER_OK != status)
        {
            goto Leave;
        }
    }

    status = m_adapterCSP.Initialize(&m_AJBusAttachment, this);
    if (ER_OK != status)
    {
        goto Leave;
    }

    // add bus object description in about service
    status = m_about.AddObject(m_adapterCSP.GetBusObject(), m_adapterCSP.GetInterface());
    if (ER_OK != status)
    {
        goto Leave;
    }

    status = m_bridgeCSP.Initialize(&m_AJBusAttachment, this, m_bridgeConfig);
    if (ER_OK != status)
    {
        goto Leave;
    }
    // add bus object description in about service
    status = m_about.AddObject(m_bridgeCSP.GetBusObject(), m_bridgeCSP.GetInterface());
    if (ER_OK != status)
    {
        goto Leave;
    }

Leave:
    return status;
}

bool BridgeRT::ConfigManager::IsConfigurationAccessSecured()
{
	if (!m_bridgeConfig.BridgeKeyX()->IsEmpty())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool BridgeRT::ConfigManager::IsDeviceAccessSecured()
{
	if (!m_bridgeConfig.DeviceKeyX()->IsEmpty() ||
        (!m_bridgeConfig.DeviceUsername()->IsEmpty() &&
		 !m_bridgeConfig.DevicePassword()->IsEmpty()) ||
        (!m_bridgeConfig.DeviceEcdheEcdsaPrivateKey()->IsEmpty() &&
         !m_bridgeConfig.DeviceEcdheEcdsaCertChain()->IsEmpty()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

AllJoynFileTransfer* ConfigManager::GetAllJoynFileTransferInstance(_In_ alljoyn_busobject busObject)
{
    AllJoynFileTransfer *objectInstance = nullptr;

    if (m_bridgeCSP.GetBusObject() == busObject)
    {
        objectInstance = reinterpret_cast<AllJoynFileTransfer *> (&m_bridgeCSP);
    }
    else if (m_adapterCSP.GetBusObject() == busObject)
    {
        objectInstance = reinterpret_cast<AllJoynFileTransfer *> (&m_adapterCSP);
    }

    return objectInstance;
}

HRESULT ConfigManager::SetDeviceConfig(_In_ std::wstring &tempFileName, _Out_ HANDLE *finalEvent)
{
    QStatus updateStatus = ER_OK;
    HRESULT hr = S_OK;
    String^ bridgeConfigFileName = ref new String(tempFileName.c_str());
    BridgeConfig newConfig;

    String^ keyX = m_bridgeConfig.BridgeKeyX();

    String^ deviceKeyX = m_bridgeConfig.DeviceKeyX();
    String^ deviceUserName = m_bridgeConfig.DeviceUsername();
    String^ devicePassword = m_bridgeConfig.DevicePassword();
    String^ deviceEcdheEcdsaPrivateKey = m_bridgeConfig.DeviceEcdheEcdsaPrivateKey();
    String^ deviceEcdheEcdsaCertChain = m_bridgeConfig.DeviceEcdheEcdsaCertChain();

    // default final event to nothing
    if (nullptr != finalEvent)
    {
        *finalEvent = NULL;
    }

    // Try to load the new configuration file
    hr = newConfig.Init(bridgeConfigFileName);
    if (FAILED(hr))
    {
        goto Leave;
    }

    // Try to merge the new configuration file with the original configuration file.
    // On failure, reload the in-memory version from file
    hr = m_bridgeConfig.MergeFrom(newConfig);
    if (FAILED(hr))
    {
        m_bridgeConfig.Init(BRIDGE_CONFIG_FILE);
        goto Leave;
    }

    // Update the bridge configuration file with the new data values
    hr = m_bridgeConfig.ToFile();
    if (FAILED(hr))
    {
        goto Leave;
    }
 
    // If one of the authentication methods has changed, signal a reset request.  
    // This will shutdown all devices including the Bridge and then regenerate them
    // with whatever visibility was specified in the current bridge config file
    if (keyX != m_bridgeConfig.BridgeKeyX() ||
        deviceKeyX != m_bridgeConfig.DeviceKeyX() ||
        deviceUserName != m_bridgeConfig.DeviceUsername() ||
        devicePassword != m_bridgeConfig.DevicePassword() ||
        deviceEcdheEcdsaPrivateKey != m_bridgeConfig.DeviceEcdheEcdsaPrivateKey() ||
        deviceEcdheEcdsaCertChain != m_bridgeConfig.DeviceEcdheEcdsaCertChain())
    {
        if (nullptr != finalEvent)
        {
            // set post write file event to reset event
            *finalEvent = this->m_parent->m_ctrlEvents.hResetEvt;
        }
    }
    // Otherwise, update the status of each adapter device to either expose or hide 
    // it from AllJoyn based on the new bridge configuration settings
    else
    {
        if (nullptr != m_parent)
        {
            AutoLock bridgeLocker(&DsbBridge::SingleInstance()->GetLock(), true);
            updateStatus = m_parent->InitializeDevices(true);
            if (updateStatus != ER_OK)
            {
                hr = HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED);
            }
        }
    }

Leave:
    return hr;
}


HRESULT ConfigManager::SetAdapterConfig(_In_ Array<BYTE>^ adapterConfig)
{
    m_adapter->SetConfiguration(adapterConfig);
    return S_OK;
}


HRESULT ConfigManager::GetAdapterConfig(_Out_ Array<BYTE>^* pAdapterConfig)
{
    m_adapter->GetConfiguration(pAdapterConfig);
    return S_OK;
}

bool ConfigManager::GetObjectConfigItem(_In_ IAdapterDevice ^device, _Out_ DsbObjectConfig& objConfigItem)
{
    bool bConfigItemAdded = false;
    // Select this object from the object configuration file
    HRESULT hr = m_bridgeConfig.FindObject(device->SerialNumber, objConfigItem);
    if (FAILED(hr))
    {
        // Unexpected error searching for device configuration
        throw ref new Exception(hr);
    }

    // If the object wasn't found, then try to add it to the configuration file, defaulted to false
    // If it can't be added, we will still not expose it
    if (hr == S_FALSE)
    {
        // Try to add the object to the in memory configuration.  (Try is here to handle potential exceptions from wstring)
        try
        {
            objConfigItem.bVisible = m_bridgeConfig.DefaultVisibility();
            objConfigItem.id = device->SerialNumber;
            objConfigItem.description = device->Model;
            hr = m_bridgeConfig.AddObject(objConfigItem);
            bConfigItemAdded = true;
        }

        catch (...)
        {
            throw;
        }
    }

    // Return whether or not this is a new item (indicates that the in-memory XML file needs to be saved back to disk)
    // Save is not automatically done here to allow caller a chance to decide when it is appropriate to perform update)
    return bConfigItemAdded;
}

void ConfigManager::ToFile()
{
    // Try to update configuration file, do not fail on write error
    (void)m_bridgeConfig.ToFile();
}

QCC_BOOL AJ_CALL ConfigManager::AcceptSessionJoinerCallback(const void * context, alljoyn_sessionport sessionPort, const char * joiner, const alljoyn_sessionopts opts)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(joiner);
    UNREFERENCED_PARAMETER(opts);

    if (DSB_SERVICE_PORT != sessionPort)
    {
        return QCC_FALSE;
    }
    else
    {
        return QCC_TRUE;
    }
}

void ConfigManager::SessionJoined(_In_ const void *context, _In_ alljoyn_sessionport sessionPort, _In_ alljoyn_sessionid id, _In_ const char *joiner)
{
    QStatus status = ER_OK;
    const ConfigManager *configManager = nullptr;
    uint32_t timeOut = SESSION_LINK_TIMEOUT;

    UNREFERENCED_PARAMETER(sessionPort);
    UNREFERENCED_PARAMETER(joiner);

    configManager = reinterpret_cast<const ConfigManager *> (context);
    if (nullptr == configManager)
    {
        goto leave;
    }

    // Enable concurrent callbacks since some of the calls below could block
    alljoyn_busattachment_enableconcurrentcallbacks(configManager->m_AJBusAttachment);

    // set session listener and time-out
    status = alljoyn_busattachment_setsessionlistener(configManager->m_AJBusAttachment, id, configManager->m_sessionListener);
    if (status != ER_OK)
    {
        goto leave;
    }
    status = alljoyn_busattachment_setlinktimeout(configManager->m_AJBusAttachment, id, &timeOut);
    if (status != ER_OK)
    {
        goto leave;
    }

leave:
    return;
}

void ConfigManager::MemberRemoved(_In_  void* context, _In_ alljoyn_sessionid sessionid, _In_ const char* uniqueName)
{
    ConfigManager *configManager = nullptr;

    UNREFERENCED_PARAMETER(sessionid);
    UNREFERENCED_PARAMETER(uniqueName);

    // get config manager instance
    configManager = reinterpret_cast<ConfigManager *> (context);
    if (nullptr == configManager)
    {
        goto leave;
    }

    // reset access
    configManager->m_authHandler.ResetAccess(uniqueName);

    // end CSP file transfer
    configManager->m_adapterCSP.EndTransfer();
    configManager->m_bridgeCSP.EndTransfer();

leave:
    return;
}

QStatus ConfigManager::BuildServiceName()
{
    QStatus status = ER_OK;
    std::string tempString;

    m_serviceName.clear();

    // sanity check
    if (nullptr == m_adapter)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    // set root/prefix for AllJoyn service name (aka bus name) and interface names :
    // 'prefixForAllJoyn'.DeviceSystemBridge.'AdapterName
    AllJoynHelper::EncodeStringForRootServiceName(m_adapter->ExposedAdapterPrefix, tempString);
    if (tempString.empty())
    {
        status = ER_BUS_BAD_BUS_NAME;
        goto leave;
    }
    m_serviceName += tempString;
    m_serviceName += ".DeviceSystemBridge";

    AllJoynHelper::EncodeStringForServiceName(DsbBridge::SingleInstance()->GetAdapter()->AdapterName, tempString);
    if (tempString.empty())
    {
        status = ER_BUS_BAD_BUS_NAME;
        goto leave;
    }
    m_serviceName += ".";
    m_serviceName += tempString;


leave:
    return status;
}
