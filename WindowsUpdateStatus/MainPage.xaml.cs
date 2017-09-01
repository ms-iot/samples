using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Threading.Tasks;
using Microsoft.Devices.Management;
using Windows.Foundation.Diagnostics;


namespace WindowsUpdateStatus
{
    /// <summary>
    /// Get the windows update status.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        DeviceManagement deviceManagement;
        public MainPage()
        {
            this.InitializeComponent();
            this.InitializeDeviceClient();
            windowsUpdateHeader.Visibility = Visibility.Visible;
            windowsUpdateStatus.Visibility = Visibility.Visible;
            WindowsUpdateStatus();
        }

        private string Parser(string input)
        {
            return input.Replace("/", ", ");
        }
        private async void WindowsUpdateStatus()
        {
            DeviceManagement.WindowsUpdateStatus status = await deviceManagement.ReportWindowsUpdateStatusAsync();

            if(status.lastScanTime != "")
            {
                Status.Text = "No updates available.";
                
                LastScanTime.Text = "Last checked time for updates : " + Parser(status.lastScanTime);
            }

            if (status.deferUpgrade)
            {
                DeferUpgrade.Text = "Upgrades deferred until the next period."; 
            }

            if(status.installable != "")
            {
                Status.Text = "An update is ready to be installed.";
                InstallableUpdates.Text = "Your device is not up to date. The following update is installable: " + Parser(status.installable);
            }

            if(status.approved != "")
            {
                Approved.Text = "Updates have been approved. Update Information : " + Parser(status.approved);
            }

            if(status.installed != "")
            {
                Status.Text = "Update installed successfully.";
                Installed.Text = "Last successfully installed update : " + Parser(status.installed);
            }

            if (status.failed != "")
            {
                Status.Text = "Device failed to update successfully";
                FailedUpdates.Text = "Update Failed. Failure Information : " + Parser(status.failed);
            }

            if (status.pendingReboot != "")
            {
                Status.Text = "Device is pending reboot.";
                PendingReboot.Text = "Reboot is pending for the following upgrade: " + Parser(status.pendingReboot);
            }

        }
        private async void Check_WindowsUpdateAsync(object sender, RoutedEventArgs e)
        {
            windowsUpdateStatus.Visibility = Visibility.Collapsed;
            Status.Text = "Checking update status...";

            WindowsUpdateStatus();

            await Task.Delay(TimeSpan.FromSeconds(1));
            windowsUpdateStatus.Visibility = Visibility.Visible;
        }
       
        private void InitializeDeviceClient()
        {
            try
            {
                // Create the DeviceManagement, the main entry point into device management without azure connection
                var newDeviceManagement = DeviceManagement.CreateWithoutAzure();
                this.deviceManagement = newDeviceManagement;
            }
            catch (Exception e)
            {
                var msg = "Exception: " + e.Message + "\n" + e.StackTrace;
                System.Diagnostics.Debug.WriteLine(msg);
                Logger.Log(msg, LoggingLevel.Error);
            }
            
        }

    }
}
