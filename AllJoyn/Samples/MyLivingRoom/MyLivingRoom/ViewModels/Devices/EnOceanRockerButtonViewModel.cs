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
using com.enocean.Bridge.RockerButton.interface_1;
using MyLivingRoom.Extensions;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public class EnOceanRockerButtonViewModel : BaseDeviceViewModel
    {
        #region com.enocean.Bridge.RockerButton.interface_1

        private AllJoynBusAttachment _interface1BusAttachment;
        private interface_1Consumer _interface1Consumer;
        private interface_1Watcher _interface1Watcher;

        private async void powerinterface_1Watcher_Added(interface_1Watcher sender, AllJoynServiceInfo args)
        {
            var joinResult = await interface_1Consumer.JoinSessionAsync(args, sender);
            if (joinResult.Status == AllJoynStatus.Ok)
            {
                _interface1Consumer = joinResult.Consumer;
                _interface1Consumer.SessionLost += this.Consumer_SessionLost;

                // subscribe to value changes
                _interface1Consumer.ButtonA0Changed += this.interface1Consumer_ButtonA0Changed;
                _interface1Consumer.ButtonAIChanged += this.interface1Consumer_ButtonAIChanged;
                _interface1Consumer.ButtonB0Changed += this.interface1Consumer_ButtonB0Changed;
                _interface1Consumer.ButtonBIChanged += this.interface1Consumer_ButtonBIChanged;

                // populate initial values

                //var asyncA0 = _interface1Consumer.GetButtonA0Async();
                //var asyncAI = _interface1Consumer.GetButtonAIAsync();
                //var asyncB0 = _interface1Consumer.GetButtonB0Async();
                //var asyncBI = _interface1Consumer.GetButtonBIAsync();

                //await Task.WhenAll(asyncA0.AsTask(), asyncAI.AsTask(), asyncB0.AsTask(), asyncBI.AsTask());

                //var resultA0 = asyncA0.GetResults();
                //var resultAI = asyncAI.GetResults();
                //var resultB0 = asyncB0.GetResults();
                //var resultBI = asyncBI.GetResults();

                //if (resultA0.Status != AllJoynStatus.Ok) return;
                //if (resultAI.Status != AllJoynStatus.Ok) return;
                //if (resultB0.Status != AllJoynStatus.Ok) return;
                //if (resultBI.Status != AllJoynStatus.Ok) return;

                //using (this.CreateInternalPropertyChangeScope())
                //{
                //    this.IsButtonA0Pressed = object.Equals(resultA0.ButtonA0, "1");
                //    this.IsButtonAIPressed = object.Equals(resultAI.ButtonAI, "1");
                //    this.IsButtonB0Pressed = object.Equals(resultB0.ButtonB0, "1");
                //    this.IsButtonBIPressed = object.Equals(resultBI.ButtonBI, "1");
                //}

                this.IsConnected = true;
            }
        }

        #region IsButtonA0Pressed

        public bool IsButtonA0Pressed
        {
            get { return _isButtonA0Pressed; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _isButtonA0Pressed = v, _isButtonA0Pressed, value))
                {
                    //if (!isInternalPropertyChange)
                    //{
                    //    var unused = _interface1Consumer?.SetButtonA0Async(value ? "1" : "0");
                    //}

                    if (value)
                    {
                        this.Dispatcher.ExecuteOrDispatchAsync(() => ButtonA0ActionInvocation.SelectTopic());
                    }
                }
            }
        }
        private bool _isButtonA0Pressed;

        private void interface1Consumer_ButtonA0Changed(interface_1Consumer consumer, object value)
        {
            this.IsButtonA0Pressed = object.Equals(value, "1");
        }

        #endregion IsButtonA0Pressed

        #region IsButtonAIPressed

        public bool IsButtonAIPressed
        {
            get { return _isButtonAIPressed; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _isButtonAIPressed = v, _isButtonAIPressed, value))
                {
                    //if (!isInternalPropertyChange)
                    //{
                    //    var unused = _interface1Consumer?.SetButtonAIAsync(value ? "1" : "0");
                    //}

                    if (value)
                    {
                        this.Dispatcher.ExecuteOrDispatchAsync(() => ButtonAIActionInvocation.SelectTopic());
                    }
                }
            }
        }
        private bool _isButtonAIPressed;

        private void interface1Consumer_ButtonAIChanged(interface_1Consumer consumer, object value)
        {
            this.IsButtonAIPressed = object.Equals(value, "1");
        }

        #endregion IsButtonAIPressed

        #region IsButtonB0Pressed

        public bool IsButtonB0Pressed
        {
            get { return _isButtonB0Pressed; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _isButtonB0Pressed = v, _isButtonB0Pressed, value))
                {
                    //if (!isInternalPropertyChange)
                    //{
                    //    var unused = _interface1Consumer?.SetButtonB0Async(value ? "1" : "0");
                    //}

                    if (value)
                    {
                        this.Dispatcher.ExecuteOrDispatchAsync(() => ButtonB0ActionInvocation.SelectTopic());
                    }
                }
            }
        }
        private bool _isButtonB0Pressed;

        private void interface1Consumer_ButtonB0Changed(interface_1Consumer consumer, object value)
        {
            this.IsButtonB0Pressed = object.Equals(value, "1");
        }

        #endregion IsButtonB0Pressed

        #region IsButtonBIPressed

        public bool IsButtonBIPressed
        {
            get { return _isButtonBIPressed; }
            set
            {
                var isInternalPropertyChange = this.IsInternalPropertyChange;
                if (this.SetPropertyOnDispatcher(v => _isButtonBIPressed = v, _isButtonBIPressed, value))
                {
                    //if (!isInternalPropertyChange)
                    //{
                    //    var unused = _interface1Consumer?.SetButtonBIAsync(value ? "1" : "0");
                    //}

                    if (value)
                    {
                        this.Dispatcher.ExecuteOrDispatchAsync(() => ButtonBIActionInvocation.SelectTopic());
                    }
                }
            }
        }
        private bool _isButtonBIPressed;

        private void interface1Consumer_ButtonBIChanged(interface_1Consumer consumer, object value)
        {
            this.IsButtonBIPressed = object.Equals(value, "1");
        }

        #endregion IsButtonBIPressed

        #endregion com.enocean.Bridge.RockerButton.interface_1

        #region Overrides

        protected override void OnStart()
        {
            _interface1Watcher = new interface_1Watcher(this.CreateBusAttachment(ref _interface1BusAttachment));
            _interface1Watcher.Added += this.powerinterface_1Watcher_Added;
            _interface1Watcher.Start();
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
            this.ResetWatcher(ref _interface1Watcher, ref _interface1BusAttachment, w => { w.Stop(); w.Added -= this.powerinterface_1Watcher_Added; });
            this.ResetConsumer(ref _interface1Consumer, c =>
            {
                c.SessionLost -= this.Consumer_SessionLost;
                _interface1Consumer.ButtonA0Changed -= this.interface1Consumer_ButtonA0Changed;
                _interface1Consumer.ButtonAIChanged -= this.interface1Consumer_ButtonAIChanged;
                _interface1Consumer.ButtonB0Changed -= this.interface1Consumer_ButtonB0Changed;
                _interface1Consumer.ButtonBIChanged -= this.interface1Consumer_ButtonBIChanged;
            });
        }

        #endregion Overrides

        private static ActionInvocationTopicDefinition ButtonA0ActionInvocation { get; } = new ActionInvocationTopicDefinition
        {
            Actions = { new InvocationAction { DeviceId = "LIFXLightbulb", PropertyName = "IsOn", PropertyValue = true } }
        };

        private static ActionInvocationTopicDefinition ButtonAIActionInvocation { get; } = new ActionInvocationTopicDefinition
        {
            Actions = { new InvocationAction { DeviceId = "LIFXLightbulb", PropertyName = "IsOn", PropertyValue = false } }
        };

        private static ActionInvocationTopicDefinition ButtonB0ActionInvocation { get; } = new ActionInvocationTopicDefinition
        {
            Actions = { new InvocationAction { DeviceId = "ZWaveSwitch", PropertyName = "IsOn", PropertyValue = true } }
        };

        private static ActionInvocationTopicDefinition ButtonBIActionInvocation { get; } = new ActionInvocationTopicDefinition
        {
            Actions = { new InvocationAction { DeviceId = "ZWaveSwitch", PropertyName = "IsOn", PropertyValue = false } }
        };
    }
}
