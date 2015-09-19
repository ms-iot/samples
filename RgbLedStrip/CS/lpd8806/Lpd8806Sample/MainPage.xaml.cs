// Copyright (c) Microsoft. All rights reserved.

namespace Lpd8806Sample
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading.Tasks;
    using Windows.Foundation;
    using Windows.UI;
    using Windows.UI.Xaml;
    using Windows.UI.Xaml.Controls;

    public sealed partial class MainPage : Page
    {
        private Lpd8806Strip pixelStrip;
        private IAsyncAction fancyAction;
        private bool fancyActionGo = false;
        private byte brightness = 0;
        private bool brightnessRising = true;

        public MainPage()
        {
            this.InitializeComponent();

            // Construct the object for the LED strip.  Change the 120 to however many "pixels" you have.
            this.pixelStrip = new Lpd8806Strip(120);

            this.Loaded += this.MainPage_Loaded;
        }

        private async void MainPage_Loaded(object sender, RoutedEventArgs eventArgs)
        {
            try
            {
                // Initialize the led strip
                await this.pixelStrip.Begin();
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.ToString());
            }
        }

        /// <summary>
        /// Simple button to generate a bunch of colors and send them to the LED strip
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void button_Click(object sender, RoutedEventArgs e)
        {
            List<Color> pixelColors = new List<Color>();

            // Fill the list with pixel values in a test pattern (WHITE RED GREEN BLUE, repeating)
            for (int i = 0; i < this.pixelStrip.PixelCount / 4; i++)
            {
                pixelColors.Add(Color.FromArgb(255, 255, 255, 255)); // White
                pixelColors.Add(Color.FromArgb(255, 255, 0, 0));     // Red
                pixelColors.Add(Color.FromArgb(255, 0, 255, 0));     // Green
                pixelColors.Add(Color.FromArgb(255, 0, 0, 255));     // Blue
            }

            this.pixelStrip.SendPixels(pixelColors);
        }

        /// <summary>
        /// When the fancy colors button gets toggled on, starts an animation pulsing the brightness
        /// of all the LEDs in the strip.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void FancyColorsButton_Checked(object sender, RoutedEventArgs e)
        {
            Task.Run(() =>
            {
                this.fancyActionGo = true;
                while (this.fancyActionGo)
                {
                    List<Color> pixelColors = new List<Color>();

                    // Fill the list with pixel values in a test pattern (WHITE RED GREEN BLUE, repeating)
                    for (int i = 0; i < this.pixelStrip.PixelCount / 4; i++)
                    {
                        pixelColors.Add(Color.FromArgb(255, brightness, brightness, brightness)); // White
                        pixelColors.Add(Color.FromArgb(255, brightness, 0, 0));                   // Red
                        pixelColors.Add(Color.FromArgb(255, 0, brightness, 0));                   // Green
                        pixelColors.Add(Color.FromArgb(255, 0, 0, brightness));                   // Blue
                    }

                    this.pixelStrip.SendPixels(pixelColors);
                    if (brightnessRising)
                    {
                        brightness = (byte)(brightness + 1);

                        if (brightness == 255)
                        {
                            brightnessRising = false;
                        }
                    }
                    else
                    {
                        brightness = (byte)(brightness - 1);

                        if (brightness == 0)
                        {
                            brightnessRising = true;
                        }
                    }
                    
                    pixelColors = null;
                }
            });
        }

        private void FancyColorsButton_Unchecked(object sender, RoutedEventArgs e)
        {
            this.fancyActionGo = false;
        }
    }
}
