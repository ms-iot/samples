// © Copyright(C) Microsoft. All rights reserved.

using System;
using Windows.Devices.Enumeration;
using Windows.Devices.Gpio;

namespace bertha
{
    class LED
    {
        private int LEDStatus;
        private GpioOutputPin OutPin;

        public LED()
        {
            LEDStatus = 0;
            InitGPIO();
        }

        private async void InitGPIO()
        {
            var deviceId = GpioController.GetDeviceSelector("GPIO_S5");
            var deviceInfos = await DeviceInformation.FindAllAsync(deviceId, null);
            var controller = await GpioController.FromIdAsync(deviceInfos[0].Id);

            GpioPinInfo pinInfo;
            controller.Pins.TryGetValue(0, out pinInfo);
            pinInfo.TryOpenOutput(GpioPinValue.High, GpioSharingMode.Exclusive, out OutPin);
        }

        public int FlipLED()
        {
            if (LEDStatus == 0)
            {
                LEDStatus = 1;

                if (OutPin != null)
                {
                    OutPin.Value = GpioPinValue.Low;
                }
            }
            else
            {
                LEDStatus = 0;

                if(OutPin != null)
                {
                    OutPin.Value = GpioPinValue.High;
                }
            }

            return LEDStatus;
        }

        public int TurnOffLED()
        {
            if (LEDStatus == 1)
            {
                return FlipLED();
            }

            return 0;
        }
    }
}
