#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

std::map<alljoyn_busobject, WeakReference*> LampDetailsProducer::SourceObjects;
std::map<alljoyn_interfacedescription, WeakReference*> LampDetailsProducer::SourceInterfaces;

LampDetailsProducer::LampDetailsProducer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_sessionListener(nullptr),
    m_busObject(nullptr),
    m_sessionPort(0),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    ServiceObjectPath = ref new String(L"/Service");
    m_signals = ref new LampDetailsSignals();
    m_busAttachmentStateChangedToken.Value = 0;
}

LampDetailsProducer::~LampDetailsProducer()
{
    UnregisterFromBus();
    delete m_weak;
}

void LampDetailsProducer::UnregisterFromBus()
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

bool LampDetailsProducer::OnAcceptSessionJoiner(_In_ alljoyn_sessionport sessionPort, _In_ PCSTR joiner, _In_ const alljoyn_sessionopts opts)
{
    UNREFERENCED_PARAMETER(sessionPort); UNREFERENCED_PARAMETER(joiner); UNREFERENCED_PARAMETER(opts);
    
    return true;
}

void LampDetailsProducer::OnSessionJoined(_In_ alljoyn_sessionport sessionPort, _In_ alljoyn_sessionid id, _In_ PCSTR joiner)
{
    UNREFERENCED_PARAMETER(joiner);

    // We initialize the Signals object after the session has been joined, because it needs
    // the session id.
    m_signals->Initialize(BusObject, id);
    m_sessionPort = sessionPort;
    m_sessionId = id;

    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<LampDetailsProducer>,
        AllJoynHelpers::SessionMemberAddedHandler<LampDetailsProducer>,
        AllJoynHelpers::SessionMemberRemovedHandler<LampDetailsProducer>
    };

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_busattachment_setsessionlistener(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), id, SessionListener);
}

void LampDetailsProducer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void LampDetailsProducer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void LampDetailsProducer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

void LampDetailsProducer::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {   
        QStatus result = AllJoynHelpers::CreateProducerSession<LampDetailsProducer>(m_busAttachment, m_weak);
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

QStatus LampDetailsProducer::AddMethodHandler(_In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_methodhandler_ptr handler)
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

QStatus LampDetailsProducer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

QStatus LampDetailsProducer::OnPropertyGet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _Inout_ alljoyn_msgarg value)
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
    if (0 == strcmp(propertyName, "Make"))
    {
        auto task = create_task(Service->GetMakeAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Make));
    }
    if (0 == strcmp(propertyName, "Model"))
    {
        auto task = create_task(Service->GetModelAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Model));
    }
    if (0 == strcmp(propertyName, "Type"))
    {
        auto task = create_task(Service->GetTypeAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Type));
    }
    if (0 == strcmp(propertyName, "LampType"))
    {
        auto task = create_task(Service->GetLampTypeAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->LampType));
    }
    if (0 == strcmp(propertyName, "LampBaseType"))
    {
        auto task = create_task(Service->GetLampBaseTypeAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->LampBaseType));
    }
    if (0 == strcmp(propertyName, "LampBeamAngle"))
    {
        auto task = create_task(Service->GetLampBeamAngleAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->LampBeamAngle));
    }
    if (0 == strcmp(propertyName, "Dimmable"))
    {
        auto task = create_task(Service->GetDimmableAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->Dimmable));
    }
    if (0 == strcmp(propertyName, "Color"))
    {
        auto task = create_task(Service->GetColorAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->Color));
    }
    if (0 == strcmp(propertyName, "VariableColorTemp"))
    {
        auto task = create_task(Service->GetVariableColorTempAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->VariableColorTemp));
    }
    if (0 == strcmp(propertyName, "HasEffects"))
    {
        auto task = create_task(Service->GetHasEffectsAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->HasEffects));
    }
    if (0 == strcmp(propertyName, "MinVoltage"))
    {
        auto task = create_task(Service->GetMinVoltageAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->MinVoltage));
    }
    if (0 == strcmp(propertyName, "MaxVoltage"))
    {
        auto task = create_task(Service->GetMaxVoltageAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->MaxVoltage));
    }
    if (0 == strcmp(propertyName, "Wattage"))
    {
        auto task = create_task(Service->GetWattageAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->Wattage));
    }
    if (0 == strcmp(propertyName, "IncandescentEquivalent"))
    {
        auto task = create_task(Service->GetIncandescentEquivalentAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->IncandescentEquivalent));
    }
    if (0 == strcmp(propertyName, "MaxLumens"))
    {
        auto task = create_task(Service->GetMaxLumensAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->MaxLumens));
    }
    if (0 == strcmp(propertyName, "MinTemperature"))
    {
        auto task = create_task(Service->GetMinTemperatureAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->MinTemperature));
    }
    if (0 == strcmp(propertyName, "MaxTemperature"))
    {
        auto task = create_task(Service->GetMaxTemperatureAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->MaxTemperature));
    }
    if (0 == strcmp(propertyName, "ColorRenderingIndex"))
    {
        auto task = create_task(Service->GetColorRenderingIndexAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "u", result->ColorRenderingIndex));
    }
    if (0 == strcmp(propertyName, "LampID"))
    {
        auto task = create_task(Service->GetLampIDAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "s", result->LampID));
    }

    return ER_BUS_NO_SUCH_PROPERTY;
}

QStatus LampDetailsProducer::OnPropertySet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _In_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    return ER_BUS_NO_SUCH_PROPERTY;
}

void LampDetailsProducer::Start()
{
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    QStatus result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_LampDetailsIntrospectionXml);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AllJoynHelpers::CreateBusObject<LampDetailsProducer>(m_weak);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    alljoyn_interfacedescription interfaceDescription = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.allseen.LSF.LampDetails");
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

    m_busAttachmentStateChangedToken = m_busAttachment->StateChanged += ref new TypedEventHandler<AllJoynBusAttachment^,AllJoynBusAttachmentStateChangedEventArgs^>(this, &LampDetailsProducer::BusAttachmentStateChanged);
    m_busAttachment->Connect();
}

void LampDetailsProducer::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void LampDetailsProducer::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}

int32 LampDetailsProducer::RemoveMemberFromSession(_In_ String^ uniqueName)
{
    return alljoyn_busattachment_removesessionmember(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        m_sessionId,
        AllJoynHelpers::PlatformToMultibyteString(uniqueName).data());
}

PCSTR org::allseen::LSF::c_LampDetailsIntrospectionXml = "<interface name=\"org.allseen.LSF.LampDetails\">"
"  <property name=\"Version\" type=\"u\" access=\"read\" />"
"  <property name=\"Make\" type=\"u\" access=\"read\" />"
"  <property name=\"Model\" type=\"u\" access=\"read\" />"
"  <property name=\"Type\" type=\"u\" access=\"read\" />"
"  <property name=\"LampType\" type=\"u\" access=\"read\" />"
"  <property name=\"LampBaseType\" type=\"u\" access=\"read\" />"
"  <property name=\"LampBeamAngle\" type=\"u\" access=\"read\" />"
"  <property name=\"Dimmable\" type=\"b\" access=\"read\" />"
"  <property name=\"Color\" type=\"b\" access=\"read\" />"
"  <property name=\"VariableColorTemp\" type=\"b\" access=\"read\" />"
"  <property name=\"HasEffects\" type=\"b\" access=\"read\" />"
"  <property name=\"MinVoltage\" type=\"u\" access=\"read\" />"
"  <property name=\"MaxVoltage\" type=\"u\" access=\"read\" />"
"  <property name=\"Wattage\" type=\"u\" access=\"read\" />"
"  <property name=\"IncandescentEquivalent\" type=\"u\" access=\"read\" />"
"  <property name=\"MaxLumens\" type=\"u\" access=\"read\" />"
"  <property name=\"MinTemperature\" type=\"u\" access=\"read\" />"
"  <property name=\"MaxTemperature\" type=\"u\" access=\"read\" />"
"  <property name=\"ColorRenderingIndex\" type=\"u\" access=\"read\" />"
"  <property name=\"LampID\" type=\"s\" access=\"read\" />"
"</interface>"
;
