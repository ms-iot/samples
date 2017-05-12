using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Gpio;

namespace LEDTube74HC595
{
    public class LED74HC595Driver
    {
        #region Pre-defined LED Hex Values

        private byte[] LEDDIGITS =
        {
            // 0    1    2    3    4	5	 6	  7	   8    9
    	    0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90
        };

        private byte PERIOD = 0x7F; // .
        private byte LEDTESTCHAR = 0x00; // 8.
        private byte UNDERSCORE = 0xF7; // _
        private byte HYPHEN = 0xBF; // -
        private byte OVERSCORE = 0xFE; // 上划线
        private byte BLANK = 0xFF; // 空

        //0x83 // b
        //0x86 // E
        //0x88 // A
        //0x89 // H
        //0x8b // h
        //0x8c // P
        //0x8E // F
        //0x91 // y
        //0x98 // q
        //0x9c // o+
        //0x9E // c+
        //0xA1 // d
        //0xA3 // o-
        //0xA7 // c-
        //0xC1 // U
        //0xC6 // C
        //0xC7 // L
        //0xC8 // n
        //0xDC // n+
        //0xE3 // u

        #endregion

        #region GPIO Pins

        /// <summary>
        /// Serial Digital Input
        /// </summary>
        public GpioPin PinDIO { get; set; }

        /// <summary>
        /// Register Clock
        /// </summary>
        public GpioPin PinRCLK { get; set; }

        /// <summary>
        /// Serial Clock
        /// </summary>
        public GpioPin PinSCLK { get; set; }

        #endregion

        /// <summary>
        /// Create a new instance of LED74HC595Driver
        /// </summary>
        /// <param name="dioPin">DIO Pin Number</param>
        /// <param name="rclkPin">RCLK Pin Number</param>
        /// <param name="sclkPin">SCLK Pin Number</param>
        public LED74HC595Driver(int dioPin, int rclkPin, int sclkPin)
        {
            var gpio = GpioController.GetDefault();

            // setup the pins
            PinDIO = gpio.OpenPin(dioPin);
            PinDIO.SetDriveMode(GpioPinDriveMode.Output);

            PinRCLK = gpio.OpenPin(rclkPin);
            PinRCLK.SetDriveMode(GpioPinDriveMode.Output);

            PinSCLK = gpio.OpenPin(sclkPin);
            PinSCLK.SetDriveMode(GpioPinDriveMode.Output);

            // initialize the pins to low
            PinDIO.Write(GpioPinValue.Low);
            PinRCLK.Write(GpioPinValue.Low);
            PinSCLK.Write(GpioPinValue.Low);
        }

        /// <summary>
        /// Test every LED on the Tube
        /// </summary>
        /// <param name="round">How many test runs</param>
        /// <returns>Task</returns>
        public async Task TestTubeAsync(int round)
        {
            for (int i = 0; i < round + 1; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    DisplayChar(j, LEDTESTCHAR);
                    await Task.Delay(200);
                }
            }
        }

        /// <summary>
        /// Display Time on the Tube
        /// </summary>
        /// <param name="time">Time</param>
        public void DisplayTime(DateTime time)
        {
            string timeStr = time.ToString("HHmm");
            int digit = int.Parse(timeStr);

            DisplayDigits(digit);
            DisplayChar(2, PERIOD);
        }

        public void DisplayDigitsF(double number)
        {
            var zhenShu = Math.Truncate(number);
            var xiaoShu = (float)(number - zhenShu);

            // performance issue?
            var numStr = number.ToString();
            int weiShu = numStr.Length - numStr.IndexOf('.') - 1;

            var digits = (int)Math.Round((zhenShu * (int)(Math.Pow(10, weiShu)) + xiaoShu * ((int)Math.Pow(10, weiShu))));

            //Debug.WriteLine($"zhenShu: {zhenShu}, xiaoShu: {xiaoShu}, weiShu: {weiShu}, digits: {digits}");

            DisplayDigits(digits, weiShu);
        }

        /// <summary>
        /// Display 1-4 Digital Numbers on the Tube
        /// </summary>
        /// <param name="number">Positive Number (0-9999)</param>
        public void DisplayDigits(int number, int floatIndex = 0)
        {
            if (number < 0 || number > 9999)
            {
                throw new ArgumentOutOfRangeException(nameof(number), "number must be between 0-9999");
            }

            // 老司机写法
            var numStr = number.ToString();
            for (int i = 0; i < numStr.Length; i++)
            {
                var pos = numStr[numStr.Length - i - 1] - '0';
                DisplayChar(i, LEDDIGITS[pos]);
            }

            DisplayChar(floatIndex, PERIOD);

            // 新司机写法

            //// 个位
            //var ge = GetDigitPos(number, 0);

            //// 十位
            //var shi = GetDigitPos(number, 1);

            //// 百位
            //var bai = GetDigitPos(number, 2);

            //// 千位
            //var qian = GetDigitPos(number, 3);

            //DisplayChar(0, LEDDIGITS[ge]);
            //DisplayChar(1, LEDDIGITS[shi]);
            //DisplayChar(2, LEDDIGITS[bai]);
            //DisplayChar(3, LEDDIGITS[qian]);
        }

        /// <summary>
        /// Display A Character on the Tube
        /// </summary>
        /// <param name="index">Tube 0-3, from right to left</param>
        /// <param name="b">character byte (HEX value)</param>
        public void DisplayChar(int index, byte b)
        {
            if (index < 0 || index > 3)
            {
                throw new ArgumentOutOfRangeException(nameof(index), "index just be between 0-3.");
            }

            int i = 0x01;
            i = i << index;

            WriteSIPO(b);
            WriteSIPO((byte)i);
            PulseRCLK();
        }

        private int GetDigitPos(int number, int index)
        {
            return number / (int)Math.Pow(10, index) % 10;
        }

        #region GPIO Private Functions

        // Serial-In-Parallel-Out
        private void WriteSIPO(byte b)
        {
            for (int i = 8; i >= 1; i--)
            {
                PinDIO.Write((b & 0x80) > 0 ? GpioPinValue.High : GpioPinValue.Low);
                b <<= 1;
                PulseSCLK();
            }
        }

        // Pulse Register Clock
        private void PulseRCLK()
        {
            PinRCLK.Write(GpioPinValue.Low);
            PinRCLK.Write(GpioPinValue.High);
        }

        // Pulse Serial Clock
        private void PulseSCLK()
        {
            PinSCLK.Write(GpioPinValue.Low);
            PinSCLK.Write(GpioPinValue.High);
        }

        #endregion
    }
}
