#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

std::map<alljoyn_busobject, WeakReference*> LampParametersProducer::SourceObjects;
std::map<alljoyn_interfacedescription, WeakReference*> LampParametersProducer::SourceInterfaces;

LampParametersProducer::LampParametersProducer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_sessionListener(nullptr),
    m_busObject(nullptr),
    m_sessionPort(0),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    ServiceObjectPath = ref new String(L"/Service");
    m_signals = ref new LampParametersSignals();
    m_busAttachmentStateChangedToken.Value = 0;
}

LampParametersProducer::~LampParametersProducer()
{
    UnregisterFromBus();
    delete m_weak;
}

void LampParametersProducer::UnregisterFromBus()
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

bool LampParametersProducer::OnAcceptSessionJoiner(_In_ alljoyn_sessionport sessionPort, _In_ PCSTR joiner, _In_ const alljoyn_sessionopts opts)
{
    UNREFERENCED_PARAMETER(sessionPort); UNREFERENCED_PARAMETER(joiner); UNREFERENCED_PARAMETER(opts);
    
    return true;
}

void LampParametersProducer::OnSessionJoined(_In_ alljoyn_sessionport sessionPort, _In_ alljoyn_sessionid id, _In_ PCSTR joiner)
{
    UNREFERENCED_PARAMETER(joiner);

    // We initialize the Signals object after the session has been joined, because it needs
    // the session id.
    m_signals->Initialize(BusObject, id);
    m_sessionPort = sessionPort;
    m_sessionId = id;

    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<LampParametersProducer>,
        AllJoynHelpers::SessionMemberAddedHandler<LampParametersProducer>,
        AllJoynHelpers::SessionMemberRemovedHandler<LampParametersProducer>
    };

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_busattachment_setsessionlistener(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), id, SessionListener);
}

void LampParametersProducer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void LampParametersProducer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void LampParametersProducer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

void LampParametersProducer::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {   
        QStatus result = AllJoynHelpers::CreateProducerSession<LampParametersProducer>(m_busAttachment, m_weak);
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

QStatus LampParametersProducer::AddMethodHandler(_In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_methodhandler_ptr handler)
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

QStatus LampParametersProducer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

QStatus LampParametersProducer::OnPropertyGet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _Inout_ alljoyn_msgarg value)
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
    if (0 == strcmp(propertyName, "Energy_Usage_Milliwatts"))
    {
        auto task = create_task(Service->GetEnergy_Usage_MilliwattsAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Energy_Usage_Milliwatts));
    }
    if (0 == strcmp(propertyName, "Brightness_Lumens"))
    {
        auto task = create_task(Service->GetBrightness_LumensAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Brightness_Lumens));
    }

    return ER_BUS_NO_SUCH_PROPERTY;
}

QStatus LampParametersProducer::OnPropertySet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _In_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    return ER_BUS_NO_SUCH_PROPERTY;
}

void LampParametersProducer::Start()
{
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    QStatus result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_LampParametersIntrospectionXml);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AllJoynHelpers::CreateBusObject<LampParametersProducer>(m_weak);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    alljoyn_interfacedescription interfaceDescription = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.allseen.LSF.LampParameters");
    if (interfaceDescription == nullptr)
    {
        StopInternal(ER_FAIL);
        return;
    }
    alljoyn_busobject_addinterface_announced(BusObject, interfaceDescription);

    
    SourceObjects[m_busObject] = m_weak;
    SourceInterfaces[interfaceDescription] = m_weak;
    
    result = alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    m_busAttachmentStateChangedToken = m_busAttachment->StateChanged += ref new TypedEventHandler<AllJoynBusAttachment^,AllJoynBusAttachmentStateChangedEventArgs^>(this, &LampParametersProducer::BusAttachmentStateChanged);
    m_busAttachment->Connect();
}

void LampParametersProducer::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void LampParametersProducer::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}

int32 LampParametersProducer::RemoveMemberFromSession(_In_ String^ uniqueName)
{
    return alljoyn_busattachment_removesessionmember(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        m_sessionId,
        AllJoynHelpers::PlatformToMultibyteString(uniqueName).data());
}

PCSTR org::allseen::LSF::c_LampParametersIntrospectionXml = "<interface name=\"org.allseen.LSF.LampParameters\">"
"  <property name=\"Version\" type=\"u\" access=\"read\" />"
"  <property name=\"Energy_Usage_Milliwatts\" type=\"u\" access=\"read\" />"
"  <property name=\"Brightness_Lumens\" type=\"u\" access=\"read\" />"
"</interface>"
;
