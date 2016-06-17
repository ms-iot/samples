using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.UI.Xaml.Controls.Maps;

namespace IoTViewer
{
    class MapManager
    {
        private MapControl myMap;
        private MapIcon deviceLoc;
        public MapManager(MapControl map)
        {
            myMap = map;
            deviceLoc = new MapIcon();
            myMap.MapElements.Add(deviceLoc);
        }
        public async void SetMapLocation(Message locMsg)
        {
            var center =
               new Geopoint(new BasicGeoposition()
               {
                   Latitude = Double.Parse(locMsg.lat),
                   Longitude = Double.Parse(locMsg.lng)

               });

            // retrieve map
            await myMap.TrySetSceneAsync(MapScene.CreateFromLocationAndRadius(center, 250));
            deviceLoc.Location = center;
            deviceLoc.Title = locMsg.timestamp.ToString();

        }
    }
}
