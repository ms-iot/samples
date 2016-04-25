#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

std::map<alljoyn_busobject, WeakReference*> LampStateProducer::SourceObjects;
std::map<alljoyn_interfacedescription, WeakReference*> LampStateProducer::SourceInterfaces;

LampStateProducer::LampStateProducer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_sessionListener(nullptr),
    m_busObject(nullptr),
    m_sessionPort(0),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    ServiceObjectPath = ref new String(L"/Service");
    m_signals = ref new LampStateSignals();
    m_busAttachmentStateChangedToken.Value = 0;
}

LampStateProducer::~LampStateProducer()
{
    UnregisterFromBus();
    delete m_weak;
}

void LampStateProducer::UnregisterFromBus()
{
    if ((nullptr != m_busAttachment) && (0 != m_busAttachmentStateChangedToken.Value))
    {
        m_busAttachment->StateChanged -= m_busAttachmentStateChangedToken;
        m_busAttachmentStateChangedToken.Value = 0;
    }
    if (nullptr != SessionPortListener)
    {
        alljoyn_busattachment_unbindsessionport(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), m_sessionPort);
        alljoyn_sessionportlistener_destroy(SessionPortListener);
        SessionPortListener = nullptr;
    }
    if (nullptr != BusObject)
    {
        alljoyn_busattachment_unregisterbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
        alljoyn_busobject_destroy(BusObject);
        BusObject = nullptr;
    }
    if (nullptr != SessionListener)
    {
        alljoyn_sessionlistener_destroy(SessionListener);
        SessionListener = nullptr;
    }
}

bool LampStateProducer::OnAcceptSessionJoiner(_In_ alljoyn_sessionport sessionPort, _In_ PCSTR joiner, _In_ const alljoyn_sessionopts opts)
{
    UNREFERENCED_PARAMETER(sessionPort); UNREFERENCED_PARAMETER(joiner); UNREFERENCED_PARAMETER(opts);
    
    return true;
}

void LampStateProducer::OnSessionJoined(_In_ alljoyn_sessionport sessionPort, _In_ alljoyn_sessionid id, _In_ PCSTR joiner)
{
    UNREFERENCED_PARAMETER(joiner);

    // We initialize the Signals object after the session has been joined, because it needs
    // the session id.
    m_signals->Initialize(BusObject, id);
    m_sessionPort = sessionPort;
    m_sessionId = id;

    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<LampStateProducer>,
        AllJoynHelpers::SessionMemberAddedHandler<LampStateProducer>,
        AllJoynHelpers::SessionMemberRemovedHandler<LampStateProducer>
    };

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_busattachment_setsessionlistener(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), id, SessionListener);
}

void LampStateProducer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void LampStateProducer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void LampStateProducer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

void LampStateProducer::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {   
        QStatus result = AllJoynHelpers::CreateProducerSession<LampStateProducer>(m_busAttachment, m_weak);
        if (ER_OK != result)
        {
            StopInternal(result);
            return;
        }
    }
    else if (args->State == AllJoynBusAttachmentState::Disconnected)
    {
        StopInternal(ER_BUS_STOPPING);
    }
}

void LampStateProducer::CallTransitionLampStateHandler(_Inout_ alljoyn_busobject busObject, _In_ alljoyn_message message)
{
    auto source = SourceObjects.find(busObject);
    if (source == SourceObjects.end())
    {
        return;
    }

    LampStateProducer^ producer = source->second->Resolve<LampStateProducer>();
    if (producer->Service != nullptr)
    {
        AllJoynMessageInfo^ callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));

        uint64 inputArg0;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "t", &inputArg0);
        Windows::Foundation::Collections::IMapView<Platform::String^,Platform::Object^>^ inputArg1;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 1), "a{sv}", &inputArg1);
        uint32 inputArg2;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 2), "u", &inputArg2);

        create_task(producer->Service->TransitionLampStateAsync(callInfo, inputArg0, inputArg1, inputArg2)).then([busObject, message](LampStateTransitionLampStateResult^ result)
        {
            size_t argCount = 1;
            alljoyn_msgarg outputs = alljoyn_msgarg_array_create(argCount);
            int32 status;
            status = TypeConversionHelpers::SetAllJoynMessageArg(alljoyn_msgarg_array_element(outputs, 0), "u", result->LampResponseCode);
            if (AllJoynStatus::Ok != status)
            {
                alljoyn_busobject_methodreply_status(busObject, message, static_cast<QStatus>(status));
                alljoyn_msgarg_destroy(outputs);
                return;
            }
            alljoyn_busobject_methodreply_args(busObject, message, outputs, argCount);
            alljoyn_msgarg_destroy(outputs);
        }).wait();
    }
}

void LampStateProducer::CallApplyPulseEffectHandler(_Inout_ alljoyn_busobject busObject, _In_ alljoyn_message message)
{
    auto source = SourceObjects.find(busObject);
    if (source == SourceObjects.end())
    {
        return;
    }

    LampStateProducer^ producer = source->second->Resolve<LampStateProducer>();
    if (producer->Service != nullptr)
    {
        AllJoynMessageInfo^ callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));

        Windows::Foundation::Collections::IMapView<Platform::String^,Platform::Object^>^ inputArg0;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "a{sv}", &inputArg0);
        Windows::Foundation::Collections::IMapView<Platform::String^,Platform::Object^>^ inputArg1;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 1), "a{sv}", &inputArg1);
        uint32 inputArg2;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 2), "u", &inputArg2);
        uint32 inputArg3;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 3), "u", &inputArg3);
        uint32 inputArg4;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 4), "u", &inputArg4);
        uint64 inputArg5;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 5), "t", &inputArg5);

        create_task(producer->Service->ApplyPulseEffectAsync(callInfo, inputArg0, inputArg1, inputArg2, inputArg3, inputArg4, inputArg5)).then([busObject, message](LampStateApplyPulseEffectResult^ result)
        {
            size_t argCount = 1;
            alljoyn_msgarg outputs = alljoyn_msgarg_array_create(argCount);
            int32 status;
            status = TypeConversionHelpers::SetAllJoynMessageArg(alljoyn_msgarg_array_element(outputs, 0), "u", result->LampResponseCode);
            if (AllJoynStatus::Ok != status)
            {
                alljoyn_busobject_methodreply_status(busObject, message, static_cast<QStatus>(status));
                alljoyn_msgarg_destroy(outputs);
                return;
            }
            alljoyn_busobject_methodreply_args(busObject, message, outputs, argCount);
            alljoyn_msgarg_destroy(outputs);
        }).wait();
    }
}

void LampStateProducer::CallLampStateChangedSignalHandler(_In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message message)
{
    auto source = SourceInterfaces.find(member->iface);
    if (source == SourceInterfaces.end())
    {
        return;
    }

    auto producer = source->second->Resolve<LampStateProducer>();
    if (producer->Signals != nullptr)
    {
        auto callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));
        auto eventArgs = ref new LampStateLampStateChangedReceivedEventArgs();
        eventArgs->MessageInfo = callInfo;

        Platform::String^ argument0;
        TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "s", &argument0);
        eventArgs->LampID = argument0;

        producer->Signals->CallLampStateChangedReceived(producer->Signals, eventArgs);
    }
}

QStatus LampStateProducer::AddMethodHandler(_In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_methodhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busobject_addmethodhandler(
        m_busObject,
        member,
        handler,
        m_weak);
}

QStatus LampStateProducer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

QStatus LampStateProducer::OnPropertyGet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _Inout_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    if (0 == strcmp(propertyName, "Version"))
    {
        auto task = create_task(Service->GetVersionAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Version));
    }
    if (0 == strcmp(propertyName, "OnOff"))
    {
        auto task = create_task(Service->GetOnOffAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->OnOff));
    }
    if (0 == strcmp(propertyName, "Hue"))
    {
        auto task = create_task(Service->GetHueAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Hue));
    }
    if (0 == strcmp(propertyName, "Saturation"))
    {
        auto task = create_task(Service->GetSaturationAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Saturation));
    }
    if (0 == strcmp(propertyName, "ColorTemp"))
    {
        auto task = create_task(Service->GetColorTempAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->ColorTemp));
    }
    if (0 == strcmp(propertyName, "Brightness"))
    {
        auto task = create_task(Service->GetBrightnessAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Brightness));
    }

    return ER_BUS_NO_SUCH_PROPERTY;
}

QStatus LampStateProducer::OnPropertySet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _In_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    if (0 == strcmp(propertyName, "OnOff"))
    {
        bool argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

        auto task = create_task(Service->SetOnOffAsync(nullptr, argument));
        return static_cast<QStatus>(task.get());
    }
    if (0 == strcmp(propertyName, "Hue"))
    {
        uint32 argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

        auto task = create_task(Service->SetHueAsync(nullptr, argument));
        return static_cast<QStatus>(task.get());
    }
    if (0 == strcmp(propertyName, "Saturation"))
    {
        uint32 argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

        auto task = create_task(Service->SetSaturationAsync(nullptr, argument));
        return static_cast<QStatus>(task.get());
    }
    if (0 == strcmp(propertyName, "ColorTemp"))
    {
        uint32 argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

        auto task = create_task(Service->SetColorTempAsync(nullptr, argument));
        return static_cast<QStatus>(task.get());
    }
    if (0 == strcmp(propertyName, "Brightness"))
    {
        uint32 argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

        auto task = create_task(Service->SetBrightnessAsync(nullptr, argument));
        return static_cast<QStatus>(task.get());
    }
    return ER_BUS_NO_SUCH_PROPERTY;
}

void LampStateProducer::Start()
{
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    QStatus result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_LampStateIntrospectionXml);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AllJoynHelpers::CreateBusObject<LampStateProducer>(m_weak);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    alljoyn_interfacedescription interfaceDescription = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.allseen.LSF.LampState");
    if (interfaceDescription == nullptr)
    {
        StopInternal(ER_FAIL);
        return;
    }
    alljoyn_busobject_addinterface_announced(BusObject, interfaceDescription);

    result = AddMethodHandler(
        interfaceDescription, 
        "TransitionLampState", 
        [](alljoyn_busobject busObject, const alljoyn_interfacedescription_member* member, alljoyn_message message) { UNREFERENCED_PARAMETER(member); CallTransitionLampStateHandler(busObject, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AddMethodHandler(
        interfaceDescription, 
        "ApplyPulseEffect", 
        [](alljoyn_busobject busObject, const alljoyn_interfacedescription_member* member, alljoyn_message message) { UNREFERENCED_PARAMETER(member); CallApplyPulseEffectHandler(busObject, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AddSignalHandler(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        interfaceDescription,
        "LampStateChanged",
        [](const alljoyn_interfacedescription_member* member, PCSTR srcPath, alljoyn_message message) { UNREFERENCED_PARAMETER(srcPath); CallLampStateChangedSignalHandler(member, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }
    
    SourceObjects[m_busObject] = m_weak;
    SourceInterfaces[interfaceDescription] = m_weak;
    
    result = alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    m_busAttachmentStateChangedToken = m_busAttachment->StateChanged += ref new TypedEventHandler<AllJoynBusAttachment^,AllJoynBusAttachmentStateChangedEventArgs^>(this, &LampStateProducer::BusAttachmentStateChanged);
    m_busAttachment->Connect();
}

void LampStateProducer::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void LampStateProducer::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}

int32 LampStateProducer::RemoveMemberFromSession(_In_ String^ uniqueName)
{
    return alljoyn_busattachment_removesessionmember(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        m_sessionId,
        AllJoynHelpers::PlatformToMultibyteString(uniqueName).data());
}

PCSTR org::allseen::LSF::c_LampStateIntrospectionXml = "<interface name=\"org.allseen.LSF.LampState\">"
"  <property name=\"Version\" type=\"u\" access=\"read\" />"
"  <method name=\"TransitionLampState\">"
"    <arg name=\"Timestamp\" type=\"t\" direction=\"in\" />"
"    <arg name=\"NewState\" type=\"a{sv}\" direction=\"in\" />"
"    <arg name=\"TransitionPeriod\" type=\"u\" direction=\"in\" />"
"    <arg name=\"LampResponseCode\" type=\"u\" direction=\"out\" />"
"  </method>"
"  <method name=\"ApplyPulseEffect\">"
"    <arg name=\"FromState\" type=\"a{sv}\" direction=\"in\" />"
"    <arg name=\"ToState\" type=\"a{sv}\" direction=\"in\" />"
"    <arg name=\"period\" type=\"u\" direction=\"in\" />"
"    <arg name=\"duration\" type=\"u\" direction=\"in\" />"
"    <arg name=\"numPulses\" type=\"u\" direction=\"in\" />"
"    <arg name=\"timestamp\" type=\"t\" direction=\"in\" />"
"    <arg name=\"LampResponseCode\" type=\"u\" direction=\"out\" />"
"  </method>"
"  <signal name=\"LampStateChanged\">"
"    <arg name=\"LampID\" type=\"s\" />"
"  </signal>"
"  <property name=\"OnOff\" type=\"b\" access=\"readwrite\" />"
"  <property name=\"Hue\" type=\"u\" access=\"readwrite\" />"
"  <property name=\"Saturation\" type=\"u\" access=\"readwrite\" />"
"  <property name=\"ColorTemp\" type=\"u\" access=\"readwrite\" />"
"  <property name=\"Brightness\" type=\"u\" access=\"readwrite\" />"
"</interface>"
;
