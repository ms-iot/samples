#pragma once

namespace org { namespace alljoyn { namespace Control {
public ref class VolumeMuteChangedReceivedEventArgs sealed
{
public:
    property Windows::Devices::AllJoyn::AllJoynMessageInfo^ MessageInfo
    {
        Windows::Devices::AllJoyn::AllJoynMessageInfo^ get() { return m_messageInfo; }
        void set(_In_ Windows::Devices::AllJoyn::AllJoynMessageInfo^ value) { m_messageInfo = value; }
    }

    property bool newMute
    {
        bool get() { return m_newMute; }
    internal:
        void set(_In_ bool value) { m_newMute = value; }
    }

private:
    Windows::Devices::AllJoyn::AllJoynMessageInfo^ m_messageInfo;

    bool m_newMute;
};
public ref class VolumeVolumeChangedReceivedEventArgs sealed
{
public:
    property Windows::Devices::AllJoyn::AllJoynMessageInfo^ MessageInfo
    {
        Windows::Devices::AllJoyn::AllJoynMessageInfo^ get() { return m_messageInfo; }
        void set(_In_ Windows::Devices::AllJoyn::AllJoynMessageInfo^ value) { m_messageInfo = value; }
    }

    property int16 newVolume
    {
        int16 get() { return m_newVolume; }
    internal:
        void set(_In_ int16 value) { m_newVolume = value; }
    }

private:
    Windows::Devices::AllJoyn::AllJoynMessageInfo^ m_messageInfo;

    int16 m_newVolume;
};
} } } 