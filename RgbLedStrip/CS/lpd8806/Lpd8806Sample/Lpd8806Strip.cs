// Copyright (c) Microsoft. All rights reserved.

namespace Lpd8806Sample
{
    using System;
    using System.Collections.Generic;
    using System.Threading.Tasks;
    using Windows.Devices.Enumeration;
    using Windows.Devices.Spi;
    using Windows.UI;

    public class Lpd8806Strip
    {
        /// <summary>
        /// Name of the SPI controller to use for communication.
        /// </summary>
        /// <remarks>Hardcoding to SPI1 because it generally has a cleaner signal.  Could use SPI0 at a lower frequency, though.</remarks>
        private const string SpiControllerName = "SPI1";

        /// <summary>
        /// Line the SPI library uses to signal chip select.
        /// </summary>
        /// <remarks>The APA102 doesn't actually support this line, so safe to ignore this.</remarks>
        private const int SpiChipSelectLine = 0;

        /// <summary>
        /// Object for communicating with the LED strip.
        /// </summary>
        private SpiDevice spiDevice;

        /// <summary>
        /// Gets a value representing the count of pixels in the LED strip.
        /// </summary>
        public int PixelCount { get; private set; }

        /// <summary>
        /// Settings for the SPI connection to talk with LPD8806 SPI leds.
        /// </summary>
        private readonly SpiConnectionSettings settings = new SpiConnectionSettings(SpiChipSelectLine)
        {
            // SPI clock speed in Hz.  Super brief testing worked fine anywhere as low as 40khz (below about 200khz was noticeably slow), all the way
            // up to 30mhz (above 30mhz the color data got completely corrupted).  10mhz is probably a good baseline value to use
            // unless you run in to problems or need more perf.  :)
            ClockFrequency = 10000000,
            // LPD8806 uses SPI mode 0, CPOL = 0 (clock is low when inactive), CPHA = 0 (data is valid on the clock's leading edge)
            Mode = SpiMode.Mode0,
            DataBitLength = 8
        };

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="pixelCount">Number of LEDs in the strip</param>
        public Lpd8806Strip(int pixelCount)
        {
            this.PixelCount = pixelCount;

            // The actual logic here is that we need a zero to be sent for every 32 pixels in the strip to 'reset' the 
            // pixel addressing once we're done sending down the color data.  
            int frameResetSize = ((this.PixelCount + 31) / 32);

            // By initializing an int array of that specific length, it gets initialized with ints of default value (0).  :)
            this.frameResetBytes = new byte[frameResetSize];
        }

        /// <summary>
        /// Byte array representing the start and end of a frame of color data. 
        /// </summary>
        private byte[] frameResetBytes;

        /// <summary>
        /// Initializes the SPI connection to the strip
        /// </summary>
        /// <returns>Task representing the async action</returns>
        public async Task Begin()
        {
            this.spiDevice = await this.getSpiDevice();
        }

        /// <summary>
        /// Sends a bunch of colors out to the LED strip
        /// </summary>
        /// <param name="pixels">List of <see cref="Color"/>s to send to the strip.</param>
        public void SendPixels(List<Color> pixels)
        {
            List<byte> spiDataBytes = new List<byte>();
            spiDataBytes.AddRange(this.frameResetBytes);

            foreach (var pixel in pixels)
            {
                // Color data for the LPD8806 based strips consists of 7bit color values with the high bit set to one.
                // To get that, we shift the byte left one, discarding the least significant bit, and then OR with
                // 0x80 (binary 10000000) to force the high bit to one.
                spiDataBytes.Add((byte)((pixel.G >> 1) | 0x80));
                spiDataBytes.Add((byte)((pixel.R >> 1) | 0x80));
                spiDataBytes.Add((byte)((pixel.B >> 1) | 0x80));
            }

            spiDataBytes.AddRange(this.frameResetBytes);

            this.spiDevice.Write(spiDataBytes.ToArray());
        }
        
        /// <summary>
        /// Gets the actual SpiDevice handle
        /// </summary>
        /// <returns>Task of type SpiDevice, whose result will be the SpiDevice requested if successful</returns>
        private async Task<SpiDevice> getSpiDevice()
        {
            var controller = await SpiController.GetDefaultAsync(); 
            return  controller.GetDevice(this.settings);
        }
    }
}
