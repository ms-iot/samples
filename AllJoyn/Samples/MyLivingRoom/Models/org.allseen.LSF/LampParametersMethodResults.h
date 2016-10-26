#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampParametersConsumer;

public ref class LampParametersJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property LampParametersConsumer^ Consumer
    {
        LampParametersConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ LampParametersConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    LampParametersConsumer^ m_consumer;
};

public ref class LampParametersGetVersionResult sealed
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

    static LampParametersGetVersionResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampParametersGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampParametersGetEnergy_Usage_MilliwattsResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Energy_Usage_Milliwatts
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampParametersGetEnergy_Usage_MilliwattsResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampParametersGetEnergy_Usage_MilliwattsResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Energy_Usage_Milliwatts = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampParametersGetBrightness_LumensResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Brightness_Lumens
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampParametersGetBrightness_LumensResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampParametersGetBrightness_LumensResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Brightness_Lumens = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

} } } 