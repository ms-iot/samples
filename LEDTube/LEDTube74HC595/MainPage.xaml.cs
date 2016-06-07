using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.Threading;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace LEDTube74HC595
{
    public sealed partial class MainPage : Page
    {
        public LED74HC595Driver Led74Hc595Driver { get; set; }

        public MainPage()
        {
            InitializeComponent();

            Led74Hc595Driver = new LED74HC595Driver(26, 6, 5);

            BackgroundWorker worker = new BackgroundWorker { WorkerSupportsCancellation = true };
            worker.DoWork += (sender, args) =>
            {
                while (!worker.CancellationPending)
                {
                    //Led74Hc595Driver.DisplayTime(DateTime.Now);
                    Led74Hc595Driver.DisplayDigitsF(12.34);
                }
            };

            Loaded += async (sender, args) =>
            {
                Debug.WriteLine("Testing...");
                await Led74Hc595Driver.TestTubeAsync(3);
                await Task.Delay(300);

                Debug.WriteLine("Running worker...");
                worker.RunWorkerAsync();

                //await Task.Delay(2000);
                //Debug.WriteLine("Cancelling worker...");
                //worker.CancelAsync();

                Debug.WriteLine(worker.IsBusy);
            };
        }
    }
}
