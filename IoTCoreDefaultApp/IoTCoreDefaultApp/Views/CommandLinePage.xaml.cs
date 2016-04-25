// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Text;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Documents;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class CommandLinePage : Page
    {
        private const string CommandLineProcesserExe = "c:\\windows\\system32\\cmd.exe";
        private const uint BufSize = 8192;

        private string currentDirectory = "C:\\";
        private List<string> commandLineHistory = new List<string>();
        private int currentCommandLine = -1;
        private ResourceLoader resourceLoader = new Windows.ApplicationModel.Resources.ResourceLoader();

        public CommandLinePage()
        {
            this.InitializeComponent();
            this.DataContext = LanguageManager.GetInstance();
            CommandLine.PlaceholderText = String.Format(resourceLoader.GetString("CommandLinePlaceholderText"), currentDirectory);
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            await Dispatcher.RunAsync(
                CoreDispatcherPriority.Normal,
                () => CommandLine.Focus(FocusState.Keyboard));
        }

        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private async Task RunProcess()
        {

            if (string.IsNullOrWhiteSpace(CommandLine.Text))
            {
                return;
            }

            commandLineHistory.Add(CommandLine.Text);
            currentCommandLine = commandLineHistory.Count;

            Run cmdLineRun = new Run();
            cmdLineRun.Foreground = new SolidColorBrush(Windows.UI.Colors.LightGray);
            cmdLineRun.FontWeight = FontWeights.Bold;
            cmdLineRun.Text = currentDirectory + "> " + CommandLine.Text + "\n";

            Run stdOutRun = new Run();
            Run stdErrRun = new Run();
            stdErrRun.Foreground = new SolidColorBrush(Windows.UI.Colors.Red);

            var commandLineText = CommandLine.Text.Trim();

            if (commandLineText.Equals("cls", StringComparison.CurrentCultureIgnoreCase))
            {
                StdOutputText.Blocks.Clear();
                return;
            }
            else if (commandLineText.StartsWith("cd ", StringComparison.CurrentCultureIgnoreCase) || commandLineText.Equals("cd", StringComparison.CurrentCultureIgnoreCase))
            {
                stdErrRun.Text = resourceLoader.GetString("CdNotSupported");
            }
            else if (commandLineText.Equals("exit", StringComparison.CurrentCultureIgnoreCase))
            {
                NavigationUtils.GoBack();
            }
            else
            {
                var standardOutput = new InMemoryRandomAccessStream();
                var standardError = new InMemoryRandomAccessStream();
                var options = new ProcessLauncherOptions
                {
                    StandardOutput = standardOutput,
                    StandardError = standardError
                };

                try
                {
                    var args = "/C \"cd \"" + currentDirectory + "\" & " + commandLineText + "\"";
                    var result = await ProcessLauncher.RunToCompletionAsync(CommandLineProcesserExe, args, options);

                    // First write std out
                    using (var outStreamRedirect = standardOutput.GetInputStreamAt(0))
                    {
                        using (var dataReader = new DataReader(outStreamRedirect))
                        {
                            await ReadText(dataReader, stdOutRun);
                        }
                    }

                    // Then write std err
                    using (var errStreamRedirect = standardError.GetInputStreamAt(0))
                    {
                        using (var dataReader = new DataReader(errStreamRedirect))
                        {
                            await ReadText(dataReader, stdErrRun);
                        }
                    }
                }
                catch (UnauthorizedAccessException uex)
                {
                    var errorMessage = uex.Message + "\n\n" + resourceLoader.GetString("CmdNotEnabled");
                    stdErrRun.Text = errorMessage;
                }
                catch (Exception ex)
                {
                    var errorMessage = ex.Message + "\n" + ex.StackTrace + "\n";
                    stdErrRun.Text = errorMessage;
                }
            }

            await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                Paragraph paragraph = new Paragraph();

                paragraph.Inlines.Add(cmdLineRun);
                paragraph.Inlines.Add(stdOutRun);
                paragraph.Inlines.Add(stdErrRun);

                StdOutputText.Blocks.Add(paragraph);
            });
        }

        private async Task ReadText(DataReader dataReader, Run run)
        {
            StringBuilder textOutput = new StringBuilder((int)BufSize);
            uint bytesLoaded = 0;
            while ((bytesLoaded = await dataReader.LoadAsync(BufSize)) > 0)
            {
                textOutput.Append(dataReader.ReadString(bytesLoaded));
            }

            new System.Threading.ManualResetEvent(false).WaitOne(10);
            if ((bytesLoaded = await dataReader.LoadAsync(BufSize)) > 0)
            {
                textOutput.Append(dataReader.ReadString(bytesLoaded));
            }

            run.Text = textOutput.ToString();
        }

        private async void CommandLine_KeyUp(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == VirtualKey.Enter)
            {
                await DoRunCommand(true);
            }
            else if (e.Key == VirtualKey.Up)
            {
                currentCommandLine = Math.Max(0, currentCommandLine - 1);
                if (currentCommandLine < commandLineHistory.Count)
                {
                    UpdateCommandLineFromHistory();
                }
            }
            else if (e.Key == VirtualKey.Down)
            {
                currentCommandLine = Math.Min(commandLineHistory.Count , currentCommandLine + 1);
                if (currentCommandLine < commandLineHistory.Count && currentCommandLine >= 0)
                {
                    UpdateCommandLineFromHistory();
                }
                else
                {
                    CommandLine.Text = string.Empty;
                }
            }
        }

        private void UpdateCommandLineFromHistory()
        {
            CommandLine.Text = commandLineHistory[currentCommandLine];
            if (CommandLine.Text.Length > 0)
            {
                CommandLine.SelectionStart = CommandLine.Text.Length;
                CommandLine.SelectionLength = 0;
            }
        }

        private async void RunButton_Click(object sender, RoutedEventArgs e)
        {
            await DoRunCommand(false);
        }

        private async Task DoRunCommand(bool isFocus)
        {
            await Dispatcher.RunAsync(
                CoreDispatcherPriority.Normal,
                async () =>
                {
                    RunButton.IsEnabled = false;
                    CommandLine.IsEnabled = false;
                    ClearButton.IsEnabled = false;

                    await RunProcess();
                    CommandLine.Text = string.Empty;

                    RunButton.IsEnabled = true;
                    CommandLine.IsEnabled = true;
                    ClearButton.IsEnabled = true;
                    if (isFocus)
                    {
                        CommandLine.Focus(FocusState.Keyboard);
                    }
                });
        }

        private void StdOutputText_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            StdOutputScroller.ScrollToVerticalOffset(StdOutputScroller.ScrollableHeight);
        }

        private void ClearButton_Click(object sender, RoutedEventArgs e)
        {
            StdOutputText.Blocks.Clear();
        }
    }
}
