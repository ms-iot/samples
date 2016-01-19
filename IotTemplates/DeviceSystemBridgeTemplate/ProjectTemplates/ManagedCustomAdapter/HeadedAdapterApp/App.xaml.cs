
using System;
using AdapterLib;
using BackgroundHost.Headed;
using BackgroundHost.Headed.Models;
using Windows.ApplicationModel.Activation;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.Core;
using Windows.UI.Xaml;

namespace HeadedAdapterApp
{

    sealed partial class App : Application
    {
        public App()
        {
            string bgTaskEntryPoint = typeof(HeadedBackgroundTask).FullName;
            _applicationImplementation = new ApplicationImplementation(bgTaskEntryPoint);
        }

        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            _applicationImplementation.OnLaunched(e);
        }       

        private ApplicationImplementation _applicationImplementation;
    }

  
}
