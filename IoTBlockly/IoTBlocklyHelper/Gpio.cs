using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Gpio;

namespace IoTBlocklyHelper
{
    public sealed class Gpio
    {
        // notice how we keep the "state" static so that it can be reused when we stop/restart/modify the js script
        static private GpioController controller;
        static private Dictionary<int, GpioPin> pins = new Dictionary<int, GpioPin>();
        private const int RPi2_ONBOARD_LED_PIN = 47;

        public Gpio()
        {
            if (controller == null)
            {
                controller = GpioController.GetDefault();
            }
        }

        public static bool Valid()
        {
            return (controller != null);
        }

        public void LedOn()
        {
            SetOnboardLED(1);
        }

        public void LedOff()
        {
            SetOnboardLED(0);
        }

        public void SetOnboardLED(int value)
        {
            SetGPIOPin(RPi2_ONBOARD_LED_PIN, value);
        }

        public void SetGPIOPin(int pinNumber, int value)
        {
            if (!Valid()) { return; }
            var pin = GetPin(pinNumber);
            if (pin == null) { return; }
            if (value == 0)
            {
                pin.Write(GpioPinValue.Low);
            }
            else
            {
                pin.Write(GpioPinValue.High);
            }
        }

        private static GpioPin GetPin(int pinNumber)
        {
            if (!Valid()) { return null; }
            if (!pins.ContainsKey(pinNumber))
            {
                try
                {
                    var pin = controller.OpenPin(pinNumber);
                    pin.Write(GpioPinValue.Low);
                    pin.SetDriveMode(GpioPinDriveMode.Output);
                    pins[pinNumber] = pin;
                }
                catch
                {
                    pins[pinNumber] = null;
                }
            }
            return pins[pinNumber];
        }

    }
}
