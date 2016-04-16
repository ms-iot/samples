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
using System.Windows.Input;
using com.microsoft.OICBridge.oic.r.audio;
using com.microsoft.OICBridge.oic.r.channel;
using com.microsoft.OICBridge.oic.r.Switch.binary;
using MyLivingRoom.Extensions;
using Prism.Commands;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class TelevisionViewModel : BaseDeviceViewModel
    {
        private const string _expectedBusObjectPath = "television";

        #region com.microsoft.OICBridge.oic.r.audio

        private AllJoynBusAttachment _audioBusAttachment;
        private audioWatcher _audioWatcher;
        private audioConsumer _audioConsumer;

        private async void audioWatcher_Added(audioWatcher watcher, AllJoynServiceInfo args)
        {
            if (_audioConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinResult = await audioConsumer.JoinSessionAsync(args, watcher);
                if (joinResult.Status == AllJoynStatus.Ok)
                {
                    _audioConsumer = joinResult.Consumer;
                    _audioConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _audioConsumer.MuteChanged += this.audioConsumer_MuteChanged;
                    _audioConsumer.VolumeChanged += this.audioConsumer_VolumeChanged;

                    // populate initial values
                    var muteResult = await _audioConsumer.GetMuteAsync();
                    if (muteResult.Status != AllJoynStatus.Ok) return;
                    this.Mute = muteResult.Mute;

                    var volumeResult = await _audioConsumer.GetVolumeAsync();
                    if (volumeResult.Status != AllJoynStatus.Ok) return;
                    this.Volume = volumeResult.Volume;

                    this.IsConnected = true;
                }
            }
        }

        #region Mute

        public bool Mute
        {
            get { return _mute; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _mute = v, _mute, value))
                {
                    var unused = _audioConsumer?.SetMuteAsync(_mute);
                }
            }
        }
        private bool _mute;

        private void audioConsumer_MuteChanged(audioConsumer consumer, object value)
        {
            if (value is bool)
            {
                this.Mute = (bool)value;
            }
        }

        #endregion Mute

        #region Volume

        public Int64 Volume
        {
            get { return _volume; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _volume = v, _volume, value))
                {
                    var unused = _audioConsumer?.SetVolumeAsync(_volume);
                }
            }
        }
        private Int64 _volume;

        private void audioConsumer_VolumeChanged(audioConsumer consumer, object value)
        {
            if (value is Int64)
            {
                this.Volume = (Int64)value;
            }
        }

        #endregion Volume

        #endregion com.microsoft.OICBridge.oic.r.audio

        #region com.microsoft.OICBridge.oic.r.channel

        private const string _setChannelDownValue = "down";
        private const string _setChannelUpValue = "up";

        private AllJoynBusAttachment _channelBusAttachment;
        private channelWatcher _channelWatcher;
        private channelConsumer _channelConsumer;

        private async void channelWatcher_Added(channelWatcher watcher, AllJoynServiceInfo args)
        {
            if (_channelConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinSessionResult = await channelConsumer.JoinSessionAsync(args, watcher);
                if (joinSessionResult.Status == AllJoynStatus.Ok)
                {
                    _channelConsumer = joinSessionResult.Consumer;
                    _channelConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _channelConsumer.ChannelUpDownChanged += this.channelConsumer_ChannelUpDownChanged;

                    // populate initial values
                    var channelUpDownResult = await _channelConsumer.GetChannelUpDownAsync();
                    if (channelUpDownResult.Status != AllJoynStatus.Ok) return;
                    this.ChannelUpDown = channelUpDownResult.ChannelUpDown;

                    this.IsConnected = true;
                }
            }
        }

        public string ChannelUpDown
        {
            get { return _channelUpDown; }
            set
            {
                this.SetPropertyOnDispatcher(v => _channelUpDown = v, _channelUpDown, value);

                // In this case we want to set the channelUpDown whether it changed or not, repeatedly
                // setting the value to "down" keeps decreasing the channel.
                var unused = _channelConsumer?.SetChannelUpDownAsync(_channelUpDown);
            }
        }
        private string _channelUpDown;

        private void channelConsumer_ChannelUpDownChanged(channelConsumer consumer, object value)
        {
            if (value is string)
            {
                this.ChannelUpDown = (string)value;
            }
        }

        #endregion com.microsoft.OICBridge.oic.r.channel

        #region com.microsoft.OICBridge.oic.r.Switch.binary

        private AllJoynBusAttachment _powerSwitchBusAttachment;
        private binaryWatcher _powerSwitchWatcher;
        private binaryConsumer _powerSwitchConsumer;

        private async void powerSwitchWatcher_Added(binaryWatcher watcher, AllJoynServiceInfo args)
        {
            if (_powerSwitchConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinResult = await binaryConsumer.JoinSessionAsync(args, watcher);
                if (joinResult.Status == AllJoynStatus.Ok)
                {
                    _powerSwitchConsumer = joinResult.Consumer;
                    _powerSwitchConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _powerSwitchConsumer.ValueChanged += this.powerSwitchConsumer_ValueChanged;

                    // populate initial values
                    var valueResult = await _powerSwitchConsumer.GetValueAsync();
                    if (valueResult.Status != AllJoynStatus.Ok) return;
                    this.IsOn = valueResult.Value;

                    this.IsConnected = true;
                }
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

        private void powerSwitchConsumer_ValueChanged(binaryConsumer consumer, object value)
        {
            if (value is bool)
            {
                this.IsOn = (bool)value;
            }
        }

        #endregion com.microsoft.OICBridge.oic.r.Switch.binary

        #region Commands

        public ICommand ChannelUpCommand
        {
            get { return new DelegateCommand(() => this.ChannelUpDown = _setChannelUpValue); }
        }

        public ICommand ChannelDownCommand
        {
            get { return new DelegateCommand(() => this.ChannelUpDown = _setChannelDownValue); }
        }

        public ICommand VolumeUpCommand
        {
            get { return new DelegateCommand(() => ++this.Volume); }
        }

        public ICommand VolumeDownCommand
        {
            get { return new DelegateCommand(() => --this.Volume); }
        }

        public ICommand MuteUnmuteCommand
        {
            get { return new DelegateCommand(() => this.Mute = !_mute); }
        }

        #endregion Commands

        #region Overrides

        protected override void OnStart()
        {
            _audioWatcher = new audioWatcher(this.CreateBusAttachment(ref _audioBusAttachment));
            _audioWatcher.Added += this.audioWatcher_Added;
            _audioWatcher.Start();

            _channelWatcher = new channelWatcher(this.CreateBusAttachment(ref _channelBusAttachment));
            _channelWatcher.Added += this.channelWatcher_Added;
            _channelWatcher.Start();

            _powerSwitchWatcher = new binaryWatcher(this.CreateBusAttachment(ref _powerSwitchBusAttachment));
            _powerSwitchWatcher.Added += this.powerSwitchWatcher_Added;
            _powerSwitchWatcher.Start();
        }

        protected override void OnSessionLost(object sender, AllJoynSessionLostEventArgs args)
        {
            // Reset everything

            this.ResetWatcher(ref _audioWatcher, ref _audioBusAttachment, w => { w.Stop(); w.Added -= this.audioWatcher_Added; });
            this.ResetConsumer(ref _audioConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.MuteChanged -= this.audioConsumer_MuteChanged;
                c.VolumeChanged -= this.audioConsumer_VolumeChanged;
            });

            this.ResetWatcher(ref _channelWatcher, ref _channelBusAttachment, w => { w.Stop(); w.Added -= this.channelWatcher_Added; });
            this.ResetConsumer(ref _channelConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.ChannelUpDownChanged -= this.channelConsumer_ChannelUpDownChanged;
            });

            this.ResetWatcher(ref _powerSwitchWatcher, ref _powerSwitchBusAttachment, w => { w.Stop(); w.Added -= this.powerSwitchWatcher_Added; });
            this.ResetConsumer(ref _powerSwitchConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.ValueChanged -= this.powerSwitchConsumer_ValueChanged;
            });
        }

        #endregion Overrides
    }
}
