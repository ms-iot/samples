#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

ref class ControlPanelConsumer;

public ref class ControlPanelJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property ControlPanelConsumer^ Consumer
    {
        ControlPanelConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ ControlPanelConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    ControlPanelConsumer^ m_consumer;
};

public ref class ControlPanelGetVersionResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint16 Version
    {
        uint16 get() { return m_value; }
    internal:
        void set(_In_ uint16 value) { m_value = value; }
    }

    static ControlPanelGetVersionResult^ CreateSuccessResult(_In_ uint16 value)
    {
        auto result = ref new ControlPanelGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint16 m_value;
};

} } } 