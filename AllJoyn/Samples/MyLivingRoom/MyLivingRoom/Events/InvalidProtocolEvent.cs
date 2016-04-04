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
using System.Text;
using MyLivingRoom.Resources;
using Prism.Events;
using Windows.Foundation;

namespace MyLivingRoom.Events
{

    public class InvalidProtocolEvent : PubSubEvent<InvalidProtocolEventArgs>
    {
    }

    public class InvalidProtocolEventArgs
    {
        public InvalidProtocolEventArgs(Exception exception)
            : this(null, null)
        {
        }

        public InvalidProtocolEventArgs(Uri uri, IEnumerable<string> remainingSegments, IWwwFormUrlDecoderEntry invalidQueryEntry = null)
        {
            this.Uri = uri;
            this.RemainingSegments = remainingSegments != null ? remainingSegments.ToList() : (IEnumerable<string>)new string[0];
            if (invalidQueryEntry != null)
            {
                this.QueryEntryName = invalidQueryEntry.Name;
                this.QueryEntryValue = invalidQueryEntry.Value;
            }
        }

        public Exception Exception { get; }

        public Uri Uri { get; }

        public IEnumerable<string> RemainingSegments { get; }

        public string QueryEntryName { get; }

        public string QueryEntryValue { get; }

        public override string ToString()
        {
            if (this.Exception != null)
            {
                return this.Exception.Message;
            }

            var hasQueryValue = this.QueryEntryName == null || this.QueryEntryValue == null;

            var formatString = default(string);
            if (this.RemainingSegments.Count() != 0)
            {
                formatString = hasQueryValue ? Strings.InvalidUriAtSegmentAndQuery : Strings.InvalidUriAtSegment;
            }
            else
            {
                formatString = hasQueryValue ? Strings.InvalidUriAtQuery : string.Empty;
            }

            return string.Format(formatString, string.Join("/", this.RemainingSegments), this.QueryEntryName, this.QueryEntryValue);
        }
    }
}
