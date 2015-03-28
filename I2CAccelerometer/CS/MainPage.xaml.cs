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
using Windows.Devices.I2c;

namespace I2CAccelerometer
{
    /// <summary>
    /// Sample app that reads data over I2C from an attached ADXL345 accelerometer
    /// </summary>
    public sealed partial class MainPage : Page
    {
        /* Important! Set the correct I2C controller name for your target device here */
        private const string I2C_CONTROLLER_NAME = "I2C5";        /* For Minnowboard Max, use I2C5 */
        //private const string I2C_CONTROLLER_NAME = "I2C1";        /* For Raspberry Pi 2, use I2C1 */

        private const byte ACCEL_I2C_ADDR = 0x53;           /* 7-bit I2C address of the ADXL345      */
        private const byte ACCEL_REG_POWER_CONTROL = 0x2D;  /* Address of the Power Control register */
        private const byte ACCEL_REG_DATA_FORMAT = 0x31;    /* Address of the Data Format register   */
        private const byte ACCEL_REG_X = 0x32;              /* Address of the X Axis data register   */
        private const byte ACCEL_REG_Y = 0x34;              /* Address of the Y Axis data register   */
        private const byte ACCEL_REG_Z = 0x36;              /* Address of the Z Axis data register   */

        private I2cDevice I2CAccel;
        private DispatcherTimer periodicTimer;

        public MainPage()
        {
            this.InitializeComponent();

            /* Register for the unloaded event so we can clean up upon exit */
            Unloaded += MainPage_Unloaded;

            /* Initialize the I2C bus, accelerometer, and timer */
            InitI2CAccel();
        }

        private async void InitI2CAccel()
        {
            /* Initialize the I2C bus */
            try {
                var settings = new I2cConnectionSettings(ACCEL_I2C_ADDR); 
                settings.BusSpeed = I2cBusSpeed.FastMode;

                string aqs = I2cDevice.GetDeviceSelector(I2C_CONTROLLER_NAME);  /* Find the selector string for the I2C bus controller                   */
                var dis = await DeviceInformation.FindAllAsync(aqs);            /* Find the I2C bus controller device with our selector string           */
                I2CAccel = await I2cDevice.FromIdAsync(dis[0].Id, settings);    /* Create an I2cDevice with our selected bus controller and I2C settings */

            }
            /* If initialization fails, display the exception and stop running */
            catch (Exception e)
            {
                Text_Status.Text = "Exception: " + e.Message;
                return;
            }

            /* 
             * Initialize the accelerometer:
             *
             * For this device, we create 2-byte write buffers:
             * The first byte is the register address we want to write to.
             * The second byte is the contents that we want to write to the register. 
             */
            byte[] WriteBuf_DataFormat = new byte[] { ACCEL_REG_DATA_FORMAT, 0x01 };        /* 0x01 sets range to +- 4Gs                         */
            byte[] WriteBuf_PowerControl = new byte[] { ACCEL_REG_POWER_CONTROL, 0x08 };    /* 0x08 puts the accelerometer into measurement mode */

            /* Write the register settings */
            try
            {
                I2CAccel.Write(WriteBuf_DataFormat);
                I2CAccel.Write(WriteBuf_PowerControl);
            }
            /* If the write fails display the error and stop running */
            catch (Exception)
            {
                Text_Status.Text = "Status: Accelerometer initialization failed";
                return;
            }

            /* Now that everything is initialized, create a timer so we read data every 100mS */
            periodicTimer = new DispatcherTimer();
            periodicTimer.Interval = TimeSpan.FromMilliseconds(100);
            periodicTimer.Tick += Timer_Tick;
            periodicTimer.Start();
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            /* Cleanup */
            I2CAccel.Dispose();
        }

        private void ReadI2CAccel()
        {
            const string FLOAT_FORMAT = "F3";   /* Specify that we want 3 decimal points for floating point numbers                */
            const int ACCEL_RES = 1024;         /* The ADXL345 has 10 bit resolution giving 1024 unique values                     */
            const int ACCEL_DYN_RANGE_G = 8;    /* The ADXL345 had a total dynamic range of 8G, since we're configuring it to +-4G */
            const int UNITS_PER_G = ACCEL_RES / ACCEL_DYN_RANGE_G;  /* Ratio of raw int values to G units                          */
     
            Int16 AccelerationRawX, AccelerationRawY, AccelerationRawZ; /* Raw readings from the accelerometer */
            double AccelerationX, AccelerationY, AccelerationZ;         /* Readings converted to G units       */

            byte[] RegAddrBuf = new byte[] { ACCEL_REG_X }; /* Register address we want to read from                                         */
            byte[] ReadBuf = new byte[6];                   /* We read 6 bytes sequentially to get all 3 two-byte axes registers in one read */

            try
            {
                /* 
                 * Read from the accelerometer 
                 * We call WriteRead() so we first write the address of the X-Axis I2C register, then read all 3 axes
                 */
                I2CAccel.WriteRead(RegAddrBuf, ReadBuf);
            }
            catch (Exception e)
            {
                /* If WriteRead() fails, display error messages */
                Text_X_Axis.Text = "X Axis: Error";
                Text_Y_Axis.Text = "Y Axis: Error";
                Text_Z_Axis.Text = "Z Axis: Error";
                Text_Status.Text = "Exception: " + e.Message;
                return;
            }

            /* 
             * In order to get the raw 16-bit data values, we need to concatenate two 8-bit bytes from the I2C read for each axis.
             * We accomplish this by using bit shift and logical OR operations
             */
            AccelerationRawX = (Int16)(ReadBuf[0] | ReadBuf[1] << 8);
            AccelerationRawY = (Int16)(ReadBuf[2] | ReadBuf[3] << 8);
            AccelerationRawZ = (Int16)(ReadBuf[4] | ReadBuf[5] << 8);

            /* Convert raw values to G's */
            AccelerationX = (double)AccelerationRawX / UNITS_PER_G;
            AccelerationY = (double)AccelerationRawY / UNITS_PER_G;
            AccelerationZ = (double)AccelerationRawZ / UNITS_PER_G;

            /* Display the values */
            Text_X_Axis.Text = "X Axis: " + AccelerationX.ToString(FLOAT_FORMAT) + "G";
            Text_Y_Axis.Text = "Y Axis: " + AccelerationY.ToString(FLOAT_FORMAT) + "G";
            Text_Z_Axis.Text = "Z Axis: " + AccelerationZ.ToString(FLOAT_FORMAT) + "G";
            Text_Status.Text = "Status: Running";
        }

        private void Timer_Tick(object sender, object e)
        {
            ReadI2CAccel(); /* Read data from the I2C accelerometer and display it */
        }

    }
}
