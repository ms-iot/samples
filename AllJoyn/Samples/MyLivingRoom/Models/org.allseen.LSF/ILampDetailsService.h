#pragma once

namespace org { namespace allseen { namespace LSF {

public interface class ILampDetailsService
{
public:
    // Implement this function to handle requests for the value of the Version property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetVersionResult^>^ GetVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Make property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMakeResult^>^ GetMakeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Model property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetModelResult^>^ GetModelAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Type property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetTypeResult^>^ GetTypeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the LampType property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampTypeResult^>^ GetLampTypeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the LampBaseType property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampBaseTypeResult^>^ GetLampBaseTypeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the LampBeamAngle property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampBeamAngleResult^>^ GetLampBeamAngleAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Dimmable property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetDimmableResult^>^ GetDimmableAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Color property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetColorResult^>^ GetColorAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the VariableColorTemp property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetVariableColorTempResult^>^ GetVariableColorTempAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the HasEffects property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetHasEffectsResult^>^ GetHasEffectsAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the MinVoltage property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMinVoltageResult^>^ GetMinVoltageAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the MaxVoltage property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMaxVoltageResult^>^ GetMaxVoltageAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Wattage property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetWattageResult^>^ GetWattageAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the IncandescentEquivalent property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetIncandescentEquivalentResult^>^ GetIncandescentEquivalentAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the MaxLumens property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMaxLumensResult^>^ GetMaxLumensAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the MinTemperature property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMinTemperatureResult^>^ GetMinTemperatureAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the MaxTemperature property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetMaxTemperatureResult^>^ GetMaxTemperatureAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the ColorRenderingIndex property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetColorRenderingIndexResult^>^ GetColorRenderingIndexAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the LampID property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampDetailsGetLampIDResult^>^ GetLampIDAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

};

} } } 