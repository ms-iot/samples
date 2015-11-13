using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.I2c;
using Windows.Devices.Enumeration;

namespace WeatherDataReporter
{
    class WeatherDataProvider
    {
        public static async Task<WeatherDataProvider> Create()
        {
            String aqs = I2cDevice.GetDeviceSelector("I2C1");
            IReadOnlyList<DeviceInformation> dis = await DeviceInformation.FindAllAsync(aqs);
            //Ox40 was determined by looking at the datasheet for the device
            var sensor = await I2cDevice.FromIdAsync(dis[0].Id, new I2cConnectionSettings(0x40));
            return new WeatherDataProvider(sensor);
        }

        private I2cDevice sensor;

        private WeatherDataProvider(I2cDevice sensor)
        {
            this.sensor = sensor;
        }

        public double GetHumidity()
        {
            byte[] humidityCommand = new byte[1] { 0xE5 };
            byte[] humidityData = new byte[2];
            try
            {
                sensor.WriteRead(humidityCommand, humidityData);
                var rawHumidityReading = humidityData[0] << 8 | humidityData[1];
                var humidityRatio = rawHumidityReading / (float)65536;
                double humidity = -6 + (125 * humidityRatio);
                return humidity;
            }
            catch (Exception)
            {
                return 0.0;
            }
        }

        public double GetTemperature()
        {
            try
            {
                byte[] tempCommand = new byte[1] { 0xE3 };
                byte[] tempData = new byte[2];
                sensor.WriteRead(tempCommand, tempData);
                var rawTempReading = tempData[0] << 8 | tempData[1];
                var tempRatio = rawTempReading / (float)65536;
                double temperature = -46.85 + (175.72 * tempRatio);
                return temperature;
            }
            catch (Exception)
            {
                return 0.0;
            }
        }
    }
}
