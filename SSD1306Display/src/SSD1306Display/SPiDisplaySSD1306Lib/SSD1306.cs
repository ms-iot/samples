using DisplayFont;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.Gpio;
using Windows.Devices.Spi;


namespace SPiDisplaySSD1306Lib
{
    public class SSD1306
    {
        #region const

        /* Definitions for SPI and GPIO */
        private SpiDevice SpiDisplay;
        private GpioController IoController;
        private GpioPin DataCommandPin;
        private GpioPin ResetPin;
        private SSD1306DeviceConfiguration DeviceConfig;


        public SSD1306()
        {
            var test = "";
        }


        #endregion const

        public async Task InitAll()
        {

            DeviceConfig = new SSD1306DeviceConfiguration();
            InitGpio();             /* Initialize the GPIO controller and GPIO pins */
            await InitSpi();        /* Initialize the SPI controller                */
            await InitDisplay();    /* Initialize the display                       */

        }

        /* Initialize the GPIO */

        public void InitGpio()
        {
            IoController = GpioController.GetDefault(); /* Get the default GPIO controller on the system */
            if (IoController == null)
            {
                throw new Exception("GPIO does not exist on the current system.");
            }

            /* Initialize a pin as output for the Data/Command line on the display  */
            DataCommandPin = IoController.OpenPin(SSD1306DeviceConfiguration.DATA_COMMAND_PIN);
            DataCommandPin.Write(GpioPinValue.High);
            DataCommandPin.SetDriveMode(GpioPinDriveMode.Output);

            /* Initialize a pin as output for the hardware Reset line on the display */
            ResetPin = IoController.OpenPin(SSD1306DeviceConfiguration.RESET_PIN);
            ResetPin.Write(GpioPinValue.High);
            ResetPin.SetDriveMode(GpioPinDriveMode.Output);
        }

        /* Send SPI commands to power up and initialize the display */

        public async Task InitDisplay()
        {
            /* Initialize the display */
            try
            {
                /* See the datasheet for more details on these commands: http://www.adafruit.com/datasheets/SSD1306.pdf             */
                await ResetDisplay();                   /* Perform a hardware reset on the display                                  */
                DisplaySendCommand(DeviceConfig.InitCommand());
            }
            catch (Exception ex)
            {
                throw new Exception("Display Initialization Failed", ex);
            }
        }


        /* Perform a hardware reset of the display */

        public async Task ResetDisplay()
        {
            ResetPin.Write(GpioPinValue.Low);   /* Put display into reset                       */
            await Task.Delay(1);                /* Wait at least 3uS (We wait 1mS since that is the minimum delay we can specify for Task.Delay() */
            ResetPin.Write(GpioPinValue.High);  /* Bring display out of reset                   */
            await Task.Delay(100);              /* Wait at least 100mS before sending commands  */
        }

        /* Send commands to the screen */

        public void DisplaySendCommand(byte[] Command)
        {
            /* When the Data/Command pin is low, SPI data is treated as commands for the display controller */
            DataCommandPin.Write(GpioPinValue.Low);
            SpiDisplay.Write(Command);
        }

        public void DisposeAll()
        {
            SpiDisplay.Dispose();
            ResetPin.Dispose();
            DataCommandPin.Dispose();
        }

        /* Initialize the SPI bus */

        public async Task InitSpi()
        {
            try
            {
                var settings = new SpiConnectionSettings(SSD1306DeviceConfiguration.SPI_CHIP_SELECT_LINE); /* Create SPI initialization settings                               */
                settings.ClockFrequency = 10000000;                             /* Datasheet specifies maximum SPI clock frequency of 10MHz         */
                settings.Mode = SpiMode.Mode3;                                  /* The display expects an idle-high clock polarity, we use Mode3
                                                                                 * to set the clock polarity and phase to: CPOL = 1, CPHA = 1
                                                                                 */

                var controller = await SpiController.GetDefaultAsync();
                SpiDisplay = controller.GetDevice(settings);  /* Create an SpiDevice with our bus controller and SPI settings */
            }
            /* If initialization fails, display the exception and stop running */
            catch (Exception ex)
            {
                throw new Exception("SPI Initialization Failed", ex);
            }
        }

        /* Sets all pixels in the screen buffer to 0 */

        public void ClearDisplayBuf()
        {
            Array.Clear(DeviceConfig.DisplayBuffer, 0, DeviceConfig.DisplayBuffer.Length);
        }

        /*
         * NAME:        WriteLineDisplayBuf
         * DESCRIPTION: Writes a string to the display screen buffer (DisplayUpdate() needs to be called subsequently to output the buffer to the screen)
         * INPUTS:
         *
         * Line:      The string we want to render. In this sample, special characters like tabs and newlines are not supported.
         * Col:       The horizontal column we want to start drawing at. This is equivalent to the 'X' axis pixel position.
         * Row:       The vertical row we want to write to. The screen is divided up into 4 rows of 16 pixels each, so valid values for Row are 0,1,2,3.
         *
         * RETURN VALUE:
         * None. We simply return when we encounter characters that are out-of-bounds or aren't available in the font.
         */

        public void WriteLineDisplayBuf(String Line, UInt32 Col, UInt32 Row)
        {

            UInt32 CharWidth = 0;
            foreach (Char Character in Line)
            {
                CharWidth = WriteCharDisplayBuf(Character, Col, Row);
                Col += CharWidth;   /* Increment the column so we can track where to write the next character   */
                if (CharWidth == 0) /* Quit if we encounter a character that couldn't be printed                */
                {
                    return;
                }
            }
        }

        public void InitBuffer()
        {
            DeviceConfig.InitBuffer();

        }



        /*
        * NAME:        WriteCharDisplayBuf
        * DESCRIPTION: Writes one character to the display screen buffer (DisplayUpdate() needs to be called subsequently to output the buffer to the screen)
        * INPUTS:
        *
        * Character: The character we want to draw. In this sample, special characters like tabs and newlines are not supported.
        * Col:       The horizontal column we want to start drawing at. This is equivalent to the 'X' axis pixel position.
        * Row:       The vertical row we want to write to. The screen is divided up into 4 rows of 16 pixels each, so valid values for Row are 0,1,2,3.
        *
        * RETURN VALUE:
        * We return the number of horizontal pixels used. This value is 0 if Row/Col are out-of-bounds, or if the character isn't available in the font.
        */
        public UInt32 WriteCharDisplayBuf(Char Chr, UInt32 Col, UInt32 Row)
        {
            /* Check that we were able to find the font corresponding to our character */
            FontCharacterDescriptor CharDescriptor = DisplayFontTable.GetCharacterDescriptor(Chr);
            if (CharDescriptor == null)
            {
                return 0;
            }

            /* Make sure we're drawing within the boundaries of the screen buffer */
            UInt32 MaxRowValue = (DeviceConfig.ScreenHeigthPages / DisplayFontTable.FontHeightBytes) - 1;
            UInt32 MaxColValue = DeviceConfig.ScreenWidthPx;
            if (Row > MaxRowValue)
            {
                return 0;
            }
            if ((Col + CharDescriptor.CharacterWidthPx + DisplayFontTable.FontCharSpacing) > MaxColValue)
            {
                return 0;
            }

            UInt32 CharDataIndex = 0;
            UInt32 StartPage = Row * 2;
            UInt32 EndPage = StartPage + CharDescriptor.CharacterHeightBytes;
            UInt32 StartCol = Col;
            UInt32 EndCol = StartCol + CharDescriptor.CharacterWidthPx;
            UInt32 CurrentPage = 0;
            UInt32 CurrentCol = 0;

            /* Copy the character image into the display buffer */
            for (CurrentPage = StartPage; CurrentPage < EndPage; CurrentPage++)
            {
                for (CurrentCol = StartCol; CurrentCol < EndCol; CurrentCol++)
                {
                    DeviceConfig.DisplayBuffer[CurrentCol, CurrentPage] = CharDescriptor.CharacterData[CharDataIndex];
                    CharDataIndex++;
                }
            }

            /* Pad blank spaces to the right of the character so there exists space between adjacent characters */
            for (CurrentPage = StartPage; CurrentPage < EndPage; CurrentPage++)
            {
                for (; CurrentCol < EndCol + DisplayFontTable.FontCharSpacing; CurrentCol++)
                {
                    DeviceConfig.DisplayBuffer[CurrentCol, CurrentPage] = 0x00;
                }
            }

            /* Return the number of horizontal pixels used by the character */
            return CurrentCol - StartCol;
        }


        /* Writes the Display Buffer out to the physical screen for display */

        public void DisplayUpdate()
        {

            int Index = 0;
            /* We convert our 2-dimensional array into a serialized string of bytes that will be sent out to the display */
            for (int PageY = 0; PageY < DeviceConfig.ScreenHeigthPages; PageY++)
            {
                for (int PixelX = 0; PixelX < DeviceConfig.ScreenWidthPx; PixelX++)
                {
                    DeviceConfig.SerializedDisplayBuffer[Index] = DeviceConfig.DisplayBuffer[PixelX, PageY];
                    Index++;
                }
            }

            /* Write the data out to the screen */
            DisplaySendCommand(SSD1306DeviceConfiguration.CMD_RESETCOLADDR);         /* Reset the column address pointer back to 0 */
            DisplaySendCommand(SSD1306DeviceConfiguration.CMD_RESETPAGEADDR);        /* Reset the page address pointer back to 0   */
            DisplaySendData(DeviceConfig.SerializedDisplayBuffer);     /* Send the data over SPI                     */
        }

        /* Send graphics data to the screen */

        public void DisplaySendData(byte[] Data)
        {
            /* When the Data/Command pin is high, SPI data is treated as graphics data  */
            DataCommandPin.Write(GpioPinValue.High);
            SpiDisplay.Write(Data);
        }

        public void InvertDisplay(bool _invert)
        {
            DisplaySendCommand(_invert ? SSD1306DeviceConfiguration.CMD_INVERTDISPLAY : SSD1306DeviceConfiguration.CMD_NORMALDISPLAY);

        }
    }
}
