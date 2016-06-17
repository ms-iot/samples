using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
namespace IoTSender
{
    class LocationProvider
    {
        private GeolocationAccessStatus accessStatus;
        public LocationProvider()
        {
            this.Initialize();
        }
        private async void Initialize()
        {
            accessStatus = await Geolocator.RequestAccessAsync();
        }

        public async Task<string> GetLocation()
        {
            string coords = "";
            switch (accessStatus)
            {
                case GeolocationAccessStatus.Allowed:
                    Geolocator gl = new Geolocator();
                    gl.DesiredAccuracy = PositionAccuracy.High;
                    Geoposition pos = await gl.GetGeopositionAsync();
                    coords = pos.Coordinate.Latitude.ToString() + ", " + pos.Coordinate.Longitude.ToString();
                    break;
                default:
                    coords = "Access to location is denied. Please enable location.";
                    break;
            }
            return coords;
        }
    }
    
}
