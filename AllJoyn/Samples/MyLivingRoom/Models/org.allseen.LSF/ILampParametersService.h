#pragma once

namespace org { namespace allseen { namespace LSF {

public interface class ILampParametersService
{
public:
    // Implement this function to handle requests for the value of the Version property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampParametersGetVersionResult^>^ GetVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Energy_Usage_Milliwatts property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampParametersGetEnergy_Usage_MilliwattsResult^>^ GetEnergy_Usage_MilliwattsAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Brightness_Lumens property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampParametersGetBrightness_LumensResult^>^ GetBrightness_LumensAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

};

} } } 