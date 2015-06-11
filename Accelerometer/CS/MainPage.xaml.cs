// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading;
using Windows.UI.Xaml.Controls;
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
using Windows.Devices.Spi;

namespace Accelerometer
{
    struct Acceleration
    {
        public double X;
        public double Y;
        public double Z;
    };

    enum Protocol { NONE, SPI, I2C};
    
    /// <summary>
    /// Sample app that reads data over either I2C or SPI from an attached ADXL345 accelerometer
    /// </summary>
    public sealed partial class MainPage : Page
    {
        /* Important! Change this to either Protocol.I2C or Protocol.SPI based on how your accelerometer is wired   */
        private Protocol HW_PROTOCOL = Protocol.NONE;       

        private const byte ACCEL_REG_POWER_CONTROL = 0x2D;  /* Address of the Power Control register                */
        private const byte ACCEL_REG_DATA_FORMAT = 0x31;    /* Address of the Data Format register                  */
        private const byte ACCEL_REG_X = 0x32;              /* Address of the X Axis data register                  */
        private const byte ACCEL_REG_Y = 0x34;              /* Address of the Y Axis data register                  */
        private const byte ACCEL_REG_Z = 0x36;              /* Address of the Z Axis data register                  */

        private const byte ACCEL_I2C_ADDR = 0x53;           /* 7-bit I2C address of the ADXL345 with SDO pulled low */

        private const byte SPI_CHIP_SELECT_LINE = 0;        /* Chip select line to use                              */
        private const byte ACCEL_SPI_RW_BIT = 0x80;         /* Bit used in SPI transactions to indicate read/write  */
        private const byte ACCEL_SPI_MB_BIT = 0x40;         /* Bit used to indicate multi-byte SPI transactions     */

        private I2cDevice I2CAccel;
        private SpiDevice SPIAccel;
        private Timer periodicTimer;

        public MainPage()
        {
            this.InitializeComponent();

            /* Register for the unloaded event so we can clean up upon exit */
            Unloaded += MainPage_Unloaded;

            /* Initialize the I2C bus, accelerometer, and timer */
            InitAccel();
        }

        private void InitAccel()
        {
            /* Initialize the protocol and accelerometer */
            switch (HW_PROTOCOL)
            {
                case Protocol.SPI:
                    InitSPIAccel();
                    break;
                case Protocol.I2C:
                    InitI2CAccel();
                    break;
                case Protocol.NONE:
                    Text_Status.Text = "Please change the HW_PROTOCOL variable to either I2C or SPI";
                    break;
                default:
                    break;
            }
        }

        /* Initialization for I2C accelerometer */
        private async void InitI2CAccel()
        {
            try
            {
                var settings = new I2cConnectionSettings(ACCEL_I2C_ADDR);       
                settings.BusSpeed = I2cBusSpeed.FastMode;                       /* 400KHz bus speed */

                string aqs = I2cDevice.GetDeviceSelector();                     /* Get a selector string that will return all I2C controllers on the system */
                var dis = await DeviceInformation.FindAllAsync(aqs);            /* Find the I2C bus controller devices with our selector string             */
                I2CAccel = await I2cDevice.FromIdAsync(dis[0].Id, settings);    /* Create an I2cDevice with our selected bus controller and I2C settings    */
                if (I2CAccel == null)
                {
                    Text_Status.Text = string.Format(
                        "Slave address {0} on I2C Controller {1} is currently in use by " +
                        "another application. Please ensure that no other applications are using I2C.",
                        settings.SlaveAddress,
                        dis[0].Id);
                    return;
                }
            }
            catch (Exception ex)
            {
                Text_Status.Text = "I2C Initialization failed. Exception: " + ex.Message;
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
            catch (Exception ex)
            {
                Text_Status.Text = "Failed to communicate with device: " + ex.Message;
                return;
            }

            /* Now that everything is initialized, create a timer so we read data every 100mS */
            periodicTimer = new Timer(this.TimerCallback, null, 0, 100);
        }

        /* Initialization for SPI accelerometer */
        private async void InitSPIAccel()
        {
            try {
                var settings = new SpiConnectionSettings(SPI_CHIP_SELECT_LINE);
                settings.ClockFrequency = 5000000;                              /* 5MHz is the rated speed of the ADXL345 accelerometer                     */
                settings.Mode = SpiMode.Mode3;                                  /* The accelerometer expects an idle-high clock polarity, we use Mode3    
                                                                                 * to set the clock polarity and phase to: CPOL = 1, CPHA = 1         
                                                                                 */

                string aqs = SpiDevice.GetDeviceSelector();                     /* Get a selector string that will return all SPI controllers on the system */
                var dis = await DeviceInformation.FindAllAsync(aqs);            /* Find the SPI bus controller devices with our selector string             */
                SPIAccel = await SpiDevice.FromIdAsync(dis[0].Id, settings);    /* Create an SpiDevice with our bus controller and SPI settings             */
                if (SPIAccel == null)
                {
                    Text_Status.Text = string.Format(
                        "SPI Controller {0} is currently in use by " +
                        "another application. Please ensure that no other applications are using SPI.",
                        dis[0].Id);
                    return;
                }
            }
            catch (Exception ex)
            {
                Text_Status.Text = "SPI Initialization failed. Exception: " + ex.Message;
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
                SPIAccel.Write(WriteBuf_DataFormat);
                SPIAccel.Write(WriteBuf_PowerControl);
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
            switch (HW_PROTOCOL)
            {
                case Protocol.SPI:
                    SPIAccel.Dispose();
                    break;
                case Protocol.I2C:
                    I2CAccel.Dispose();
                    break;
                default:
                    break;
            }  
        }
        
        private void TimerCallback(object state)
        {
            string xText, yText, zText;
            string statusText;
            
			/* Read and format accelerometer data */
            try
            {
                Acceleration accel = ReadAccel();
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

        private Acceleration ReadAccel()
        {
            const int ACCEL_RES = 1024;         /* The ADXL345 has 10 bit resolution giving 1024 unique values                     */
            const int ACCEL_DYN_RANGE_G = 8;    /* The ADXL345 had a total dynamic range of 8G, since we're configuring it to +-4G */
            const int UNITS_PER_G = ACCEL_RES / ACCEL_DYN_RANGE_G;  /* Ratio of raw int values to G units                          */

            byte[] ReadBuf;                 
            byte[] RegAddrBuf;

            /* 
             * Read from the accelerometer 
             * We first write the address of the X-Axis register, then read all 3 axes into ReadBuf
             */
            switch (HW_PROTOCOL)
            {
                case Protocol.SPI:
                    ReadBuf = new byte[6 + 1];      /* Read buffer of size 6 bytes (2 bytes * 3 axes) + 1 byte padding */
                    RegAddrBuf = new byte[1 + 6];   /* Register address buffer of size 1 byte + 6 bytes padding        */
                    /* Register address we want to read from with read and multi-byte bit set                          */
                    RegAddrBuf[0] =  ACCEL_REG_X | ACCEL_SPI_RW_BIT | ACCEL_SPI_MB_BIT ;
                    SPIAccel.TransferFullDuplex(RegAddrBuf, ReadBuf);
                    Array.Copy(ReadBuf, 1, ReadBuf, 0, 6);  /* Discard first dummy byte from read                      */
                    break;
                case Protocol.I2C:
                    ReadBuf = new byte[6];  /* We read 6 bytes sequentially to get all 3 two-byte axes                 */
                    RegAddrBuf = new byte[] { ACCEL_REG_X }; /* Register address we want to read from                  */
                    I2CAccel.WriteRead(RegAddrBuf, ReadBuf);
                    break;
                default:    /* Code should never get here */
                    ReadBuf = new byte[6];
                    break;
            }
                         
            /* Check the endianness of the system and flip the bytes if necessary */
            if (!BitConverter.IsLittleEndian)
            {
                Array.Reverse(ReadBuf, 0, 2);
                Array.Reverse(ReadBuf, 2, 2);
                Array.Reverse(ReadBuf, 4, 2);
            }

            /* In order to get the raw 16-bit data values, we need to concatenate two 8-bit bytes for each axis */
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
