namespace I2CCompass.Sensors
{
    /// <summary>
    /// Defines the measurement mode of the magnetometer
    /// </summary>
    public enum MagnetometerMeasurementMode
    {
        /// <summary>
        /// Device is in idle mode
        /// </summary>
        Idle = 0,

        /// <summary>
        /// Device is in continuous measurements mode
        /// </summary>
        Continuous = 1,

        /// <summary>
        /// Device is in single measurement mode
        /// </summary>
        Single = 2
    }
}
