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
using Windows.UI.Text;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace MyLivingRoom.Controls
{
    public class FontIconDefinition : IconDefinition
    {
        public FontFamily FontFamily { get; set; }
        public double FontSize { get; set; } = 20;
        public FontStyle FontStyle { get; set; } = FontStyle.Normal;
        public FontWeight FontWeight { get; set; } = FontWeights.Normal;
        public string Glyph { get; set; }

        public override UIElement CreateChildElement()
        {
            return new TextBlock
            {
                FontFamily = this.FontFamily,
                FontSize = this.FontSize,
                FontStyle = this.FontStyle,
                FontWeight = this.FontWeight,
                Text = this.Glyph,
                TextWrapping = TextWrapping.NoWrap,
                TextAlignment = TextAlignment.Center,
                IsTextSelectionEnabled = false,
                TextTrimming = TextTrimming.None,
                LineStackingStrategy = LineStackingStrategy.BlockLineHeight,
                MaxLines = 1,
            };
        }
    }
}
