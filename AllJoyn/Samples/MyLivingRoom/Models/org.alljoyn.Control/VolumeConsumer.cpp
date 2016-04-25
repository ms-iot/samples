#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::alljoyn::Control;

std::map<alljoyn_interfacedescription, WeakReference*> VolumeConsumer::SourceInterfaces;

VolumeConsumer::VolumeConsumer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_proxyBusObject(nullptr),
    m_busObject(nullptr),
    m_sessionListener(nullptr),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    m_signals = ref new VolumeSignals();
}

VolumeConsumer::~VolumeConsumer()
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

void VolumeConsumer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void VolumeConsumer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void VolumeConsumer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

QStatus VolumeConsumer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

IAsyncOperation<VolumeJoinSessionResult^>^ VolumeConsumer::JoinSessionAsync(
    _In_ AllJoynServiceInfo^ serviceInfo, _Inout_ VolumeWatcher^ watcher)
{
    return create_async([serviceInfo, watcher]() -> VolumeJoinSessionResult^
    {
        auto result = ref new VolumeJoinSessionResult();
        result->Status = AllJoynStatus::Ok;
        result->Consumer = nullptr;

        result->Consumer = ref new VolumeConsumer(watcher->BusAttachment);
        result->Status = result->Consumer->JoinSession(serviceInfo);

        return result;
    });
}

IAsyncOperation<VolumeAdjustVolumeResult^>^ VolumeConsumer::AdjustVolumeAsync(_In_ int16 increments)
{
    return create_async([this, increments]() -> VolumeAdjustVolumeResult^
    {
        auto result = ref new VolumeAdjustVolumeResult();
        
        alljoyn_message message = alljoyn_message_create(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment));
        size_t argCount = 1;
        alljoyn_msgarg inputs = alljoyn_msgarg_array_create(argCount);

        TypeConversionHelpers::SetAllJoynMessageArg(alljoyn_msgarg_array_element(inputs, 0), "n", increments);
        
        QStatus status = alljoyn_proxybusobject_methodcall(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "AdjustVolume",
            inputs,
            argCount,
            message,
            c_MessageTimeoutInMilliseconds,
            0);
        result->Status = static_cast<int>(status);
        
        alljoyn_message_destroy(message);
        alljoyn_msgarg_destroy(inputs);

        return result;
    });
}

IAsyncOperation<int>^ VolumeConsumer::SetMuteAsync(_In_ bool value)
{
    return create_async([this, value]() -> int
    {
        PropertySetContext setContext;

        alljoyn_msgarg inputArgument = alljoyn_msgarg_create();
        TypeConversionHelpers::SetAllJoynMessageArg(inputArgument, "b", value);

        alljoyn_proxybusobject_setpropertyasync(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "Mute",
            inputArgument,
            [](QStatus status, alljoyn_proxybusobject obj, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertySetContext*>(context);
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &setContext);

        alljoyn_msgarg_destroy(inputArgument);

        setContext.Wait();
        return setContext.GetStatus();
    });
}

IAsyncOperation<VolumeGetMuteResult^>^ VolumeConsumer::GetMuteAsync()
{
    return create_async([this]()->VolumeGetMuteResult^
    {
        PropertyGetContext<bool> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "Mute",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<bool>*>(context);

                if (ER_OK == status)
                {
                    bool argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new VolumeGetMuteResult();
        result->Status = getContext.GetStatus();
        result->Mute = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<VolumeGetVersionResult^>^ VolumeConsumer::GetVersionAsync()
{
    return create_async([this]()->VolumeGetVersionResult^
    {
        PropertyGetContext<uint16> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "Version",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint16>*>(context);

                if (ER_OK == status)
                {
                    uint16 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "q", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new VolumeGetVersionResult();
        result->Status = getContext.GetStatus();
        result->Version = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<int>^ VolumeConsumer::SetVolumeAsync(_In_ int16 value)
{
    return create_async([this, value]() -> int
    {
        PropertySetContext setContext;

        alljoyn_msgarg inputArgument = alljoyn_msgarg_create();
        TypeConversionHelpers::SetAllJoynMessageArg(inputArgument, "n", value);

        alljoyn_proxybusobject_setpropertyasync(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "Volume",
            inputArgument,
            [](QStatus status, alljoyn_proxybusobject obj, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertySetContext*>(context);
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &setContext);

        alljoyn_msgarg_destroy(inputArgument);

        setContext.Wait();
        return setContext.GetStatus();
    });
}

IAsyncOperation<VolumeGetVolumeResult^>^ VolumeConsumer::GetVolumeAsync()
{
    return create_async([this]()->VolumeGetVolumeResult^
    {
        PropertyGetContext<int16> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "Volume",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<int16>*>(context);

                if (ER_OK == status)
                {
                    int16 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "n", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new VolumeGetVolumeResult();
        result->Status = getContext.GetStatus();
        result->Volume = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<VolumeGetVolumeRangeResult^>^ VolumeConsumer::GetVolumeRangeAsync()
{
    return create_async([this]()->VolumeGetVolumeRangeResult^
    {
        PropertyGetContext<VolumeVolumeRange^> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.alljoyn.Control.Volume",
            "VolumeRange",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<VolumeVolumeRange^>*>(context);

                if (ER_OK == status)
                {
                    VolumeVolumeRange^ argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "(nnn)", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new VolumeGetVolumeRangeResult();
        result->Status = getContext.GetStatus();
        result->VolumeRange = getContext.GetValue();
        return result;
    });
}

void VolumeConsumer::OnPropertyChanged(_In_ alljoyn_proxybusobject obj, _In_ PCSTR interfaceName, _In_ const alljoyn_msgarg changed, _In_ const alljoyn_msgarg invalidated)
{
    UNREFERENCED_PARAMETER(obj);
    UNREFERENCED_PARAMETER(interfaceName);
    UNREFERENCED_PARAMETER(changed);
    UNREFERENCED_PARAMETER(invalidated);
}

void VolumeConsumer::CallMuteChangedSignalHandler(_In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message message)
{
    auto source = SourceInterfaces.find(member->iface);
    if (source == SourceInterfaces.end())
    {
        return;
    }

    auto consumer = source->second->Resolve<VolumeConsumer>();
    if (consumer->Signals != nullptr)
    {
        auto callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));
        auto eventArgs = ref new VolumeMuteChangedReceivedEventArgs();
        eventArgs->MessageInfo = callInfo;

        bool argument0;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "b", &argument0);

        eventArgs->newMute = argument0;

        consumer->Signals->CallMuteChangedReceived(consumer->Signals, eventArgs);
    }
}

void VolumeConsumer::CallVolumeChangedSignalHandler(_In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message message)
{
    auto source = SourceInterfaces.find(member->iface);
    if (source == SourceInterfaces.end())
    {
        return;
    }

    auto consumer = source->second->Resolve<VolumeConsumer>();
    if (consumer->Signals != nullptr)
    {
        auto callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));
        auto eventArgs = ref new VolumeVolumeChangedReceivedEventArgs();
        eventArgs->MessageInfo = callInfo;

        int16 argument0;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "n", &argument0);

        eventArgs->newVolume = argument0;

        consumer->Signals->CallVolumeChangedReceived(consumer->Signals, eventArgs);
    }
}

int32 VolumeConsumer::JoinSession(_In_ AllJoynServiceInfo^ serviceInfo)
{
    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<VolumeConsumer>,
        AllJoynHelpers::SessionMemberAddedHandler<VolumeConsumer>,
        AllJoynHelpers::SessionMemberRemovedHandler<VolumeConsumer>
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

    RETURN_IF_QSTATUS_ERROR(AllJoynHelpers::CreateBusObject<VolumeConsumer>(m_weak));

    alljoyn_interfacedescription description = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.alljoyn.Control.Volume");
    if (nullptr == description)
    {
        return AllJoynStatus::Fail;
    }

    RETURN_IF_QSTATUS_ERROR(alljoyn_proxybusobject_addinterface(ProxyBusObject, description));
    RETURN_IF_QSTATUS_ERROR(alljoyn_busobject_addinterface(BusObject, description));

    QStatus result = AddSignalHandler(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        description,
        "MuteChanged",
        [](const alljoyn_interfacedescription_member* member, PCSTR srcPath, alljoyn_message message) { UNREFERENCED_PARAMETER(srcPath); CallMuteChangedSignalHandler(member, message); });
    if (result != ER_OK)
    {
        return static_cast<int32>(result);
    }
    result = AddSignalHandler(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        description,
        "VolumeChanged",
        [](const alljoyn_interfacedescription_member* member, PCSTR srcPath, alljoyn_message message) { UNREFERENCED_PARAMETER(srcPath); CallVolumeChangedSignalHandler(member, message); });
    if (result != ER_OK)
    {
        return static_cast<int32>(result);
    }

    SourceInterfaces[description] = m_weak;
    RETURN_IF_QSTATUS_ERROR(alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject));
    m_signals->Initialize(BusObject, m_sessionId);

    alljoyn_sessionopts_destroy(sessionOpts);

    return AllJoynStatus::Ok;
}
