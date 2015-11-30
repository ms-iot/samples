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

using BridgeRT;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace AdapterLib
{
    class SignalParameter: IAdapterValue
    {
        public object Data { get; set; }
        public string Name { get; }

        internal SignalParameter(string name)
        {
            Name = name;
            Data = null;
        }
    }
    //
    // AdapterSignal.
    // Description:
    // The class that implements IAdapterSignal from BridgeRT.
    //
    class AdapterSignal : IAdapterSignal
    {
        // public properties
        public string Name { get; }

        public IList<IAdapterValue> Params { get; }

        internal AdapterSignal(string ObjectName)
        {
            this.Name = ObjectName;

            try
            {
                this.Params = new List<IAdapterValue>();
            }
            catch (OutOfMemoryException ex)
            {
                Debug.WriteLine(ex);
                throw;
            }
        }

        internal AdapterSignal(AdapterSignal Other)
        {
            Name = Other.Name;

            try
            {
                Params = new List<IAdapterValue>(Other.Params);
            }
            catch (OutOfMemoryException ex)
            {
                Debug.WriteLine(ex);
                throw;
            }
        }

        internal void AddParam(string name)
        {
            SignalParameter param = new SignalParameter(name);
            Params.Add(param);
        }
    }
}
