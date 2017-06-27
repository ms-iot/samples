using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Gpio;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace GpioOneWire
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private const int INDEX_INPUT_PIN = 4;
        private const int INDEX_OUTPUT_PIN = 5;

        private const int INTERVAL_DATA_READING = 3000;
        private const int TIMEOUT_DATA_READING = 100;

        private readonly DispatcherTimer _timer;
        private readonly GpioChangeReader _changeReader;

        private GpioPin _outputPin;

        private GpioPin _inputPin;
        private GpioPinDriveMode _inputDriveMode;

        public MainPage()
        {
            InitializeComponent();

            try
            {
                var gpioController = GpioController.GetDefault();

                _inputPin = gpioController.OpenPin(INDEX_INPUT_PIN, GpioSharingMode.Exclusive);
                _outputPin = gpioController.OpenPin(INDEX_OUTPUT_PIN, GpioSharingMode.Exclusive);

                // Use InputPullUp if supported, otherwise fall back to Input (floating)
                _inputDriveMode =
                    _inputPin.IsDriveModeSupported(GpioPinDriveMode.InputPullUp) ?
                    GpioPinDriveMode.InputPullUp : GpioPinDriveMode.Input;

                _inputPin.SetDriveMode(_inputDriveMode);

                _outputPin.Write(GpioPinValue.Low);
                _outputPin.SetDriveMode(GpioPinDriveMode.Output);

                _changeReader = new GpioChangeReader(_inputPin, 43)
                {
                    Polarity = GpioChangePolarity.Falling,
                };

                _timer = new DispatcherTimer();
                _timer.Interval = TimeSpan.FromMilliseconds(INTERVAL_DATA_READING);
                _timer.Tick += HandleEvent_DispatcherTimer_Tick;
            }
            catch (Exception ex)
            {
                UpdateStatus(ex.Message);
            }
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            
            _timer?.Start();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            _timer?.Stop();
            _changeReader?.Stop();
        }

        private async Task SendInitialPulseAsync()
        {
            // Bring the DHT data line low for 10ms. The output pin is driving the
            // gate of a transistor, so we bring the pin high to pull the DHT22 signal low.

            _outputPin.Write(GpioPinValue.High);

            await Task.Delay(10);

            _outputPin.Write(GpioPinValue.Low);
        }

        private async Task ReadValuesFromDhtAsync()
        {
            try
            {
                var reading = new DhtSensorReading();

                CancellationTokenSource cancellationSource = new CancellationTokenSource();
                cancellationSource.CancelAfter(TIMEOUT_DATA_READING);

                _changeReader.Clear();
                _changeReader.Start();

                await SendInitialPulseAsync();

                // Wait for 43 falling edges to show up
                await _changeReader.WaitForItemsAsync(43).AsTask(cancellationSource.Token);

                // discard first two falling edges
                _changeReader.GetNextItem();
                _changeReader.GetNextItem();

                // pulse widths greater than 110us are considered 1's
                TimeSpan oneThreshold = new TimeSpan(110 * 10000000 / 1000000);

                TimeSpan current = _changeReader.GetNextItem().RelativeTime;

                for (int i = 0; i < 40; i++)
                {
                    TimeSpan next = _changeReader.GetNextItem().RelativeTime;
                    TimeSpan pulseWidth = next.Duration() - current.Duration();

                    if (pulseWidth.Duration() > oneThreshold.Duration())
                    {
                        reading.Bits[40 - i - 1] = true;
                    }

                    current = next;
                }

                if (reading.IsValid())
                {
                    UpdateValuesOnUI(reading.Temperature(), reading.Humidity());
                    UpdateStatus("OK");
                }
                else
                {
                    throw new Exception("Invalid sensor data received!");
                }
            }
            catch (Exception ex)
            {
                UpdateValuesOnUI(double.NaN, double.NaN);

                if (ex is TaskCanceledException)
                {
                    UpdateStatus("Timeout while reading sensor data!");
                }
                else
                {
                    UpdateStatus(ex.Message);
                }
            }
            finally
            {
                _changeReader.Stop();
            }
        }

        private void UpdateStatus(string statusText)
        {
            txt_Status.Text = statusText;
        }

        private void UpdateValuesOnUI(double temperature, double humidity)
        {
            txt_Temperature.Text = double.IsNaN(temperature) ? "-" : temperature + "°C";
            txt_Humidity.Text = double.IsNaN(humidity) ? "-" : humidity + "%";
        }

        private async void HandleEvent_DispatcherTimer_Tick(object sender, object eventArgs)
        {
            await ReadValuesFromDhtAsync();
        }
    }
}
