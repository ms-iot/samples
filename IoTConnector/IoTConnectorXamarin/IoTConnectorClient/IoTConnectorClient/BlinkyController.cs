using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Gpio;

namespace IoTConnectorClient
{
    class BlinkyController
    {
        private static GpioPin myPin;
        private static GpioPinValue pvalue;

        public static bool Run(int pinNum)
        {
            if(!BlinkyController.Initialize(pinNum))
            {
                return false;
            }
            BlinkyController.Toggle();
            return true;

        }
        private static bool Initialize(int pinNum)
        {
            if(myPin != null)
            {
                if (myPin.PinNumber != pinNum)
                {
                    return InitializePin(pinNum);
                }
                return true;
            }
            return InitializePin(pinNum);
            
            
        }
        private static bool InitializePin(int pinNum)
        {
            if (myPin != null)
            {
                myPin.Dispose();
            }
            var gpio = GpioController.GetDefault();
            if (gpio == null)
            {
                myPin = null;
                return false;
            }
            try
            {
                myPin = gpio.OpenPin(pinNum);
                pvalue = GpioPinValue.High;
                myPin.Write(pvalue);
                myPin.SetDriveMode(GpioPinDriveMode.Output);
                return true;
            } catch (System.ArgumentException)
            {
                return false;
            }
            
        }
        private static void Toggle()
        {
            if (pvalue == GpioPinValue.High)
            {
                pvalue = GpioPinValue.Low;
                myPin.Write(pvalue);
            }
            else
            {
                pvalue = GpioPinValue.High;
                myPin.Write(pvalue);
            }
        }
        public static string PinStatus()
        {
            if (pvalue == GpioPinValue.High && myPin != null)
            {
                return "HIGH";
            } else if (pvalue == GpioPinValue.Low && myPin != null)
            {
                return "LOW";
            } else if (myPin != null)
            {
                return "NOT ACTIVATED";
            } else
            {
                return "ERROR: Pin was unassigned or invalid";
            }
        }
        public static string PinNumber()
        {
            if(myPin == null)
            {
                return "NOT ASSIGNED";
            } else
            {
                return myPin.PinNumber.ToString();
            }
        }
    }
}
