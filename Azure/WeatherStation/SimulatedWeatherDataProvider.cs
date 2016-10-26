using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WeatherDataReporter
{
    class SimulatedWeatherDataProvider : IWeatherDataProvider
    {
        private int i = 0;
        private const int delay = 1000;
        private const int humPeriod = 20 * delay;
        private const int humMin = 60;
        private const int humMax = 80;

        private const int tempPeriod = 25 * delay;
        private const int tempMin = 68;
        private const int tempMax = 82;

        private static readonly Random rnd = new Random();

        public static Task<SimulatedWeatherDataProvider> Create()
        {
            return Task.FromResult(new SimulatedWeatherDataProvider());
        }

        static private double CalculateNextValue(int i, int period, int min, int max)
        {
            double x = Math.PI * (((double)i * delay) / period);
            double val = ((1.0 + Math.Sin(x)) / 2.0) * (max - min) + min;

            // Add random fluctuation
            if (true)
            {
                var delta = (double)rnd.Next(min - max, max - min);
                val += delta / 20;
            }

            return val;
        }

        public double GetHumidity()
        {
            return CalculateNextValue(i++, humPeriod, humMin, humMax);
        }

        public double GetTemperature()
        {
            return CalculateNextValue(i++, tempPeriod, tempMin, tempMax);
        }
    }
}
