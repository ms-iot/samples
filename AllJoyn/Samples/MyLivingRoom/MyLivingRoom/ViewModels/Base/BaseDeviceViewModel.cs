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
using System.Threading;
using System.Threading.Tasks;
using MyLivingRoom.Core;
using MyLivingRoom.Extensions;
using Windows.Devices.AllJoyn;

namespace MyLivingRoom.ViewModels
{
    public abstract class BaseDeviceViewModel : BaseTopicViewModel
    {
        private const int _startupDelay = 500;

        public BaseDeviceViewModel()
        {
            Task.Delay(_startupDelay).ContinueWith(t => this.Start(),
                TaskContinuationOptions.ExecuteSynchronously);
        }

        #region Properties

        public bool IsConnected
        {
            get { return _isConnected; }
            protected set
            {
                this.SetPropertyOnDispatcher(v => this.IsBusy = !(_isConnected = v), _isConnected, value);
            }
        }
        private bool _isConnected;

        public bool IsBusy
        {
            get { return _isBusy; }
            private set { this.SetPropertyOnDispatcher(v => _isBusy = v, _isBusy, value); }
        }
        private bool _isBusy = true;

        #endregion Properties

        #region Protected Implementation

        protected bool IsInternalPropertyChange
        {
            get { return _internalPropertyChangeCount > 0; }
        }

        protected IDisposable CreateInternalPropertyChangeScope()
        {
            return new DisposeActionScope(
                () => Interlocked.Increment(ref _internalPropertyChangeCount),
                () => Interlocked.Decrement(ref _internalPropertyChangeCount));
        }
        private int _internalPropertyChangeCount;

        protected void Start()
        {
            this.OnStart();
        }

        protected void Restart()
        {
            this.Start();
        }

        protected AllJoynBusAttachment CreateBusAttachment(ref AllJoynBusAttachment busAttachment)
        {
            busAttachment?.Disconnect();
            busAttachment = new AllJoynBusAttachment();
            busAttachment.Connect();
            return busAttachment;
        }

        protected void Consumer_SessionLost<T>(T sender, AllJoynSessionLostEventArgs args)
        {
            // No longer connected
            this.IsConnected = false;

            // Reset everything
            this.OnSessionLost(sender, args);

            // Restart
            this.Start();
        }

        protected void ResetWatcher<T>(ref T watcher, ref AllJoynBusAttachment _busAttachment, Action<T> watcherStop)
            where T : class, IDisposable
        {
            _busAttachment.Disconnect();
            _busAttachment = null;

            using (var disposable = watcher)
            {
                watcherStop?.Invoke(watcher);
                watcher = null;
            }
        }

        protected void ResetConsumer<T>(ref T consumer, Action<T> consumerUnsubscribe = null)
            where T : class, IDisposable
        {
            using (var disposable = consumer)
            {
                consumerUnsubscribe?.Invoke(consumer);
                consumer = null;
            }
        }

        #endregion Protected Implementation

        #region Abstract Methods

        protected abstract void OnStart();

        protected abstract void OnSessionLost(object sender, AllJoynSessionLostEventArgs args);

        #endregion Abstract Methods
    }
}
