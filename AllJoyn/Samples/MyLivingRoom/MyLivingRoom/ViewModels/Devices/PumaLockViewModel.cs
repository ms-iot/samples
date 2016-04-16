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
using System.Windows.Input;
using com.microsoft.OICBridge.oic.r.openlevel;
using MyLivingRoom.Extensions;
using Prism.Commands;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class PumaLockViewModel : SimpleLockViewModel
    {
        #region com.microsoft.OICBridge.oic.r.openlevel

        private AllJoynBusAttachment _openlevelBusAttachment;
        private openlevelConsumer _openlevelConsumer;
        private openlevelWatcher _openlevelWatcher;

        private async void openlevelWatcher_Added(openlevelWatcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await openlevelConsumer.JoinSessionAsync(args, sender);
            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _openlevelConsumer = joinResult.Consumer;
                _openlevelConsumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes
                _openlevelConsumer.OpenLevelChanged += this.openlevelConsumer_OpenLevelChanged;

                // populate initial values
                var valueResult = await _openlevelConsumer.GetOpenLevelAsync();
                if (valueResult.Status != AllJoynStatus.Ok) return;
                this.OpenLevel = valueResult.OpenLevel;

                this.IsConnected = true;
            }
        }

        public override bool IsLocked
        {
            get { return this.OpenLevel == 0; }
            set { this.OpenLevel = value ? 0 : 100; }
        }

        private Int64 OpenLevel
        {
            get { return _openlevel; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _openlevel = v, _openlevel, value, nameof(this.IsLocked)))
                {
                    var unused = _openlevelConsumer?.SetOpenLevelAsync(_openlevel);
                }
            }
        }
        private Int64 _openlevel;

        public ICommand LockCommand
        {
            get { return new DelegateCommand(() => this.IsLocked = true); }
        }

        public ICommand UnlockCommand
        {
            get { return new DelegateCommand(() => this.IsLocked = false); }
        }

        private void openlevelConsumer_OpenLevelChanged(openlevelConsumer sender, object value)
        {
            if (value is Int64)
            {
                this.OpenLevel = (Int64)value;
            }
        }

        #endregion com.microsoft.OICBridge.oic.r.openlevel

        #region Overrides

        protected override void OnStart()
        {
            _openlevelWatcher = new openlevelWatcher(this.CreateBusAttachment(ref _openlevelBusAttachment));
            _openlevelWatcher.Added += this.openlevelWatcher_Added;
            _openlevelWatcher.Start();
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

        #endregion Overrides

        #region Event Handlers

        protected override void OnSessionLost(object consumer, AllJoynSessionLostEventArgs args)
        {
            this.ResetWatcher(ref _openlevelWatcher, ref _openlevelBusAttachment, w => { w.Stop(); w.Added -= this.openlevelWatcher_Added; });
            this.ResetConsumer(ref _openlevelConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.OpenLevelChanged -= this.openlevelConsumer_OpenLevelChanged;
            });
        }

        #endregion Event Handlers
    }
}
