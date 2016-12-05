using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;
using Xamarin.Forms.Maps;

namespace XamarinIoTViewer
{
    class MainPage : ContentPage, IViewer
    {
        private ObservableCollection<string> messages;
        private MessageController controller;
        private Entry pinEntry, deviceName;
        private Map map;
        private Pin mapPin;
        public MainPage()
        {
            ListView list = new ListView();
            controller = new MessageController(this);
            messages = new ObservableCollection<string>();
            messages.Add("testing");
            Label directions = new Label
            {
                Text = "Enter the GPIO pin number and device id to control"
            };
            pinEntry = new Entry {
                Placeholder = "pin",
                HorizontalOptions = LayoutOptions.FillAndExpand
            };
            deviceName = new Entry {
                Placeholder = "Device Id",
                HorizontalOptions = LayoutOptions.FillAndExpand
            };
            Button button = new Button
            {
                Text = "light",
                Font = Font.SystemFontOfSize(NamedSize.Large),
                BorderWidth = 1
            };
            button.Clicked += SendMessage;
            list.ItemsSource = messages;
            list.HasUnevenRows = true;
            
            map = new Map(
                MapSpan.FromCenterAndRadius(
                    new Position(48, -122), Distance.FromMiles(50.0))) {
                IsShowingUser = true,
                HeightRequest = 400,
                WidthRequest = 400,
                VerticalOptions = LayoutOptions.FillAndExpand
            };
            Content = new StackLayout
            {
                VerticalOptions = LayoutOptions.Center,
                Children = {
                        directions,
                        pinEntry,
                        deviceName,
                        button,
                        map
                    }
            };
            
        }
        private async void SendMessage(object sender, EventArgs e)
        {
            await controller.SendLightMessage(pinEntry.Text, deviceName.Text);
        }

        public void showMessage(JObject message)
        {
            messages.Add(message.ToString());
            displayMapLocation(message["message"]);
        }
        public void displayMapLocation(JToken obj)
        {
            double lat = double.Parse(obj["latitude"].ToString());
            double lng = double.Parse(obj["longitude"].ToString());
            Position pos = new Position(lat, lng);
            map.MoveToRegion(MapSpan.FromCenterAndRadius(pos, Distance.FromMiles(0.5)));
            if(mapPin != null)
            {
                map.Pins.Clear();
                mapPin.Position = pos;
                mapPin.Label = "GPIO Pin: " + obj["pin"].ToString();
                mapPin.Address = "Status: " + obj["pinStatus"].ToString();
                map.Pins.Add(mapPin);
            } else
            {
                mapPin = new Pin
                {
                    Type = PinType.Place,
                    Position = pos,
                    Label = "GPIO Pin: " + obj["pin"].ToString(),
                    Address = "Status: " + obj["pinStatus"].ToString()
                };
                map.Pins.Add(mapPin);

            }
        }
    }
}
