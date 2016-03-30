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
using System.Threading.Tasks;
using MyLivingRoom.Extensions;
using org.allseen.LSF;
using Windows.Devices.AllJoyn;
using Windows.UI;

namespace MyLivingRoom.ViewModels
{
    public class HueLightViewModel : BaseDeviceViewModel
    {
        #region org.allseen.LSF

        private AllJoynBusAttachment _lampStateBusAttachment;
        private LampStateConsumer _lampStateConsumer;
        private LampStateWatcher _lampStateWatcher;

        private async void LampStateWatcher_Added(LampStateWatcher sender, AllJoynServiceInfo args)
        {
            var aboutData = await AllJoynAboutDataView.GetDataBySessionPortAsync(args.UniqueName, _lampStateBusAttachment, args.SessionPort);

            if (aboutData.Manufacturer.ToLower().Contains("philips"))
            {
                var joinResult = await LampStateConsumer.JoinSessionAsync(args, sender);

                if (joinResult.Status == AllJoynStatus.Ok)
                {
                    _lampStateConsumer = joinResult.Consumer;
                    _lampStateConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _lampStateConsumer.Signals.LampStateChangedReceived += this.lampStateConsumer_LampStateChangedReceived;

                    // populate initial values
                    this.GetCurrentValuesAsync();

                    this.IsConnected = true;
                }
            }
        }

        private async void GetCurrentValuesAsync()
        {
            var stateResult = await _lampStateConsumer.GetOnOffAsync();
            if (stateResult.Status != AllJoynStatus.Ok) return;
            this.IsOn = stateResult.OnOff;

            var hueResult = await _lampStateConsumer.GetHueAsync();
            if (hueResult.Status != AllJoynStatus.Ok) return;
            this.Hue = hueResult.Hue >> 16;

            var brightnessResult = await _lampStateConsumer.GetBrightnessAsync();
            if (brightnessResult.Status != AllJoynStatus.Ok) return;
            var brightness = brightnessResult.Brightness >> 16;
            this.Brightness = brightness * 100 / ushort.MaxValue;

            var saturationResult = await _lampStateConsumer.GetSaturationAsync();
            if (saturationResult.Status != AllJoynStatus.Ok) return;
            this.Saturation = saturationResult.Saturation >> 16;
        }

        public double Hue
        {
            get { return _hue; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _hue = v, _hue, (uint)value))
                {
                    var unused = this.SetLightColorAsync(_hue);
                }
            }
        }
        private uint _hue;

        public double Saturation
        {
            get { return _saturation; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _saturation = v, _saturation, (uint)value))
                {
                    var unused = this.SetSaturationAsync(_saturation);
                }
            }
        }
        private uint _saturation;

        public double Brightness
        {
            get { return _brightness; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _brightness = v, _brightness, (uint)value))
                {
                    var newBrightness = value * ushort.MaxValue / 100;
                    var unused = SetBrightnessAsync((uint)newBrightness);
                }
            }
        }
        private uint _brightness;

        public Color Color
        {
            get { return _color; }
            set
            {
                this.SetPropertyOnDispatcher(v => _color = v, _color, value);
                this.SetColorHSV(value);
            }
        }
        private Color _color;

        public bool IsOn
        {
            get { return _isOn; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _isOn = v, _isOn, value))
                {
                    var unused = this.SetLampStateAsync(_isOn);
                }
            }
        }
        private bool _isOn;

        private async void SetColorHSV(Color value)
        {
            double hue, saturation, brightness;
            value.ToHSV(out hue, out saturation, out brightness);

            await this.SetLightColorAsync((uint)hue);
            await this.SetBrightnessAsync((uint)brightness);
            await this.SetSaturationAsync((uint)saturation);
        }

        private async Task SetLightColorAsync(uint hue)
        {
            if (_lampStateConsumer != null)
            {
                // Set only if the new and current values are different
                // Getting current value also gives LSF a little space when setting HBV simultaneously
                hue <<= 16;
                var hueResult = await _lampStateConsumer.GetHueAsync();
                if (hueResult.Status != AllJoynStatus.Ok || hue != hueResult.Hue)
                {
                    await _lampStateConsumer.SetHueAsync(hue);
                }
            }
        }

        private async Task SetBrightnessAsync(uint brightness)
        {
            if (_lampStateConsumer != null)
            {
                brightness <<= 16;
                var brightnessResult = await _lampStateConsumer.GetBrightnessAsync();
                if (brightnessResult.Status != AllJoynStatus.Ok || brightness != brightnessResult.Brightness)
                {
                    await _lampStateConsumer.SetBrightnessAsync(brightness);
                }
            }
        }

        private async Task SetSaturationAsync(uint saturation)
        {
            if (_lampStateConsumer != null)
            {
                saturation <<= 16;
                var saturationResult = await _lampStateConsumer.GetSaturationAsync();
                if (saturationResult.Status != AllJoynStatus.Ok || saturation != saturationResult.Saturation)
                {
                    await _lampStateConsumer.SetSaturationAsync(saturation);
                }
            }
        }

        private async Task SetLampStateAsync(bool bState)
        {
            if (_lampStateConsumer != null)
            {
                await _lampStateConsumer.SetOnOffAsync(bState);
            }
        }

        private void lampStateConsumer_LampStateChangedReceived(LampStateSignals sender, LampStateLampStateChangedReceivedEventArgs args)
        {
            this.GetCurrentValuesAsync();
        }

        #endregion org.allseen.LSF

        #region Overrides

        protected override void OnStart()
        {
            _lampStateWatcher = new LampStateWatcher(this.CreateBusAttachment(ref _lampStateBusAttachment));
            _lampStateWatcher.Added += this.LampStateWatcher_Added;
            _lampStateWatcher.Start();
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
            this.ResetWatcher(ref _lampStateWatcher, ref _lampStateBusAttachment, w => { w.Stop(); w.Added -= this.LampStateWatcher_Added; });
            this.ResetConsumer(ref _lampStateConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.Signals.LampStateChangedReceived -= this.lampStateConsumer_LampStateChangedReceived;
            });
        }

        #endregion Overrides
    }
}
