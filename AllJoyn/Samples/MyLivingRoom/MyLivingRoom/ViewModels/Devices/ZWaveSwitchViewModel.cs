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
using com.microsoft.ZWaveBridge.SwitchBinary.Switch;
using MyLivingRoom.Extensions;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class ZWaveSwitchViewModel : BaseDeviceViewModel
    {
        #region com.microsoft.ZWaveBridge.SwitchBinary.Switch

        private AllJoynBusAttachment _powerSwitchBusAttachment;
        private SwitchConsumer _powerSwitchConsumer;
        private SwitchWatcher _powerSwitchWatcher;

        private async void powerSwitchWatcher_Added(SwitchWatcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await SwitchConsumer.JoinSessionAsync(args, sender);

            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _powerSwitchConsumer = joinResult.Consumer;
                _powerSwitchConsumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes
                _powerSwitchConsumer.ValueChanged += this.powerSwitchConsumer_ValueChanged;

                // populate initial values
                var result = await _powerSwitchConsumer.GetValueAsync();
                if (result.Status != AllJoynStatus.Ok) return;
                this.IsOn = result.Value;

                this.IsConnected = true;
            }
        }

        public bool IsOn
        {
            get { return _isOn; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _isOn = v, _isOn, value))
                {
                    var unused = _powerSwitchConsumer?.SetValueAsync(_isOn);
                }
            }
        }
        private bool _isOn;

        private void powerSwitchConsumer_ValueChanged(SwitchConsumer consumer, object value)
        {
            if (value is bool)
            {
                this.IsOn = (bool)value;
            }
        }

        #endregion com.microsoft.ZWaveBridge.SwitchBinary.Switch

        #region Overrides

        protected override void OnStart()
        {
            _powerSwitchWatcher = new SwitchWatcher(this.CreateBusAttachment(ref _powerSwitchBusAttachment));
            _powerSwitchWatcher.Added += this.powerSwitchWatcher_Added;
            _powerSwitchWatcher.Start();
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

        protected override void OnSessionLost(object consumerObject, AllJoynSessionLostEventArgs args)
        {
            this.ResetWatcher(ref _powerSwitchWatcher, ref _powerSwitchBusAttachment, w => { w.Stop(); w.Added -= this.powerSwitchWatcher_Added; });
            this.ResetConsumer(ref _powerSwitchConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                _powerSwitchConsumer.ValueChanged -= this.powerSwitchConsumer_ValueChanged;
            });
        }

        #endregion Overrides
    }
}
