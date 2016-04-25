#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::allseen::LSF;

std::map<alljoyn_interfacedescription, WeakReference*> LampDetailsConsumer::SourceInterfaces;

LampDetailsConsumer::LampDetailsConsumer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_proxyBusObject(nullptr),
    m_busObject(nullptr),
    m_sessionListener(nullptr),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    m_signals = ref new LampDetailsSignals();
}

LampDetailsConsumer::~LampDetailsConsumer()
{
    if (SessionListener != nullptr)
    {
        alljoyn_busattachment_leavesession(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), m_sessionId);
        alljoyn_sessionlistener_destroy(SessionListener);
    }
    if (nullptr != ProxyBusObject)
    {
        alljoyn_proxybusobject_destroy(ProxyBusObject);
    }
    if (nullptr != BusObject)
    {
        alljoyn_busobject_destroy(BusObject);
    }
    delete m_weak;
}

void LampDetailsConsumer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void LampDetailsConsumer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void LampDetailsConsumer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

QStatus LampDetailsConsumer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

IAsyncOperation<LampDetailsJoinSessionResult^>^ LampDetailsConsumer::JoinSessionAsync(
    _In_ AllJoynServiceInfo^ serviceInfo, _Inout_ LampDetailsWatcher^ watcher)
{
    return create_async([serviceInfo, watcher]() -> LampDetailsJoinSessionResult^
    {
        auto result = ref new LampDetailsJoinSessionResult();
        result->Status = AllJoynStatus::Ok;
        result->Consumer = nullptr;

        result->Consumer = ref new LampDetailsConsumer(watcher->BusAttachment);
        result->Status = result->Consumer->JoinSession(serviceInfo);

        return result;
    });
}


IAsyncOperation<LampDetailsGetVersionResult^>^ LampDetailsConsumer::GetVersionAsync()
{
    return create_async([this]()->LampDetailsGetVersionResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Version",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetVersionResult();
        result->Status = getContext.GetStatus();
        result->Version = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetMakeResult^>^ LampDetailsConsumer::GetMakeAsync()
{
    return create_async([this]()->LampDetailsGetMakeResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Make",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetMakeResult();
        result->Status = getContext.GetStatus();
        result->Make = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetModelResult^>^ LampDetailsConsumer::GetModelAsync()
{
    return create_async([this]()->LampDetailsGetModelResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Model",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetModelResult();
        result->Status = getContext.GetStatus();
        result->Model = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetTypeResult^>^ LampDetailsConsumer::GetTypeAsync()
{
    return create_async([this]()->LampDetailsGetTypeResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Type",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetTypeResult();
        result->Status = getContext.GetStatus();
        result->Type = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetLampTypeResult^>^ LampDetailsConsumer::GetLampTypeAsync()
{
    return create_async([this]()->LampDetailsGetLampTypeResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "LampType",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetLampTypeResult();
        result->Status = getContext.GetStatus();
        result->LampType = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetLampBaseTypeResult^>^ LampDetailsConsumer::GetLampBaseTypeAsync()
{
    return create_async([this]()->LampDetailsGetLampBaseTypeResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "LampBaseType",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetLampBaseTypeResult();
        result->Status = getContext.GetStatus();
        result->LampBaseType = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetLampBeamAngleResult^>^ LampDetailsConsumer::GetLampBeamAngleAsync()
{
    return create_async([this]()->LampDetailsGetLampBeamAngleResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "LampBeamAngle",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetLampBeamAngleResult();
        result->Status = getContext.GetStatus();
        result->LampBeamAngle = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetDimmableResult^>^ LampDetailsConsumer::GetDimmableAsync()
{
    return create_async([this]()->LampDetailsGetDimmableResult^
    {
        PropertyGetContext<bool> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Dimmable",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<bool>*>(context);

                if (ER_OK == status)
                {
                    bool argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetDimmableResult();
        result->Status = getContext.GetStatus();
        result->Dimmable = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetColorResult^>^ LampDetailsConsumer::GetColorAsync()
{
    return create_async([this]()->LampDetailsGetColorResult^
    {
        PropertyGetContext<bool> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Color",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<bool>*>(context);

                if (ER_OK == status)
                {
                    bool argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetColorResult();
        result->Status = getContext.GetStatus();
        result->Color = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetVariableColorTempResult^>^ LampDetailsConsumer::GetVariableColorTempAsync()
{
    return create_async([this]()->LampDetailsGetVariableColorTempResult^
    {
        PropertyGetContext<bool> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "VariableColorTemp",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<bool>*>(context);

                if (ER_OK == status)
                {
                    bool argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetVariableColorTempResult();
        result->Status = getContext.GetStatus();
        result->VariableColorTemp = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetHasEffectsResult^>^ LampDetailsConsumer::GetHasEffectsAsync()
{
    return create_async([this]()->LampDetailsGetHasEffectsResult^
    {
        PropertyGetContext<bool> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "HasEffects",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<bool>*>(context);

                if (ER_OK == status)
                {
                    bool argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetHasEffectsResult();
        result->Status = getContext.GetStatus();
        result->HasEffects = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetMinVoltageResult^>^ LampDetailsConsumer::GetMinVoltageAsync()
{
    return create_async([this]()->LampDetailsGetMinVoltageResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "MinVoltage",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetMinVoltageResult();
        result->Status = getContext.GetStatus();
        result->MinVoltage = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetMaxVoltageResult^>^ LampDetailsConsumer::GetMaxVoltageAsync()
{
    return create_async([this]()->LampDetailsGetMaxVoltageResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "MaxVoltage",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetMaxVoltageResult();
        result->Status = getContext.GetStatus();
        result->MaxVoltage = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetWattageResult^>^ LampDetailsConsumer::GetWattageAsync()
{
    return create_async([this]()->LampDetailsGetWattageResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "Wattage",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetWattageResult();
        result->Status = getContext.GetStatus();
        result->Wattage = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetIncandescentEquivalentResult^>^ LampDetailsConsumer::GetIncandescentEquivalentAsync()
{
    return create_async([this]()->LampDetailsGetIncandescentEquivalentResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "IncandescentEquivalent",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetIncandescentEquivalentResult();
        result->Status = getContext.GetStatus();
        result->IncandescentEquivalent = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetMaxLumensResult^>^ LampDetailsConsumer::GetMaxLumensAsync()
{
    return create_async([this]()->LampDetailsGetMaxLumensResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "MaxLumens",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetMaxLumensResult();
        result->Status = getContext.GetStatus();
        result->MaxLumens = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetMinTemperatureResult^>^ LampDetailsConsumer::GetMinTemperatureAsync()
{
    return create_async([this]()->LampDetailsGetMinTemperatureResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "MinTemperature",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetMinTemperatureResult();
        result->Status = getContext.GetStatus();
        result->MinTemperature = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetMaxTemperatureResult^>^ LampDetailsConsumer::GetMaxTemperatureAsync()
{
    return create_async([this]()->LampDetailsGetMaxTemperatureResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "MaxTemperature",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetMaxTemperatureResult();
        result->Status = getContext.GetStatus();
        result->MaxTemperature = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetColorRenderingIndexResult^>^ LampDetailsConsumer::GetColorRenderingIndexAsync()
{
    return create_async([this]()->LampDetailsGetColorRenderingIndexResult^
    {
        PropertyGetContext<uint32> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "ColorRenderingIndex",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<uint32>*>(context);

                if (ER_OK == status)
                {
                    uint32 argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "u", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetColorRenderingIndexResult();
        result->Status = getContext.GetStatus();
        result->ColorRenderingIndex = getContext.GetValue();
        return result;
    });
}

IAsyncOperation<LampDetailsGetLampIDResult^>^ LampDetailsConsumer::GetLampIDAsync()
{
    return create_async([this]()->LampDetailsGetLampIDResult^
    {
        PropertyGetContext<Platform::String^> getContext;
        
        alljoyn_proxybusobject_getpropertyasync(
            ProxyBusObject,
            "org.allseen.LSF.LampDetails",
            "LampID",
            [](QStatus status, alljoyn_proxybusobject obj, const alljoyn_msgarg value, void* context)
            {
                UNREFERENCED_PARAMETER(obj);
                auto propertyContext = static_cast<PropertyGetContext<Platform::String^>*>(context);

                if (ER_OK == status)
                {
                    Platform::String^ argument;
                    TypeConversionHelpers::GetAllJoynMessageArg(value, "s", &argument);

                    propertyContext->SetValue(argument);
                }
                propertyContext->SetStatus(status);
                propertyContext->SetEvent();
            },
            c_MessageTimeoutInMilliseconds,
            &getContext);

        getContext.Wait();

        auto result = ref new LampDetailsGetLampIDResult();
        result->Status = getContext.GetStatus();
        result->LampID = getContext.GetValue();
        return result;
    });
}

void LampDetailsConsumer::OnPropertyChanged(_In_ alljoyn_proxybusobject obj, _In_ PCSTR interfaceName, _In_ const alljoyn_msgarg changed, _In_ const alljoyn_msgarg invalidated)
{
    UNREFERENCED_PARAMETER(obj);
    UNREFERENCED_PARAMETER(interfaceName);
    UNREFERENCED_PARAMETER(changed);
    UNREFERENCED_PARAMETER(invalidated);
}

int32 LampDetailsConsumer::JoinSession(_In_ AllJoynServiceInfo^ serviceInfo)
{
    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<LampDetailsConsumer>,
        AllJoynHelpers::SessionMemberAddedHandler<LampDetailsConsumer>,
        AllJoynHelpers::SessionMemberRemovedHandler<LampDetailsConsumer>
    };

    alljoyn_busattachment_enableconcurrentcallbacks(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment));

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_sessionopts sessionOpts = alljoyn_sessionopts_create(ALLJOYN_TRAFFIC_TYPE_MESSAGES, true, ALLJOYN_PROXIMITY_ANY, ALLJOYN_TRANSPORT_ANY);

    std::vector<char> sessionNameUtf8 = AllJoynHelpers::PlatformToMultibyteString(serviceInfo->UniqueName);
    RETURN_IF_QSTATUS_ERROR(alljoyn_busattachment_joinsession(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        &sessionNameUtf8[0],
        serviceInfo->SessionPort,
        SessionListener,
        &m_sessionId,
        sessionOpts));

    ServiceObjectPath = serviceInfo->ObjectPath;
    std::vector<char> objectPath = AllJoynHelpers::PlatformToMultibyteString(ServiceObjectPath);

    if (objectPath.empty())
    {
        return AllJoynStatus::Fail;
    }

    ProxyBusObject = alljoyn_proxybusobject_create(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), &sessionNameUtf8[0], &objectPath[0], m_sessionId);
    if (nullptr == ProxyBusObject)
    {
        return AllJoynStatus::Fail;
    }

    RETURN_IF_QSTATUS_ERROR(AllJoynHelpers::CreateBusObject<LampDetailsConsumer>(m_weak));

    alljoyn_interfacedescription description = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.allseen.LSF.LampDetails");
    if (nullptr == description)
    {
        return AllJoynStatus::Fail;
    }

    RETURN_IF_QSTATUS_ERROR(alljoyn_proxybusobject_addinterface(ProxyBusObject, description));
    RETURN_IF_QSTATUS_ERROR(alljoyn_busobject_addinterface(BusObject, description));


    SourceInterfaces[description] = m_weak;
    RETURN_IF_QSTATUS_ERROR(alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject));
    m_signals->Initialize(BusObject, m_sessionId);

    alljoyn_sessionopts_destroy(sessionOpts);

    return AllJoynStatus::Ok;
}
