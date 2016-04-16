#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

public interface class IControlPanelService
{
public:
    // Implement this function to handle requests for the value of the Version property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<ControlPanelGetVersionResult^>^ GetVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

};

} } } 