// Copyright (c) Microsoft. All rights reserved.

using System;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Foundation.Metadata;
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
        }

        public async Task UpdateAsync()
        {
            UpdateWiFiImage();
            UpdateNetworkKeyVisibility();
            await UpdateConnectivityLevel();
            await UpdateWpsPushbuttonAvailable();
        }

        private void UpdateNetworkKeyVisibility()
        {
            // Only show the password box if needed
            if ((AvailableNetwork.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Open80211 &&
                 AvailableNetwork.SecuritySettings.NetworkEncryptionType == NetworkEncryptionType.None) ||
                 IsEapAvailable)
            {
                NetworkKeyInfoVisibility = false;
            }
            else
            {
                NetworkKeyInfoVisibility = true;
            }
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

        public async Task UpdateConnectivityLevel()
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

        public async Task UpdateWpsPushbuttonAvailable()
        { 
            IsWpsPushButtonAvailable = await IsWpsPushButtonAvailableAsync();
            OnPropertyChanged("IsWpsPushButtonAvailable");
        }

        public void Disconnect()
        {
            adapter.Disconnect();
        }

        public bool IsWpsPushButtonAvailable { get; set; }

        public bool NetworkKeyInfoVisibility { get; set; }

        private bool usePassword = false;
        public bool UsePassword
        {
            get
            {
                return usePassword;
            }
            set
            {
                usePassword = value;
                OnPropertyChanged("UsePassword");
            }
        }

        private bool connectAutomatically = false;
        public bool ConnectAutomatically
        {
            get
            {
                return connectAutomatically;
            }
            set
            {
                connectAutomatically = value;
                OnPropertyChanged("ConnectAutomatically");
            }
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

        private string userName;
        public string UserName
        {
            get { return userName; }
            set { userName = value; OnPropertyChanged("UserName"); }
        }

        private string password;
        public string Password
        {
            get { return password; }
            set { password = value; OnPropertyChanged("Password"); }
        }

        private string domain;
        public string Domain
        {
            get { return domain; }
            set { domain = value; OnPropertyChanged("Domain"); }
        }

        public bool IsEapAvailable
        {
            get
            {
                return ((availableNetwork.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Rsna) ||
                    (availableNetwork.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Wpa));
            }
        }

        public async Task<bool> IsWpsPushButtonAvailableAsync()
        {
            if (ApiInformation.IsApiContractPresent("Windows.Foundation.UniversalApiContract", 5, 0))
            {
                var result = await adapter.GetWpsConfigurationAsync(availableNetwork);
                if (result.SupportedWpsKinds.Contains(WiFiWpsKind.PushButton))
                    return true;
            }

            return false;
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
