#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampDetailsConsumer;

public ref class LampDetailsJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property LampDetailsConsumer^ Consumer
    {
        LampDetailsConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ LampDetailsConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    LampDetailsConsumer^ m_consumer;
};

public ref class LampDetailsGetVersionResult sealed
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

    static LampDetailsGetVersionResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetMakeResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Make
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetMakeResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetMakeResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Make = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetModelResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Model
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetModelResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetModelResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Model = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetTypeResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Type
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetTypeResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetTypeResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Type = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetLampTypeResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 LampType
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetLampTypeResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetLampTypeResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->LampType = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetLampBaseTypeResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 LampBaseType
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetLampBaseTypeResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetLampBaseTypeResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->LampBaseType = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetLampBeamAngleResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 LampBeamAngle
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetLampBeamAngleResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetLampBeamAngleResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->LampBeamAngle = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetDimmableResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property bool Dimmable
    {
        bool get() { return m_value; }
    internal:
        void set(_In_ bool value) { m_value = value; }
    }

    static LampDetailsGetDimmableResult^ CreateSuccessResult(_In_ bool value)
    {
        auto result = ref new LampDetailsGetDimmableResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Dimmable = value;
        return result;
    }

private:
    int m_status;
    bool m_value;
};

public ref class LampDetailsGetColorResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property bool Color
    {
        bool get() { return m_value; }
    internal:
        void set(_In_ bool value) { m_value = value; }
    }

    static LampDetailsGetColorResult^ CreateSuccessResult(_In_ bool value)
    {
        auto result = ref new LampDetailsGetColorResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Color = value;
        return result;
    }

private:
    int m_status;
    bool m_value;
};

public ref class LampDetailsGetVariableColorTempResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property bool VariableColorTemp
    {
        bool get() { return m_value; }
    internal:
        void set(_In_ bool value) { m_value = value; }
    }

    static LampDetailsGetVariableColorTempResult^ CreateSuccessResult(_In_ bool value)
    {
        auto result = ref new LampDetailsGetVariableColorTempResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->VariableColorTemp = value;
        return result;
    }

private:
    int m_status;
    bool m_value;
};

public ref class LampDetailsGetHasEffectsResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property bool HasEffects
    {
        bool get() { return m_value; }
    internal:
        void set(_In_ bool value) { m_value = value; }
    }

    static LampDetailsGetHasEffectsResult^ CreateSuccessResult(_In_ bool value)
    {
        auto result = ref new LampDetailsGetHasEffectsResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->HasEffects = value;
        return result;
    }

private:
    int m_status;
    bool m_value;
};

public ref class LampDetailsGetMinVoltageResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 MinVoltage
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetMinVoltageResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetMinVoltageResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->MinVoltage = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetMaxVoltageResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 MaxVoltage
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetMaxVoltageResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetMaxVoltageResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->MaxVoltage = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetWattageResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 Wattage
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetWattageResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetWattageResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Wattage = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetIncandescentEquivalentResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 IncandescentEquivalent
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetIncandescentEquivalentResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetIncandescentEquivalentResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->IncandescentEquivalent = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetMaxLumensResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 MaxLumens
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetMaxLumensResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetMaxLumensResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->MaxLumens = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetMinTemperatureResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 MinTemperature
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetMinTemperatureResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetMinTemperatureResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->MinTemperature = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetMaxTemperatureResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 MaxTemperature
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetMaxTemperatureResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetMaxTemperatureResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->MaxTemperature = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetColorRenderingIndexResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property uint32 ColorRenderingIndex
    {
        uint32 get() { return m_value; }
    internal:
        void set(_In_ uint32 value) { m_value = value; }
    }

    static LampDetailsGetColorRenderingIndexResult^ CreateSuccessResult(_In_ uint32 value)
    {
        auto result = ref new LampDetailsGetColorRenderingIndexResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->ColorRenderingIndex = value;
        return result;
    }

private:
    int m_status;
    uint32 m_value;
};

public ref class LampDetailsGetLampIDResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property Platform::String^ LampID
    {
        Platform::String^ get() { return m_value; }
    internal:
        void set(_In_ Platform::String^ value) { m_value = value; }
    }

    static LampDetailsGetLampIDResult^ CreateSuccessResult(_In_ Platform::String^ value)
    {
        auto result = ref new LampDetailsGetLampIDResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->LampID = value;
        return result;
    }

private:
    int m_status;
    Platform::String^ m_value;
};

} } } 