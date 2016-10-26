#pragma once

namespace org { namespace allseen { namespace LSF {
public ref class LampStateLampStateChangedReceivedEventArgs sealed
{
public:
    property Windows::Devices::AllJoyn::AllJoynMessageInfo^ MessageInfo
    {
        Windows::Devices::AllJoyn::AllJoynMessageInfo^ get() { return m_messageInfo; }
        void set(_In_ Windows::Devices::AllJoyn::AllJoynMessageInfo^ value) { m_messageInfo = value; }
    }

    property Platform::String^ LampID
    {
        Platform::String^ get() { return m_LampID; }
    internal:
        void set(_In_ Platform::String^ value) { m_LampID = value; }
    }

private:
    Windows::Devices::AllJoyn::AllJoynMessageInfo^ m_messageInfo;

    Platform::String^ m_LampID;
};
} } } 