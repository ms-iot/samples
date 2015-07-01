/**
    Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.
   The MIT License(MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
**/

using System;
using System.Collections.Generic;
using System.Linq;
using Windows.Foundation.Collections;

namespace DigitalSignageUAP.Common
{
    /// <summary>
    /// Implementation of IObservableMap that supports reentrancy for use as a default view
    /// model.
    /// </summary>
    public class ObservableDictionary : IObservableMap<string, object>
    {
        private class ObservableDictionaryChangedEventArgs : IMapChangedEventArgs<string>
        {
            public ObservableDictionaryChangedEventArgs(CollectionChange change, string key)
            {
                this.CollectionChange = change;
                this.Key = key;
            }

            public CollectionChange CollectionChange { get; private set; }
            public string Key { get; private set; }
        }

        private Dictionary<string, object> _dictionary = new Dictionary<string, object>();
        public event MapChangedEventHandler<string, object> MapChanged;

        private void InvokeMapChanged(CollectionChange change, string key)
        {
            var eventHandler = MapChanged;
            if (eventHandler != null)
            {
                eventHandler(this, new ObservableDictionaryChangedEventArgs(change, key));
            }
        }

        public void Add(string key, object value)
        {
            this._dictionary.Add(key, value);
            this.InvokeMapChanged(CollectionChange.ItemInserted, key);
        }

        public void Add(KeyValuePair<string, object> item)
        {
            this.Add(item.Key, item.Value);
        }

        public bool Remove(string key)
        {
            if (this._dictionary.Remove(key))
            {
                this.InvokeMapChanged(CollectionChange.ItemRemoved, key);
                return true;
            }
            return false;
        }

        public bool Remove(KeyValuePair<string, object> item)
        {
            object currentValue;
            if (this._dictionary.TryGetValue(item.Key, out currentValue) &&
                Object.Equals(item.Value, currentValue) && this._dictionary.Remove(item.Key))
            {
                this.InvokeMapChanged(CollectionChange.ItemRemoved, item.Key);
                return true;
            }
            return false;
        }

        public object this[string key]
        {
            get
            {
                return this._dictionary[key];
            }
            set
            {
                this._dictionary[key] = value;
                this.InvokeMapChanged(CollectionChange.ItemChanged, key);
            }
        }

        public void Clear()
        {
            var priorKeys = this._dictionary.Keys.ToArray();
            this._dictionary.Clear();
            foreach (var key in priorKeys)
            {
                this.InvokeMapChanged(CollectionChange.ItemRemoved, key);
            }
        }

        public ICollection<string> Keys
        {
            get { return this._dictionary.Keys; }
        }

        public bool ContainsKey(string key)
        {
            return this._dictionary.ContainsKey(key);
        }

        public bool TryGetValue(string key, out object value)
        {
            return this._dictionary.TryGetValue(key, out value);
        }

        public ICollection<object> Values
        {
            get { return this._dictionary.Values; }
        }

        public bool Contains(KeyValuePair<string, object> item)
        {
            return this._dictionary.Contains(item);
        }

        public int Count
        {
            get { return this._dictionary.Count; }
        }

        public bool IsReadOnly
        {
            get { return false; }
        }

        public IEnumerator<KeyValuePair<string, object>> GetEnumerator()
        {
            return this._dictionary.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return this._dictionary.GetEnumerator();
        }

        public void CopyTo(KeyValuePair<string, object>[] array, int arrayIndex)
        {
            int arraySize = array.Length;
            foreach (var pair in this._dictionary)
            {
                if (arrayIndex >= arraySize) break;
                array[arrayIndex++] = pair;
            }
        }
    }
}
