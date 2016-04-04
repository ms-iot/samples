#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

LampParametersWatcher::LampParametersWatcher(AllJoynBusAttachment^ busAttachment) :
    m_aboutListener(nullptr)
{
    m_busAttachment = busAttachment;
    m_weak = new WeakReference(this);
    m_busAttachmentStateChangedToken.Value = 0;
}

LampParametersWatcher::~LampParametersWatcher()
{
    UnregisterFromBus();
}

void LampParametersWatcher::UnregisterFromBus()
{
    if (nullptr != m_aboutListener)
    {
        PCSTR interfaces[] = { "org.allseen.LSF.LampParameters" };
        alljoyn_busattachment_cancelwhoimplements_interfaces(
            AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
            interfaces,
            _countof(interfaces));

        alljoyn_busattachment_unregisteraboutlistener(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), m_aboutListener);
        alljoyn_aboutlistener_destroy(m_aboutListener);
        m_aboutListener = nullptr;
    }
    if ((nullptr != m_busAttachment) && (0 != m_busAttachmentStateChangedToken.Value))
    {
        m_busAttachment->StateChanged -= m_busAttachmentStateChangedToken;
    }
}

void LampParametersWatcher::OnAnnounce(
    _In_ PCSTR name,
    _In_ uint16_t version,
    _In_ alljoyn_sessionport port,
    _In_ alljoyn_msgarg objectDescriptionArg,
    _In_ const alljoyn_msgarg aboutDataArg)
{
    UNREFERENCED_PARAMETER(version);
    UNREFERENCED_PARAMETER(aboutDataArg);

    alljoyn_aboutobjectdescription objectDescription = alljoyn_aboutobjectdescription_create_full(objectDescriptionArg);

    if (alljoyn_aboutobjectdescription_hasinterface(objectDescription, "org.allseen.LSF.LampParameters"))
    {
        AllJoynServiceInfo^ args = ref new AllJoynServiceInfo(
            AllJoynHelpers::MultibyteToPlatformString(name),
            AllJoynHelpers::GetObjectPath(objectDescription, "org.allseen.LSF.LampParameters"),
            port);
        Added(this, args);
    }
    alljoyn_aboutobjectdescription_destroy(objectDescription);
}

void LampParametersWatcher::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {
        alljoyn_aboutlistener_callback callbacks = 
        {
            AllJoynHelpers::AnnounceHandler<LampParametersWatcher>
        };
        m_aboutListener = alljoyn_aboutlistener_create(&callbacks, m_weak);

        alljoyn_busattachment_registeraboutlistener(AllJoynHelpers::GetInternalBusAttachment(sender), m_aboutListener);
        PCSTR interfaces[] = { "org.allseen.LSF.LampParameters" };
        
        auto status = alljoyn_busattachment_whoimplements_interfaces(
            AllJoynHelpers::GetInternalBusAttachment(sender), 
            interfaces,
            _countof(interfaces));
        if (ER_OK != status)
        {
            StopInternal(status);
        }
    }
    else if (args->State == AllJoynBusAttachmentState::Disconnected)
    {
        StopInternal(ER_BUS_STOPPING);
    }
}

void LampParametersWatcher::Start()
{
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    int32 result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_LampParametersIntrospectionXml);
    if (result != AllJoynStatus::Ok)
    {
        StopInternal(result);
        return;
    }

    m_busAttachmentStateChangedToken = m_busAttachment->StateChanged += ref new TypedEventHandler<AllJoynBusAttachment^, AllJoynBusAttachmentStateChangedEventArgs^>(this, &LampParametersWatcher::BusAttachmentStateChanged);
    m_busAttachment->Connect();
}

void LampParametersWatcher::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void LampParametersWatcher::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}