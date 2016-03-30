#pragma once

namespace org { namespace allseen { namespace LSF {

public interface class ILampServiceService
{
public:
    // Implement this function to handle calls to the ClearLampFault method.
    Windows::Foundation::IAsyncOperation<LampServiceClearLampFaultResult^>^ ClearLampFaultAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info , _In_ uint32 LampFaultCode);

    // Implement this function to handle requests for the value of the Version property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampServiceGetVersionResult^>^ GetVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the LampServiceVersion property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampServiceGetLampServiceVersionResult^>^ GetLampServiceVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the LampFaults property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<LampServiceGetLampFaultsResult^>^ GetLampFaultsAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

};

} } } 