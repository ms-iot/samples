#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampServiceConsumer;

public ref class LampServiceClearLampFaultResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property uint32 LampResponseCode
    {
        uint32 get() { return m_LampResponseCode; }
    internal:
        void set(_In_ uint32 value) { m_LampResponseCode = value; }
    }
    property uint32 LampFaultCode
    {
        uint32 get() { return m_LampFaultCode; }
    internal:
        void set(_In_ uint32 value) { m_LampFaultCode = value; }
    }
private:
    int32 m_status;
    uint32 m_LampResponseCode;
    uint32 m_LampFaultCode;
};

public ref class LampServiceJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property LampServiceConsumer^ Consumer
    {
        LampServiceConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ LampServiceConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    LampServiceConsumer^ m_consumer;
};

public ref class LampServiceGetVersionResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Version
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampServiceGetVersionResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampServiceGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampServiceGetLampServiceVersionResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 LampServiceVersion
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampServiceGetLampServiceVersionResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampServiceGetLampServiceVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->LampServiceVersion = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampServiceGetLampFaultsResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property Windows::Foundation::Collections::IVector<uint32>^ LampFaults
    {
        Windows::Foundation::Collections::IVector<uint32>^ get() { return m_value; }
    internal:
        void set(_In_ Windows::Foundation::Collections::IVector<uint32>^ value) { m_value = value; }
    }

    static LampServiceGetLampFaultsResult^ CreateSuccessResult(_In_ Windows::Foundation::Collections::IVector<uint32>^ value)
    {
        auto result = ref new LampServiceGetLampFaultsResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->LampFaults = value;
        return result;
    }

private:
    int m_status;
    Windows::Foundation::Collections::IVector<uint32>^ m_value;
};

} } } 