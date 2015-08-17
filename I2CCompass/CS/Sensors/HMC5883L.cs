using System;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;

namespace I2CCompass.Sensors
{
    /// <summary>
    /// This class encapsulates the HMC5883L magnetometer
    /// </summary>
    public sealed class HMC5883L : ICompass, IDisposable
    {
        public event EventHandler<CompassReading> CompassReadingChangedEvent;

        #region HMC5883L addresses

        private const int ADDRESS = 0x1E;

        /// <summary>
        /// 7-bit I2C read address of the HMC5883L
        /// </summary>
        private const int ADDRESS_READ = 0x3D;

        /// <summary>
        /// 7-bit I2C write address of the HMC5883L
        /// </summary>
        private const int ADDRESS_WRITE = 0x3C;

        #endregion

        #region Register List

        /// <summary>
        /// Configuration Register A (Read/Write)
        /// </summary>
        private const byte CONFIGURATION_REGISTER_A = 0x00;

        /// <summary>
        /// Configuration Register B (Read/Write)
        /// </summary>
        private const byte CONFIGURATION_REGISTER_B = 0x01;

        /// <summary>
        /// Mode Register (Read/Write)
        /// </summary>
        private const byte MODE_REGISTER = 0x02;

        /// <summary>
        /// Data Output X MSB Register (Read only)
        /// </summary>
        private const byte DATA_OUTPUT_X_MSB_REGISTER = 0x03;

        /// <summary>
        /// Data Output X LSB Register (Read only)
        /// </summary>
        private const byte DATA_OUTPUT_X_LSB_REGISTER = 0x04;

        /// <summary>
        /// Data Output Z MSB Register (Read only)
        /// </summary>
        private const byte DATA_OUTPUT_Z_MSB_REGISTER = 0x05;

        /// <summary>
        /// Data Output Z LSB Register (Read only)
        /// </summary>
        private const byte DATA_OUTPUT_Z_LSB_REGISTER = 0x06;

        /// <summary>
        /// Data Output Y MSB Register (Read only)
        /// </summary>
        private const byte DATA_OUTPUT_Y_MSB_REGISTER = 0x07;

        /// <summary>
        /// Data Output Y LSB Register (Read only)
        /// </summary>
        private const byte DATA_OUTPUT_Y_LSB_REGISTER = 0x08;

        /// <summary>
        /// Status Register (Read only)
        /// </summary>
        private const byte STATUS_REGISTER = 0x09;

        /// <summary>
        /// Identification Register A (Read only)
        /// </summary>
        private const byte IDENTIFICATION_REGISTER_A = 0x10;

        /// <summary>
        /// Identification Register B (Read only)
        /// </summary>
        private const byte IDENTIFICATION_REGISTER_B = 0x11;

        /// <summary>
        /// Identification Register C (Read only)
        /// </summary>
        private const byte IDENTIFICATION_REGISTER_C = 0x12;

        #endregion

        #region Number Of Samples Averaged 

        /// <summary>
        /// 1 sample averaged per measurement output
        /// </summary>
        private const byte SAMPLES_AVERAGED_1 = 0x00;

        /// <summary>
        /// 2 samples averaged per measurement output
        /// </summary>
        private const byte SAMPLES_AVERAGED_2 = 0x20;

        /// <summary>
        /// 4 samples averaged per measurement output
        /// </summary>
        private const byte SAMPLES_AVERAGED_4 = 0x40;

        /// <summary>
        /// 8 samples averaged per measurement output
        /// </summary>
        private const byte SAMPLES_AVERAGED_8 = 0x60;

        #endregion

        #region Output Rates

        /// <summary>
        /// 0.75 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_0_75 = 0x00;

        /// <summary>
        /// 1.5 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_1_5 = 0x04;

        /// <summary>
        /// 3 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_3 = 0x08;

        /// <summary>
        /// 7.5 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_7_5 = 0x0C;

        /// <summary>
        /// 15 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_15 = 0x10;

        /// <summary>
        /// 30 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_30 = 0x14;

        /// <summary>
        /// 75 Hz Output Rate
        /// </summary>
        private const byte OUTPUT_RATE_75 = 0x18;

        #endregion

        #region Gain Settings

        /// <summary>
        /// Sensor Field Range ± 0.88 Ga (1370 LSb/Gauss)
        /// </summary>
        private const byte GAIN_1370 = 0x00;

        /// <summary>
        /// Sensor Field Range ± 1.3 Ga (1090 LSb/Gauss)
        /// </summary>
        private const byte GAIN_1090 = 0x20;

        /// <summary>
        /// Sensor Field Range ± 1.9 Ga (820 LSb/Gauss)
        /// </summary>
        private const byte GAIN_820 = 0x40;

        /// <summary>
        /// Sensor Field Range ± 2.5 Ga (660 LSb/Gauss)
        /// </summary>
        private const byte GAIN_660 = 0x60;

        /// <summary>
        /// Sensor Field Range ± 4.0 Ga (440 LSb/Gauss)
        /// </summary>
        private const byte GAIN_440 = 0x80;

        /// <summary>
        /// Sensor Field Range ± 4.7 Ga (390 LSb/Gauss)
        /// </summary>
        private const byte GAIN_390 = 0xA0;

        /// <summary>
        /// Sensor Field Range ± 5.6 Ga (330 LSb/Gauss)
        /// </summary>
        private const byte GAIN_330 = 0xC0;

        /// <summary>
        /// Sensor Field Range ± 8.1 Ga (230 LSb/Gauss)
        /// </summary>
        private const byte GAIN_230 = 0xE0;

        #endregion

        #region Measurement Modes

        /// <summary>
        /// Normal measurement configuration (Default).
        /// In normal measurement configuration the device follows normal measurement flow.
        /// The positive and negative pins of the resistive load are left floating and high impedance.
        /// </summary>
        private const byte MEASUREMENT_MODE_NORMAL = 0x00;

        /// <summary>
        /// Positive bias configuration for X, Y, and Z axes.
        /// In this configuration, a positive current is forced across the resistive load for all three axes.
        /// </summary>
        private const byte MEASUREMENT_MODE_POSITIVE_BIAS = 0x01;

        /// <summary>
        /// Negative bias configuration for X, Y and Z axes.
        /// In this configuration, a negative current is forced across the resistive load for all three axes.
        /// </summary>
        private const byte MEASUREMENT_MODE_NEGATIVE_BIAS = 0x01;

        #endregion

        #region Operating Modes

        /// <summary>
        /// Continuous measurement mode
        /// </summary>
        private const byte OPERATING_MODE_CONTINUOUS = 0x00;

        /// <summary>
        /// Single measurement mode
        /// </summary>
        private const byte OPERATING_MODE_SINGLE = 0x01;

        /// <summary>
        /// Idle mode
        /// </summary>
        private const byte OPERATING_MODE_IDLE = 0x02;

        #endregion

        private bool _isDisposed;

        /// <summary>
        /// I2C Controller
        /// </summary>
        private I2cDevice _i2cController;

        /// <summary>
        /// Timer used for continuous measurements
        /// </summary>
        private Timer _timer;

        /// <summary>
        /// Default constructor
        /// </summary>
        public HMC5883L()
        { }

        /// <summary>
        /// Retuns true if the device is initialized
        /// </summary>
        public bool IsInitialized { get; private set; }

        /// <summary>
        /// Measurement mode
        /// </summary>
        public MagnetometerMeasurementMode MeasurementMode { get; private set; }

        /// <summary>
        /// Initializes the I2C controller
        /// </summary>
        private async Task InitializeAsync()
        {
            if (IsInitialized)
            {
                throw new InvalidOperationException("The I2C controller is already initialized.");
            }

            // Get a selector string that will return all I2C controllers on the system
            var advancedQuerySyntaxString = I2cDevice.GetDeviceSelector();

            // Find the I2C bus controller devices with our selector string
            var controllerDeviceIds = await DeviceInformation.FindAllAsync(advancedQuerySyntaxString);

            // Ensure we have an I2C controler
            if (controllerDeviceIds == null || controllerDeviceIds.Count == 0)
            {
                throw new I2CControllerFoundException();
            }
            var i2cControllerDeviceId = controllerDeviceIds[0].Id;


            // Setup the settings to address 0x1E with a 400KHz bus speed
            var i2cSettings = new I2cConnectionSettings(ADDRESS);
            i2cSettings.BusSpeed = I2cBusSpeed.FastMode;

            // Create an I2cDevice with our selected bus controller ID and I2C settings
            _i2cController = await I2cDevice.FromIdAsync(i2cControllerDeviceId, i2cSettings);

            // For some bizare reason, the debugger goes into this if despite _i2cController having a value
            //if (_i2cController == null)
            //{
            //    throw new I2CControllerAddressException(i2cSettings.SlaveAddress, i2cControllerDeviceId);
            //}

            IsInitialized = true;
        }

        public async Task SetModeAsync(MagnetometerMeasurementMode measurementMode)
        {
            if (_isDisposed)
            {
                throw new ObjectDisposedException("HMC5883L");
            }
            if (!IsInitialized)
            {
                await InitializeAsync();
            }

            byte[] writeBuffer;

            // Write CRA (00) – send 0x3C 0x00 0x70 (8-average, 15 Hz default, normal measurement)
            writeBuffer = new byte[] { CONFIGURATION_REGISTER_A, SAMPLES_AVERAGED_8 | OUTPUT_RATE_15 | MEASUREMENT_MODE_NORMAL };
            _i2cController.Write(writeBuffer);

            // Write CRB (01) – send 0x3C 0x01 0xA0 (Gain=5, or any other desired gain)
            writeBuffer = new byte[] { CONFIGURATION_REGISTER_B, GAIN_390 };
            _i2cController.Write(writeBuffer);

            switch (measurementMode)
            {
                case MagnetometerMeasurementMode.Continuous:

                    // Write Mode (02) – send 0x3C 0x02 0x00 (Continuous-measurement mode)
                    writeBuffer = new byte[] { MODE_REGISTER, OPERATING_MODE_CONTINUOUS };
                    _i2cController.Write(writeBuffer);

                    // Wait 6 ms for setup then take a reading every 200 ms
                    _timer = new Timer(OnMeasurementReady, null, 6, 200);

                    break;

                case MagnetometerMeasurementMode.Single:

                    // Write Mode (02) – send 0x3C 0x02 0x01 (Single measurement mode)
                    writeBuffer = new byte[] { MODE_REGISTER, OPERATING_MODE_SINGLE };
                    _i2cController.Write(writeBuffer);

                    await Task.Delay(100);

                    OnMeasurementReady(null);

                    break;

                case MagnetometerMeasurementMode.Idle:

                    // Write Mode (02) – send 0x3C 0x02 0x02 (Idle mode)
                    writeBuffer = new byte[] { MODE_REGISTER, OPERATING_MODE_IDLE };
                    _i2cController.Write(writeBuffer);

                    break;
            }

            MeasurementMode = measurementMode;
        }

        private void OnMeasurementReady(object state)
        {
            byte[] writeBuffer;
            byte[] readBuffer = new byte[6];

            // Send 0x3D 0x06 (Read all 6 bytes. If gain is changed then this data set is using previous gain)
            writeBuffer = new byte[] { 0x06 };
            _i2cController.WriteRead(writeBuffer, readBuffer);

            // Send 0x3C 0x03(point to first data register 03)
            writeBuffer = new byte[] { DATA_OUTPUT_X_MSB_REGISTER };
            _i2cController.Write(writeBuffer);

            //TODO: Revise the bearing calculation an perform a self-test to calibrate the IC

            //Check the endianness of the system and flip the bytes if necessary
            if (!BitConverter.IsLittleEndian)
            {
                Array.Reverse(readBuffer, 0, 2);
                Array.Reverse(readBuffer, 2, 2);
                Array.Reverse(readBuffer, 4, 2);
            }

            // Convert three 16-bit 2’s compliment hex values to decimal values and assign to X, Z, Y, respectively.
            int x = BitConverter.ToInt16(readBuffer, 0);
            int y = BitConverter.ToInt16(readBuffer, 2);
            int z = BitConverter.ToInt16(readBuffer, 4);

            double headingDegrees;
            if (y > 0)
            {
                headingDegrees = 90 - Math.Atan(x / y) * 180 / Math.PI;
            }
            else if (y < 0)
            {
                headingDegrees = 270 - Math.Atan(x / y) * 180 / Math.PI;
            }
            else if (x < 0)
            {
                headingDegrees = 180.0;
            }
            else
            {
                headingDegrees = 0.0;
            }

            if (CompassReadingChangedEvent != null)
            {
                CompassReadingChangedEvent(this, new CompassReading(headingDegrees));
            }
        }

        public void Dispose()
        {
            if (_timer != null)
            {
                _timer.Dispose();
                _timer = null;
            }
            if (_i2cController != null)
            {
                _i2cController.Dispose();
                _i2cController = null;
            }
            _isDisposed = true;
            GC.SuppressFinalize(this);
        }
    }
}
