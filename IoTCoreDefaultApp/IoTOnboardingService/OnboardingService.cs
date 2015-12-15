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

namespace IoTOnboardingService
{
    public sealed class OnboardingService : IOnboardingService, IIconService
    {
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

        private ResourceLoader _resourceLoader;

        public OnboardingService()
        {
            _resourceLoader = ResourceLoader.GetForCurrentView("IoTOnboardingService/Resources");

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

                if (_softAccessPoint == null)
                {
                    _softAccessPoint = new OnboardingAccessPoint(string.Format(_resourceLoader.GetString("SoftApSsidTemplate"), _onboardingInstanceId), _resourceLoader.GetString("SoftApPassword"));
                }

                if (_busAttachment == null)
                {
                    _busAttachment = new AllJoynBusAttachment();
                    _busAttachment.AboutData.DefaultDescription = string.Format(_resourceLoader.GetString("DefaultDescriptionTemplate"), _onboardingInstanceId);
                    _busAttachment.AboutData.DefaultManufacturer = _resourceLoader.GetString("DefaultManufacturer");
                    _busAttachment.AboutData.ModelNumber = _resourceLoader.GetString("ModelNumber");

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
