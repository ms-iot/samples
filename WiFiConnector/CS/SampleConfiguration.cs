// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using Windows.UI.Xaml.Controls;
using WiFiConnect;

namespace SDKTemplate
{
    public partial class MainPage : Page
    {
        public const string FEATURE_NAME = "WiFi";

        List<Scenario> scenarios = new List<Scenario>
        {
            new Scenario() { Title="WiFi Connect", ClassType=typeof(WiFiConnect_Scenario)}
        };
    }

    public class Scenario
    {
        public string Title { get; set; }
        public Type ClassType { get; set; }
    }
}
