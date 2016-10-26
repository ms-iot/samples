#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::alljoyn::ControlPanel;

void PropertySignals::Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId)
{
    m_busObject = busObject;
    m_sessionId = sessionId;

    auto interfaceDefinition = alljoyn_busattachment_getinterface(alljoyn_busobject_getbusattachment(busObject), "org.alljoyn.ControlPanel.Property");
    alljoyn_interfacedescription_getmember(interfaceDefinition, "MetadataChanged", &m_memberMetadataChanged);
    alljoyn_interfacedescription_getmember(interfaceDefinition, "ValueChanged", &m_memberValueChanged);
}

void PropertySignals::MetadataChanged()
{
    if (nullptr == m_busObject)
    {
        return;
    }

    size_t argCount = 0;
    alljoyn_msgarg arguments = alljoyn_msgarg_array_create(argCount);
    
    alljoyn_busobject_signal(
        m_busObject, 
        NULL,  // Generated code only supports broadcast signals.
        m_sessionId,
        m_memberMetadataChanged,
        arguments,
        argCount, 
        0, // A signal with a TTL of 0 will be sent to every member of the session, regardless of how long it takes to deliver the message
        ALLJOYN_MESSAGE_FLAG_GLOBAL_BROADCAST, // Broadcast to everyone in the session.
        NULL); // The generated code does not need the generated signal message

    alljoyn_msgarg_destroy(arguments);
}

void PropertySignals::CallMetadataChangedReceived(_In_ PropertySignals^ sender, _In_ PropertyMetadataChangedReceivedEventArgs^ args)
{
    MetadataChangedReceived(sender, args);
}

void PropertySignals::ValueChanged()
{
    if (nullptr == m_busObject)
    {
        return;
    }

    size_t argCount = 0;
    alljoyn_msgarg arguments = alljoyn_msgarg_array_create(argCount);
    
    alljoyn_busobject_signal(
        m_busObject, 
        NULL,  // Generated code only supports broadcast signals.
        m_sessionId,
        m_memberValueChanged,
        arguments,
        argCount, 
        0, // A signal with a TTL of 0 will be sent to every member of the session, regardless of how long it takes to deliver the message
        ALLJOYN_MESSAGE_FLAG_GLOBAL_BROADCAST, // Broadcast to everyone in the session.
        NULL); // The generated code does not need the generated signal message

    alljoyn_msgarg_destroy(arguments);
}

void PropertySignals::CallValueChangedReceived(_In_ PropertySignals^ sender, _In_ PropertyValueChangedReceivedEventArgs^ args)
{
    ValueChangedReceived(sender, args);
}

