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
    public sealed partial class SettingsFlyout1 : SettingsFlyout
    {
        bool noSettingYet = false;
        string oldConfigSetting;
        ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
        string configValueName = "ConfigFilePath";
        private double OriginalFlyoutWidth;

        public SettingsFlyout1()
        {
            this.InitializeComponent();
            
            Loaded += SettingsFlyout1_Loaded;
            Unloaded += SettingsFlyout1_Unloaded;

            OriginalFlyoutWidth = this.Width;
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
            if (noSettingYet || oldConfigSetting != configFileTextBoxValue) // if there was no setting when it's launched, or when the text is changed, we fire a settings change event!
            {
                TelemetryHelper.eventLogger.Write(TelemetryHelper.SettingsChangedEvent, TelemetryHelper.TelemetryInfoOption);
            }
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
                FlyoutStackpanel.Width = SIP_TextBox.Width;
                SIP_TextBox.Visibility = Visibility.Visible;
            }
            else
            {
                FlyoutStackpanel.Width = OriginalFlyoutWidth;
                SIP_TextBox.Visibility = Visibility.Collapsed;
            }

            /* Need to set focus so the on screen keyboard's content buffer gets updated */
            textBox.Focus(FocusState.Programmatic);
        }
    }
}
