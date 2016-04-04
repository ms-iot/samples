#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

public interface class IContainerService
{
public:
    // Implement this function to handle requests for the value of the Version property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<ContainerGetVersionResult^>^ GetVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the States property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<ContainerGetStatesResult^>^ GetStatesAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the OptParams property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<ContainerGetOptParamsResult^>^ GetOptParamsAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

};

} } } 