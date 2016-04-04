#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

ref class PropertySignals;

public interface class IPropertySignals
{
    event Windows::Foundation::TypedEventHandler<PropertySignals^, PropertyMetadataChangedReceivedEventArgs^>^ MetadataChangedReceived;
    event Windows::Foundation::TypedEventHandler<PropertySignals^, PropertyValueChangedReceivedEventArgs^>^ ValueChangedReceived;
};

public ref class PropertySignals sealed : [Windows::Foundation::Metadata::Default] IPropertySignals
{
public:
    // Calling this method will send the MetadataChanged signal to every member of the session.
    void MetadataChanged();

    // This event fires whenever the MetadataChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<PropertySignals^, PropertyMetadataChangedReceivedEventArgs^>^ MetadataChangedReceived;

    // Calling this method will send the ValueChanged signal to every member of the session.
    void ValueChanged();

    // This event fires whenever the ValueChanged signal is sent by another member of the session.
    virtual event Windows::Foundation::TypedEventHandler<PropertySignals^, PropertyValueChangedReceivedEventArgs^>^ ValueChangedReceived;

internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);
    void CallMetadataChangedReceived(_In_ PropertySignals^ sender, _In_ PropertyMetadataChangedReceivedEventArgs^ args);
    void CallValueChangedReceived(_In_ PropertySignals^ sender, _In_ PropertyValueChangedReceivedEventArgs^ args);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

    alljoyn_interfacedescription_member m_memberMetadataChanged;
    alljoyn_interfacedescription_member m_memberValueChanged;
};

} } } 