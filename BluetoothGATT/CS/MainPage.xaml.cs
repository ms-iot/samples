// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

// Required APIs to use Bluetooth GATT
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;

// Required APIs to use built in GUIDs
using Windows.Devices.Enumeration;

// Required APIs for buffer manipulation & async operations
using Windows.Storage.Streams;
using System.Threading.Tasks;

namespace BluetoothGATT
{
    /// <summary>
    /// Sample app that communicates with Bluetooth device using the GATT profile
    /// </summary>
    public sealed partial class MainPage : Page
    {
        // Arrays for information that needs to be saved
        private byte[] baroCalibrationData;
        private GattDeviceService[] serviceList = new GattDeviceService[7];
        private GattCharacteristic[] activeCharacteristics = new GattCharacteristic[7];

        // IDs for Sensors
        const int NUM_SENSORS = 7;
        const int IR_SENSOR = 0;
        const int ACCELEROMETER = 1;
        const int HUMIDITY = 2;
        const int MAGNETOMETER = 3;
        const int BAROMETRIC_PRESSURE = 4;
        const int GYROSCOPE = 5;
        const int KEYS = 6;

        public MainPage()
        {
            this.InitializeComponent();
        }

        // Setup
        // Saves GATT service object in array
        private async Task<bool> init()
        {
            // Retrieve instances of the GATT services that we will use
            for (int i = 0; i < NUM_SENSORS; i++)
            {
                // Setting Service GUIDs
                // Built in enumerations are found in the GattServiceUuids class like this: GattServiceUuids.GenericAccess
                Guid BLE_GUID;
                if (i < 6)
                    BLE_GUID = new Guid("F000AA" + i + "0-0451-4000-B000-000000000000");
                else
                    BLE_GUID = new Guid("0000FFE0-0000-1000-8000-00805F9B34FB");

                // Retrieving and saving GATT services
                var services = await DeviceInformation.FindAllAsync(GattDeviceService.GetDeviceSelectorFromUuid(BLE_GUID), null);
                if(services != null && services.Count > 0)
                {
                    if (services[0].IsEnabled)
                    {
                        GattDeviceService service = await GattDeviceService.FromIdAsync(services[0].Id);
                        if(service.Device.ConnectionStatus == BluetoothConnectionStatus.Connected)
                        {
                            serviceList[i] = service;
                        }
                        else
                        {
                            return false;
                        }
                             
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            return true;
        }


        // ---------------------------------------------------
        //     Hardware Configuration Helper Functions
        // ---------------------------------------------------

        // Retrieve Barometer Calibration data
        private async void calibrateBarometer()
        {
            GattDeviceService gattService = serviceList[BAROMETRIC_PRESSURE];
            if (gattService != null)
            {
                // Set Barometer configuration to 2, so that calibration data is saved
                var characteristicList = gattService.GetCharacteristics(new Guid("F000AA42-0451-4000-B000-000000000000"));
                if (characteristicList != null)
                {
                    GattCharacteristic characteristic = characteristicList[0];

                    if (characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Write))
                    {
                        var writer = new Windows.Storage.Streams.DataWriter();
                        writer.WriteByte((Byte)0x02);
                        await characteristic.WriteValueAsync(writer.DetachBuffer());
                    }
                }

                // Save Barometer calibration data
                characteristicList = gattService.GetCharacteristics(new Guid("F000AA43-0451-4000-B000-000000000000"));
                if (characteristicList != null)
                {
                    GattReadResult result = await characteristicList[0].ReadValueAsync(BluetoothCacheMode.Uncached);
                    baroCalibrationData = new byte[result.Value.Length];
                    DataReader.FromBuffer(result.Value).ReadBytes(baroCalibrationData);
                }
            }
        }

        // Set sensor update period 
        private async void setSensorPeriod(int sensor, int period)
        {
            GattDeviceService gattService = serviceList[sensor];
            if (sensor != KEYS && gattService != null)
            {
                var characteristicList = gattService.GetCharacteristics(new Guid("F000AA" + sensor + "3-0451-4000-B000-000000000000"));
                if (characteristicList != null)
                {
                    GattCharacteristic characteristic = characteristicList[0];

                    if (characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Write))
                    {
                        var writer = new Windows.Storage.Streams.DataWriter();
                        // Accelerometer period = [Input * 10]ms
                        writer.WriteByte((Byte)(period / 10));
                        await characteristic.WriteValueAsync(writer.DetachBuffer());
                    }
                }
            }
        }

        // Enable and subscribe to specified GATT characteristic
        private async void enableSensor(int sensor)
        {
            GattDeviceService gattService = serviceList[sensor];
            if (gattService != null)
            {
                // Turn on notifications
                IReadOnlyList<GattCharacteristic> characteristicList;
                if (sensor >= 0 && sensor <= 5)
                    characteristicList = gattService.GetCharacteristics(new Guid("F000AA" + sensor + "1-0451-4000-B000-000000000000"));
                else
                    characteristicList = gattService.GetCharacteristics(new Guid("0000FFE1-0000-1000-8000-00805F9B34FB"));

                if (characteristicList != null)
                {
                    GattCharacteristic characteristic = characteristicList[0];
                    if (characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Notify))
                    {
                        switch (sensor)
                        {
                            case (IR_SENSOR):
                                characteristic.ValueChanged += tempChanged;
                                IRTitle.Foreground = new SolidColorBrush(Colors.Green);
                                break;
                            case (ACCELEROMETER):
                                characteristic.ValueChanged += accelChanged;
                                AccelTitle.Foreground = new SolidColorBrush(Colors.Green);
                                setSensorPeriod(ACCELEROMETER, 250);
                                break;
                            case (HUMIDITY):
                                characteristic.ValueChanged += humidChanged;
                                HumidTitle.Foreground = new SolidColorBrush(Colors.Green);
                                break;
                            case (MAGNETOMETER):
                                characteristic.ValueChanged += magnoChanged;
                                MagnoTitle.Foreground = new SolidColorBrush(Colors.Green);
                                break;
                            case (BAROMETRIC_PRESSURE):
                                characteristic.ValueChanged += pressureChanged;
                                BaroTitle.Foreground = new SolidColorBrush(Colors.Green);
                                calibrateBarometer();
                                break;
                            case (GYROSCOPE):
                                characteristic.ValueChanged += gyroChanged;
                                GyroTitle.Foreground = new SolidColorBrush(Colors.Green);
                                break;
                            case (KEYS):
                                characteristic.ValueChanged += keyChanged;
                                KeyTitle.Foreground = new SolidColorBrush(Colors.Green);
                                break;
                            default:
                                break;
                        }

                        // Save a reference to each active characteristic, so that handlers do not get prematurely killed
                        activeCharacteristics[sensor] = characteristic;

                        // Set the notify enable flag
                        await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify);
                    }
                }

                // Turn on sensor
                if (sensor >= 0 && sensor <= 5)
                {
                    characteristicList = gattService.GetCharacteristics(new Guid("F000AA" + sensor + "2-0451-4000-B000-000000000000"));
                    if (characteristicList != null)
                    {
                        GattCharacteristic characteristic = characteristicList[0];
                        if (characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Write))
                        {
                            var writer = new Windows.Storage.Streams.DataWriter();
                            // Special value for Gyroscope to enable all 3 axes
                            if (sensor == GYROSCOPE)
                                writer.WriteByte((Byte)0x07);
                            else
                                writer.WriteByte((Byte)0x01);

                            await characteristic.WriteValueAsync(writer.DetachBuffer());
                        }
                    }
                }
            }
        }

        // Disable notifications to specified GATT characteristic
        private async void disableSensor(int sensor)
        {
            GattDeviceService gattService = serviceList[sensor];
            if (gattService != null)
            {
                // Disable notifications
                IReadOnlyList<GattCharacteristic> characteristicList;
                if (sensor >= 0 && sensor <= 5)
                    characteristicList = gattService.GetCharacteristics(new Guid("F000AA" + sensor + "1-0451-4000-B000-000000000000"));
                else
                    characteristicList = gattService.GetCharacteristics(new Guid("0000FFE1-0000-1000-8000-00805F9B34FB"));

                if (characteristicList != null)
                {
                    GattCharacteristic characteristic = characteristicList[0];
                    if (characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Notify))
                    {
                        GattCommunicationStatus status = await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.None);
                    }
                }
            }

            switch (sensor)
            {
                case (IR_SENSOR):
                    IRTitle.Foreground = new SolidColorBrush(Colors.White);
                    break;
                case (ACCELEROMETER):
                    AccelTitle.Foreground = new SolidColorBrush(Colors.White);
                    break;
                case (HUMIDITY):
                    HumidTitle.Foreground = new SolidColorBrush(Colors.White);
                    break;
                case (MAGNETOMETER):
                    MagnoTitle.Foreground = new SolidColorBrush(Colors.White);
                    break;
                case (BAROMETRIC_PRESSURE):
                    BaroTitle.Foreground = new SolidColorBrush(Colors.White);
                    break;
                case (GYROSCOPE):
                    GyroTitle.Foreground = new SolidColorBrush(Colors.White);
                    break;
                case (KEYS):
                    KeyTitle.Foreground = new SolidColorBrush(Colors.White);
                    KeyROut.Background = new SolidColorBrush(Colors.Red);
                    KeyLOut.Background = new SolidColorBrush(Colors.Red);
                    break;
                default:
                    break;
            }
            activeCharacteristics[sensor] = null;
        }


        // ---------------------------------------------------
        //             Button Click Handlers
        // ---------------------------------------------------

        private async void StartButton_Click(object sender, RoutedEventArgs e)
        {
            UserOut.Text = "Setting up SensorTag";
            bool okay = await init();
            if (okay)
            {
                for (int i = 0; i < NUM_SENSORS; i++)
                {
                    enableSensor(i);
                }
                UserOut.Text = "Sensors on!";
            }
            else
            {
                UserOut.Text = "Something went wrong!";
            }
        }

        private void EnableButton_Click(object sender, RoutedEventArgs e)
        {
            enableSensor(SensorList.SelectedIndex);
        }

        private void DisableButton_Click(object sender, RoutedEventArgs e)
        {
            disableSensor(SensorList.SelectedIndex);
        }

        // ---------------------------------------------------
        //           GATT Notification Handlers
        // ---------------------------------------------------

        // IR temperature change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#IR_Temperature_Sensor
        async void tempChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);
            double AmbTemp = (double)(((UInt16)bArray[3] << 8) + (UInt16)bArray[2]);
            AmbTemp /= 128.0;

            Int16 temp = (Int16)(((UInt16)bArray[1] << 8) + (UInt16)bArray[0]);
            double Vobj2 = (double)temp;
            Vobj2 *= 0.00000015625;
            double Tdie = AmbTemp + 273.15;

            const double S0 = 5.593E-14;            // Calibration factor
            const double a1 = 1.75E-3;
            const double a2 = -1.678E-5;
            const double b0 = -2.94E-5;
            const double b1 = -5.7E-7;
            const double b2 = 4.63E-9;
            const double c2 = 13.4;
            const double Tref = 298.15;

            double S = S0 * (1 + a1 * (Tdie - Tref) + a2 * Math.Pow((Tdie - Tref), 2));
            double Vos = b0 + b1 * (Tdie - Tref) + b2 * Math.Pow((Tdie - Tref), 2);
            double fObj = (Vobj2 - Vos) + c2 * Math.Pow((Vobj2 - Vos), 2);
            double tObj = Math.Pow(Math.Pow(Tdie, 4) + (fObj / S), 0.25);

            tObj = (tObj - 273.15);
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                AmbTempOut.Text = string.Format("Chip:\t{0:0.0####}", AmbTemp);
                ObjTempOut.Text = string.Format("IR:  \t{0:0.0####}", tObj);
            });
        }

        // Accelerometer change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Accelerometer_2
        async void accelChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);

            double x = (SByte)bArray[0] / 64.0;
            double y = (SByte)bArray[1] / 64.0;
            double z = (SByte)bArray[2] / 64.0 * -1;

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                RecTranslateTransform.X = x * 90;
                RecTranslateTransform.Y = y * -90;

                AccelXOut.Text = "X: " + x.ToString();
                AccelYOut.Text = "Y: " + y.ToString();
                AccelZOut.Text = "Z: " + z.ToString();
            });
        }

        // Humidity change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Humidity_Sensor_2
        async void humidChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);

            double humidity = (double)((((UInt16)bArray[1] << 8) + (UInt16)bArray[0]) & ~0x0003);
            humidity = (-6.0 + 125.0 / 65536 * humidity); // RH= -6 + 125 * SRH/2^16
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                HumidOut.Text = humidity.ToString();
            });
        }

        // Magnetometer change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Magnetometer
        async void magnoChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);

            Int16 data = (Int16)(((UInt16)bArray[1] << 8) + (UInt16)bArray[0]);
            double x = (double)data * (2000.0 / 65536);
            data = (Int16)(((UInt16)bArray[3] << 8) + (UInt16)bArray[2]);
            double y = (double)data * (2000.0 / 65536);
            data = (Int16)(((UInt16)bArray[5] << 8) + (UInt16)bArray[4]);
            double z = (double)data * (2000.0 / 65536);

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                MagnoXOut.Text = "X: " + x.ToString();
                MagnoYOut.Text = "Y: " + y.ToString();
                MagnoZOut.Text = "Z: " + z.ToString();
            });
        }

        // Barometric Pressure change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Barometric_Pressure_Sensor_2
        async void pressureChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            UInt16 c3 = (UInt16)(((UInt16)baroCalibrationData[5] << 8) + (UInt16)baroCalibrationData[4]);
            UInt16 c4 = (UInt16)(((UInt16)baroCalibrationData[7] << 8) + (UInt16)baroCalibrationData[6]);
            Int16 c5 = (Int16)(((UInt16)baroCalibrationData[9] << 8) + (UInt16)baroCalibrationData[8]);
            Int16 c6 = (Int16)(((UInt16)baroCalibrationData[11] << 8) + (UInt16)baroCalibrationData[10]);
            Int16 c7 = (Int16)(((UInt16)baroCalibrationData[13] << 8) + (UInt16)baroCalibrationData[12]);
            Int16 c8 = (Int16)(((UInt16)baroCalibrationData[15] << 8) + (UInt16)baroCalibrationData[14]);

            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);

            Int64 s, o, p, val;
            UInt16 Pr = (UInt16)(((UInt16)bArray[3] << 8) + (UInt16)bArray[2]);
            Int16 Tr = (Int16)(((UInt16)bArray[1] << 8) + (UInt16)bArray[0]);

            // Sensitivity
            s = (Int64)c3;
            val = (Int64)c4 * Tr;
            s += (val >> 17);
            val = (Int64)c5 * Tr * Tr;
            s += (val >> 34);

            // Offset
            o = (Int64)c6 << 14;
            val = (Int64)c7 * Tr;
            o += (val >> 3);
            val = (Int64)c8 * Tr * Tr;
            o += (val >> 19);

            // Pressure (Pa)
            p = ((Int64)(s * Pr) + o) >> 14;
            double pres = (double)p / 100;

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                BaroOut.Text = pres.ToString();
            });
        }

        // Gyroscope change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Gyroscope_2
        async void gyroChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);

            Int16 data = (Int16)(((UInt16)bArray[1] << 8) + (UInt16)bArray[0]);
            double x = (double)data * (500.0 / 65536);
            data = (Int16)(((UInt16)bArray[3] << 8) + (UInt16)bArray[2]);
            double y = (double)data * (500.0 / 65536);
            data = (Int16)(((UInt16)bArray[5] << 8) + (UInt16)bArray[4]);
            double z = (double)data * (500.0 / 65536);

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                GyroXOut.Text = "X: " + x.ToString();
                GyroYOut.Text = "Y: " + y.ToString();
                GyroZOut.Text = "Z: " + z.ToString();
            });
        }

        // Key press change handler
        // Algorithm taken from http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Simple_Key_Service
        async void keyChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            byte[] bArray = new byte[eventArgs.CharacteristicValue.Length];
            DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(bArray);

            byte data = bArray[0];

            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                if((data & 0x01) == 0x01)
                    KeyROut.Background = new SolidColorBrush(Colors.Green);
                else
                    KeyROut.Background = new SolidColorBrush(Colors.Red);

                if ((data & 0x02) == 0x02)
                    KeyLOut.Background = new SolidColorBrush(Colors.Green);
                else
                    KeyLOut.Background = new SolidColorBrush(Colors.Red);
            });
        }
    }
}
