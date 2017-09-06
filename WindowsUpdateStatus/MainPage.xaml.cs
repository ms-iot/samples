// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Threading.Tasks;
using Microsoft.Devices.Management;
using System.Diagnostics; 

namespace WindowsUpdateStatus
{
    /// <summary>
    /// Get the windows update status.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        DeviceManagement deviceManagement;

        //To ensure that the "Update installed successfully." status shows up only once. 
        bool successfulUpdateNotification = false;
        public MainPage()
        {
            this.InitializeComponent();
            this.InitializeDeviceClient();

#pragma warning disable 4014
            this.WindowsUpdateStatusAsync();
#pragma warning restore 4014
        }

        private string ConvertToDisplayString(string input)
        {
            return input.Replace("/", ", ");
        }

        private async Task WindowsUpdateStatusAsync()
        {
            DeviceManagement.WindowsUpdateStatus status = await this.deviceManagement.ReportWindowsUpdateStatusAsync();

            if(status.lastScanTime != "")
            {
                Status.Text = "Your device is up to date. No installable updates available.";
                
                LastScanTime.Text = "Last checked time for updates : " + status.lastScanTime;
            }

            if (status.deferUpgrade)
            {
                DeferUpgrade.Text = "Upgrades deferred until the next period."; 
            }
            else
            {
                DeferUpgrade.Text = "Upgrades not deferred until the next period.";
            }

            if(status.installable != "")
            {
                Status.Text = "An update is ready to be installed.";
                InstallableUpdates.Text = "Your device is not up to date. The following update is installable: " + ConvertToDisplayString(status.installable);
            }

            if(status.approved != "")
            {
                Approved.Text = "Approved updates. Update Information : " + ConvertToDisplayString(status.approved);
            }

            if (status.installed != "")
            {
                if (!successfulUpdateNotification)
                {
                    Status.Text = "Update installed successfully.";
                    successfulUpdateNotification = true;
                }
                Installed.Text = "Last successfully installed update : " + ConvertToDisplayString(status.installed);
            }

            if (status.failed != "")
            {
                Status.Text = "Device failed to update successfully.";
                FailedUpdates.Text = "Update Failed. Failure Information : " + ConvertToDisplayString(status.failed);
            }

            if (status.pendingReboot != "")
            {
                Status.Text = "Device is pending reboot.";
                PendingReboot.Text = "Reboot is pending for the following upgrade: " + ConvertToDisplayString(status.pendingReboot);
                successfulUpdateNotification = false;
            }

        }
        private async void CheckWindowsUpdateAsync_click(object sender, RoutedEventArgs e)
        {
            windowsUpdateStatus.Visibility = Visibility.Collapsed;
            Status.Text = "Checking update status...";

            await WindowsUpdateStatusAsync();

            await Task.Delay(TimeSpan.FromSeconds(1));
            windowsUpdateStatus.Visibility = Visibility.Visible;
        }
       
        private void InitializeDeviceClient()
        {
            try
            {
                // Create the DeviceManagement, the main entry point into device management without azure connection
                this.deviceManagement = DeviceManagement.CreateWithoutAzure();
            }
            catch (Exception e)
            {
                var msg = "Exception: " + e.Message + "\n" + e.StackTrace;
                Debug.WriteLine(msg);
            }
            
        }

    }
}
