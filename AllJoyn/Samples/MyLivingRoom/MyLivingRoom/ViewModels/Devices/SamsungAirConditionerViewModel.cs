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
using System.Linq;
using com.microsoft.OICBridge.oic.r.airflow;
using com.microsoft.OICBridge.oic.r.mode;
using com.microsoft.OICBridge.oic.r.Switch.binary;
using com.microsoft.OICBridge.oic.r.temperature;
using MyLivingRoom.Extensions;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class SamsungAirConditionerViewModel : BaseDeviceViewModel
    {
        private const string _expectedBusObjectPath = "aircon";

        #region com.microsoft.OICBridge.oic.r.airflow

        private AllJoynBusAttachment _airflowBusAttachment;
        private airflowWatcher _airflowWatcher;
        private airflowConsumer _airflowConsumer;

        private async void airflowWatcher_Added(airflowWatcher watcher, AllJoynServiceInfo args)
        {
            if (_airflowConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinSessionResult = await airflowConsumer.JoinSessionAsync(args, watcher);

                if (joinSessionResult.Status == AllJoynStatus.Ok)
                {
                    _airflowConsumer = joinSessionResult.Consumer;
                    _airflowConsumer.SessionLost += Consumer_SessionLost;

                    // subscribe to value changes
                    _airflowConsumer.DirectionChanged += this.airflowConsumer_DirectionChanged;
                    _airflowConsumer.RangeChanged += this.airflowConsumer_RangeChanged;
                    _airflowConsumer.SpeedChanged += this.airflowConsumer_SpeedChanged;

                    // populate initial values
                    var directionResult = await _airflowConsumer.GetDirectionAsync();
                    if (directionResult.Status != AllJoynStatus.Ok) return;
                    this.AirflowDirection = directionResult.Direction;

                    var rangeResult = await _airflowConsumer.GetRangeAsync();
                    if (rangeResult.Status != AllJoynStatus.Ok) return;
                    this.AirflowDirection = rangeResult.Range;

                    var speedResult = await _airflowConsumer.GetSpeedAsync();
                    if (speedResult.Status != AllJoynStatus.Ok) return;
                    this.AirflowSpeed = speedResult.Speed;


                    this.IsConnected = true;
                }
            }
        }

        public List<string> AirflowDirectionOptions
        {
            get { return new List<string> { "up", "down", "left", "right" }; }
        }

        #region AirflowDirection

        public string AirflowDirection
        {
            get { return _airflowDirection; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _airflowDirection = v, _airflowDirection, value))
                {
                    var unused = _airflowConsumer?.SetDirectionAsync(_airflowDirection);
                }
            }
        }
        private string _airflowDirection;

        private void airflowConsumer_DirectionChanged(airflowConsumer consumer, object value)
        {
            if (value is string)
            {
                this.AirflowDirection = (string)value;
            }
        }

        #endregion AirflowDirection

        #region AirflowRange

        public string AirflowRange
        {
            get { return _airflowRange; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _airflowRange = v, _airflowRange, value))
                {
                    var unused = _airflowConsumer?.SetRangeAsync(_airflowRange);
                }
            }
        }
        private string _airflowRange;

        private void airflowConsumer_RangeChanged(airflowConsumer consumer, object value)
        {
            if (value is string)
            {
                this.AirflowRange = (string)value;
            }
        }

        #endregion AirflowRange

        #region AirflowSpeed

        public Int64 AirflowSpeed
        {
            get { return _airflowSpeed; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _airflowSpeed = v, _airflowSpeed, value))
                {
                    var unused = _airflowConsumer?.SetSpeedAsync(_airflowSpeed);
                }
            }
        }
        private Int64 _airflowSpeed;

        private void airflowConsumer_SpeedChanged(airflowConsumer consumer, object value)
        {
            if (value is Int64)
            {
                this.AirflowSpeed = (Int64)value;
            }
        }

        #endregion AirflowSpeed

        #endregion com.microsoft.OICBridge.oic.r.airflow

        #region com.microsoft.OICBridge.oic.r.mode

        private AllJoynBusAttachment _modeBusAttachment;
        private modeWatcher _modeWatcher;
        private modeConsumer _modeConsumer;

        private async void modeWatcher_Added(modeWatcher watcher, AllJoynServiceInfo args)
        {
            if (_modeConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinSessionResult = await modeConsumer.JoinSessionAsync(args, watcher);
                if (joinSessionResult.Status == AllJoynStatus.Ok)
                {
                    _modeConsumer = joinSessionResult.Consumer;
                    _modeConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _modeConsumer.ModesChanged += this.modeConsumer_ModesChanged;
                    _modeConsumer.SupportedModesChanged += this.modeConsumer_SupportedModesChanged;

                    // populate initial values
                    var modesResult = await _modeConsumer.GetModesAsync();
                    if (modesResult.Status != AllJoynStatus.Ok) return;
                    this.Modes = modesResult.Modes;

                    var supportedModesResult = await _modeConsumer.GetSupportedModesAsync();
                    if (supportedModesResult.Status != AllJoynStatus.Ok) return;
                    this.SupportedModes = supportedModesResult.SupportedModes;

                    this.IsConnected = true;
                }
            }
        }

        #region Modes

        public string Modes
        {
            get { return _modes; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _modes = v, _modes, value))
                {
                    var unused = _modeConsumer?.SetModesAsync(_modes);
                }
            }
        }
        private string _modes;

        private void modeConsumer_ModesChanged(modeConsumer consumer, object value)
        {
            if (value is string)
            {
                this.Modes = (string)value;
            }
        }

        #endregion Modes

        #region SupportedModes

        public List<string> SupportedModesList
        {
            get { return this.SupportedModes.Split(',').ToList(); }
        }

        private string SupportedModes
        {
            get { return _supportedModes; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _supportedModes = v, _supportedModes, value))
                {
                    this.RaisePropertyChangedOnDispatcher(nameof(this.SupportedModesList));
                    var unused = _modeConsumer?.SetSupportedModesAsync(_supportedModes);
                }
            }
        }
        private string _supportedModes = string.Empty;

        private void modeConsumer_SupportedModesChanged(modeConsumer consumer, object value)
        {
            if (value is string)
            {
                this.SupportedModes = (string)value;
            }
        }

        #endregion SupportedModes

        #endregion com.microsoft.OICBridge.oic.r.mode

        #region com.microsoft.OICBridge.oic.r.Switch.binary

        private AllJoynBusAttachment _binaryBusAttachment;
        private binaryWatcher _binaryWatcher;
        private binaryConsumer _binaryConsumer;

        private async void binaryWatcher_Added(binaryWatcher watcher, AllJoynServiceInfo args)
        {
            if (_binaryConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinResult = await binaryConsumer.JoinSessionAsync(args, watcher);
                if (joinResult.Status == AllJoynStatus.Ok)
                {
                    _binaryConsumer = joinResult.Consumer;
                    _binaryConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _binaryConsumer.ValueChanged += this.binaryConsumer_ValueChanged;

                    // populate initial values
                    var valueResult = await _binaryConsumer.GetValueAsync();
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
                    var unused = _binaryConsumer?.SetValueAsync(_isOn);
                }
            }
        }
        private bool _isOn;

        private void binaryConsumer_ValueChanged(binaryConsumer consumer, object value)
        {
            if (value is bool)
            {
                this.IsOn = (bool)value;
            }
        }

        #endregion com.microsoft.OICBridge.oic.r.Switch.binary

        #region com.microsoft.OICBridge.oic.r.temperature

        private AllJoynBusAttachment _temperatureBusAttachment;
        private temperatureWatcher _temperatureWatcher;
        private temperatureConsumer _temperatureConsumer;

        private async void temperatureWatcher_Added(temperatureWatcher watcher, AllJoynServiceInfo args)
        {
            if (_temperatureConsumer == null && args.ObjectPath.Contains(_expectedBusObjectPath))
            {
                var joinSessionResult = await temperatureConsumer.JoinSessionAsync(args, watcher);

                if (joinSessionResult.Status == AllJoynStatus.Ok)
                {
                    _temperatureConsumer = joinSessionResult.Consumer;
                    _temperatureConsumer.SessionLost += this.Consumer_SessionLost;

                    // subscribe to value changes
                    _temperatureConsumer.TemperatureChanged += this.temperatureConsumer_TemperatureChanged;
                    _temperatureConsumer.RangeChanged += this.temperatureConsumer_RangeChanged;
                    _temperatureConsumer.UnitsChanged += this.temperatureConsumer_UnitsChanged;

                    // populate initial values
                    var temperatureResult = await _temperatureConsumer.GetTemperatureAsync();
                    if (temperatureResult.Status != AllJoynStatus.Ok) return;
                    this.Temperature = temperatureResult.Temperature;

                    var temperatureRangeResult = await _temperatureConsumer.GetRangeAsync();
                    if (temperatureRangeResult.Status != AllJoynStatus.Ok) return;
                    this.TemperatureRange = temperatureRangeResult.Range;

                    var temperatureUnitsResult = await _temperatureConsumer.GetUnitsAsync();
                    if (temperatureUnitsResult.Status != AllJoynStatus.Ok) return;
                    this.TemperatureUnits = temperatureUnitsResult.Units;

                    this.IsConnected = true;
                }
            }
        }

        #region Temperature

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

        private void temperatureConsumer_TemperatureChanged(temperatureConsumer consumer, object value)
        {
            if (value is double)
            {
                this.Temperature = (double)value;
            }
        }

        #endregion Temperature

        #region TemperatureRange

        public string TemperatureRange
        {
            get { return _temperatureRange; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _temperatureRange = v, _temperatureRange, value))
                {
                    var unused = _temperatureConsumer?.SetRangeAsync(_temperatureRange);
                }
            }
        }
        private string _temperatureRange;

        private void temperatureConsumer_RangeChanged(temperatureConsumer consumer, object value)
        {
            if (value is string)
            {
                this.TemperatureRange = (string)value;
            }
        }

        #endregion TemperatureRange

        #region TemperatureUnits

        public string TemperatureUnits
        {
            get { return _temperatureUnits; }
            set
            {
                if (this.SetPropertyOnDispatcher(v => _temperatureUnits = v, _temperatureUnits, value))
                {
                    var unused = _temperatureConsumer?.SetUnitsAsync(_temperatureUnits);
                }
            }
        }
        private string _temperatureUnits;

        private void temperatureConsumer_UnitsChanged(temperatureConsumer consumer, object value)
        {
            if (value is string)
            {
                this.TemperatureUnits = (string)value;
            }
        }

        #endregion TemperatureUnits

        #endregion com.microsoft.OICBridge.oic.r.temperature

        #region Overrides

        protected override void OnStart()
        {
            _modeWatcher = new modeWatcher(this.CreateBusAttachment(ref _modeBusAttachment));
            _modeWatcher.Added += this.modeWatcher_Added;
            _modeWatcher.Start();

            _airflowWatcher = new airflowWatcher(this.CreateBusAttachment(ref _airflowBusAttachment));
            _airflowWatcher.Added += this.airflowWatcher_Added;
            _airflowWatcher.Start();

            _binaryWatcher = new binaryWatcher(this.CreateBusAttachment(ref _binaryBusAttachment));
            _binaryWatcher.Added += this.binaryWatcher_Added;
            _binaryWatcher.Start();

            _temperatureWatcher = new temperatureWatcher(this.CreateBusAttachment(ref _temperatureBusAttachment));
            _temperatureWatcher.Added += this.temperatureWatcher_Added;
            _temperatureWatcher.Start();
        }

        protected override void OnSessionLost(object sender, AllJoynSessionLostEventArgs args)
        {
            // Reset all watchers and consumers

            this.ResetWatcher(ref _modeWatcher, ref _modeBusAttachment, w => { w.Stop(); w.Added -= this.modeWatcher_Added; });
            this.ResetConsumer(ref _modeConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.ModesChanged -= this.modeConsumer_ModesChanged;
                c.SupportedModesChanged -= this.modeConsumer_SupportedModesChanged;
            });

            this.ResetWatcher(ref _binaryWatcher, ref _binaryBusAttachment, w => { w.Stop(); w.Added -= this.binaryWatcher_Added; });
            this.ResetConsumer(ref _binaryConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.ValueChanged -= this.binaryConsumer_ValueChanged;
            });

            this.ResetWatcher(ref _airflowWatcher, ref _airflowBusAttachment, w => { w.Stop(); w.Added -= this.airflowWatcher_Added; });
            this.ResetConsumer(ref _airflowConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.DirectionChanged -= this.airflowConsumer_DirectionChanged;
                c.RangeChanged -= this.airflowConsumer_RangeChanged;
                c.SpeedChanged -= this.airflowConsumer_SpeedChanged;
            });

            this.ResetWatcher(ref _temperatureWatcher, ref _temperatureBusAttachment, w => { w.Stop(); w.Added -= this.temperatureWatcher_Added; });
            this.ResetConsumer(ref _temperatureConsumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                c.TemperatureChanged -= this.temperatureConsumer_TemperatureChanged;
                c.RangeChanged -= this.temperatureConsumer_RangeChanged;
                c.UnitsChanged -= this.temperatureConsumer_UnitsChanged;
            });
        }

        #endregion Overrides
    }
}
