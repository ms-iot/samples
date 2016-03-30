#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::alljoyn::Control;

void VolumeSignals::Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId)
{
    m_busObject = busObject;
    m_sessionId = sessionId;

    auto interfaceDefinition = alljoyn_busattachment_getinterface(alljoyn_busobject_getbusattachment(busObject), "org.alljoyn.Control.Volume");
    alljoyn_interfacedescription_getmember(interfaceDefinition, "MuteChanged", &m_memberMuteChanged);
    alljoyn_interfacedescription_getmember(interfaceDefinition, "VolumeChanged", &m_memberVolumeChanged);
}

void VolumeSignals::MuteChanged(_In_ bool newMute)
{
    if (nullptr == m_busObject)
    {
        return;
    }

    size_t argCount = 1;
    alljoyn_msgarg arguments = alljoyn_msgarg_array_create(argCount);
    TypeConversionHelpers::SetAllJoynMessageArg(alljoyn_msgarg_array_element(arguments, 0), "b", newMute);
    
    alljoyn_busobject_signal(
        m_busObject, 
        NULL,  // Generated code only supports broadcast signals.
        m_sessionId,
        m_memberMuteChanged,
        arguments,
        argCount, 
        0, // A signal with a TTL of 0 will be sent to every member of the session, regardless of how long it takes to deliver the message
        ALLJOYN_MESSAGE_FLAG_GLOBAL_BROADCAST, // Broadcast to everyone in the session.
        NULL); // The generated code does not need the generated signal message

    alljoyn_msgarg_destroy(arguments);
}

void VolumeSignals::CallMuteChangedReceived(_In_ VolumeSignals^ sender, _In_ VolumeMuteChangedReceivedEventArgs^ args)
{
    MuteChangedReceived(sender, args);
}

void VolumeSignals::VolumeChanged(_In_ int16 newVolume)
{
    if (nullptr == m_busObject)
    {
        return;
    }

    size_t argCount = 1;
    alljoyn_msgarg arguments = alljoyn_msgarg_array_create(argCount);
    TypeConversionHelpers::SetAllJoynMessageArg(alljoyn_msgarg_array_element(arguments, 0), "n", newVolume);
    
    alljoyn_busobject_signal(
        m_busObject, 
        NULL,  // Generated code only supports broadcast signals.
        m_sessionId,
        m_memberVolumeChanged,
        arguments,
        argCount, 
        0, // A signal with a TTL of 0 will be sent to every member of the session, regardless of how long it takes to deliver the message
        ALLJOYN_MESSAGE_FLAG_GLOBAL_BROADCAST, // Broadcast to everyone in the session.
        NULL); // The generated code does not need the generated signal message

    alljoyn_msgarg_destroy(arguments);
}

void VolumeSignals::CallVolumeChangedReceived(_In_ VolumeSignals^ sender, _In_ VolumeVolumeChangedReceivedEventArgs^ args)
{
    VolumeChangedReceived(sender, args);
}

