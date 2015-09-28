// Copyright (c) Microsoft. All rights reserved.

using Windows.Devices.WiFiDirect;
using Windows.Security.Credentials;

namespace IoTOnboardingService
{
    class OnboardingAccessPoint
    {
        private WiFiDirectAdvertisementPublisher _publisher;
        private bool _active;

        public OnboardingAccessPoint(string ssid, string password)
        {
            _active = false;

            // Begin advertising for legacy clients
            _publisher = new WiFiDirectAdvertisementPublisher();

            // Note: If this flag is not set, the legacy parameters are ignored
            _publisher.Advertisement.IsAutonomousGroupOwnerEnabled = true;

            // Setup Advertisement to use a custom SSID and WPA2 passphrase
            _publisher.Advertisement.LegacySettings.IsEnabled = true;
            _publisher.Advertisement.LegacySettings.Ssid = ssid;
            _publisher.Advertisement.LegacySettings.Passphrase = new PasswordCredential { Password = password };

            _publisher.StatusChanged += OnAdvertisementStatusChanged;
        }

        public void Start()
        {
            if (!_active)
            {
                _publisher.Start();
            }
        }

        public void Stop()
        {
            if (_active)
            {
                _publisher.Stop();
            }
        }

        private void OnAdvertisementStatusChanged(WiFiDirectAdvertisementPublisher sender, WiFiDirectAdvertisementPublisherStatusChangedEventArgs args)
        {
            if (args.Status == WiFiDirectAdvertisementPublisherStatus.Started)
            {
                _active = true;
            }
            else if (args.Status == WiFiDirectAdvertisementPublisherStatus.Stopped)
            {
                _active = false;
            }
            else if (args.Status == WiFiDirectAdvertisementPublisherStatus.Aborted)
            {
                _active = false;
                if (args.Error == WiFiDirectError.RadioNotAvailable)
                {
                    // The radio was turned off.
                }
                else if (args.Error == WiFiDirectError.ResourceInUse)
                {
                    // The stack couldn't accept any additional IEs.  Need to turn off any other applications which could be advertising
                }
            }
        }
    }
}
