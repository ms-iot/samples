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
using System.Runtime.CompilerServices;
using System.Threading;
using MyLivingRoom.Services;

namespace MyLivingRoom.Extensions
{
    public static class ViewModelExtensions
    {
        public static bool SetPropertyOnDispatcher<T,V>(this T @this, Action<V> setter, V oldValue, V newValue, [CallerMemberName] string propertyName = null)
            where T : IDispatcherObject, IRaisePropertyChanged
        {
            var propertyChanged = false;
            @this.Dispatcher.ExecuteOrDispatchAsync(() =>
            {
                propertyChanged = !object.Equals(oldValue, newValue);
                if (propertyChanged)
                {
                    setter(newValue);
                    @this.Dispatcher.ExecuteOrDispatchAsync(() => @this.RaisePropertyChanged(propertyName));
                }

            }).Wait(Timeout.Infinite);

            return propertyChanged;
        }

        public static void RaisePropertyChangedOnDispatcher<T>(this T @this, params string[] propertyNames)
            where T : IDispatcherObject, IRaisePropertyChanged
        {
            if (propertyNames?.Length > 0)
            {
                @this.Dispatcher.ExecuteOrDispatchAsync(() =>
                {
                    foreach (var propertyName in propertyNames)
                    {
                        @this.RaisePropertyChanged(propertyName);
                    }
                });
            }

        }
    }
}
