#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::alljoyn::Control;

VolumeWatcher::VolumeWatcher(AllJoynBusAttachment^ busAttachment) :
    m_aboutListener(nullptr)
{
    m_busAttachment = busAttachment;
    m_weak = new WeakReference(this);
    m_busAttachmentStateChangedToken.Value = 0;
}

VolumeWatcher::~VolumeWatcher()
{
    UnregisterFromBus();
}

void VolumeWatcher::UnregisterFromBus()
{
    if (nullptr != m_aboutListener)
    {
        PCSTR interfaces[] = { "org.alljoyn.Control.Volume" };
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

void VolumeWatcher::OnAnnounce(
    _In_ PCSTR name,
    _In_ uint16_t version,
    _In_ alljoyn_sessionport port,
    _In_ alljoyn_msgarg objectDescriptionArg,
    _In_ const alljoyn_msgarg aboutDataArg)
{
    UNREFERENCED_PARAMETER(version);
    UNREFERENCED_PARAMETER(aboutDataArg);

    alljoyn_aboutobjectdescription objectDescription = alljoyn_aboutobjectdescription_create_full(objectDescriptionArg);

    if (alljoyn_aboutobjectdescription_hasinterface(objectDescription, "org.alljoyn.Control.Volume"))
    {
        AllJoynServiceInfo^ args = ref new AllJoynServiceInfo(
            AllJoynHelpers::MultibyteToPlatformString(name),
            AllJoynHelpers::GetObjectPath(objectDescription, "org.alljoyn.Control.Volume"),
            port);
        Added(this, args);
    }
    alljoyn_aboutobjectdescription_destroy(objectDescription);
}

void VolumeWatcher::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {
        alljoyn_aboutlistener_callback callbacks = 
        {
            AllJoynHelpers::AnnounceHandler<VolumeWatcher>
        };
        m_aboutListener = alljoyn_aboutlistener_create(&callbacks, m_weak);

        alljoyn_busattachment_registeraboutlistener(AllJoynHelpers::GetInternalBusAttachment(sender), m_aboutListener);
        PCSTR interfaces[] = { "org.alljoyn.Control.Volume" };
        
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

void VolumeWatcher::Start()
{
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    int32 result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_VolumeIntrospectionXml);
    if (result != AllJoynStatus::Ok)
    {
        StopInternal(result);
        return;
    }

    m_busAttachmentStateChangedToken = m_busAttachment->StateChanged += ref new TypedEventHandler<AllJoynBusAttachment^, AllJoynBusAttachmentStateChangedEventArgs^>(this, &VolumeWatcher::BusAttachmentStateChanged);
    m_busAttachment->Connect();
}

void VolumeWatcher::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void VolumeWatcher::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}