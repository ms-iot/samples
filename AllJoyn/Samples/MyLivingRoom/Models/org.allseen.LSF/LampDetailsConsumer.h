#pragma once

namespace org { namespace allseen { namespace LSF {

public interface class ILampDetailsConsumer
{
    event Windows::Foundation::TypedEventHandler<LampDetailsConsumer^, Windows::Devices::AllJoyn::AllJoynSessionLostEventArgs^>^ SessionLost;
    event Windows::Foundation::TypedEventHandler<LampDetailsConsumer^, Windows::Devices::AllJoyn::AllJoynSessionMemberAddedEventArgs^>^ SessionMemberAdded;
    event Windows::Foundation::TypedEventHandler<LampDetailsConsumer^, Windows::Devices::AllJoyn::AllJoynSessionMemberRemovedEventArgs^>^ SessionMemberRemoved;
};

public ref class LampDetailsConsumer sealed  : [Windows::Foundation::Metadata::Default] ILampDetailsConsumer
{
public:
    LampDetailsConsumer(Windows::Devices::AllJoyn::AllJoynBusAttachment^ busAttachment);
    virtual ~LampDetailsConsumer();

    // Join the AllJoyn session specified by sessionName.
    //
    // This will usually be called after the unique name of a producer has been reported
    // in the Added callback on the Watcher.
    static Windows::Foundation::IAsyncOperation<LampDetailsJoinSessionResult^>^ JoinSessionAsync(_In_ Windows::Devices::AllJoyn::AllJoynServiceInfo^ serviceInfo, _Inout_ LampDetailsWatcher^ watcher);

    // Get the value of the Version property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetVersionResult^>^ GetVersionAsync();

    // Get the value of the Make property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMakeResult^>^ GetMakeAsync();

    // Get the value of the Model property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetModelResult^>^ GetModelAsync();

    // Get the value of the Type property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetTypeResult^>^ GetTypeAsync();

    // Get the value of the LampType property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampTypeResult^>^ GetLampTypeAsync();

    // Get the value of the LampBaseType property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampBaseTypeResult^>^ GetLampBaseTypeAsync();

    // Get the value of the LampBeamAngle property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampBeamAngleResult^>^ GetLampBeamAngleAsync();

    // Get the value of the Dimmable property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetDimmableResult^>^ GetDimmableAsync();

    // Get the value of the Color property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetColorResult^>^ GetColorAsync();

    // Get the value of the VariableColorTemp property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetVariableColorTempResult^>^ GetVariableColorTempAsync();

    // Get the value of the HasEffects property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetHasEffectsResult^>^ GetHasEffectsAsync();

    // Get the value of the MinVoltage property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMinVoltageResult^>^ GetMinVoltageAsync();

    // Get the value of the MaxVoltage property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMaxVoltageResult^>^ GetMaxVoltageAsync();

    // Get the value of the Wattage property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetWattageResult^>^ GetWattageAsync();

    // Get the value of the IncandescentEquivalent property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetIncandescentEquivalentResult^>^ GetIncandescentEquivalentAsync();

    // Get the value of the MaxLumens property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMaxLumensResult^>^ GetMaxLumensAsync();

    // Get the value of the MinTemperature property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMinTemperatureResult^>^ GetMinTemperatureAsync();

    // Get the value of the MaxTemperature property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMaxTemperatureResult^>^ GetMaxTemperatureAsync();

    // Get the value of the ColorRenderingIndex property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetColorRenderingIndexResult^>^ GetColorRenderingIndexAsync();

    // Get the value of the LampID property.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampIDResult^>^ GetLampIDAsync();


    // Used to send signals or register functions to handle received signals.
    property LampDetailsSignals^ Signals
    {
        LampDetailsSignals^ get() { return m_signals; }
    }

    // This event will fire whenever the consumer loses the session that it is a member of.
    virtual event Windows::Foundation::TypedEventHandler<LampDetailsConsumer^, Windows::Devices::AllJoyn::AllJoynSessionLostEventArgs^>^ SessionLost;

    // This event will fire whenever a member joins the session.
    virtual event Windows::Foundation::TypedEventHandler<LampDetailsConsumer^, Windows::Devices::AllJoyn::AllJoynSessionMemberAddedEventArgs^>^ SessionMemberAdded;

    // This event will fire whenever a member leaves the session.
    virtual event Windows::Foundation::TypedEventHandler<LampDetailsConsumer^, Windows::Devices::AllJoyn::AllJoynSessionMemberRemovedEventArgs^>^ SessionMemberRemoved;

internal:
    // Consumers do not support property get.
    QStatus OnPropertyGet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _Inout_ alljoyn_msgarg val) 
    { 
        UNREFERENCED_PARAMETER(interfaceName); UNREFERENCED_PARAMETER(propertyName); UNREFERENCED_PARAMETER(val); 
        return ER_NOT_IMPLEMENTED; 
    }

    // Consumers do not support property set.
    QStatus OnPropertySet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _In_ alljoyn_msgarg val) 
    { 
        UNREFERENCED_PARAMETER(interfaceName); UNREFERENCED_PARAMETER(propertyName); UNREFERENCED_PARAMETER(val);
        return ER_NOT_IMPLEMENTED;
    }

    void OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason);
    void OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName);
    void OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName);

    void OnPropertyChanged(_In_ alljoyn_proxybusobject obj, _In_ PCSTR interfaceName, _In_ const alljoyn_msgarg changed, _In_ const alljoyn_msgarg invalidated);

    property Platform::String^ ServiceObjectPath
    {
        Platform::String^ get() { return m_ServiceObjectPath; }
        void set(Platform::String^ value) { m_ServiceObjectPath = value; }
    }

    property alljoyn_proxybusobject ProxyBusObject
    {
        alljoyn_proxybusobject get() { return m_proxyBusObject; }
        void set(alljoyn_proxybusobject value) { m_proxyBusObject = value; }
    }

    property alljoyn_busobject BusObject
    {
        alljoyn_busobject get() { return m_busObject; }
        void set(alljoyn_busobject value) { m_busObject = value; }
    }
    
    property alljoyn_sessionlistener SessionListener
    {
        alljoyn_sessionlistener get() { return m_sessionListener; }
        void set(alljoyn_sessionlistener value) { m_sessionListener = value; }
    }

    property alljoyn_sessionid SessionId
    {
        alljoyn_sessionid get() { return m_sessionId; }
    }
    
private:
    int32 JoinSession(_In_ Windows::Devices::AllJoyn::AllJoynServiceInfo^ serviceInfo);

    // Register a callback function to handle incoming signals.
    QStatus AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler);

    
    Windows::Devices::AllJoyn::AllJoynBusAttachment^ m_busAttachment;
    LampDetailsSignals^ m_signals;
    Platform::String^ m_ServiceObjectPath;

    alljoyn_proxybusobject m_proxyBusObject;
    alljoyn_busobject m_busObject;
    alljoyn_sessionlistener m_sessionListener;
    alljoyn_sessionid m_sessionId;

    // Used to pass a pointer to this class to callbacks
    Platform::WeakReference* m_weak;

    // This map is required because we need a way to pass the consumer to the signal
    // handlers, but the current AllJoyn C API does not allow passing a context to these
    // callbacks.
    static std::map<alljoyn_interfacedescription, Platform::WeakReference*> SourceInterfaces;
};

} } } 
