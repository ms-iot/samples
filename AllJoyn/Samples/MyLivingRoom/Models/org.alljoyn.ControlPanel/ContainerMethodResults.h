#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

ref class ContainerConsumer;

public ref class ContainerJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property ContainerConsumer^ Consumer
    {
        ContainerConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ ContainerConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    ContainerConsumer^ m_consumer;
};

public ref class ContainerGetVersionResult sealed
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

    static ContainerGetVersionResult^ CreateSuccessResult(_In_ uint16 value)
    {
        auto result = ref new ContainerGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint16 m_value;
};

public ref class ContainerGetStatesResult sealed
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

    static ContainerGetStatesResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new ContainerGetStatesResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->States = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class ContainerGetOptParamsResult sealed
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

    static ContainerGetOptParamsResult^ CreateSuccessResult(_In_ Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ value)
    {
        auto result = ref new ContainerGetOptParamsResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->OptParams = value;
        return result;
    }

private:
    int m_status;
    Windows::Foundation::Collections::IMap<uint16,Platform::Object^>^ m_value;
};

} } } 