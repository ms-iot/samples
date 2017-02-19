#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampDetailsWatcher;

public interface class ILampDetailsWatcher
{
    event Windows::Foundation::TypedEventHandler<LampDetailsWatcher^, Windows::Devices::AllJoyn::AllJoynServiceInfo^>^ Added;
    event Windows::Foundation::TypedEventHandler<LampDetailsWatcher^, Windows::Devices::AllJoyn::AllJoynProducerStoppedEventArgs^>^ Stopped;
};

public ref class LampDetailsWatcher sealed : [Windows::Foundation::Metadata::Default] ILampDetailsWatcher
{
public:
    LampDetailsWatcher(Windows::Devices::AllJoyn::AllJoynBusAttachment^ busAttachment);
    virtual ~LampDetailsWatcher();

    // This event will fire whenever a producer for this service is found.
    virtual event Windows::Foundation::TypedEventHandler<LampDetailsWatcher^, Windows::Devices::AllJoyn::AllJoynServiceInfo^>^ Added;

    // This event will fire whenever the watcher is stopped.
    virtual event Windows::Foundation::TypedEventHandler<LampDetailsWatcher^, Windows::Devices::AllJoyn::AllJoynProducerStoppedEventArgs^>^ Stopped;

    // Start watching for producers advertising this service.
    void Start();

    // Stop watching for producers for this service.
    void Stop();

internal:
    void OnAnnounce(
        _In_ PCSTR name,
        _In_ uint16_t version,
        _In_ alljoyn_sessionport port,
        _In_ alljoyn_msgarg objectDescriptionArg,
        _In_ const alljoyn_msgarg aboutDataArg);

    void OnPropertyChanged(_In_ PCSTR prop_name, _In_ alljoyn_msgarg prop_value)
    {
        UNREFERENCED_PARAMETER(prop_name); UNREFERENCED_PARAMETER(prop_value);
    }

    property Windows::Devices::AllJoyn::AllJoynBusAttachment^ BusAttachment
    {
        Windows::Devices::AllJoyn::AllJoynBusAttachment^ get() { return m_busAttachment; }
    }

    // Stop watching for producers advertising this service and pass status to anyone listening for the Stopped event.
    void StopInternal(int32 status);

    void BusAttachmentStateChanged(_In_ Windows::Devices::AllJoyn::AllJoynBusAttachment^ sender, _In_ Windows::Devices::AllJoyn::AllJoynBusAttachmentStateChangedEventArgs^ args);

private:
    void UnregisterFromBus();

    Windows::Devices::AllJoyn::AllJoynBusAttachment^ m_busAttachment;
    Windows::Foundation::EventRegistrationToken m_busAttachmentStateChangedToken;

    alljoyn_aboutlistener m_aboutListener;
    
    // Used to pass a pointer to this class to callbacks.
    Platform::WeakReference* m_weak;
};

} } } 