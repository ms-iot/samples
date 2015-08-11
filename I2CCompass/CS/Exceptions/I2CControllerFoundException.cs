using System;

namespace I2CCompass
{
    public class I2CControllerFoundException : Exception
    {
        public I2CControllerFoundException()
            : base("Could not find the I2C controller")
        { }
    }
}
