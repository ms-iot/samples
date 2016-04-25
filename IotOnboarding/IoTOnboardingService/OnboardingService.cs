// Copyright (c) Microsoft. All rights reserved.

using org.alljoyn.Icon;
using org.alljoyn.Onboarding;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Devices.AllJoyn;
using Windows.Devices.Enumeration;
using Windows.Devices.WiFi;
using Windows.Foundation;
using Windows.Networking.Connectivity;
using Windows.Security.Credentials;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.ApplicationModel;
using Windows.Data.Xml.Dom;

namespace IoTOnboardingService
{
    public sealed class OnboardingService : IOnboardingService, IIconService
    {
        private struct SoftAPConfig
        {
            public bool enabled;
            public string ssid;
            public string password;
        };
        private SoftAPConfig _softAPConfig;
        private struct AJOnboardingConfig
        {
            public bool enabled;
            public string defaultDescription;
            public string defaultManufacturer;
            public string modelNumber;
        }
        private AJOnboardingConfig _ajOnboardingConfig;

        private const string CONFIG_FILE_NAME = "Config.xml";

        private const string NODE_SOFTAPENABLE = "IoTOnboarding/SoftAP/Enabled";
        private const string NODE_SOFTAPSSIDTEMPLATE = "IoTOnboarding/SoftAP/Ssid";
        private const string NODE_SOFTAPPASSWORD = "IoTOnboarding/SoftAP/Password";

        private const string NODE_ALLJOYNONBOARDINGENABLE = "IoTOnboarding/AllJoynOnboarding/Enabled";
        private const string NODE_DEFAULTDESCRIPTIONTEMPLATE = "IoTOnboarding/AllJoynOnboarding/DefaultDescription";
        private const string NODE_DEFAULTMANUFACTURER = "IoTOnboarding/AllJoynOnboarding/DefaultManufacturer";
        private const string NODE_MODELNUMBER = "IoTOnboarding/AllJoynOnboarding/ModelNumber";
        private const string ATTRIBUTE_VALUE = "value";

        private const string SOFTAP_SSID_AJONBOARDING_PREFIX = "AJ_";
         
        private static ushort _onboardingInterfaceVersion = 1412;
        private static ushort _iconInterfaceVersion = 1412;
        private static string _onboardingInstanceIdSettingName = "OnboardingInstanceId";

        private string _onboardingInstanceId;
        private OnboardingProducer _onboardingProducer;
        private IconProducer _iconProducer;
        private AllJoynBusAttachment _busAttachment;

        private OnboardingAccessPoint _softAccessPoint;

        private DeviceWatcher _deviceWatcher;
        private string _wlanAdapterId;

        private string _personalApSsid;
        private string _personalApPassword;

        private AuthType _authType;
        private OnboardingState _state;
        private OnboardingError _error;
        private string _errorMessage;
        private object _stateLock;

        private StorageFile _iconFile;

        private async Task ResetConfig()
        {
            var installedLocation = Package.Current.InstalledLocation;
            var localFolder = ApplicationData.Current.LocalFolder;
            StorageFile configFile;

            // delete config if it exists
            var currentConfig = await localFolder.TryGetItemAsync(CONFIG_FILE_NAME);
            if (currentConfig != null)
            {
                await currentConfig.DeleteAsync();
            }

            // copy config from package
            var pkgFile = await installedLocation.GetFileAsync(CONFIG_FILE_NAME);
            configFile = await pkgFile.CopyAsync(localFolder);
        }

        private async Task<bool> ReadConfig()
        {
            bool retVal = true;

            try
            {
                // get config file from app folder
                var localFolder = ApplicationData.Current.LocalFolder;
                StorageFile configFile;
                if (await localFolder.TryGetItemAsync(CONFIG_FILE_NAME) == null)
                {
                    // config file doesn't exist => copy it from the app package
                    var installedLocation = Package.Current.InstalledLocation;
                    var pkgFile = await installedLocation.GetFileAsync(CONFIG_FILE_NAME);
                    configFile = await pkgFile.CopyAsync(localFolder);
                }
                else
                {
                    configFile = await localFolder.GetFileAsync(CONFIG_FILE_NAME);
                }

                var xmlConfig = await XmlDocument.LoadFromFileAsync(configFile);

                // SoftAP config
                var xmlNode = xmlConfig.SelectSingleNode(NODE_SOFTAPENABLE);
                var tempString = GetXmlNodeValue(xmlNode);
                if (tempString == "true")
                {
                    _softAPConfig.enabled = true;
                }
                else
                {
                    _softAPConfig.enabled = false;
                }
                xmlNode = xmlConfig.SelectSingleNode(NODE_SOFTAPSSIDTEMPLATE);
                _softAPConfig.ssid = GetXmlNodeValue(xmlNode);
                xmlNode = xmlConfig.SelectSingleNode(NODE_SOFTAPPASSWORD);
                _softAPConfig.password = GetXmlNodeValue(xmlNode);

                // AllJoyn Onboarding config
                xmlNode = xmlConfig.SelectSingleNode(NODE_ALLJOYNONBOARDINGENABLE);
                if (tempString == "true")
                {
                    _ajOnboardingConfig.enabled = true;
                }
                else
                {
                    _ajOnboardingConfig.enabled = false;
                }
                xmlNode = xmlConfig.SelectSingleNode(NODE_DEFAULTDESCRIPTIONTEMPLATE);
                _ajOnboardingConfig.defaultDescription = GetXmlNodeValue(xmlNode);
                xmlNode = xmlConfig.SelectSingleNode(NODE_DEFAULTMANUFACTURER);
                _ajOnboardingConfig.defaultManufacturer = GetXmlNodeValue(xmlNode);
                xmlNode = xmlConfig.SelectSingleNode(NODE_MODELNUMBER);
                _ajOnboardingConfig.modelNumber = GetXmlNodeValue(xmlNode);
            }
            catch (Exception ex)
            {
                retVal = false;
            }


            return retVal; 
        }
        public OnboardingService()
        {
            _state = OnboardingState.NotConfigured;
            _stateLock = new object();

            var settings = ApplicationData.Current.LocalSettings.Values;
            if (settings.ContainsKey(_onboardingInstanceIdSettingName))
            {
                _onboardingInstanceId = settings[_onboardingInstanceIdSettingName] as string;
            }
            else
            {
                var guid = Guid.NewGuid();
                _onboardingInstanceId = guid.GetHashCode().ToString("X8");
                settings[_onboardingInstanceIdSettingName] = _onboardingInstanceId;
            }
        }

        public async void Start()
        {
            try
            {
                _iconFile = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///IoTOnboardingService/icon72x72.png"));

                var connectionProfiles = NetworkInformation.GetConnectionProfiles();
                foreach (var profile in connectionProfiles)
                {
                    if (profile.IsWlanConnectionProfile)
                    {
                        lock (_stateLock)
                        {
                            _state = OnboardingState.ConfiguredValidated;
                        }
                        break;
                    }
                }

                // read configuration
                bool configOk = await ReadConfig();
                if (!configOk)
                {
                    // for some reason config file doesn't seem to be OK
                    // => reset config and try another time 
                    await ResetConfig();
                    configOk = await ReadConfig();
                    if(!configOk)
                    {
                        throw new System.IO.InvalidDataException("Invalid configuration");
                    }
                }

                // create softAP 
                if (_softAccessPoint == null &&
                    (_softAPConfig.enabled ||
                     _ajOnboardingConfig.enabled))
                {
                    string prefix = "";
                    string suffix = "";
                    string ssid = "";
                    if(_ajOnboardingConfig.enabled)
                    {
                        prefix = SOFTAP_SSID_AJONBOARDING_PREFIX;
                        suffix = "_" + _onboardingInstanceId;
                    }

                    ssid = prefix + _softAPConfig.ssid + suffix;
                    _softAccessPoint = new OnboardingAccessPoint(ssid, _softAPConfig.password);                    
                }

                // create AllJoyn related things
                if (_busAttachment == null &&
                    _ajOnboardingConfig.enabled)
                {
                    _busAttachment = new AllJoynBusAttachment();

                    _busAttachment.AboutData.DefaultDescription = _ajOnboardingConfig.defaultDescription  + " instance Id " + _onboardingInstanceId;
                    _busAttachment.AboutData.DefaultManufacturer = _ajOnboardingConfig.defaultManufacturer;
                    _busAttachment.AboutData.ModelNumber = _ajOnboardingConfig.modelNumber;

                    _onboardingProducer = new OnboardingProducer(_busAttachment);
                    _onboardingProducer.Service = this;

                    _iconProducer = new IconProducer(_busAttachment);
                    _iconProducer.Service = this;
                }

                if (_deviceWatcher == null)
                {
                    var accessStatus = await WiFiAdapter.RequestAccessAsync();
                    if (accessStatus == WiFiAccessStatus.Allowed)
                    {
                        _deviceWatcher = DeviceInformation.CreateWatcher(WiFiAdapter.GetDeviceSelector());
                        _deviceWatcher.Added += this.HandleAdapterAdded;
                        _deviceWatcher.Removed += this.HandleAdapterRemoved;

                        _deviceWatcher.Start();
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }

        public void Stop()
        {
            if (_onboardingProducer != null)
            {
                _onboardingProducer.Stop();
            }
            if (_iconProducer != null)
            {
                _iconProducer.Stop();
            }
            if (_softAccessPoint != null)
            {
                _softAccessPoint.Stop();
            }
            if (_deviceWatcher != null)
            {
                _deviceWatcher.Stop();
            }
        }

        private string GetXmlNodeValue(IXmlNode xmlNode)
        {
            if(null == xmlNode)
            {
                throw new System.ArgumentNullException("GetXmlNodeValue wrong xmlNode");
            }

            var attribute = xmlNode.Attributes.GetNamedItem(ATTRIBUTE_VALUE);
            if(attribute == null)
            {
                throw new System.IO.InvalidDataException("xml node " + xmlNode.LocalName + " has no attribute named " + ATTRIBUTE_VALUE);
            }

            return attribute.InnerText;
        }

        private void HandleAdapterAdded(DeviceWatcher sender, DeviceInformation information)
        {
            if (_wlanAdapterId == null)
            {
                _wlanAdapterId = information.Id;

                lock (_stateLock)
                {
                    if (_state != OnboardingState.ConfiguredValidated)
                    {
                        _softAccessPoint.Start();
                    }
                }
                _onboardingProducer.Start();
                _iconProducer.Start();
            }
        }

        private void HandleAdapterRemoved(DeviceWatcher sender, DeviceInformationUpdate information)
        {
            if (_wlanAdapterId != null && _wlanAdapterId == information.Id)
            {
                _softAccessPoint.Stop();
                _wlanAdapterId = null;

                _onboardingProducer.Stop();
                _iconProducer.Stop();
            }
        }

        private async void ConnectToNetwork(WiFiAdapter adapter, WiFiAvailableNetwork network)
        {
            lock (_stateLock)
            {
                _state = OnboardingState.ConfiguredValidating;
            }

            WiFiConnectionResult connectionResult;
            if (network.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Open80211)
            {
                connectionResult = await adapter.ConnectAsync(network, WiFiReconnectionKind.Automatic);
            }
            else
            {
                connectionResult = await adapter.ConnectAsync(network, WiFiReconnectionKind.Automatic, new PasswordCredential { Password = _personalApPassword });
            }

            lock (_stateLock)
            {
                if (connectionResult.ConnectionStatus == WiFiConnectionStatus.Success)
                {
                    _error = OnboardingError.Validated;
                    _errorMessage = null;
                    _state = OnboardingState.ConfiguredValidated;
                }
                else
                {
                    _state = OnboardingState.ConfiguredError;
                    _errorMessage = connectionResult.ConnectionStatus.ToString();

                    switch (connectionResult.ConnectionStatus)
                    {
                        case WiFiConnectionStatus.AccessRevoked:
                        case WiFiConnectionStatus.InvalidCredential:
                            {
                                _error = OnboardingError.Unauthorized;
                                break;
                            }
                        case WiFiConnectionStatus.UnsupportedAuthenticationProtocol:
                            {
                                _error = OnboardingError.UnsupportedProtocol;
                                break;
                            }
                        case WiFiConnectionStatus.NetworkNotAvailable:
                        case WiFiConnectionStatus.Timeout:
                        case WiFiConnectionStatus.UnspecifiedFailure:
                        default:
                            {
                                _error = OnboardingError.ErrorMessage;
                                break;
                            }
                    }
                }
            }
        }

        public IAsyncOperation<OnboardingConfigureWifiResult> ConfigureWifiAsync(AllJoynMessageInfo info, string interfaceMemberSSID, string interfaceMemberPassphrase, short interfaceMemberAuthType)
        {
            return Task.Run(() =>
            {
                // make sure a valid value is provided for auth type
                if (!Enum.IsDefined(typeof(AuthType), interfaceMemberAuthType))
                {
                    return OnboardingConfigureWifiResult.CreateFailureResult(AllJoynStatus.InvalidArgument3);
                }

                // May want to switch this to concurrent mode if possible:
                // "Concurrent step used to validate the personal AP connection. In this case, the Onboarder application must wait for the
                // ConnectionResult signal to arrive over the AllJoyn session established over the SoftAP link."
                _personalApSsid = interfaceMemberSSID;
                _personalApPassword = interfaceMemberPassphrase;
                _authType = (AuthType)interfaceMemberAuthType;

                lock (_stateLock)
                {
                    _state = OnboardingState.ConfiguredNotValidated;
                }

                // Status "1" indicates the SoftAP will remain available until the Connect method is invoked
                return OnboardingConfigureWifiResult.CreateSuccessResult(1);

            }).AsAsyncOperation();
        }

        public IAsyncOperation<OnboardingConnectResult> ConnectAsync(AllJoynMessageInfo info)
        {
            return Task.Run(async () =>
            {
                // Find the network with the specified Ssid
                var adapter = await WiFiAdapter.FromIdAsync(_wlanAdapterId);
                foreach (var network in adapter.NetworkReport.AvailableNetworks)
                {
                    if (network.Ssid == _personalApSsid)
                    {
                        _softAccessPoint?.Stop();
                        this.ConnectToNetwork(adapter, network);
                    }
                }
                return OnboardingConnectResult.CreateSuccessResult();

            }).AsAsyncOperation();
        }

        public IAsyncOperation<OnboardingOffboardResult> OffboardAsync(AllJoynMessageInfo info)
        {
            return Task.Run(() =>
            {
                lock (_stateLock)
                {
                    if (_state == OnboardingState.ConfiguredValidated || _state == OnboardingState.ConfiguredValidating)
                    {
                        var adapter = WiFiAdapter.FromIdAsync(_wlanAdapterId).AsTask().Result;
                        adapter.Disconnect();
                    }
                    _state = OnboardingState.NotConfigured;
                    _personalApSsid = null;
                    _personalApPassword = null;
                    _authType = AuthType.Any;

                    _softAccessPoint?.Start();
                }
                return OnboardingOffboardResult.CreateSuccessResult();

            }).AsAsyncOperation();
        }

        public IAsyncOperation<OnboardingGetScanInfoResult> GetScanInfoAsync(AllJoynMessageInfo info)
        {
            return Task.Run(async () =>
            {
                var adapter = await WiFiAdapter.FromIdAsync(_wlanAdapterId);
                await adapter.ScanAsync();

                var availableNetworks = new List<OnboardingScanListItem>();
                foreach (var network in adapter.NetworkReport.AvailableNetworks)
                {
                    var listItem = new OnboardingScanListItem { Value1 = network.Ssid };

                    switch (network.SecuritySettings.NetworkAuthenticationType)
                    {
                        case NetworkAuthenticationType.Open80211:
                            {
                                listItem.Value2 = (short)AuthType.Open;
                                break;
                            }
                        case NetworkAuthenticationType.Wpa:
                            {
                                listItem.Value2 = (short)AuthType.WPA_AUTO;
                                break;
                            }
                        default:
                            {
                                listItem.Value2 = (short)AuthType.Any;
                                break;
                            }
                    }

                    if (availableNetworks.Find(x => x.Value1 == listItem.Value1 && x.Value2 == listItem.Value2) == null)
                    {
                        availableNetworks.Add(listItem);
                    }
                }

                return OnboardingGetScanInfoResult.CreateSuccessResult(0, availableNetworks);

            }).AsAsyncOperation();
        }

        public IAsyncOperation<OnboardingGetVersionResult> GetVersionAsync(AllJoynMessageInfo info)
        {
            return Task.FromResult(OnboardingGetVersionResult.CreateSuccessResult(_onboardingInterfaceVersion)).AsAsyncOperation();
        }

        public IAsyncOperation<OnboardingGetStateResult> GetStateAsync(AllJoynMessageInfo info)
        {
            return Task.Run(() =>
            {
                lock (_stateLock)
                {
                    return OnboardingGetStateResult.CreateSuccessResult((short)_state);
                }
            }).AsAsyncOperation();
        }

        public IAsyncOperation<OnboardingGetLastErrorResult> GetLastErrorAsync(AllJoynMessageInfo info)
        {
            return Task.Run(() =>
            {
                lock (_stateLock)
                {
                    var error = new OnboardingLastError { Value1 = (short)_error, Value2 = _errorMessage == null ? "" : _errorMessage };
                    return OnboardingGetLastErrorResult.CreateSuccessResult(error);
                }
            }).AsAsyncOperation();
        }

        public IAsyncOperation<IconGetContentResult> GetContentAsync(AllJoynMessageInfo info)
        {
            return Task.Run(async () =>
            {
                var fileProps = await _iconFile.GetBasicPropertiesAsync();
                var dataReader = new DataReader(await _iconFile.OpenSequentialReadAsync());
                await dataReader.LoadAsync((uint)fileProps.Size);

                var buffer = new byte[fileProps.Size];
                dataReader.ReadBytes(buffer);

                return IconGetContentResult.CreateSuccessResult(buffer);

            }).AsAsyncOperation();
        }

        public IAsyncOperation<IconGetMimeTypeResult> GetMimeTypeAsync(AllJoynMessageInfo info)
        {
            //return Task.FromResult(IconGetMimeTypeResult.CreateSuccessResult(_iconFile?.ContentType)).AsAsyncOperation();
            return Task.FromResult(IconGetMimeTypeResult.CreateSuccessResult("image/png")).AsAsyncOperation();
        }

        public IAsyncOperation<IconGetSizeResult> GetSizeAsync(AllJoynMessageInfo info)
        {
            return Task.Run(async () =>
            {
                var properties = await _iconFile.GetBasicPropertiesAsync();
                return IconGetSizeResult.CreateSuccessResult((uint)properties.Size);

            }).AsAsyncOperation();
        }

        public IAsyncOperation<IconGetUrlResult> GetUrlAsync(AllJoynMessageInfo info)
        {
            return Task.FromResult(IconGetUrlResult.CreateSuccessResult("")).AsAsyncOperation();
        }

        IAsyncOperation<IconGetVersionResult> IIconService.GetVersionAsync(AllJoynMessageInfo info)
        {
            return Task.FromResult(IconGetVersionResult.CreateSuccessResult(_iconInterfaceVersion)).AsAsyncOperation();
        }

        // As defined here https://allseenalliance.org/developers/learn/base-services/onboarding/interface-14-02
        private enum AuthType : short
        {
            WPA2_AUTO = -3,
            WPA_AUTO = -2,
            Any = -1,
            Open = 0,
            WEP = 1,
            WPA_TKIP = 2,
            WPA_CCMP = 3,
            WPA2_TKIP = 4,
            WPA5_CCMP = 5,
            WPS = 6
        };

        private enum OnboardingState
        {
            NotConfigured,
            ConfiguredNotValidated,
            ConfiguredValidating,
            ConfiguredValidated,
            ConfiguredError,
            ConfiguredRetry
        }

        private enum OnboardingError
        {
            Validated,
            Unreachable,
            UnsupportedProtocol,
            Unauthorized,
            ErrorMessage
        }
    }
}
