using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Devices.Enumeration;
using Windows.Devices.Gpio;
using Windows.ApplicationModel.AppService;
using Windows.UI.Core;

namespace BlinkyWebService
{
    public sealed partial class MainPage : Page
    {
        AppServiceConnection _appServiceConnection;

        public MainPage()
        {
            InitializeComponent();
            InitGPIO();
            InitAppSvc();
        }

        private async void InitGPIO()
        {
            try
            {
                var deviceId = GpioController.GetDeviceSelector("GPIO_S5");
                var deviceInfos = await DeviceInformation.FindAllAsync(deviceId, null);
                var controller = await GpioController.FromIdAsync(deviceInfos[0].Id);
                GpioPinInfo pinInfo;
                controller.Pins.TryGetValue(0, out pinInfo);
                pinInfo.TryOpenOutput(GpioPinValue.High, GpioSharingMode.Exclusive, out outPin);
                GpioStatus.Text = "GPIO pin initialized correctly.";
            }
            catch (Exception)
            {
                GpioStatus.Text = "There were problems initializing the GPIO pin.";
            }
        }

        private async void InitAppSvc()
        {
            // Initialize the AppServiceConnection
            _appServiceConnection = new AppServiceConnection();
            _appServiceConnection.PackageFamilyName = "WebServer_hz258y3tkez3a";
            _appServiceConnection.AppServiceName = "App2AppComService";

            // Send a initialize request 
            var res = await _appServiceConnection.OpenAsync();
            if (res == AppServiceConnectionStatus.Success)
            {
                var message = new ValueSet();
                message.Add("Command", "Initialize");
                var response = await _appServiceConnection.SendMessageAsync(message);
                if (response.Status != AppServiceResponseStatus.Success)
                {
                    throw new Exception("Failed to send message");
                }
                _appServiceConnection.RequestReceived += OnMessageReceived;
            }
        }

        private async void OnMessageReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var message = args.Request.Message;
            string newState = message["State"] as string;
            switch (newState)
            {
                case "On":
                    {
                        await Dispatcher.RunAsync(
                              CoreDispatcherPriority.High,
                             () =>
                             {
                                 TurnOnLED();
                             }); 
                        break;
                    }
                case "Off":
                    {
                        await Dispatcher.RunAsync(
                        CoreDispatcherPriority.High,
                        () =>
                        {
                            TurnOffLED();
                         });
                        break;
                    }
                case "Unspecified":
                default:
                    {
                        // Do nothing 
                        break;
                    }
            }
        }

        private void FlipLED()
        {
            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                if (outPin != null)
                {
                    // to turn on the LED, we need to push the pin 'low'
                    outPin.Value = GpioPinValue.Low;
                }
                LED.Fill = redBrush;
                StateText.Text = "On";
            }
            else
            {
                LEDStatus = 0;
                if (outPin != null)
                {
                    outPin.Value = GpioPinValue.High;
                }
                LED.Fill = grayBrush;
                StateText.Text = "Off"; 
            }
        }

        private void TurnOffLED()
        {
            if (LEDStatus == 1)
            {
                FlipLED();
            }
        }
        private void TurnOnLED()
        {
            if (LEDStatus == 0)
            {
                FlipLED();
            }
        }

        private int LEDStatus = 0;
        private GpioOutputPin outPin;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
    }
}
