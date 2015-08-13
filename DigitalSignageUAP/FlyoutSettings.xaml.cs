/**
    Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.
   The MIT License(MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
**/

using System;
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
using Windows.Storage;

// The Settings Flyout item template is documented at http://go.microsoft.com/fwlink/?LinkId=273769

namespace DigitalSignageUAP
{
    /// <summary>
    /// A customized SettingsFlyout control to set the config file path for ADMIN
    /// </summary>
    public sealed partial class FlyoutSettings : Flyout
    {
        bool noSettingYet = false;
        string oldConfigSetting;
        ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
        string configValueName = "ConfigFilePath";
        private double OriginalFlyoutWidth;

        public FlyoutSettings()
        {
            this.InitializeComponent();
            
            SIP_TextBox.RegisterEditControl(textBox);
        }

        /// <summary>
        /// Save the new settings
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SettingsFlyout1_Unloaded(object sender, RoutedEventArgs e)
        {
            localSettings.Values[configValueName] = configFileTextBoxValue;
        }

        /// <summary>
        /// Load current settings from App settings
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void SettingsFlyout1_Loaded(object sender, RoutedEventArgs e)
        {
            if (localSettings.Values[configValueName] == null) // we don't have this configured yet
            {
                configFileTextBoxValue = "";
                noSettingYet = true;
            }
            else
            {
                configFileTextBoxValue = (string)localSettings.Values[configValueName];
                oldConfigSetting = configFileTextBoxValue;
            }
        }

        /// <summary>
        /// A string that's bound to the config textbox value
        /// </summary>
        public string configFileTextBoxValue
        {
            get { return SIP_TextBox.OutputString; }
            set { SIP_TextBox.OutputString = value; }
        }

        private void OSK_Button_Click(object sender, RoutedEventArgs e)
        {
            if(SIP_TextBox.Visibility == Visibility.Collapsed)
            {
                SIP_TextBox.Visibility = Visibility.Visible;
            }
            else
            {
                SIP_TextBox.Visibility = Visibility.Collapsed;
            }

            /* Need to set focus so the on screen keyboard's content buffer gets updated */
            textBox.Focus(FocusState.Programmatic);
        }

        private void closeButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            FlyoutStackpanel.Hide();
        }
    }
}
