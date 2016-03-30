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
using net.allplay.MCU;
using net.allplay.MediaPlayer;
using Prism.Commands;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class MediaPlayerViewModel : BaseDeviceViewModel
    {
        #region net.allplay.MCU

        private AllJoynBusAttachment _mcuBusAttachment;
        private MCUConsumer _mcuConsumer;
        private MCUWatcher _mcuWatcher;

        private async void MCUWatcher_Added(MCUWatcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await MCUConsumer.JoinSessionAsync(args, sender);

            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _mcuConsumer = joinResult.Consumer;
                _mcuConsumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes

                // populate initial values

                this.IsConnected = true;
            }
        }

        public string Url
        {
            get { return _url; }
            set { this.SetProperty(ref _url, value); }
        }
        private string _url = "http://twobitrepo.com/build.mp3";

        public ICommand PlayCommand
        {
            get { return _playCommand ?? (_playCommand = new DelegateCommand(this.PlayCommandExecuted)); }
        }
        private DelegateCommand _playCommand;

        private void PlayCommandExecuted()
        {
            var unused = _mcuConsumer?.PlayItemAsync(this.Url, "", "", "", 0, "", "");
        }

        #endregion net.allplay.MCU

        #region net.allplay.MediaPlayer

        private AllJoynBusAttachment _mediaPlayerBusAttachment;
        private MediaPlayerConsumer _mediaPlayerConsumer;
        private MediaPlayerWatcher _mediaPlayerWatcher;

        private async void MediaPlayerWatcher_Added(MediaPlayerWatcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await MediaPlayerConsumer.JoinSessionAsync(args, sender);

            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _mediaPlayerConsumer = joinResult.Consumer;
                _mediaPlayerConsumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes

                // populate initial values

                this.IsConnected = true;
            }
        }

        public ICommand PauseCommand
        {
            get { return _pauseCommand ?? (_pauseCommand = new DelegateCommand(this.PauseCommandExecuted)); }
        }
        private DelegateCommand _pauseCommand;

        private void PauseCommandExecuted()
        {
            var unused = _mediaPlayerConsumer?.PauseAsync();
        }

        #endregion net.allplay.MediaPlayer

        #region Overrides

        protected override void OnStart()
        {
            _mcuWatcher = new MCUWatcher(this.CreateBusAttachment(ref _mcuBusAttachment));
            _mcuWatcher.Added += this.MCUWatcher_Added;
            _mcuWatcher.Start();

            _mediaPlayerWatcher = new MediaPlayerWatcher(this.CreateBusAttachment(ref _mediaPlayerBusAttachment));
            _mediaPlayerWatcher.Added += this.MediaPlayerWatcher_Added;
            _mediaPlayerWatcher.Start();
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

        protected override void OnSessionLost(object sender, AllJoynSessionLostEventArgs args)
        {
            this.ResetWatcher(ref _mcuWatcher, ref _mcuBusAttachment, w => { w.Stop(); w.Added -= this.MCUWatcher_Added; });
            this.ResetConsumer(ref _mcuConsumer, c => c.SessionLost -= this.Consumer_SessionLost);

            this.ResetWatcher(ref _mediaPlayerWatcher, ref _mediaPlayerBusAttachment, w => { w.Stop(); w.Added -= this.MediaPlayerWatcher_Added; });
            this.ResetConsumer(ref _mediaPlayerConsumer, c => c.SessionLost -= this.Consumer_SessionLost);
        }

        #endregion Overrides
    }
}
