using System;
using System.Threading.Tasks;

namespace I2CCompass.Sensors
{
    public interface ICompass : IDisposable
    {
        event EventHandler<CompassReading> CompassReadingChangedEvent;

        bool IsInitialized { get; }

        MagnetometerMeasurementMode MeasurementMode { get; }

        Task SetModeAsync(MagnetometerMeasurementMode measurementMode);
    }
}
