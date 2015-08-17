using System;

namespace I2CCompass
{
    public class I2CControllerAddressException : Exception
    {
        public I2CControllerAddressException(int address, string deviceId)
            : base(string.Format("The address {0} on I2C Controller {1} is currently in use.", address, deviceId))
        { }
    }
}
