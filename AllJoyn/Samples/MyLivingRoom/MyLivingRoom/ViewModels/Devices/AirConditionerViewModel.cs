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
using System.Windows.Input;
using MyLivingRoom.Extensions;
using org.alljoyn.ControlPanel;
using Prism.Commands;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class AirConditionerViewModel : BaseDeviceViewModel
    {
        private AllJoynBusAttachment _controlPanelBusAttachment;
        private ControlPanelWatcher _controlPanelWatcher;
        private ControlPanelConsumer _controlPanelConsumer;

        private AllJoynBusAttachment _propertyBusAttachment;
        private PropertyWatcher _propertyWatcher;

        public bool Power
        {
            get { return _power; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _power = v, _power, value) && !isInternalPropertyChange)
                {
                    var unused = _powerPropertyConsumer?.SetValueAsync((ushort)(_power ? 1 : 0));
                }
            }
        }
        private bool _power;
        private PropertyConsumer _powerPropertyConsumer;

        public ushort CurrentTemperature
        {
            get { return _currentTemperature; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _currentTemperature = v, _currentTemperature, value) && !isInternalPropertyChange)
                {
                    var unused = _currentTemperaturePropertyConsumer?.SetValueAsync(_currentTemperature);
                }
            }
        }
        private ushort _currentTemperature;
        private PropertyConsumer _currentTemperaturePropertyConsumer;

        public ushort TargetTemperature
        {
            get { return _targetTemperature; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _targetTemperature = Math.Min(value, (ushort)100), _targetTemperature, value) && !isInternalPropertyChange)
                {
                    var unused = _targetTemperaturePropertyConsumer?.SetValueAsync(_targetTemperature);
                }
            }
        }
        private ushort _targetTemperature;
        private PropertyConsumer _targetTemperaturePropertyConsumer;

        public IEnumerable<int> AvailableWindStrengths = new List<int> { 2, 4, 6, 8 };

        public ushort WindStrength
        {
            get { return _windStrength; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _windStrength = v, _windStrength, value) && !isInternalPropertyChange)
                {
                    var unused = _windStrengthPropertyConsumer?.SetValueAsync(value);
                }
            }
        }
        private ushort _windStrength = 2;
        private PropertyConsumer _windStrengthPropertyConsumer;

        protected override void OnStart()
        {
            _controlPanelWatcher = new ControlPanelWatcher(this.CreateBusAttachment(ref _controlPanelBusAttachment));
            _controlPanelWatcher.Added += this.controlPanelWatcher_Added;
            _controlPanelWatcher.Start();

            _propertyWatcher = new PropertyWatcher(this.CreateBusAttachment(ref _propertyBusAttachment));
            _propertyWatcher.Start();
        }

        /// <summary>
        ///
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private async void controlPanelWatcher_Added(ControlPanelWatcher sender, AllJoynServiceInfo args)
        {
            // CONTROL PANEL
            if (args.ObjectPath.Contains("AirConditioner"))
            {
                var joinResult = await ControlPanelConsumer.JoinSessionAsync(args, sender);
                if (joinResult.Status == AllJoynStatus.Ok)
                {
                    _controlPanelConsumer = joinResult.Consumer;
                    _controlPanelConsumer.SessionLost += OnSessionLost;

                    // PROPERTY
                    // Get a child Property
                    var propertyServiceInfo = new AllJoynServiceInfo(
                        args.UniqueName,
                        args.ObjectPath + "/en/airconSet/power",
                        args.SessionPort);

                    var propJoinResult = await PropertyConsumer.JoinSessionAsync(propertyServiceInfo, _propertyWatcher);
                    if (propJoinResult.Status == AllJoynStatus.Ok)
                    {
                        _powerPropertyConsumer = propJoinResult.Consumer;
                        _powerPropertyConsumer.Signals.ValueChangedReceived += this.powerPropertyConsumer_ValueChangedReceived;
                    }

                    propertyServiceInfo = new AllJoynServiceInfo(
                        args.UniqueName,
                        args.ObjectPath + "/en/set_temperature",
                        args.SessionPort);

                    propJoinResult = await PropertyConsumer.JoinSessionAsync(propertyServiceInfo, _propertyWatcher);
                    if (propJoinResult.Status == AllJoynStatus.Ok)
                    {
                        _targetTemperaturePropertyConsumer = propJoinResult.Consumer;
                        _targetTemperaturePropertyConsumer.Signals.ValueChangedReceived += this.targetTemperaturePropertyConsumer_ValueChangedReceived;
                    }

                    propertyServiceInfo = new AllJoynServiceInfo(
                        args.UniqueName,
                        args.ObjectPath + "/en/set_CurrentTemperature",
                        args.SessionPort);

                    propJoinResult = await PropertyConsumer.JoinSessionAsync(propertyServiceInfo, _propertyWatcher);
                    if (propJoinResult.Status == AllJoynStatus.Ok)
                    {
                        _currentTemperaturePropertyConsumer = propJoinResult.Consumer;
                        _currentTemperaturePropertyConsumer.Signals.ValueChangedReceived += this.targetTemperaturePropertyConsumer_ValueChangedReceived;
                    }

                    propertyServiceInfo = new AllJoynServiceInfo(
                        args.UniqueName,
                        args.ObjectPath + "/en/airconSet2/windStrength",
                        args.SessionPort);

                    propJoinResult = await PropertyConsumer.JoinSessionAsync(propertyServiceInfo, _propertyWatcher);
                    if (propJoinResult.Status == AllJoynStatus.Ok)
                    {
                        _windStrengthPropertyConsumer = propJoinResult.Consumer;
                        _windStrengthPropertyConsumer.Signals.ValueChangedReceived += this.windStrengthPropertyConsumer_ValueChangedReceived;
                    }

                    this.IsConnected = true;


                    // jtasler: Is this really neccessary? Doesn't the device send change updates?
                    var unusedTask = Task.Run(async () =>
                    {
                        while (this.IsConnected)
                        {
                            this.powerPropertyConsumer_ValueChangedReceived(null, null);
                            this.windStrengthPropertyConsumer_ValueChangedReceived(null, null);
                            this.targetTemperaturePropertyConsumer_ValueChangedReceived(null, null);
                            this.currentTemperaturePropertyConsumer_ValueChangedReceived(null, null);
                            await Task.Delay(2000);
                        }
                    });
                }
            }
        }

        private async void powerPropertyConsumer_ValueChangedReceived(PropertySignals sender, PropertyValueChangedReceivedEventArgs args)
        {
            var valueResult = await _powerPropertyConsumer?.GetValueAsync();
            if (valueResult.Status == AllJoynStatus.Ok && valueResult.Value is ushort)
            {
                using (this.CreateInternalPropertyChangeScope())
                {
                    this.Power = (ushort)valueResult.Value != 0;
                }
            }
        }

        private async void targetTemperaturePropertyConsumer_ValueChangedReceived(PropertySignals sender, PropertyValueChangedReceivedEventArgs args)
        {
            var valueResult = await _targetTemperaturePropertyConsumer?.GetValueAsync();
            if (valueResult.Status == AllJoynStatus.Ok && valueResult.Value is ushort)
            {
                using (this.CreateInternalPropertyChangeScope())
                {
                    this.TargetTemperature = (ushort)valueResult.Value;
                }
            }
        }

        private async void currentTemperaturePropertyConsumer_ValueChangedReceived(PropertySignals sender, PropertyValueChangedReceivedEventArgs args)
        {
            var valueResult = await _currentTemperaturePropertyConsumer?.GetValueAsync();
            if (valueResult.Status == AllJoynStatus.Ok && valueResult.Value is ushort)
            {
                using (this.CreateInternalPropertyChangeScope())
                {
                    this.CurrentTemperature = (ushort)valueResult.Value;
                }
            }
        }

        private async void windStrengthPropertyConsumer_ValueChangedReceived(PropertySignals sender, PropertyValueChangedReceivedEventArgs args)
        {
            var valueResult = await _windStrengthPropertyConsumer?.GetValueAsync();
            if (valueResult.Status == AllJoynStatus.Ok && valueResult.Value is ushort)
            {
                using (this.CreateInternalPropertyChangeScope())
                {
                    this.WindStrength = (ushort)valueResult.Value;
                }
            }
        }

        public ICommand DecreaseTargetTemperatureCommand
        {
            get { return new DelegateCommand(() => --this.TargetTemperature); }
        }

        public ICommand IncreaseTargetTemperatureCommand
        {
            get { return new DelegateCommand(() => ++this.TargetTemperature); }
        }

        protected override void OnSessionLost(object consumer, AllJoynSessionLostEventArgs args)
        {
            this.ResetWatcher(ref _controlPanelWatcher, ref _controlPanelBusAttachment, w => { w.Stop(); w.Added -= this.controlPanelWatcher_Added; });
            this.ResetWatcher(ref _propertyWatcher, ref _propertyBusAttachment, w => w.Stop());

            this.ResetConsumer(ref _controlPanelConsumer, c => c.SessionLost -= this.Consumer_SessionLost);
            this.ResetConsumer(ref _powerPropertyConsumer, c => { /* TODO: Unsubscribe from value changed signals */ });
            this.ResetConsumer(ref _currentTemperaturePropertyConsumer, c => { /* TODO: Unsubscribe from value changed signals */ });
            this.ResetConsumer(ref _targetTemperaturePropertyConsumer, c => { /* TODO: Unsubscribe from value changed signals */ });
            this.ResetConsumer(ref _windStrengthPropertyConsumer, c => { /* TODO: Unsubscribe from value changed signals */ });
        }
    }
}
