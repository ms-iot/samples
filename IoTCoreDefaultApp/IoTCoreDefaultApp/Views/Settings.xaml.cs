// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.ObjectModel;
using Windows.Devices.WiFi;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Security.Credentials;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Settings : Page
    {
        private LanguageManager languageManager;
        private UIElement visibleContent;
        private NetworkPresenter networkPresenter;
        private bool Automatic = true;
        private string CurrentPassword = string.Empty;
        // Device watcher
        private DeviceWatcher deviceWatcher = null;
        private TypedEventHandler<DeviceWatcher, DeviceInformation> handlerAdded = null;
        private TypedEventHandler<DeviceWatcher, DeviceInformationUpdate> handlerUpdated = null;
        private TypedEventHandler<DeviceWatcher, DeviceInformationUpdate> handlerRemoved = null;
        private TypedEventHandler<DeviceWatcher, Object> handlerEnumCompleted = null;
        private TypedEventHandler<DeviceWatcher, Object> handlerStopped = null;
        // Pairing controls and notifications
        private enum MessageType { YesNoMessage, OKMessage, InformationalMessage };
        Windows.UI.Xaml.Controls.Primitives.FlyoutBase savedPairButtonFlyout;
        Windows.Devices.Enumeration.DevicePairingRequestedEventArgs pairingRequestedHandlerArgs;
        Windows.Foundation.Deferral deferral;
        Windows.Devices.Bluetooth.Rfcomm.RfcommServiceProvider provider = null; // To be used for inbound
        private string bluetoothConfirmOnlyFormatString;
        private string bluetoothDisplayPinFormatString;
        private string bluetoothConfirmPinMatchFormatString;

        public ObservableCollection<BluetoothDeviceInformationDisplay> bluetoothDeviceCollection
        {
            get;
            private set;
        }

        public Settings()
        {
            this.InitializeComponent();

            visibleContent = BasicPreferencesGridView;

            this.NavigationCacheMode = Windows.UI.Xaml.Navigation.NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();

            this.Loaded += (sender, e) =>
            {
                SetupLanguages();
            };
        }

        private void SetupLanguages()
        {
            languageManager = LanguageManager.GetInstance();

            LanguageListBox.ItemsSource = languageManager.LanguageDisplayNames;
            LanguageListBox.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();
        }

        private void SetupNetwork()
        {
            SetupEthernet();
            SetupWifi();
        }
        private void SetupBluetooth()
        {
            RegisterForInboundPairingRequests();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            bluetoothDeviceCollection = new ObservableCollection<BluetoothDeviceInformationDisplay>();

            // Save the flyout so it doesn't pop up unless we want it
            savedPairButtonFlyout = pairButton.Flyout;
            pairButton.Flyout = null;

            // Resource loading has to happen on the UI thread
            bluetoothConfirmOnlyFormatString = GetResourceString("BluetoothConfirmOnlyFormat");
            bluetoothDisplayPinFormatString = GetResourceString("BluetoothDisplayPinFormat");
            bluetoothConfirmPinMatchFormatString = GetResourceString("BluetoothConfirmPinMatchFormat");
            // Handle inbound pairing requests
            App.InboundPairingRequested += App_InboundPairingRequested;
        }


        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            StopWatcher();
        }

        private async void App_InboundPairingRequested(object sender, InboundPairingEventArgs inboundArgs)
        {
            await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                // Make sure the Bluetooth grid is showing
                SwitchToSelectedSettings("BluetoothListViewItem");

                // Clear any previous devices
                bluetoothDeviceCollection.Clear();
                // Add latest
                BluetoothDeviceInformationDisplay diDisplay = new BluetoothDeviceInformationDisplay(inboundArgs.DeviceInfo);
                bluetoothDeviceCollection.Add(diDisplay);

                // Restore the ceremonies we registered with
                var localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
                Object supportedPairingKinds = localSettings.Values["supportedPairingKinds"];
                int iSelectedCeremonies = (int)DevicePairingKinds.ConfirmOnly;
                if (supportedPairingKinds != null)
                {
                    iSelectedCeremonies = (int)supportedPairingKinds;
                }
                SetSelectedCeremonies(iSelectedCeremonies);
            });
        }

        private void StartWatcherButton_Click(object sender, RoutedEventArgs e)
        {
            StartWatcher();
            string confirmationMessage = GetResourceString("BluetoothStartedWatching");
            DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);
        }

        private void StopWatcherButton_Click(object sender, RoutedEventArgs e)
        {
            StopWatcher();
            string confirmationMessage = GetResourceString("BluetoothStoppedWatching");
            DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);

        }

        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void LanguageListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listBox = sender as ListBox;
            if (listBox.SelectedItem == null)
            {
                return;
            }

            languageManager.UpdateLanguage(listBox.SelectedItem as string);
        }


        private void SetupEthernet()
        {
            var ethernetProfile = NetworkPresenter.GetDirectConnectionName();

            if (ethernetProfile == null)
            {
                NoneFoundText.Visibility = Visibility.Visible;
                DirectConnectionStackPanel.Visibility = Visibility.Collapsed;
            }
            else
            {
                NoneFoundText.Visibility = Visibility.Collapsed;
                DirectConnectionStackPanel.Visibility = Visibility.Visible;
            }
        }

        private async void SetupWifi()
        {
            networkPresenter = new NetworkPresenter();

            if (await NetworkPresenter.WifiIsAvailable())
            {
                var networks = await networkPresenter.GetAvailableNetworks();

                if (networks.Count > 0)
                {
                    WifiListView.ItemsSource = networks;
                    var connectedNetwork = networkPresenter.GetCurrentWifiNetwork();

                    if (connectedNetwork != null)
                    {
                        SwitchToItemState(connectedNetwork, WifiConnectedState, true);
                    }

                    NoWifiFoundText.Visibility = Visibility.Collapsed;
                    WifiListView.Visibility = Visibility.Visible;

                    return;
                }
            }

            NoWifiFoundText.Visibility = Visibility.Visible;
            WifiListView.Visibility = Visibility.Collapsed;
        }

        private void WifiListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listView = sender as ListView;

            foreach (var item in e.RemovedItems)
            {
                SwitchToItemState(item, WifiInitialState, true);
            }

            foreach (var item in e.AddedItems)
            {
                Automatic = true;
                SwitchToItemState(item, WifiConnectState, true);
            }
        }

        private void ConnectButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var network = button.DataContext as WiFiAvailableNetwork;
            if (NetworkPresenter.IsNetworkOpen(network))
            {
                ConnectToWifi(network, null, Window.Current.Dispatcher);
            }
            else
            {
                SwitchToItemState(network, WifiPasswordState, false);
            }
        }

        private async void ConnectToWifi(WiFiAvailableNetwork network, PasswordCredential credential, CoreDispatcher dispatcher)
        {
            var didConnect = credential == null ?
                networkPresenter.ConnectToNetwork(network, Automatic) :
                networkPresenter.ConnectToNetworkWithPassword(network, Automatic, credential);

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SwitchToItemState(network, WifiConnectingState, false);
            });

            DataTemplate nextState = (await didConnect) ? WifiConnectedState : WifiInitialState;

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                var item = SwitchToItemState(network, nextState, false);
                item.IsSelected = false; 
            });
        }

        private void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            PasswordCredential credential;

            if (string.IsNullOrEmpty(CurrentPassword))
            {
                credential = null;
            }
            else
            {
                credential = new PasswordCredential()
                {
                    Password = CurrentPassword
                };
            }

            var network = button.DataContext as WiFiAvailableNetwork;
            ConnectToWifi(network, credential, Window.Current.Dispatcher);
        }

        private void CancelButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var item = SwitchToItemState(button.DataContext, WifiInitialState, false);
            item.IsSelected = false;
        }

        private ListViewItem SwitchToItemState(object dataContext, DataTemplate template, bool forceUpdate)
        {
            if (forceUpdate)
            {
                WifiListView.UpdateLayout();
            }
            var item = WifiListView.ContainerFromItem(dataContext) as ListViewItem;
            if (item != null)
            {
                item.ContentTemplate = template;
            }

            return item;
        }

        private void ConnectAutomaticallyCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;

            Automatic = checkbox.IsChecked ?? false;
        }

        private void WifiPasswordBox_PasswordChanged(object sender, RoutedEventArgs e)
        {
            var passwordBox = sender as PasswordBox;
            CurrentPassword = passwordBox.Password;
        }

        private void SettingsChoice_ItemClick(object sender, ItemClickEventArgs e)
        {
            var item = e.ClickedItem as FrameworkElement;
            if (item == null)
            {
                return;
            }

            // Language, Network, or Bluetooth settings etc.
            SwitchToSelectedSettings(item.Name);
        }

        private void SwitchToSelectedSettings(string itemName)
        {
            switch (itemName)
            {
                case "PreferencesListViewItem":
                    if (BasicPreferencesGridView.Visibility == Visibility.Collapsed)
                    {
                        visibleContent.Visibility = Visibility.Collapsed;
                        BasicPreferencesGridView.Visibility = Visibility.Visible;
                        visibleContent = BasicPreferencesGridView;
                    }
                    break;
                case "NetworkListViewItem":
                    if (NetworkGrid.Visibility == Visibility.Collapsed)
                    {
                        SetupNetwork();
                        visibleContent.Visibility = Visibility.Collapsed;
                        NetworkGrid.Visibility = Visibility.Visible;
                        visibleContent = NetworkGrid;
                    }
                    break;
                case "BluetoothListViewItem":
                    if (BluetoothGrid.Visibility == Visibility.Collapsed)
                    {
                        SetupBluetooth();
                        visibleContent.Visibility = Visibility.Collapsed;
                        BluetoothGrid.Visibility = Visibility.Visible;
                        visibleContent = BluetoothGrid;
                    }
                    break;
            }
        }

        private void RefreshButton_Click(object sender, RoutedEventArgs e)
        {
            SetupWifi();
        }

        /// <summary>
        /// Start the Device Watcher and set callbacks to handle devices appearing and disappearing
        /// </summary>
        private void StartWatcher()
        {
            //ProtocolSelectorInfo protocolSelectorInfo;
            string aqsFilter = @"System.Devices.Aep.ProtocolId:=""{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}"" OR System.Devices.Aep.ProtocolId:=""{bb7bb05e-5972-42b5-94fc-76eaa7084d49}""";  //Bluetooth + BluetoothLE

            startWatcherButton.IsEnabled = false;
            bluetoothDeviceCollection.Clear();

            // Request the IsPaired property so we can display the paired status in the UI
            string[] requestedProperties = { "System.Devices.Aep.IsPaired" };

            //// Get the device selector chosen by the UI, then 'AND' it with the 'CanPair' property
            //protocolSelectorInfo = (ProtocolSelectorInfo)selectorComboBox.SelectedItem;
            //aqsFilter = protocolSelectorInfo.Selector + " AND System.Devices.Aep.CanPair:=System.StructuredQueryType.Boolean#True";

            deviceWatcher = DeviceInformation.CreateWatcher(
                aqsFilter,
                requestedProperties,
                DeviceInformationKind.AssociationEndpoint
                );

            // Hook up handlers for the watcher events before starting the watcher

            handlerAdded = new TypedEventHandler<DeviceWatcher, DeviceInformation>(async (watcher, deviceInfo) =>
            {
                // Since we have the collection databound to a UI element, we need to update the collection on the UI thread.
                await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    bluetoothDeviceCollection.Add(new BluetoothDeviceInformationDisplay(deviceInfo));
                });
            });
            deviceWatcher.Added += handlerAdded;

            handlerUpdated = new TypedEventHandler<DeviceWatcher, DeviceInformationUpdate>(async (watcher, deviceInfoUpdate) =>
            {
                // Since we have the collection databound to a UI element, we need to update the collection on the UI thread.
                await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    // Find the corresponding updated DeviceInformation in the collection and pass the update object
                    // to the Update method of the existing DeviceInformation. This automatically updates the object
                    // for us.
                    foreach (BluetoothDeviceInformationDisplay deviceInfoDisp in bluetoothDeviceCollection)
                    {
                        if (deviceInfoDisp.Id == deviceInfoUpdate.Id)
                        {
                            deviceInfoDisp.Update(deviceInfoUpdate);
                            break;
                        }
                    }
                });
            });
            deviceWatcher.Updated += handlerUpdated;

            handlerRemoved = new TypedEventHandler<DeviceWatcher, DeviceInformationUpdate>(async (watcher, deviceInfoUpdate) =>
            {
                // Since we have the collection databound to a UI element, we need to update the collection on the UI thread.
                await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    // Find the corresponding DeviceInformation in the collection and remove it
                    foreach (BluetoothDeviceInformationDisplay deviceInfoDisp in bluetoothDeviceCollection)
                    {
                        if (deviceInfoDisp.Id == deviceInfoUpdate.Id)
                        {
                            bluetoothDeviceCollection.Remove(deviceInfoDisp);
                            break;
                        }
                    }
                });
            });
            deviceWatcher.Removed += handlerRemoved;

            handlerEnumCompleted = new TypedEventHandler<DeviceWatcher, Object>(async (watcher, obj) =>
            {
                await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    // Finished enumerating
                });
            });
            deviceWatcher.EnumerationCompleted += handlerEnumCompleted;

            handlerStopped = new TypedEventHandler<DeviceWatcher, Object>(async (watcher, obj) =>
            {
                await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    // Device watcher stopped
                });
            });
            deviceWatcher.Stopped += handlerStopped;

            // Start the Device Watcher
            deviceWatcher.Start();
            stopWatcherButton.IsEnabled = true;
        }

        /// <summary>
        /// Stop the Device Watcher
        /// </summary>
        private void StopWatcher()
        {
            stopWatcherButton.IsEnabled = false;

            if (null != deviceWatcher)
            {
                // First unhook all event handlers except the stopped handler. This ensures our
                // event handlers don't get called after stop, as stop won't block for any "in flight" 
                // event handler calls.  We leave the stopped handler as it's guaranteed to only be called
                // once and we'll use it to know when the query is completely stopped. 
                deviceWatcher.Added -= handlerAdded;
                deviceWatcher.Updated -= handlerUpdated;
                deviceWatcher.Removed -= handlerRemoved;
                deviceWatcher.EnumerationCompleted -= handlerEnumCompleted;

                if (DeviceWatcherStatus.Started == deviceWatcher.Status ||
                    DeviceWatcherStatus.EnumerationCompleted == deviceWatcher.Status)
                {
                    deviceWatcher.Stop();
                }
            }

            startWatcherButton.IsEnabled = true;
        }

        /// <summary>
        /// This is really just a replacement for MessageDialog, which you can't use on Athens
        /// </summary>
        /// <param name="confirmationMessage"></param>
        /// <param name="messageType"></param>
        private async void DisplayMessagePanel(string confirmationMessage, MessageType messageType)
        {
            // Use UI thread
            await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {

                confirmationText.Text = confirmationMessage;
                if (messageType == MessageType.OKMessage)
                {
                    yesButton.Content = "OK";
                    yesButton.Visibility = Visibility.Visible;
                    noButton.Visibility = Visibility.Collapsed;
                }
                else if (messageType == MessageType.InformationalMessage)
                {
                    // Just make the buttons invisible
                    yesButton.Visibility = Visibility.Collapsed;
                    noButton.Visibility = Visibility.Collapsed;
                }
                else
                {
                    yesButton.Content = "Yes";
                    yesButton.Visibility = Visibility.Visible;
                    noButton.Visibility = Visibility.Visible;
                }
            });
        }

        /// <summary>
        /// The Yes or OK button on the DisplayConfirmationPanelAndComplete - accepts the pairing, completes the deferral and clears the message panel
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void YesButton_Click(object sender, RoutedEventArgs e)
        {
            // Accept the pairing
            AcceptPairing();
            // Clear the confirmation message
            ClearConfirmationPanel();
        }

        private void CompleteDeferral()
        {
            // Complete the deferral
            if (deferral != null)
            {
                deferral.Complete();
                deferral = null;
            }
        }

        /// <summary>
        /// Accept the pairing and complete the deferral
        /// </summary>
        private void AcceptPairing()
        {
            if (pairingRequestedHandlerArgs != null)
            {
                pairingRequestedHandlerArgs.Accept();
                pairingRequestedHandlerArgs = null;
            }
            // Complete deferral
            CompleteDeferral();
        }

        private void ClearConfirmationPanel()
        {
            confirmationText.Text = "";
            yesButton.Visibility = Visibility.Collapsed;
            noButton.Visibility = Visibility.Collapsed;
        }

        /// <summary>
        /// The No button on the DisplayConfirmationPanelAndComplete - completes the deferral and clears the message panel
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void NoButton_Click(object sender, RoutedEventArgs e)
        {
            //Complete the deferral
            CompleteDeferral();
            // Clear the confirmation message
            ClearConfirmationPanel();
        }

        /// <summary>
        /// User wants to use custom pairing with the selected ceremony types and Default protection level
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void PairButton_Click(object sender, RoutedEventArgs e)
        {
            BluetoothDeviceInformationDisplay deviceInfoDisp = bluetoothDeviceListView.SelectedItem as BluetoothDeviceInformationDisplay;
            string formatString = GetResourceString("BluetoothAttemptingToPairFormat");
            string confirmationMessage = string.Format(formatString, deviceInfoDisp.Name, deviceInfoDisp.Id);
            DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);

            pairButton.IsEnabled = false;

            // Get ceremony type and protection level selections
            DevicePairingKinds ceremoniesSelected = GetSelectedCeremonies();
            // Get protection level
            DevicePairingProtectionLevel protectionLevel = DevicePairingProtectionLevel.Default;

            // Specify custom pairing with all ceremony types and protection level EncryptionAndAuthentication
            DeviceInformationCustomPairing customPairing = deviceInfoDisp.DeviceInformation.Pairing.Custom;

            customPairing.PairingRequested += PairingRequestedHandler;
            DevicePairingResult result = await customPairing.PairAsync(ceremoniesSelected, protectionLevel);

            if (result.Status == DevicePairingResultStatus.Paired)
            {
                formatString = GetResourceString("BluetoothPairingSuccessFormat");
                confirmationMessage = string.Format(formatString, deviceInfoDisp.Name, deviceInfoDisp.Id);
            }
            else
            {
                formatString = GetResourceString("BluetoothPairingFailureFormat");
                confirmationMessage = string.Format(formatString, result.Status.ToString(), deviceInfoDisp.Name, deviceInfoDisp.Id);
            }
            // Display the result of the pairing attempt
            DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);

            UpdatePairingButtons();
        }

        /// <summary>
        /// Called when custom pairing is initiated so that we can handle the custom ceremony
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private async void PairingRequestedHandler(
            DeviceInformationCustomPairing sender,
            DevicePairingRequestedEventArgs args)
        {
            // Save the args for use in ProvidePin case
            pairingRequestedHandlerArgs = args;

            // Save the deferral away and complete it where necessary.
            deferral = args.GetDeferral();

            string confirmationMessage;

            switch (args.PairingKind)
            {
                case DevicePairingKinds.ConfirmOnly:
                    // Windows itself will pop the confirmation dialog as part of "consent" if this is running on Desktop or Mobile
                    // If this is an App for Athens where there is no Windows Consent UX, you may want to provide your own confirmation.
                    {
                        confirmationMessage = string.Format(bluetoothConfirmOnlyFormatString, args.DeviceInformation.Name, args.DeviceInformation.Id);
                        DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);
                        // Accept the pairing which also completes the deferral
                        AcceptPairing();
                    }
                    break;

                case DevicePairingKinds.DisplayPin:
                    // We just show the PIN on this side. The ceremony is actually completed when the user enters the PIN
                    // on the target device
                    {
                        confirmationMessage = string.Format(bluetoothDisplayPinFormatString, args.Pin);
                        DisplayMessagePanel(confirmationMessage, MessageType.OKMessage);
                    }
                    break;

                case DevicePairingKinds.ProvidePin:
                    // A PIN may be shown on the target device and the user needs to enter the matching PIN on 
                    // this Windows device.
                    await MainPage.Current.UIThreadDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                    {
                        // Reattach the flyout
                        pairButton.Flyout = savedPairButtonFlyout;
                        PinEntryTextBox.Text = "";
                        pairButton.Flyout.ShowAt(pairButton);
                    });
                    break;

                case DevicePairingKinds.ConfirmPinMatch:
                    // We show the PIN here and the user responds with whether the PIN matches what they see
                    // on the target device. Response comes back and we set it on the PinComparePairingRequestedData
                    // then complete the deferral.
                    {
                        confirmationMessage = string.Format(bluetoothConfirmPinMatchFormatString, args.Pin);
                        DisplayMessagePanel(confirmationMessage, MessageType.YesNoMessage);
                    }
                    break;
            }
        }

        /// <summary>
        /// User wants to unpair from the selected device
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void unpairButton_Click(object sender, RoutedEventArgs e)
        {
            BluetoothDeviceInformationDisplay deviceInfoDisp = bluetoothDeviceListView.SelectedItem as BluetoothDeviceInformationDisplay;
            string formatString;
            string confirmationMessage;
            unpairButton.IsEnabled = false;
            DeviceUnpairingResult unpairingResult = await deviceInfoDisp.DeviceInformation.Pairing.UnpairAsync();

            if (unpairingResult.Status == DeviceUnpairingResultStatus.Unpaired)
            {
                // Device is unpaired
                formatString = GetResourceString("BluetoothUnpairingSuccessFormat");
                confirmationMessage = string.Format(formatString, deviceInfoDisp.Name, deviceInfoDisp.Id);
            }
            else
            {
                formatString = GetResourceString("BluetoothUnpairingFailureFormat");
                confirmationMessage = string.Format(formatString, unpairingResult.Status.ToString(), deviceInfoDisp.Name, deviceInfoDisp.Id);
            }
            // Display the result of the pairing attempt
            DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);

            UpdatePairingButtons();
        }

        /// <summary>
        /// User has entered a PIN and pressed <Retunr> in the PIN entry flyout</Retunr>
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void PinEntryTextBox_KeyDown(object sender, Windows.UI.Xaml.Input.KeyRoutedEventArgs e)
        {

            if (e.Key == Windows.System.VirtualKey.Enter && PinEntryTextBox.Text != "")
            {
                //  Close the flyout and save the PIN the user entered
                string pairingPIN = PinEntryTextBox.Text;
                pairButton.Flyout.Hide();
                // Use the PIN to accept the pairing
                AcceptPairingWithPIN(pairingPIN);
            }
        }

        private void AcceptPairingWithPIN(string PIN)
        {
            if (pairingRequestedHandlerArgs != null)
            {
                pairingRequestedHandlerArgs.Accept(PIN);
                pairingRequestedHandlerArgs = null;
            }
            // Complete the deferral here
            CompleteDeferral();
        }

        /// <summary>
        /// Call when selection changes on the list of discovered Bluetooth devices
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ResultsListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            UpdatePairingButtons();
        }

        /// <summary>
        /// Set the Pair and Unpair buttons' enablement to reflect whether the currently selected devices can be paired with, or unpaired from
        /// </summary>
        private void UpdatePairingButtons()
        {
            BluetoothDeviceInformationDisplay deviceInfoDisp = (BluetoothDeviceInformationDisplay)bluetoothDeviceListView.SelectedItem;

            if (null != deviceInfoDisp &&
                true == deviceInfoDisp.CanPair &&
                false == deviceInfoDisp.IsPaired)
            {
                pairButton.IsEnabled = true;
            }
            else
            {
                pairButton.IsEnabled = false;
            }

            if (null != deviceInfoDisp &&
                true == deviceInfoDisp.IsPaired)
            {
                unpairButton.IsEnabled = true;
            }
            else
            {
                unpairButton.IsEnabled = false;
            }
        }

        /// <summary>
        /// Get the set of acceptable ceremonies from the check boxes
        /// </summary>
        /// <returns></returns>
        private DevicePairingKinds GetSelectedCeremonies()
        {
            DevicePairingKinds ceremonySelection = DevicePairingKinds.None;
            if (confirmOnlyOption.IsChecked.HasValue && (bool)confirmOnlyOption.IsChecked) ceremonySelection |= DevicePairingKinds.ConfirmOnly;
            if (displayPinOption.IsChecked.HasValue && (bool)displayPinOption.IsChecked) ceremonySelection |= DevicePairingKinds.DisplayPin;
            if (providePinOption.IsChecked.HasValue && (bool)providePinOption.IsChecked) ceremonySelection |= DevicePairingKinds.ProvidePin;
            if (confirmPinMatchOption.IsChecked.HasValue && (bool)confirmPinMatchOption.IsChecked) ceremonySelection |= DevicePairingKinds.ConfirmPinMatch;
            return ceremonySelection;
        }

        /// <summary>
        /// Set the check boxes to refelect the set of acceptable ceremonies
        /// </summary>
        /// <param name="selectedCeremonies"></param>
        private void SetSelectedCeremonies(int selectedCeremonies)
        {
            int i = selectedCeremonies & (int)DevicePairingKinds.ConfirmOnly;
            confirmOnlyOption.IsChecked = (i != 0);
            i = selectedCeremonies & (int)DevicePairingKinds.DisplayPin;
            displayPinOption.IsChecked = (i != 0);
            i = selectedCeremonies & (int)DevicePairingKinds.ProvidePin;
            providePinOption.IsChecked = (i != 0);
            i = selectedCeremonies & (int)DevicePairingKinds.ConfirmPinMatch;
            confirmPinMatchOption.IsChecked = (i != 0);
        }

        private void RegisterForInboundPairingRequests()
        {
            // Make the system discoverable for Bluetooth
            MakeDiscoverable();

            string formatString;
            string confirmationMessage;

            // Get state of ceremony checkboxes
            DevicePairingKinds ceremoniesSelected = GetSelectedCeremonies();
            int iCurrentSelectedCeremonies = (int)ceremoniesSelected;

            // Find out if we changed the ceremonies we orginally registered with - if we have registered before these will be saved
            var localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
            Object supportedPairingKinds = localSettings.Values["supportedPairingKinds"];
            int iSavedSelectedCeremonies = -1; // Deliberate impossible value
            if (supportedPairingKinds != null)
            {
                iSavedSelectedCeremonies = (int)supportedPairingKinds;
            }

            if (iCurrentSelectedCeremonies != iSavedSelectedCeremonies)
            {
                if (!DeviceInformationPairing.TryRegisterForAllInboundPairingRequests(ceremoniesSelected))
                {
                    confirmationMessage = GetResourceString("BluetoothInboundRegistrationFailed");
                }
                else
                {
                    // Save off the ceremonies we registered with
                    localSettings.Values["supportedPairingKinds"] = iCurrentSelectedCeremonies;
                    formatString = GetResourceString("BluetoothInboundRegistrationSucceededFormat");
                    confirmationMessage = string.Format(formatString, ceremoniesSelected.ToString());
                }
            }
            else
            {
                formatString = GetResourceString("BluetoothInboundRegistrationAlreadyDoneFormat");
                confirmationMessage = string.Format(formatString, ceremoniesSelected.ToString());
            }

            DisplayMessagePanel(confirmationMessage, MessageType.InformationalMessage);
        }

        async void MakeDiscoverable()
        {
            // Make the system discoverable. Don'd repeatedly do this or the StartAdvertising will throw "cannot create a file when that file already exists"
            if (!App.IsBluetoothDiscoverable)
            {
                Guid BluetoothServiceUuid = new Guid("17890000-0068-0069-1532-1992D79BE4D8");
                provider = await RfcommServiceProvider.CreateAsync(RfcommServiceId.FromUuid(BluetoothServiceUuid));
                Windows.Networking.Sockets.StreamSocketListener listener = new Windows.Networking.Sockets.StreamSocketListener();
                listener.ConnectionReceived += OnConnectionReceived;
                await listener.BindServiceNameAsync(provider.ServiceId.AsString(), Windows.Networking.Sockets.SocketProtectionLevel.PlainSocket);
                //     SocketProtectionLevel.BluetoothEncryptionAllowNullAuthentication);
                // Don't bother setting SPD attributes
                provider.StartAdvertising(listener, true);
                App.IsBluetoothDiscoverable = true;
            }
        }

        /// <summary>
        /// We have to have a callback handler to handle "ConnectionReceived" but we don't do anything because
        /// the StartAdvertising is just a way to turn on Bluetooth discoverability
        /// </summary>
        /// <param name="listener"></param>
        /// <param name="args"></param>
        void OnConnectionReceived(Windows.Networking.Sockets.StreamSocketListener listener,
                                   Windows.Networking.Sockets.StreamSocketListenerConnectionReceivedEventArgs args)
        {
        }

        /// <summary>
        /// Return the named resource string
        /// </summary>
        /// <param name="resourceName"></param>
        /// <returns></returns>
        public string GetResourceString(string resourceName)
        {
            string theResourceString = "##Failed to get resource string##";
            var resourceLoader = Windows.ApplicationModel.Resources.ResourceLoader.GetForCurrentView();
            theResourceString = resourceLoader.GetString(resourceName);
            return theResourceString;
        }

    }
}
