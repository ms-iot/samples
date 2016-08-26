// Copyright (c) Microsoft. All rights reserved.

using org.alljoyn.Icon;
using org.alljoyn.Onboarding;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.AllJoyn;
using Windows.Devices.Enumeration;
using Windows.Devices.WiFi;
using Windows.Foundation;
using Windows.Networking.Connectivity;
using Windows.Security.Credentials;
using Windows.Storage;
using Windows.Storage.Search;
using Windows.Storage.Streams;
using Windows.ApplicationModel;
using Windows.Data.Xml.Dom;
using Windows.Security.Cryptography;

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
            public string presharedKey;
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
        private const string NODE_ALLJOYNPSK = "IoTOnboarding/AllJoynOnboarding/PresharedKey";

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
        private StorageFileQueryResult _query;
        private bool _bIgnoreFirstChangeNotification;

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

        private async Task<Tuple<bool, AJOnboardingConfig, SoftAPConfig>> ReadConfig()
        {
            bool retVal = true;

            AJOnboardingConfig ajOnboardCfg = new AJOnboardingConfig();
            SoftAPConfig softAPCfg = new SoftAPConfig();

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
                    softAPCfg.enabled = true;
                }
                else
                {
                    softAPCfg.enabled = false;
                }
                xmlNode = xmlConfig.SelectSingleNode(NODE_SOFTAPSSIDTEMPLATE);
                softAPCfg.ssid = GetXmlNodeValue(xmlNode);
                xmlNode = xmlConfig.SelectSingleNode(NODE_SOFTAPPASSWORD);
                softAPCfg.password = GetXmlNodeValue(xmlNode);

                // AllJoyn Onboarding config
                xmlNode = xmlConfig.SelectSingleNode(NODE_ALLJOYNONBOARDINGENABLE);
                tempString = GetXmlNodeValue(xmlNode);
                if (tempString == "true")
                {
                    ajOnboardCfg.enabled = true;
                }
                else
                {
                    ajOnboardCfg.enabled = false;
                }
                xmlNode = xmlConfig.SelectSingleNode(NODE_DEFAULTDESCRIPTIONTEMPLATE);
                ajOnboardCfg.defaultDescription = GetXmlNodeValue(xmlNode);
                xmlNode = xmlConfig.SelectSingleNode(NODE_DEFAULTMANUFACTURER);
                ajOnboardCfg.defaultManufacturer = GetXmlNodeValue(xmlNode);
                xmlNode = xmlConfig.SelectSingleNode(NODE_MODELNUMBER);
                ajOnboardCfg.modelNumber = GetXmlNodeValue(xmlNode);

                // For backwards compatbility with original config file.  If the AllJoyn PSK is not found, 
                // default to the ECDHE_NULL authentication method
                xmlNode = xmlConfig.SelectSingleNode(NODE_ALLJOYNPSK);
                if (xmlNode == null)
                {
                    ajOnboardCfg.presharedKey = "";
                }
                else
                {
                    ajOnboardCfg.presharedKey = GetXmlNodeValue(xmlNode);
                }
            }
            catch (Exception)
            {
                retVal = false;
            }

            return new Tuple<bool, AJOnboardingConfig, SoftAPConfig>(retVal, ajOnboardCfg, softAPCfg);
        }

        private async Task<bool> MonitorConfigFile()
        {
            bool retVal = true;

            try
            {
                // Watch for all ".xml" files (there is only the config.xml file in this app's data folder)
                // And add a handler for when the folder contents change
                var fileTypeQuery = new List<string>();
                fileTypeQuery.Add(".xml");
                var options = new Windows.Storage.Search.QueryOptions(Windows.Storage.Search.CommonFileQuery.DefaultQuery, fileTypeQuery);
                _query = ApplicationData.Current.LocalFolder.CreateFileQueryWithOptions(options);
                _query.ContentsChanged += FolderContentsChanged;

                // Start Monitoring.  The first time this is called (after starting the query) we want to ignore the notification
                _bIgnoreFirstChangeNotification = true;
                var files = await _query.GetFilesAsync();

            }
            catch (Exception ex)
            {
                retVal = false;
            }

            return retVal;
        }

        private async void FolderContentsChanged(Windows.Storage.Search.IStorageQueryResultBase sender, object args)
        {
            // Ignore the first change notification that gets issued when MonitorConfigFile is called
            if (_bIgnoreFirstChangeNotification)
            {
                _bIgnoreFirstChangeNotification = false;
                return;
            }

            // Read the Config file on change
            var configResult = await ReadConfig();
            if (configResult.Item1 == true)
            {

                // If something changed in the config file then restart IotOnboarding 
                if (!configResult.Item2.Equals(_ajOnboardingConfig) ||
                    !configResult.Item3.Equals(_softAPConfig))
                {
                    sender.ContentsChanged -= FolderContentsChanged;
                    Stop();
                    Start();
                }
            }
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
                var configResult = await ReadConfig();
                if (!configResult.Item1)
                {
                    // for some reason config file doesn't seem to be OK
                    // => reset config and try another time 
                    await ResetConfig();
                    configResult = await ReadConfig();
                    if (!configResult.Item1)
                    {
                        throw new System.IO.InvalidDataException("Invalid configuration");
                    }
                }
                _ajOnboardingConfig = configResult.Item2;
                _softAPConfig = configResult.Item3;

                bool monitorOk = await MonitorConfigFile();
                if (!monitorOk)
                {
                    throw new System.Exception("Unable to monitor configuration file for changes.  Unexpected.");
                }


                // If everything is disabled, then there's nothing to do.
                if ((_softAPConfig.enabled == false) && (_ajOnboardingConfig.enabled == false))
                {
                    return;
                }

                // create softAP 
                if (_softAccessPoint == null &&
                    (_softAPConfig.enabled ||
                     _ajOnboardingConfig.enabled))
                {
                    string prefix = "";
                    string suffix = "";
                    string ssid = "";
                    if (_ajOnboardingConfig.enabled)
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
                    _busAttachment.AboutData.DefaultDescription = _ajOnboardingConfig.defaultDescription + " instance Id " + _onboardingInstanceId;
                    _busAttachment.AboutData.DefaultManufacturer = _ajOnboardingConfig.defaultManufacturer;
                    _busAttachment.AboutData.ModelNumber = _ajOnboardingConfig.modelNumber;
                    _onboardingProducer = new OnboardingProducer(_busAttachment);
                    _onboardingProducer.Service = this;
                    _busAttachment.AuthenticationMechanisms.Clear();

                    if (_ajOnboardingConfig.presharedKey.Length == 0)
                    {
                        _busAttachment.AuthenticationMechanisms.Add(AllJoynAuthenticationMechanism.EcdheNull);
                    }
                    else
                    {
                        _busAttachment.AuthenticationMechanisms.Add(AllJoynAuthenticationMechanism.EcdhePsk);
                    }

                    _busAttachment.CredentialsRequested += CredentialsRequested;
                    _busAttachment.CredentialsVerificationRequested += CredentialsVerificationRequested;
                    _busAttachment.AuthenticationComplete += AuthenticationComplete;

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

        private void AuthenticationComplete(AllJoynBusAttachment sender, AllJoynAuthenticationCompleteEventArgs args)
        {
        }

        private void CredentialsVerificationRequested(AllJoynBusAttachment sender, AllJoynCredentialsVerificationRequestedEventArgs args)
        {
        }

        private void CredentialsRequested(AllJoynBusAttachment sender, AllJoynCredentialsRequestedEventArgs args)
        {
            var def = args.GetDeferral();

            if (args.Credentials.AuthenticationMechanism == AllJoynAuthenticationMechanism.EcdhePsk)
            {
                args.Credentials.PasswordCredential.Password = _ajOnboardingConfig.presharedKey;
            }

            def.Complete();
        }

        public void Stop()
        {
            if (_query != null)
            {
                _query = null;
            }
            if (_onboardingProducer != null)
            {
                _onboardingProducer.Stop();
                _onboardingProducer = null;
            }
            if (_iconProducer != null)
            {
                _iconProducer.Stop();
                _iconProducer = null;
            }
            if (_busAttachment != null)
            {
                _busAttachment = null;
            }
            if (_softAccessPoint != null)
            {
                _softAccessPoint.Stop();
                _softAccessPoint = null;
            }
            if (_deviceWatcher != null)
            {
                _deviceWatcher.Stop();
                _deviceWatcher = null;
            }
            _wlanAdapterId = null;
            _state = OnboardingState.NotConfigured;
        }

        private string GetXmlNodeValue(IXmlNode xmlNode)
        {
            if (null == xmlNode)
            {
                throw new System.ArgumentNullException("GetXmlNodeValue wrong xmlNode");
            }

            var attribute = xmlNode.Attributes.GetNamedItem(ATTRIBUTE_VALUE);
            if (attribute == null)
            {
                throw new System.IO.InvalidDataException("xml node " + xmlNode.LocalName + " has no attribute named " + ATTRIBUTE_VALUE);
            }

            return attribute.InnerText;
        }

        private void HandleAdapterAdded(DeviceWatcher sender, DeviceInformation information)
        {
            if (String.IsNullOrEmpty(_wlanAdapterId))
            {
                _wlanAdapterId = information.Id;

                lock (_stateLock)
                {
                    if (_state != OnboardingState.ConfiguredValidated)
                    {
                        _softAccessPoint.Start();
                    }
                }
                _onboardingProducer?.Start();
                _iconProducer?.Start();
            }
        }

        private void HandleAdapterRemoved(DeviceWatcher sender, DeviceInformationUpdate information)
        {
            if (!String.IsNullOrEmpty(_wlanAdapterId) && _wlanAdapterId == information.Id)
            {
                _softAccessPoint.Stop();
                _wlanAdapterId = null;

                _onboardingProducer?.Stop();
                _iconProducer?.Stop();
            }
        }

        private string ConvertHexToPassPhrase(NetworkAuthenticationType authType, string presharedKey)
        {
            // If this is a WPA/WPA2-PSK type network then convert the HEX STRING back to a passphrase
            // Note that a 64 character WPA/2 network key is expected as a 128 character HEX-ized that will be
            // converted back to a 64 character network key.
            if ((authType == NetworkAuthenticationType.WpaPsk) ||
                (authType == NetworkAuthenticationType.RsnaPsk))
            {
                var tempBuffer = CryptographicBuffer.DecodeFromHexString(presharedKey);
                var hexString = CryptographicBuffer.ConvertBinaryToString(BinaryStringEncoding.Utf8, tempBuffer);
                return hexString;
            }

            // If this is a WEP key then it should arrive here as a 10 or 26 character hex-ized 
            // string which will be passed straight through
            return presharedKey;
        }

        private async Task<WiFiConnectionStatus> ConnectToNetwork(WiFiAdapter adapter, WiFiAvailableNetwork network)
        {
            lock (_stateLock)
            {
                _state = OnboardingState.ConfiguredValidating;
            }

            string resultPassword = "";
            WiFiConnectionResult connectionResult;

            // For all open networks (when no PSK was provided) connect without a password
            // Note, that in test, we have seen some WEP networks identify themselves as Open even though
            // they required a PSK, so use the PSK as a determining factor
            if ((network.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Open80211) &&
                string.IsNullOrEmpty(_personalApPassword))
            {
                connectionResult = await adapter.ConnectAsync(network, WiFiReconnectionKind.Automatic);
            }
            // Otherwise for all WEP/WPA/WPA2 networks convert the PSK back from a hex-ized format back to a passphrase, if necessary,
            // and onboard this device to the requested network.
            else
            {
                PasswordCredential pwd = new PasswordCredential();
                resultPassword = ConvertHexToPassPhrase(network.SecuritySettings.NetworkAuthenticationType, _personalApPassword);
                pwd.Password = resultPassword;
                connectionResult = await adapter.ConnectAsync(network, WiFiReconnectionKind.Automatic, pwd);
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
#if DEBUG           
            var folder = Windows.Storage.ApplicationData.Current.LocalFolder;
            var file = await folder.CreateFileAsync("ConnectionResult.Txt", Windows.Storage.CreationCollisionOption.ReplaceExisting);
            string myout = "ConnectionResult= " + connectionResult.ConnectionStatus.ToString() + "\r\n";
            myout += "Type=" + network.SecuritySettings.NetworkAuthenticationType.ToString() + "\r\n";
            myout += "InputPassword= " + _personalApPassword + "\r\n";
            myout += "ResultPassword= " + resultPassword + "\r\n";
            await Windows.Storage.FileIO.WriteTextAsync(file, myout);
#endif
            return connectionResult.ConnectionStatus;

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
                        var result = await this.ConnectToNetwork(adapter, network);
                        if (result == WiFiConnectionStatus.Success)
                        {
                            _softAccessPoint?.Stop();
                            return OnboardingConnectResult.CreateSuccessResult();
                        }
                        break;
                    }
                }
                return OnboardingConnectResult.CreateFailureResult(1);

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
