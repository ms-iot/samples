// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Collections.Generic;
using com.microsoft.OICBridge.oic.r.temperature;
using MyLivingRoom.Extensions;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class PumaLyricViewModel : BaseDeviceViewModel
    {
        #region com.microsoft.OICBridge.oic.r.temperature

        private AllJoynBusAttachment _temperatureBusAttachment;
        private temperatureConsumer _temperatureConsumer;
        private temperatureWatcher _temperatureWatcher;

        private async void temperatureWatcher_Added(temperatureWatcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await temperatureConsumer.JoinSessionAsync(args, sender);
            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _temperatureConsumer = joinResult.Consumer;
                _temperatureConsumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes
                _temperatureConsumer.TemperatureChanged += this.temperatureConsumer_TemperatureChanged;

                // populate initial values
                var valueResult = await _temperatureConsumer.GetTemperatureAsync();
                if (valueResult.Status != AllJoynStatus.Ok) return;
                this.Temperature = valueResult.Temperature;

                this.IsConnected = true;
            }
        }

        private void temperatureConsumer_TemperatureChanged(temperatureConsumer consumer, object value)
        {
            if (value is double)
            {
                this.Temperature = (double)value;
            }
        }

        public double Temperature
        {
            get { return _temperature; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _temperature = v, _temperature, value))
                {
                    var unused = _temperatureConsumer?.SetTemperatureAsync(_temperature);
                }
            }
        }
        private double _temperature;

        #endregion com.microsoft.OICBridge.oic.r.temperature

        #region Overrides

        protected override void OnStart()
        {
            _temperatureWatcher = new temperatureWatcher(this.CreateBusAttachment(ref _temperatureBusAttachment));
            _temperatureWatcher.Added += this.temperatureWatcher_Added;
            _temperatureWatcher.Start();
        }

        public override bool ProcessProtocolUri(Uri uri, IList<string> remainingSegments)
        {
            // We're not expecting any more path segments except for our own
            if (remainingSegments.Count != 1)
            {
                return false;
            }

            // Ignoring query parameters; just navigate to the corresponding view
            this.TopicDefinition.SelectTopic();

            return true;
        }

        protected override void OnSessionLost(object consumer, AllJoynSessionLostEventArgs args)
        {
            this.ResetWatcher(ref _temperatureWatcher, ref _temperatureBusAttachment, w => { w.Stop(); w.Added -= this.temperatureWatcher_Added; });
            this.ResetConsumer(ref _temperatureConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.TemperatureChanged -= this.temperatureConsumer_TemperatureChanged;
            });
        }

        #endregion Overrides
    }
}
