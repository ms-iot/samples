#pragma once

namespace org { namespace alljoyn { namespace Control {

ref class VolumeConsumer;

public ref class VolumeAdjustVolumeResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

private:
    int32 m_status;
};

public ref class VolumeJoinSessionResult sealed
{
public:
    property int32 Status
    {
        int32 get() { return m_status; }
    internal:
        void set(_In_ int32 value) { m_status = value; }
    }

    property VolumeConsumer^ Consumer
    {
        VolumeConsumer^ get() { return m_consumer; }
    internal:
        void set(_In_ VolumeConsumer^ value) { m_consumer = value; }
    };

private:
    int32 m_status;
    VolumeConsumer^ m_consumer;
};

public ref class VolumeGetMuteResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property bool Mute
    {
        bool get() { return m_value; }
    internal:
        void set(_In_ bool value) { m_value = value; }
    }

    static VolumeGetMuteResult^ CreateSuccessResult(_In_ bool value)
    {
        auto result = ref new VolumeGetMuteResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Mute = value;
        return result;
    }

private:
    int m_status;
    bool m_value;
};

public ref class VolumeGetVersionResult sealed
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

    static VolumeGetVersionResult^ CreateSuccessResult(_In_ uint16 value)
    {
        auto result = ref new VolumeGetVersionResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Version = value;
        return result;
    }

private:
    int m_status;
    uint16 m_value;
};

public ref class VolumeGetVolumeResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property int16 Volume
    {
        int16 get() { return m_value; }
    internal:
        void set(_In_ int16 value) { m_value = value; }
    }

    static VolumeGetVolumeResult^ CreateSuccessResult(_In_ int16 value)
    {
        auto result = ref new VolumeGetVolumeResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->Volume = value;
        return result;
    }

private:
    int m_status;
    int16 m_value;
};

public ref class VolumeGetVolumeRangeResult sealed
{
public:
    property int Status
    {
        int get() { return m_status; }
    internal:
        void set(_In_ int value) { m_status = value; }
    }

    property VolumeVolumeRange^ VolumeRange
    {
        VolumeVolumeRange^ get() { return m_value; }
    internal:
        void set(_In_ VolumeVolumeRange^ value) { m_value = value; }
    }

    static VolumeGetVolumeRangeResult^ CreateSuccessResult(_In_ VolumeVolumeRange^ value)
    {
        auto result = ref new VolumeGetVolumeRangeResult();
        result->Status = Windows::Devices::AllJoyn::AllJoynStatus::Ok;
        result->VolumeRange = value;
        return result;
    }

private:
    int m_status;
    VolumeVolumeRange^ m_value;
};

} } } 