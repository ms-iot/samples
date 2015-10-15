// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using Windows.Devices.Enumeration;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Media.Imaging;

// NOTE: The following using statements are only needed in order to demonstrate many of the
// different device selectors available from Windows Runtime APIs. You will only need to include
// the namespace for the Windows Runtime API your actual scenario needs.
using Windows.Devices.HumanInterfaceDevice;
using Windows.Networking.Proximity;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.WiFiDirect;
using Windows.Media.Casting;
using Windows.Media.DialProtocol;
using Windows.Devices.Sensors;

namespace IoTCoreDefaultApp
{
    public struct ProtectionLevelSelectorInfo
    {
        public string DisplayName
        {
            get;
            set;
        }

        public Windows.Devices.Enumeration.DevicePairingProtectionLevel ProtectionLevel
        {
            get;
            set;
        }
    }

    public static class ProtectionSelectorChoices
    {
        public static List<ProtectionLevelSelectorInfo> Selectors
        {
            get
            {
                List<ProtectionLevelSelectorInfo> selectors = new List<ProtectionLevelSelectorInfo>();
                selectors.Add(new ProtectionLevelSelectorInfo() { DisplayName = "Default", ProtectionLevel= Windows.Devices.Enumeration.DevicePairingProtectionLevel.Default});
                selectors.Add(new ProtectionLevelSelectorInfo() { DisplayName = "None", ProtectionLevel = Windows.Devices.Enumeration.DevicePairingProtectionLevel.None});
                selectors.Add(new ProtectionLevelSelectorInfo() { DisplayName = "Encryption", ProtectionLevel = Windows.Devices.Enumeration.DevicePairingProtectionLevel.Encryption});
                selectors.Add(new ProtectionLevelSelectorInfo() { DisplayName = "Encryption and authentication", ProtectionLevel = Windows.Devices.Enumeration.DevicePairingProtectionLevel.EncryptionAndAuthentication});

                return selectors;
            }
        }
    }

    public class BluetoothDeviceInformationDisplay : INotifyPropertyChanged
    {
        private DeviceInformation deviceInfo;

        public BluetoothDeviceInformationDisplay(DeviceInformation deviceInfoIn)
        {
            deviceInfo = deviceInfoIn;
            UpdateGlyphBitmapImage();
        }

        public DeviceInformationKind Kind
        {
            get
            {
                return deviceInfo.Kind;
            }
        }

        public string Id
        {
            get
            {
                return deviceInfo.Id;
            }
        }

        public string Name
        {
            get
            {
                return deviceInfo.Name;
            }
        }

        public BitmapImage GlyphBitmapImage
        {
            get;
            private set;
        }

        public bool CanPair
        {
            get
            {
                return deviceInfo.Pairing.CanPair;
            }
        }

        public bool IsPaired
        {
            get
            {
                return deviceInfo.Pairing.IsPaired;
            }
        }

        public IReadOnlyDictionary<string, object> Properties
        {
            get
            {
                return deviceInfo.Properties;
            }
        }

        public DeviceInformation DeviceInformation
        {
            get
            {
                return deviceInfo;
            }

            private set
            {
                deviceInfo = value;
            }
        }

        public void Update(DeviceInformationUpdate deviceInfoUpdate)
        {
            deviceInfo.Update(deviceInfoUpdate);

            OnPropertyChanged("Kind");
            OnPropertyChanged("Id");
            OnPropertyChanged("Name");
            OnPropertyChanged("DeviceInformation");
            OnPropertyChanged("CanPair");
            OnPropertyChanged("IsPaired");

            UpdateGlyphBitmapImage();
        }

        private async void UpdateGlyphBitmapImage()
        {
            // Not available on Athens
            //DeviceThumbnail deviceThumbnail = await deviceInfo.GetGlyphThumbnailAsync();
            //BitmapImage glyphBitmapImage = new BitmapImage();
            //await glyphBitmapImage.SetSourceAsync(deviceThumbnail);
            //GlyphBitmapImage = glyphBitmapImage;
            //OnPropertyChanged("GlyphBitmapImage");
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

    public class GeneralPropertyValueConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            object property = null;

            if (value is IReadOnlyDictionary<string, object> &&
                parameter is string &&
                false == String.IsNullOrEmpty((string)parameter))
            {
                IReadOnlyDictionary<string, object> properties = value as IReadOnlyDictionary<string, object>;
                string propertyName = parameter as string;

                property = properties[propertyName];
            }

            return property;
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            throw new NotImplementedException();
        }
    }
}
