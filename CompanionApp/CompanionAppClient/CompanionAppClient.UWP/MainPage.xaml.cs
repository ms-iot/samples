// Copyright (c) Microsoft. All rights reserved.

namespace CompanionAppClient.UWP
{
    public sealed partial class MainPage
    {
        public MainPage()
        {
            this.InitializeComponent();

            LoadApplication(new CompanionAppClient.App());
        }
    }
}
