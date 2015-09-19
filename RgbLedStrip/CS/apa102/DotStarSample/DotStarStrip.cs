// Copyright (c) Microsoft. All rights reserved.

namespace DotStarSample
{
    using System;
    using System.Collections.Generic;
    using System.Threading.Tasks;
    using Windows.Devices.Enumeration;
    using Windows.Devices.Spi;
    using Windows.UI;

    public class DotStarStrip
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
        /// Settings for the SPI connection to talk with DotStar (APA102) SPI leds.
        /// </summary>
        private readonly SpiConnectionSettings settings = new SpiConnectionSettings(SpiChipSelectLine)
        {
            // SPI clock speed in Hz.  Super brief testing worked fine anywhere as low as 40khz (below about 200khz was noticeably slow), all the way
            // up to 16mhz (above 16mhz started to get corrupted data towards the end of the strip).  10mhz is probably a good baseline value to use
            // unless you run in to problems.  :)
            ClockFrequency = 10000000,
            // APA102/DotStar uses SPI mode 3, CPOL = 1 (clock is high when inactive), CPHA = 1 (data is valid on the clock's trailing edge)
            Mode = SpiMode.Mode3,
            DataBitLength = 8
        };

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="numLeds">Number of LEDs in the strip</param>
        public DotStarStrip(int numLeds)
        {
            this.PixelCount = numLeds;

            // The actual logic here is that we need to send a zero to be sent for every 16 pixels in the strip, to signify the end
            // of the color data and reset the addressing.
            int endFrameSize = (this.PixelCount + 14) / 16;

            // By initializing an int array of that specific length, it gets initialized with ints of default value (0).  :)
            this.endFrame = new byte[endFrameSize];
        }

        private byte[] startFrame = { 0, 0, 0, 0 };

        private byte[] endFrame;

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
            spiDataBytes.AddRange(this.startFrame);

            foreach (var pixel in pixels)
            {
                // Global brightness.  Not implemented currently.  0xE0 (binary 11100000) specifies the beginning of the pixel's
                // color data.  0x1F (binary 00011111) specifies the global brightness.  If you want to actually use this functionality
                // comment out this line and uncomment the next one.  Then the pixel's RGB value will get scaled based on the alpha
                // channel value from the Color.
                spiDataBytes.Add(0xE0 | 0x1F); 
                //spiDataBytes.Add((byte)(0xE0 | (byte)(pixel.A >> 3)));

                // APA102/DotStar leds take the color data in Blue, Green, Red order.  Weirdly, according to the spec these are supposed
                // to take a 0-255 value for R/G/B.  However, on the ones I have they only seem to take 0-126.  Specifying 127-255 doesn't
                // break anything, but seems to show the same exact value 0-126 would have (i.e. 127 is 0 brightness, 255 is full brightness).
                // Discarding the lowest bit from each to make the value fit in 0-126.
                spiDataBytes.Add((byte)(pixel.B >> 1));
                spiDataBytes.Add((byte)(pixel.G >> 1));
                spiDataBytes.Add((byte)(pixel.R >> 1));
            }

            spiDataBytes.AddRange(this.endFrame);

            this.spiDevice.Write(spiDataBytes.ToArray());
        }

        /// <summary>
        /// Gets the SpiDevice handle
        /// </summary>
        /// <returns>Task of type SpiDevice, whose result will be the SpiDevice requested if successful</returns>
        private async Task<SpiDevice> getSpiDevice()
        {
            string spiSelector = SpiDevice.GetDeviceSelector(SpiControllerName);
            DeviceInformationCollection devicesInfo = await DeviceInformation.FindAllAsync(spiSelector);
            return await SpiDevice.FromIdAsync(devicesInfo[0].Id, this.settings);
        }
    }
}
