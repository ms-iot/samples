/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/
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
