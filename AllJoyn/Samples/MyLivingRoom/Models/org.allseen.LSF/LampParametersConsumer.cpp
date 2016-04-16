#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

std::map<alljoyn_interfacedescription, WeakReference*> LampParametersConsumer::SourceInterfaces;

LampParametersConsumer::LampParametersConsumer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_proxyBusObject(nullptr),
    m_busObject(nullptr),
    m_sessionListener(nullptr),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    m_signals = ref new LampParametersSignals();
}

LampParametersConsumer::~LampParametersConsumer()
{
    if (SessionListener != nullptr)
    {
        alljoyn_busattachment_leavesession(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), m_sessionId);
        alljoyn_sessionlistener_destroy(SessionListener);
    }
    if (nullptr != ProxyBusObject)
    {
        alljoyn_proxybusobject_destroy(ProxyBusObject);
    }
    if (nullptr != BusObject)
    {
        alljoyn_busobject_destroy(BusObject);
    }
    delete m_weak;
}

void LampParametersConsumer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void LampParametersConsumer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void LampParametersConsumer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

QStatus LampParametersConsumer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

IAsyncOperation<LampParametersJoinSessionResult^>^ LampParametersConsumer::JoinSessionAsync(
    _In_ AllJoynServiceInfo^ serviceInfo, _Inout_ LampParametersWatcher^ watcher)
{
    return create_async([serviceInfo, watcher]() -> LampParametersJoinSessionResult^
    {
        auto result = ref new LampParametersJoinSessionResult();
        result->Status = AllJoynStatus::Ok;
        result->Consumer = nullptr;

        result->Consumer = ref new LampParametersConsumer(watcher->BusAttachment);
        result->Status = result->Consumer->JoinSession(serviceInfo);

        return result;
    });
}


IAsyncOperation<LampParametersGetVersionResult^>^ LampParametersConsumer::GetVersionAsync()
{
    return create_async([this]()->LampParametersGetVersionResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampParameters",
            "Version",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampParametersGetVersionResult();
        result->Status = getContext.GetStatus();
        result->Version = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampParametersGetEnergy_Usage_MilliwattsResult^>^ LampParametersConsumer::GetEnergy_Usage_MilliwattsAsync()
{
    return create_async([this]()->LampParametersGetEnergy_Usage_MilliwattsResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampParameters",
            "Energy_Usage_Milliwatts",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampParametersGetEnergy_Usage_MilliwattsResult();
        result->Status = getContext.GetStatus();
        result->Energy_Usage_Milliwatts = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampParametersGetBrightness_LumensResult^>^ LampParametersConsumer::GetBrightness_LumensAsync()
{
    return create_async([this]()->LampParametersGetBrightness_LumensResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampParameters",
            "Brightness_Lumens",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampParametersGetBrightness_LumensResult();
        result->Status = getContext.GetStatus();
        result->Brightness_Lumens = getContext.GetValue();
        return result;
    });
}

void LampParametersConsumer::OnPropertyChanged(_In_ alljoyn_proxybusobject obj, _In_ PCSTR interfaceName, _In_ const alljoyn_msgarg changed, _In_ const alljoyn_msgarg invalidated)
{
    UNREFERENCED_PARAMETER(obj);
    UNREFERENCED_PARAMETER(interfaceName);
    UNREFERENCED_PARAMETER(changed);
    UNREFERENCED_PARAMETER(invalidated);
}

int32 LampParametersConsumer::JoinSession(_In_ AllJoynServiceInfo^ serviceInfo)
{
    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<LampParametersConsumer>,
        AllJoynHelpers::SessionMemberAddedHandler<LampParametersConsumer>,
        AllJoynHelpers::SessionMemberRemovedHandler<LampParametersConsumer>
    };

    alljoyn_busattachment_enableconcurrentcallbacks(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment));

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_sessionopts sessionOpts = alljoyn_sessionopts_create(ALLJOYN_TRAFFIC_TYPE_MESSAGES, true, ALLJOYN_PROXIMITY_ANY, ALLJOYN_TRANSPORT_ANY);

    std::vector<char> sessionNameUtf8 = AllJoynHelpers::PlatformToMultibyteString(serviceInfo->UniqueName);
    RETURN_IF_QSTATUS_ERROR(alljoyn_busattachment_joinsession(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        &sessionNameUtf8[0],
        serviceInfo->SessionPort,
        SessionListener,
        &m_sessionId,
        sessionOpts));

    ServiceObjectPath = serviceInfo->ObjectPath;
    std::vector<char> objectPath = AllJoynHelpers::PlatformToMultibyteString(ServiceObjectPath);

    if (objectPath.empty())
    {
        return AllJoynStatus::Fail;
    }

    ProxyBusObject = alljoyn_proxybusobject_create(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), &sessionNameUtf8[0], &objectPath[0], m_sessionId);
    if (nullptr == ProxyBusObject)
    {
        return AllJoynStatus::Fail;
    }

    RETURN_IF_QSTATUS_ERROR(AllJoynHelpers::CreateBusObject<LampParametersConsumer>(m_weak));

    alljoyn_interfacedescription description = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.allseen.LSF.LampParameters");
    if (nullptr == description)
    {
        return AllJoynStatus::Fail;
    }

    RETURN_IF_QSTATUS_ERROR(alljoyn_proxybusobject_addinterface(ProxyBusObject, description));
    RETURN_IF_QSTATUS_ERROR(alljoyn_busobject_addinterface(BusObject, description));


    SourceInterfaces[description] = m_weak;
    RETURN_IF_QSTATUS_ERROR(alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject));
    m_signals->Initialize(BusObject, m_sessionId);

    alljoyn_sessionopts_destroy(sessionOpts);

    return AllJoynStatus::Ok;
}
