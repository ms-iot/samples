// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using Windows.Foundation;
#if BUILDWITHCORTANA
using Windows.Services.Cortana;
#endif
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace IoTCoreDefaultApp
{
    class CortanaHelper
    {
        public static void LaunchCortanaToConsentPageAsyncIfNeeded()
        {
#if BUILDWITHCORTANA
            // Do nothing for devices that do not support Cortana
            if (CortanaSettings.IsSupported())
            {
                // Ordinarily, this is run during a first use Out-of-box-Experience (OOBE) and voice consent will NOT be granted.
                // So, we launch Cortana to it's Consent Page as part of OOBE to give the end-user an opportunity to give consent.
                // However on development systems where the IotCoreDefaultApp may be deployed repeatedly after Cortana consent has
                // already been granted we will bypass the Cortana Launch.
                var cortanaSettings = CortanaSettings.GetDefault();
                if (!cortanaSettings.HasUserConsentToVoiceActivation)
                {
                    LaunchCortanaToConsentPageAsync();
                }
            }
#endif
        }

        public static Task<bool> LaunchCortanaToConsentPageAsync()
        {
            // NOTE: When integrating this sample code into your specific device, Microsoft recommends changing the QuerySourceSecondaryId in the URI to "IoT_<MANUFACTURER>_<DEVICE_DESCRIPTION>"
            var uri = new Uri("ms-cortana://CapabilitiesPrompt/?RequestedCapabilities=InputPersonalization,Microphone,Personalization&QuerySourceSecondaryId=IoT&QuerySource=Microphone&DismissAfterConsent=True");
            return Windows.System.Launcher.LaunchUriAsync(uri).AsTask<bool>();
        }

        public static Task<bool> LaunchCortanaToAboutMeAsync()
        {
            var uri = new Uri("ms-cortana://AboutMe");
            return Windows.System.Launcher.LaunchUriAsync(uri).AsTask<bool>();
        }
    }
}
