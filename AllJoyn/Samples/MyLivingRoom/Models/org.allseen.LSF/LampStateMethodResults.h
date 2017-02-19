#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampStateConsumer;

public ref class LampStateTransitionLampStateResult sealed
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
private:
    int32 m_status;
    uint32 m_LampResponseCode;
};

public ref class LampStateApplyPulseEffectResult sealed
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
private:
    int32 m_status;
    uint32 m_LampResponseCode;
};

public ref class LampStateJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property LampStateConsumer^ Consumer
    {
        LampStateConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ LampStateConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    LampStateConsumer^ m_consumer;
};

public ref class LampStateGetVersionResult sealed
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

    static LampStateGetVersionResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampStateGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampStateGetOnOffResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property bool OnOff
    {
        bool get() { return m_value; }
    internal:
        void set(_In_ bool value) { m_value = value; }
    }

    static LampStateGetOnOffResult^ CreateSuccessResult(_In_ bool value)
    {
        auto result = ref new LampStateGetOnOffResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->OnOff = value;
        return result;
    }

private:
    int m_status;
    bool m_value;
};

public ref class LampStateGetHueResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Hue
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampStateGetHueResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampStateGetHueResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Hue = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampStateGetSaturationResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Saturation
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampStateGetSaturationResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampStateGetSaturationResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Saturation = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampStateGetColorTempResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 ColorTemp
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampStateGetColorTempResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampStateGetColorTempResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->ColorTemp = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampStateGetBrightnessResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Brightness
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampStateGetBrightnessResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampStateGetBrightnessResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Brightness = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

} } } 