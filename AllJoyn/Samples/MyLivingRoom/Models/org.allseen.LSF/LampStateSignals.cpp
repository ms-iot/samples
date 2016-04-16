#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

void LampStateSignals::Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId)
{
    m_busObject = busObject;
    m_sessionId = sessionId;

    auto interfaceDefinition = alljoyn_busattachment_getinterface(alljoyn_busobject_getbusattachment(busObject), "org.allseen.LSF.LampState");
    alljoyn_interfacedescription_getmember(interfaceDefinition, "LampStateChanged", &m_memberLampStateChanged);
}

void LampStateSignals::LampStateChanged(_In_ Platform::String^ LampID)
{
    if (nullptr == m_busObject)
    {
        return;
    }

    size_t argCount = 1;
    alljoyn_msgarg arguments = alljoyn_msgarg_array_create(argCount);
    TypeConversionHelpers::SetAllJoynMessageArg(alljoyn_msgarg_array_element(arguments, 0), "s", LampID);
    
    alljoyn_busobject_signal(
        m_busObject, 
        NULL,  // Generated code only supports broadcast signals.
        m_sessionId,
        m_memberLampStateChanged,
        arguments,
        argCount, 
        0, // A signal with a TTL of 0 will be sent to every member of the session, regardless of how long it takes to deliver the message
        ALLJOYN_MESSAGE_FLAG_GLOBAL_BROADCAST, // Broadcast to everyone in the session.
        NULL); // The generated code does not need the generated signal message

    alljoyn_msgarg_destroy(arguments);
}

void LampStateSignals::CallLampStateChangedReceived(_In_ LampStateSignals^ sender, _In_ LampStateLampStateChangedReceivedEventArgs^ args)
{
    LampStateChangedReceived(sender, args);
}

