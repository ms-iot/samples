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

namespace XamarinIoTViewer.UWP
{
    public sealed partial class MainPage
    {
        public MainPage()
        {
            this.InitializeComponent();
            Xamarin.FormsMaps.Init("nSpxRW98b2nEdBPt7dJ2~o0Os-cfzI4utkCAC8CaHMw~ArikLpbltiClL_hHSEOEeV-obeDUfI-b2wo9sEv4bn965FMscvOlxKGEHzuBVXvg");
            LoadApplication(new XamarinIoTViewer.App());
                        

        }

    }
}
