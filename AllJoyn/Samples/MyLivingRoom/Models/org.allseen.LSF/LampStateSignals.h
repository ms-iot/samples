#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampStateSignals;

public interface class ILampStateSignals
{
    event Windows::Foundation::TypedEventHandler<LampStateSignals^, LampStateLampStateChangedReceivedEventArgs^>^ LampStateChangedReceived;
};

public ref class LampStateSignals sealed : [Windows::Foundation::Metadata::Default] ILampStateSignals
{
public:
    // Calling this method will send the LampStateChanged signal to every member of the session.
    void LampStateChanged(_In_ Platform::String^ LampID);

    // This event fires whenever the LampStateChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<LampStateSignals^, LampStateLampStateChangedReceivedEventArgs^>^ LampStateChangedReceived;

internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);
    void CallLampStateChangedReceived(_In_ LampStateSignals^ sender, _In_ LampStateLampStateChangedReceivedEventArgs^ args);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

    alljoyn_interfacedescription_member m_memberLampStateChanged;
};

} } } 