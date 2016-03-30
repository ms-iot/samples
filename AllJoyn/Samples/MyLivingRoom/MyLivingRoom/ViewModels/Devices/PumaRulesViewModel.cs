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
using com.microsoft.OICBridge.core.rule.engine;
using MyLivingRoom.Extensions;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class PumaRulesViewModel : BaseDeviceViewModel
    {
        #region com.microsoft.OICBridge.core.rule.engine

        private AllJoynBusAttachment _engineBusAttachment;
        private engineConsumer _engineConsumer;
        private engineWatcher _engineWatcher;

        private async void engineWatcher_Added(engineWatcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await engineConsumer.JoinSessionAsync(args, sender);

            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _engineConsumer = joinResult.Consumer;
                _engineConsumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes
                _engineConsumer.HomeAwayChanged += this.engineConsumer_HomeAwayChanged;

                // populate initial values
                var valueResult = await _engineConsumer.GetHomeAwayAsync();
                if (valueResult.Status != AllJoynStatus.Ok) return;
                this.IsAway = valueResult.HomeAway;

                this.IsConnected = true;
            }
        }

        public bool IsAway
        {
            get { return _isAway; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _isAway = v, _isAway, value))
                {
                    this.RaisePropertyChangedOnDispatcher(nameof(this.IsHome));
                    var unused = _engineConsumer?.SetHomeAwayAsync(_isAway);
                }
            }
        }
        private bool _isAway;

        public bool IsHome
        {
            get { return !this.IsAway; }
            set { this.IsAway = !value; }
        }

        private void engineConsumer_HomeAwayChanged(engineConsumer sender, object value)
        {
            if (value is bool)
            {
                this.IsAway = (bool)value;
            }
        }

        #endregion com.microsoft.OICBridge.core.rule.engine

        #region Overrides

        protected override void OnStart()
        {
            _engineWatcher = new engineWatcher(this.CreateBusAttachment(ref _engineBusAttachment));
            _engineWatcher.Added += this.engineWatcher_Added;
            _engineWatcher.Start();
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
            this.ResetWatcher(ref _engineWatcher, ref _engineBusAttachment, w => { w.Stop(); w.Added -= this.engineWatcher_Added; });
            this.ResetConsumer(ref _engineConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.HomeAwayChanged -= this.engineConsumer_HomeAwayChanged;
            });
        }

        #endregion Overrides
    }
}
