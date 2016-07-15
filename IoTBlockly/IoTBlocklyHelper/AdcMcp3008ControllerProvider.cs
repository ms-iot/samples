using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Devices.Adc.Provider;
using Windows.Devices.Enumeration;
using Windows.Devices.Spi;

namespace Microsoft.IoT.AdcMcp3008
{
    class AdcMcp3008ControllerProvider : IAdcControllerProvider
    {
        // Our bus interface to the chip
        private SpiDevice spiController;

        const byte MCP3008_ChannelCount = 8;
        const int MCP3008_ResolutionInBits = 10;
        const int MCP3008_MinValue = 0;
        const int MCP3008_MaxValue = 1023;

        // ADC chip operation constants
        const byte MCP3008_SingleEnded = 0x08;

        const int DEFAULT_SPI_CHIP_SELECT_LINE = 0;  // SPI0 CS0 pin 24 on the RPi2

        static public int DefaultChipSelectLine
        {
            get { return DEFAULT_SPI_CHIP_SELECT_LINE; }
        }


        const int MCP3008_Clock = 1350000; 

        private readonly Task _initializingTask;
        public AdcMcp3008ControllerProvider(int chipSelectLine) : base()
        {
            _initializingTask = Init(chipSelectLine);
        }

        private async Task Init(int chipSelectLine)
        { 
            try
            {
                // Setup the SPI bus configuration
                var settings = new SpiConnectionSettings(chipSelectLine);

                settings.ClockFrequency = MCP3008_Clock;
                settings.Mode = SpiMode.Mode0;

                // Ask Windows for the list of SpiDevices


                var controller = await SpiController.GetDefaultAsync();
                spiController = controller.GetDevice(settings);

                if (spiController == null)
                {
                    Debug.WriteLine(
                        "SPI Controller is currently in use by another application. Please ensure that no other applications are using SPI.");
                    throw new Exception();
                }

            }
            catch (Exception e)
            {
                Debug.WriteLine("Exception: " + e.Message + "\n" + e.StackTrace);
                throw;
            }

        }

        public int ChannelCount
        {
            get { return MCP3008_ChannelCount; }
        }

        ProviderAdcChannelMode channelMode = ProviderAdcChannelMode.SingleEnded;
        public ProviderAdcChannelMode ChannelMode
        {
            get { return channelMode; }
            set { channelMode = value;  }
        }

        public int MaxValue
        {
            get { return MCP3008_MaxValue; }
        }

        public int MinValue
        {
            get { return MCP3008_MinValue; }
        }

        public int ResolutionInBits
        {
            get { return MCP3008_ResolutionInBits; }
        }

        public bool IsChannelModeSupported(ProviderAdcChannelMode channelMode)
        {
            return (channelMode == ProviderAdcChannelMode.SingleEnded || channelMode == ProviderAdcChannelMode.Differential);
        }

        public int ReadValue(int channelNumber)
        {
            if (spiController == null)
            {
                _initializingTask.Wait();
            }
            byte command = (byte)channelNumber;
            if (channelMode == ProviderAdcChannelMode.SingleEnded)
            {
                command |= MCP3008_SingleEnded;
            }
            command <<= 4;

            byte[] commandBuf = new byte[] { 0x01, command, 0x00 };

            byte[] readBuf = new byte[] { 0x00, 0x00, 0x00 };

            spiController.TransferFullDuplex(commandBuf, readBuf);

            int sample = readBuf[2] + ((readBuf[1] & 0x03) << 8);

            return sample;
        }

        uint channelStatus;

        public void AcquireChannel(int channel)
        {
            uint oldChannelStatus = channelStatus;
            uint channelToAquireFlag = (uint)(1 << channel);

            // See if the channel is available
            if ((oldChannelStatus & channelToAquireFlag) == 0)
            {
                // Not currently acquired
                channelStatus |= channelToAquireFlag;
            }
            else
            {
                // Already acquired, throw an exception
                throw new UnauthorizedAccessException();
            }
        }

        public void ReleaseChannel(int channel)
        {
            uint oldChannelStatus = channelStatus;
            uint channelToAquireFlag = (uint)(1 << channel);

            channelStatus &= ~channelToAquireFlag;
        }
    }
}
