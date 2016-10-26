using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

/// <summary>
/// Demostrates how to get around PopUp class not being support in IoT-Core
/// </summary>
namespace NoPopups
{
    /// <summary>
    /// Demonstrates a pattern for 
    /// </summary>
    public sealed partial class MainPage : Page
    {
        enum DialogResult
        {
            Yes, No, OK, Cancel
        }
        public MainPage()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Perform one of three dummy actions.
        /// Give user option to cancel the action.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void button_Click(object sender, RoutedEventArgs e)
        {
            string msg = "";
            ResultMsg.Text = "";
            if (sender is Button)
            {
                string action = (string) ((Button) sender).Content;
                switch (action)
                {
                    case "Do first":
                        msg = "Do you want to do the first thing";
                    break;
                    case "Do second":
                        msg = "Do you want to do the second thing";
                        break;
                    case "Do third":
                        msg = "Do you want to do the third thing";
                        break;
                }
                if (msg !="")
                {
                    DialogResult dialogRes = await ShowDialog("Popups", msg, new List<DialogResult>() { DialogResult.Yes, DialogResult.Cancel });
                    if (dialogRes == DialogResult.Yes)
                    {
                        ResultMsg.Text = "Action was performed.";
                    }
                    else
                    {
                        ResultMsg.Text = "Action was aborted.";
                    }
                }
            
            }
        }

        private void button0_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Exit();
        }

        //Signal the awaiting method when user response is done.
        private SemaphoreSlim signal = new SemaphoreSlim(0, 1);
        //Pass back user response.
        private DialogResult YesNo;

        /// <summary>
        /// This signals the awaiting method when a button is pressed after setting the local variable YesNo which is used by the awaiting method.
        /// This could also be a hardware button event handler.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ButtonContinue_Click(object sender, RoutedEventArgs e)
        {
            if (sender is Button)
            {
                Button butt = (Button)sender;
                if ("Yes" == (string)butt.Content)
                {
                    YesNo = DialogResult.Yes;
                }
                else
                {
                    YesNo = DialogResult.No;
                }
                signal.Release();
            }
        }

        /// <summary>
        /// General Popup dialog method.
        /// Can use Popup class or use simulated popup box.
        /// Later is required for IOT-Core as Popup class isn't supported
        /// </summary>
        /// <param name="Title"></param>
        /// <param name="Message"></param>
        /// <param name="buttons"></param>
        /// <returns></returns>
        private async Task<DialogResult> ShowDialog(string Title, string Message, List<DialogResult> buttons)
        {
            DialogResult res = DialogResult.Yes;

#if (IOTCORE)
            /* */
            PopupBox.Visibility = Visibility.Visible;
            PopupMsg.Text = Message;
            signal = new SemaphoreSlim(0, 1);
            await signal.WaitAsync();
            res = YesNo;
            PopupBox.Visibility = Visibility.Collapsed;
#else
            try
            {
                MessageDialog dialog = new MessageDialog(Message);
                dialog.Title = Title;

                if (buttons.Contains(DialogResult.Yes))
                    dialog.Commands.Add(new UICommand { Label = "Yes", Id = DialogResult.Yes });
                if (buttons.Contains(DialogResult.No))
                    dialog.Commands.Add(new UICommand { Label = "No", Id = DialogResult.No });
                if (buttons.Contains(DialogResult.OK))
                    dialog.Commands.Add(new UICommand { Label = "OK", Id = DialogResult.OK });
                if (buttons.Contains(DialogResult.Cancel))
                    dialog.Commands.Add(new UICommand { Label = "Cancel", Id = DialogResult.Cancel });

                var rebootRes = await dialog.ShowAsync();

                res = (DialogResult)rebootRes.Id;

            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
#endif


            return res;
        }


    }
}
