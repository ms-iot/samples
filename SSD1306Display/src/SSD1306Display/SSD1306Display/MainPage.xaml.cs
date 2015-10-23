// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Devices.Enumeration;
using Windows.Devices.Spi;
using Windows.Devices.Gpio;
using SPiDisplaySSD1306Lib;




// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace SSD1306Display
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        SSD1306 ssd1306 = new SSD1306();

        public MainPage()
        {
            this.InitializeComponent();

            /* Register for the unloaded event so we can clean up upon exit */
            Unloaded += MainPage_Unloaded;

            /* Initialize GPIO, SPI, and the display */
            InitAll();
        }

        /* Initialize GPIO, SPI, and the display */
        private async void InitAll()
        {
            

            Display_TextBoxLine0.IsEnabled = false;
            Display_TextBoxLine1.IsEnabled = false;
            Display_TextBoxLine2.IsEnabled = false;
            Display_TextBoxLine3.IsEnabled = false;

            await ssd1306.InitAll();

            Display_TextBoxLine0.IsEnabled = true;
            Display_TextBoxLine1.IsEnabled = true;
            Display_TextBoxLine2.IsEnabled = true;
            Display_TextBoxLine3.IsEnabled = true;

            /* Register a handler so we update the SPI display anytime the user edits a textbox */
            Display_TextBoxLine0.TextChanged += Display_TextBox_TextChanged;
            Display_TextBoxLine1.TextChanged += Display_TextBox_TextChanged;
            Display_TextBoxLine2.TextChanged += Display_TextBox_TextChanged;
            Display_TextBoxLine3.TextChanged += Display_TextBox_TextChanged;

            /* Manually update the display once after initialization*/
            DisplayTextBoxContents();

            Text_Status.Text = "Status: Initialized";
        }


        /* Update the SPI display to mirror the textbox contents */
        private void DisplayTextBoxContents()
        {
            try
            {
                ssd1306.ClearDisplayBuf();  /* Blank the display buffer             */

                ssd1306.WriteLineDisplayBuf(Display_TextBoxLine0.Text, 0, 0);
                ssd1306.WriteLineDisplayBuf(Display_TextBoxLine1.Text, 0, 1);
                ssd1306.WriteLineDisplayBuf(Display_TextBoxLine2.Text, 0, 2);
                ssd1306.WriteLineDisplayBuf(Display_TextBoxLine3.Text, 0, 3);
                ssd1306.DisplayUpdate();    /* Write our changes out to the display */
            }
            /* Show an error if we can't update the display */
            catch (Exception ex)
            {
                Text_Status.Text = "Status: Failed to update display";
                Text_Status.Text = "\nException: " + ex.Message;
            }
        }

        /* Updates the display when the textbox contents change */
        private void Display_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            DisplayTextBoxContents();
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            /* Cleanup */
            ssd1306.DisposeAll();
        }
    }
}