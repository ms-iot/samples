#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {
public ref class PropertyMetadataChangedReceivedEventArgs sealed
{
public:
    property Windows::Devices::AllJoyn::AllJoynMessageInfo^ MessageInfo
    {
        Windows::Devices::AllJoyn::AllJoynMessageInfo^ get() { return m_messageInfo; }
        void set(_In_ Windows::Devices::AllJoyn::AllJoynMessageInfo^ value) { m_messageInfo = value; }
    }


private:
    Windows::Devices::AllJoyn::AllJoynMessageInfo^ m_messageInfo;

};
public ref class PropertyValueChangedReceivedEventArgs sealed
{
public:
    property Windows::Devices::AllJoyn::AllJoynMessageInfo^ MessageInfo
    {
        Windows::Devices::AllJoyn::AllJoynMessageInfo^ get() { return m_messageInfo; }
        void set(_In_ Windows::Devices::AllJoyn::AllJoynMessageInfo^ value) { m_messageInfo = value; }
    }


private:
    Windows::Devices::AllJoyn::AllJoynMessageInfo^ m_messageInfo;

};
} } } 