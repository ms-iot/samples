// Copyright (c) Microsoft. All rights reserved.

using Android.App;
using Android.Content.PM;
using Android.OS;

namespace CompanionAppClient.Droid
{
    [Activity (Label = "CompanionAppClient", Icon = "@drawable/icon", MainLauncher = true, ConfigurationChanges = ConfigChanges.ScreenSize | ConfigChanges.Orientation)]
	public class MainActivity : global::Xamarin.Forms.Platform.Android.FormsApplicationActivity
	{
		protected override void OnCreate (Bundle bundle)
		{
            MainPage.AccessPointHelper = new WifiHelper();
            base.OnCreate (bundle);

			global::Xamarin.Forms.Forms.Init (this, bundle);
			LoadApplication (new CompanionAppClient.App ());
        }
    }
}

