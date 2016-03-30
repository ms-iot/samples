#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

ref class PropertyConsumer;

public ref class PropertyJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property PropertyConsumer^ Consumer
    {
        PropertyConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ PropertyConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    PropertyConsumer^ m_consumer;
};

public ref class PropertyGetVersionResult sealed
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

    static PropertyGetVersionResult^ CreateSuccessResult(_In_ uint16 value)
    {
        auto result = ref new PropertyGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint16 m_value;
};

public ref class PropertyGetStatesResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 States
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static PropertyGetStatesResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new PropertyGetStatesResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->States = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class PropertyGetOptParamsResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ OptParams
    {
        Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ get() { return m_value; }
    internal:
        void set(_In_ Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ value) { m_value = value; }
    }

    static PropertyGetOptParamsResult^ CreateSuccessResult(_In_ Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ value)
    {
        auto result = ref new PropertyGetOptParamsResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->OptParams = value;
        return result;
    }

private:
    int m_status;
    Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ m_value;
};

public ref class PropertyGetValueResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property Platform::Object^ Value
    {
        Platform::Object^ get() { return m_value; }
    internal:
        void set(_In_ Platform::Object^ value) { m_value = value; }
    }

    static PropertyGetValueResult^ CreateSuccessResult(_In_ Platform::Object^ value)
    {
        auto result = ref new PropertyGetValueResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Value = value;
        return result;
    }

private:
    int m_status;
    Platform::Object^ m_value;
};

} } } 