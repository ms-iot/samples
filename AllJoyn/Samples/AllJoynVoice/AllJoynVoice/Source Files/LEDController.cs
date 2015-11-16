// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Collections.Generic;
using Windows.Devices.Gpio;

namespace AllJoynVoice
{
    public enum StatusLED
    {
        Idle,
        Listening,
        Speaking,
    };

    /// <summary>
    /// Handles turning on and on LEDs connected to GPIO pins.
    /// </summary>
    class LEDController
    {
        /// <summary>
        /// Maps an LED to its GPIO pin number.
        /// </summary>
        static private Dictionary<StatusLED, Int32> LEDNumbers = new Dictionary<StatusLED, Int32> {
            { StatusLED.Idle, 27 },
            { StatusLED.Listening, 22 },
            { StatusLED.Speaking, 4 },
        };

        /// <summary>
        /// Holds the opened pins for all LEDs.
        /// </summary>
        static private Dictionary<StatusLED, GpioPin> LEDs;

        /// <summary>
        /// Opens the required pins and sets them to output mode.
        /// Must be called before interacting with the LEDs.
        /// </summary>
        public static void Init()
        {
            GpioController controller = GpioController.GetDefault();

            if (controller == null)
            {
                Logger.LogError("This platform does not have a Gpio Controller.");
                return;
            }

            LEDs = new Dictionary<StatusLED, GpioPin>();

            foreach (KeyValuePair<StatusLED, Int32> pin in LEDNumbers)
            {
                try
                {
                    LEDs[pin.Key] = controller.OpenPin(pin.Value);
                    LEDs[pin.Key].SetDriveMode(GpioPinDriveMode.Output);
                    LEDs[pin.Key].Write(GpioPinValue.Low);
                }
                catch (System.IO.IOException ex)
                {
                    Logger.LogException(string.Format("Failed to open the {0} LED.", pin.Key), ex);
                }
            }
        }

        /// <summary>
        /// Turns on one LED and the rest off.
        /// </summary>
        /// <param name="pin">The LED to be turned on.</param>
        public static void SingleLED(StatusLED led) {
            foreach (KeyValuePair<StatusLED, GpioPin> pin in LEDs)
            {
                LEDWrite(led, pin.Key == led ? GpioPinValue.High : GpioPinValue.Low);
            }
        }

        /// <summary>
        /// Writes a value to an LED.
        /// </summary>
        /// <param name="led">The LED in question.</param>
        /// <param name="value">The value to be written.</param>
        public static void LEDWrite(StatusLED led, GpioPinValue value)
        {
            try
            {
                Logger.LogInfo(string.Format("Setting {0} LED value to {1}", led, value));
                LEDs[led].Write(value);
            }
            catch (Exception ex)
            {
                Logger.LogException(string.Format("Failed to update the {0} LED.", led), ex);
            }
        }

        public static void TurnOn(StatusLED led)
        {
            LEDWrite(led, GpioPinValue.High);
        }

        public static void TurnOff(StatusLED led)
        {
            LEDWrite(led, GpioPinValue.Low);
        }
    }
}
