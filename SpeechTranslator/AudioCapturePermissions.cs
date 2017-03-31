// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using Windows.Media.Capture;

namespace SpeechTranslator
{
    class AudioCapturePermissions
    {   
        // If no recording device is attached, attempting to get access to audio capture devices will throw 
        // a System.Exception object, with this HResult set.
        private static int NoCaptureDevicesHResult = -1072845856;

        /// <summary>
        /// On IOT Devices starting from build 15063 with the Cortana Package installed (the standard production image)
        /// permission to use the capture device must be granted.  Currently this permission can be granted from Cortana 
        /// during the "out-of-box-experience".  
        /// 
        /// For IoT custom IoT images without Cortana, or older images permission is automatically granted.
        ///
        /// 
        /// Developers should ideally perform a check like this every time their app gains focus, in order to 
        /// check if the user has changed the setting while the app was suspended or not in focus.
        /// </summary>
        /// <returns>true if the microphone can be accessed without any permissions problems.</returns>
        /// 
        public async static Task<bool> RequestMicrophoneCapture()
        {
            try
            {
                // Request access to the microphone only, to limit the number of capabilities we need
                // to request in the package manifest.
                MediaCaptureInitializationSettings settings = new MediaCaptureInitializationSettings();
                settings.StreamingCaptureMode = StreamingCaptureMode.Audio;
                settings.MediaCategory = MediaCategory.Speech;
                MediaCapture capture = new MediaCapture();
                await capture.InitializeAsync(settings);
            }
            catch (UnauthorizedAccessException)
            {   // The user has turned off access to the microphone. If this occurs, we should show an error, or disable
                // functionality within the app to ensure that further exceptions aren't generated when 
                // recognition is attempted.
                return false;
            }
            catch (Exception exception)
            {
                // This can be replicated by using remote desktop to a system, but not redirecting the microphone input.
                // Can also occur if using the virtual machine console tool to access a VM instead of using remote desktop.
                if (exception.HResult == NoCaptureDevicesHResult)
                {
                    return false;
                }
                else
                {
                    throw;
                }

            }
            return true;
        }
    }
}
