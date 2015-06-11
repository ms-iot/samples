// Copyright (c) Microsoft. All rights reserved.

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
using Windows.Devices.Spi;
using Windows.Devices.Gpio;
using Windows.Devices.Enumeration;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace TempSensor
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            this.InitializeComponent();
            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(500);
            timer.Tick += Timer_Tick;
            timer.Start();

            InitSPI();
        }

        private async void InitSPI()
        {
            try
            {
                var settings = new SpiConnectionSettings(SPI_CHIP_SELECT_LINE);
                settings.ClockFrequency = 500000;// 10000000;
                settings.Mode = SpiMode.Mode0; //Mode3;

                string spiAqs = SpiDevice.GetDeviceSelector(SPI_CONTROLLER_NAME);
                var deviceInfo = await DeviceInformation.FindAllAsync(spiAqs);
                SpiDisplay = await SpiDevice.FromIdAsync(deviceInfo[0].Id, settings);
            }

            /* If initialization fails, display the exception and stop running */
            catch (Exception ex)
            {
                throw new Exception("SPI Initialization Failed", ex);
            }
        }
        private void Timer_Tick(object sender, object e)
        {
            DisplayTextBoxContents();
        }
        public void DisplayTextBoxContents()
        {
            SpiDisplay.TransferFullDuplex(writeBuffer, readBuffer);
            res = convertToInt(readBuffer);
            textPlaceHolder.Text = res.ToString();

        }
        public int convertToInt(byte[] data)
        {
            /*Uncomment if you are using mcp3208/3008 which is 12 bits output */
            /*
             int result = data[1] & 0x0F;
             result <<= 8;
             result += data[2];
             return result;
             */

            /*Uncomment if you are using mcp3002*/
            int result = data[1] & 0x03;
            result <<= 8;
            result += data[2];
            return result;
        }

        /*RaspBerry Pi2  Parameters*/
        private const string SPI_CONTROLLER_NAME = "SPI0";  /* For Raspberry Pi 2, use SPI0                             */
        private const Int32 SPI_CHIP_SELECT_LINE = 0;       /* Line 0 maps to physical pin number 24 on the Rpi2        */

        /*Uncomment if you are using mcp3208/3008 which is 12 bits output */

        // byte[] readBuffer = new byte[3]; /*this is defined to hold the output data*/
        // byte[] writeBuffer = new byte[3] { 0x06, 0x00, 0x00 };//00000110 00; // It is SPI port serial input pin, and is used to load channel configuration data into the device


        /*Uncomment if you are using mcp3002*/
        byte[] readBuffer = new byte[3]; /*this is defined to hold the output data*/
        byte[] writeBuffer = new byte[3] { 0x68, 0x00, 0x00 };//01101000 00; /* It is SPI port serial input pin, and is used to load channel configuration data into the device*/


        private SpiDevice SpiDisplay;

        // create a timer
        private DispatcherTimer timer;
        int res;

    }
}
