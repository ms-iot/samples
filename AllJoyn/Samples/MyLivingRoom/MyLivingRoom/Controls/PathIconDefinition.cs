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
using Windows.Foundation;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Markup;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Shapes;

namespace MyLivingRoom.Controls
{
    [ContentProperty(Name = nameof(Data))]
    public class PathIconDefinition : IconDefinition
    {
        public Rect Bounds { get; set; }
        public FillRule FillMode { get; set; }
        public string Data { get; set; }

        public override UIElement CreateChildElement()
        {
            var element = (Path)XamlReader.Load(
                $"<Path xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' Data='{this.Data}' />");

            element.Fill = new SolidColorBrush(Colors.White);
            return element;
        }
    }
}
