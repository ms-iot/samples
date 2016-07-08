// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading;
using Windows.UI.Xaml.Controls;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;

namespace I2CAccelerometer
{
    struct Acceleration
    {
        public double X;
        public double Y;
        public double Z;
    };
    
    /// <summary>
    /// Sample app that reads data over I2C from an attached ADXL345 accelerometer
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private const byte ACCEL_I2C_ADDR = 0x53;           /* 7-bit I2C address of the ADXL345 with SDO pulled low */
        private const byte ACCEL_REG_POWER_CONTROL = 0x2D;  /* Address of the Power Control register */
        private const byte ACCEL_REG_DATA_FORMAT = 0x31;    /* Address of the Data Format register   */
        private const byte ACCEL_REG_X = 0x32;              /* Address of the X Axis data register   */
        private const byte ACCEL_REG_Y = 0x34;              /* Address of the Y Axis data register   */
        private const byte ACCEL_REG_Z = 0x36;              /* Address of the Z Axis data register   */

        private I2cDevice I2CAccel;
        private Timer periodicTimer;

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

            var settings = new I2cConnectionSettings(ACCEL_I2C_ADDR); 
            settings.BusSpeed = I2cBusSpeed.FastMode;
            var controller = await I2cController.GetDefaultAsync();
            I2CAccel = controller.GetDevice(settings);    /* Create an I2cDevice with our selected bus controller and I2C settings */


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
            catch (Exception ex)
            {
                Text_Status.Text = "Failed to communicate with device: " + ex.Message;
                return;
            }

            /* Now that everything is initialized, create a timer so we read data every 100mS */
            periodicTimer = new Timer(this.TimerCallback, null, 0, 100);
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            /* Cleanup */
            I2CAccel.Dispose();
        }
        
        private void TimerCallback(object state)
        {
            string xText, yText, zText;
            string statusText;
            
			/* Read and format accelerometer data */
            try
            {
                Acceleration accel = ReadI2CAccel();
                xText = String.Format("X Axis: {0:F3}G", accel.X);
                yText = String.Format("Y Axis: {0:F3}G", accel.Y);
                zText = String.Format("Z Axis: {0:F3}G", accel.Z);
                statusText = "Status: Running";
            }
            catch (Exception ex)
            {
                xText = "X Axis: Error";
                yText = "Y Axis: Error";
                zText = "Z Axis: Error";
                statusText = "Failed to read from Accelerometer: " + ex.Message;
            }
            
            /* UI updates must be invoked on the UI thread */
            var task = this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                Text_X_Axis.Text = xText;
                Text_Y_Axis.Text = yText;
                Text_Z_Axis.Text = zText;
                Text_Status.Text = statusText;
            });
        }

        private Acceleration ReadI2CAccel()
        {
            const int ACCEL_RES = 1024;         /* The ADXL345 has 10 bit resolution giving 1024 unique values                     */
            const int ACCEL_DYN_RANGE_G = 8;    /* The ADXL345 had a total dynamic range of 8G, since we're configuring it to +-4G */
            const int UNITS_PER_G = ACCEL_RES / ACCEL_DYN_RANGE_G;  /* Ratio of raw int values to G units                          */

            byte[] RegAddrBuf = new byte[] { ACCEL_REG_X }; /* Register address we want to read from                                         */
            byte[] ReadBuf = new byte[6];                   /* We read 6 bytes sequentially to get all 3 two-byte axes registers in one read */
            
            /* 
             * Read from the accelerometer 
             * We call WriteRead() so we first write the address of the X-Axis I2C register, then read all 3 axes
             */
            I2CAccel.WriteRead(RegAddrBuf, ReadBuf);

            /* 
             * In order to get the raw 16-bit data values, we need to concatenate two 8-bit bytes from the I2C read for each axis.
             * We accomplish this by using the BitConverter class.
             */
            short AccelerationRawX = BitConverter.ToInt16(ReadBuf, 0);
            short AccelerationRawY = BitConverter.ToInt16(ReadBuf, 2);
            short AccelerationRawZ = BitConverter.ToInt16(ReadBuf, 4);

            /* Convert raw values to G's */
            Acceleration accel;
            accel.X = (double)AccelerationRawX / UNITS_PER_G;
            accel.Y = (double)AccelerationRawY / UNITS_PER_G;
            accel.Z = (double)AccelerationRawZ / UNITS_PER_G;

            return accel;
        }
    }
}
