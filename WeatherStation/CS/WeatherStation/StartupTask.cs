// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices.I2c;
using Windows.Devices.Enumeration;
using Windows.System.Threading;


namespace WeatherStation
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        private ThreadPoolTimer timer;
        I2cDevice sensor;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();

            String aqs = I2cDevice.GetDeviceSelector("I2C1");
            IReadOnlyList<DeviceInformation> dis = await DeviceInformation.FindAllAsync(aqs);
            //Ox40 was determined by looking at the datasheet for the device
            sensor = await I2cDevice.FromIdAsync(dis[0].Id, new I2cConnectionSettings(0x40));
               
            timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(500));

        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            //Read temperature and humidity and send result to debugger output window

            byte[] tempCommand = new byte[1] { 0xE3 };
            byte[] tempData = new byte[2];
            sensor.WriteRead(tempCommand, tempData);
            var rawTempReading = tempData[0] << 8 | tempData[1];
            var tempRatio = rawTempReading / (float)65536;
            double temperature = (-46.85 + (175.72 * tempRatio)) * 9 / 5 + 32;
            System.Diagnostics.Debug.WriteLine("Temp: " + temperature.ToString());

            byte[] humidityCommand = new byte[1] { 0xE5 };
            byte[] humidityData = new byte[2];
            sensor.WriteRead(humidityCommand, humidityData);
            var rawHumidityReading = humidityData[0] << 8 | humidityData[1];
            var humidityRatio = rawHumidityReading / (float)65536;
            double humidity = -6 + (125 * humidityRatio);
            System.Diagnostics.Debug.WriteLine("Humidity: " + humidity.ToString());

        }
    }
}
