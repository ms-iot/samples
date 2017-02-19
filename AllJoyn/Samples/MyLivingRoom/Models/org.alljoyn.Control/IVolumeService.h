#pragma once

namespace org { namespace alljoyn { namespace Control {

public interface class IVolumeService
{
public:
    // Implement this function to handle calls to the AdjustVolume method.
    Windows::Foundation::IAsyncOperation<VolumeAdjustVolumeResult^>^ AdjustVolumeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info , _In_ int16 increments);

    // Implement this function to handle requests for the value of the Mute property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<VolumeGetMuteResult^>^ GetMuteAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests to set the Mute property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<int>^ SetMuteAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info, bool value);

    // Implement this function to handle requests for the value of the Version property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<VolumeGetVersionResult^>^ GetVersionAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests for the value of the Volume property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<VolumeGetVolumeResult^>^ GetVolumeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

    // Implement this function to handle requests to set the Volume property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<int>^ SetVolumeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info, int16 value);

    // Implement this function to handle requests for the value of the VolumeRange property.
    //
    // Currently, info will always be null, because no information is available about the requestor.
    Windows::Foundation::IAsyncOperation<VolumeGetVolumeRangeResult^>^ GetVolumeRangeAsync(Windows::Devices::AllJoyn::AllJoynMessageInfo^ info);

};

} } } 