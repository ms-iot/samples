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

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace MyLivingRoom.Controls
{
    public sealed class IconView : Grid
    {
        private Viewbox Viewbox { get; }

        public IconView()
        {
            this.Viewbox = new Viewbox
            {
                Stretch = this.Stretch,
                StretchDirection = this.StretchDirection
            };
        }

        public static readonly DependencyProperty IconDefinitionProperty = DependencyProperty.Register(
            nameof(IconDefinition), typeof(IconDefinition), typeof(IconView), new PropertyMetadata(null, IconDefinitionPropertyChanged));

        public IconDefinition IconDefinition
        {
            get { return (IconDefinition)this.GetValue(IconDefinitionProperty); }
            set { this.SetValue(IconDefinitionProperty, value); }
        }

        private static void IconDefinitionPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var @this = d as IconView;
            if (@this != null)
            {
                var newValue = e.NewValue as IconDefinition;
                @this.Children.Clear();
                @this.Viewbox.Child = newValue.CreateChildElement();
                @this.Children.Add(@this.Viewbox);
            }
        }

        public Stretch Stretch
        {
            get { return _stretch; }
            set
            {
                if (_stretch != value)
                {
                    _stretch = value;
                    this.Viewbox.Stretch = value;
                }
            }
        }
        private Stretch _stretch = Stretch.Uniform;

        public StretchDirection StretchDirection
        {
            get { return _stretchDirection; }
            set
            {
                if (_stretchDirection != value)
                {
                    _stretchDirection = value;
                    this.Viewbox.StretchDirection = value;
                }
            }
        }
        private StretchDirection _stretchDirection = StretchDirection.Both;
    }
}
