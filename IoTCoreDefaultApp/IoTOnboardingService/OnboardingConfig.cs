// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Data.Xml.Dom;
using Windows.Storage;

namespace IoTOnboardingService
{
    class OnboardingConfig
    {
        private XmlDocument _xmlDocument;

        public async Task InitAsync(string filename = "onboarding.config")
        {
            if (filename == null)
            {
                throw new ArgumentException("Filename cannot be null", nameof(filename));
            }

            if (_xmlDocument == null)
            {
                _xmlDocument = new XmlDocument();

                var item = await ApplicationData.Current.LocalFolder.TryGetItemAsync(filename);
                if (item == null)
                {
                    var projectFolder = await Package.Current.InstalledLocation.GetFolderAsync("IoTOnboardingService");
                    item = await projectFolder.TryGetItemAsync(filename);

                    if (item != null && item.IsOfType(StorageItemTypes.File))
                    {
                        var file = await ((StorageFile)item).CopyAsync(ApplicationData.Current.LocalFolder);
                        var content = await FileIO.ReadTextAsync(file);
                        _xmlDocument.LoadXml(content);
                    }
                }
                else if (item.IsOfType(StorageItemTypes.File))
                {
                    var content = await FileIO.ReadTextAsync((StorageFile)item);
                    _xmlDocument.LoadXml(content);
                }
            }
        }

        public string Ssid
        {
            get { return _xmlDocument?.SelectSingleNode("/OnboardingConfig/SSID")?.InnerText; }
        }

        public string Password
        {
            get { return _xmlDocument?.SelectSingleNode("/OnboardingConfig/Password")?.InnerText; }
        }

        public string DefaultDescription
        {
            get { return _xmlDocument?.SelectSingleNode("/OnboardingConfig/AboutData/DefaultDescription")?.InnerText; }
        }

        public string DefaultManufacturer
        {
            get { return _xmlDocument?.SelectSingleNode("/OnboardingConfig/AboutData/DefaultManufacturer")?.InnerText; }
        }
    }
}
