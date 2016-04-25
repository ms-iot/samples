#pragma once

namespace org { namespace alljoyn { namespace Control {

ref class VolumeSignals;

public interface class IVolumeSignals
{
    event Windows::Foundation::TypedEventHandler<VolumeSignals^, VolumeMuteChangedReceivedEventArgs^>^ MuteChangedReceived;
    event Windows::Foundation::TypedEventHandler<VolumeSignals^, VolumeVolumeChangedReceivedEventArgs^>^ VolumeChangedReceived;
};

public ref class VolumeSignals sealed : [Windows::Foundation::Metadata::Default] IVolumeSignals
{
public:
    // Calling this method will send the MuteChanged signal to every member of the session.
    void MuteChanged(_In_ bool newMute);

    // This event fires whenever the MuteChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<VolumeSignals^, VolumeMuteChangedReceivedEventArgs^>^ MuteChangedReceived;

    // Calling this method will send the VolumeChanged signal to every member of the session.
    void VolumeChanged(_In_ int16 newVolume);

    // This event fires whenever the VolumeChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<VolumeSignals^, VolumeVolumeChangedReceivedEventArgs^>^ VolumeChangedReceived;

internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);
    void CallMuteChangedReceived(_In_ VolumeSignals^ sender, _In_ VolumeMuteChangedReceivedEventArgs^ args);
    void CallVolumeChangedReceived(_In_ VolumeSignals^ sender, _In_ VolumeVolumeChangedReceivedEventArgs^ args);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

    alljoyn_interfacedescription_member m_memberMuteChanged;
    alljoyn_interfacedescription_member m_memberVolumeChanged;
};

} } } 