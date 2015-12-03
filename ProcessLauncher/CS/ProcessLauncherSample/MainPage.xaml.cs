using System;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;

/* 
* Notes:
*   - Executables packaged as part of the AppX are allowed to run by default.
      An example of such apps is SampleConsoleApp.exe which is included in the sample solution.
*
*   - System or any other app not packaged are also allowed to execute by adding it to the allowed list, AllowedExecutableFilesList, 
*     using an absolute path; e.g. c:\mypath\myapp.exe
*
*     To add an exe to the allowed exe list, add to or replace the reg value in the reg command below:
*
*        reg.exe ADD "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\EmbeddedMode\ProcessLauncher" /v AllowedExecutableFilesList /t REG_MULTI_SZ /d "c:\windows\system32\ipconfig.exe\0c:\windows\system32\tlist.exe\0"
*
*/
namespace ProcessLauncherSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private async Task RunProcess()
        {
            var options = new ProcessLauncherOptions();
            var standardOutput = new InMemoryRandomAccessStream();
            var standardError = new InMemoryRandomAccessStream();
            options.StandardOutput = standardOutput;
            options.StandardError = standardError;

            await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                try
                {
                    var result = await ProcessLauncher.RunToCompletionAsync(cmd.Text, args.Text == null ? string.Empty : args.Text, options);

                    ProcessExitCode.Text += "Process Exit Code: " + result.ExitCode;

                    using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                    {
                        var size = standardOutput.Size;
                        using (var dataReader = new DataReader(outStreamRedirect))
                        {
                            var bytesLoaded = await dataReader.LoadAsync((uint)size);
                            var stringRead = dataReader.ReadString(bytesLoaded);
                            StdOutputText.Text += stringRead;
                        }
                    }

                    using (var errStreamRedirect = standardError.GetInputStreamAt(0))
                    {
                        using (var dataReader = new DataReader(errStreamRedirect))
                        {
                            var size = standardError.Size;
                            var bytesLoaded = await dataReader.LoadAsync((uint)size);
                            var stringRead = dataReader.ReadString(bytesLoaded);
                            StdErrorText.Text += stringRead;
                        }
                    }
                }
                catch (UnauthorizedAccessException uex)
                {
                    StdErrorText.Text += "Exception Thrown: " + uex.Message + "\n";
                    StdErrorText.Text += "\nMake sure you're allowed to run the specified exe; either\n" +
                                         "\t1) Add the exe to the AppX package, or\n" +
                                         "\t2) Add the absolute path of the exe to the allow list:\n" +
                                         "\t\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\EmbeddedMode\\ProcessLauncherAllowedExecutableFilesList.\n\n" +
                                         "Also, make sure the <iot:Capability Name=\"systemManagement\" /> has been added to the AppX manifest capabilities.\n";
                }
                catch (Exception ex)
                {
                    StdErrorText.Text += "Exception Thrown:" + ex.Message + "\n";
                    StdErrorText.Text += ex.StackTrace + "\n";
                }
            });
        }

        private async void Button_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            StdErrorText.Text = String.Empty;
            StdOutputText.Text = String.Empty;
            ProcessExitCode.Text = String.Empty;
            await RunProcess();
        }
    }
}
