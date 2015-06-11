// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Networking.Connectivity;
using Windows.UI.Xaml.Media.Imaging;

namespace WiFiConnect
{
    public class WiFiNetworkDisplay : INotifyPropertyChanged
    {
        private WiFiAdapter adapter;
        public WiFiNetworkDisplay(WiFiAvailableNetwork availableNetwork, WiFiAdapter adapter)
        {
            AvailableNetwork = availableNetwork;
            this.adapter = adapter;
            UpdateWiFiImage();
            UpdateConnectivityLevel();
        }

        private void UpdateWiFiImage()
        {
            string imageFileNamePrefix = "secure";
            if (AvailableNetwork.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Open80211)
            {
                imageFileNamePrefix = "open";
            }

            string imageFileName = string.Format("ms-appx:/Assets/{0}_{1}bar.png", imageFileNamePrefix, AvailableNetwork.SignalBars);

            WiFiImage = new BitmapImage(new Uri(imageFileName));

            OnPropertyChanged("WiFiImage");

        }

        public async void UpdateConnectivityLevel()
        {
            string connectivityLevel = "Not Connected";
            string connectedSsid = null;

            var connectedProfile = await adapter.NetworkAdapter.GetConnectedProfileAsync();
            if (connectedProfile != null &&
                connectedProfile.IsWlanConnectionProfile &&
                connectedProfile.WlanConnectionProfileDetails != null)
            {
                connectedSsid = connectedProfile.WlanConnectionProfileDetails.GetConnectedSsid();
            }

            if (!string.IsNullOrEmpty(connectedSsid))
            {
                if (connectedSsid.Equals(AvailableNetwork.Ssid))
                {
                    connectivityLevel = connectedProfile.GetNetworkConnectivityLevel().ToString();
                }
            }

            ConnectivityLevel = connectivityLevel;

            OnPropertyChanged("ConnectivityLevel");
        }

        public String Ssid
        {
            get
            {
                return availableNetwork.Ssid;
            }
        }

        public String Bssid
        {
            get
            {
                return availableNetwork.Bssid;

            }
        }

        public String ChannelCenterFrequency
        {
            get
            {
                return string.Format("{0}kHz", availableNetwork.ChannelCenterFrequencyInKilohertz);
            }
        }

        public String Rssi
        {
            get
            {
                return string.Format("{0}dBm", availableNetwork.NetworkRssiInDecibelMilliwatts);
            }
        }

        public String SecuritySettings
        {
            get
            {
                return string.Format("Authentication: {0}; Encryption: {1}", availableNetwork.SecuritySettings.NetworkAuthenticationType, availableNetwork.SecuritySettings.NetworkEncryptionType);
            }
        }
        public String ConnectivityLevel
        {
            get;
            private set;
        }

        public BitmapImage WiFiImage
        {
            get;
            private set;
        }


        private WiFiAvailableNetwork availableNetwork;
        public WiFiAvailableNetwork AvailableNetwork
        {
            get
            {
                return availableNetwork;
            }

            private set
            {
                availableNetwork = value;
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }
    }
}
