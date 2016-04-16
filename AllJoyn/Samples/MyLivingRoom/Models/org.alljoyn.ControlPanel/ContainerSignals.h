#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

ref class ContainerSignals;

public interface class IContainerSignals
{
    event Windows::Foundation::TypedEventHandler<ContainerSignals^, ContainerMetadataChangedReceivedEventArgs^>^ MetadataChangedReceived;
};

public ref class ContainerSignals sealed : [Windows::Foundation::Metadata::Default] IContainerSignals
{
public:
    // Calling this method will send the MetadataChanged signal to every member of the session.
    void MetadataChanged();

    // This event fires whenever the MetadataChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<ContainerSignals^, ContainerMetadataChangedReceivedEventArgs^>^ MetadataChangedReceived;

internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);
    void CallMetadataChangedReceived(_In_ ContainerSignals^ sender, _In_ ContainerMetadataChangedReceivedEventArgs^ args);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

    alljoyn_interfacedescription_member m_memberMetadataChanged;
};

} } } 