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

using System.Windows.Input;
using Windows.UI.Xaml;

namespace MyLivingRoom.Views
{
    public sealed partial class UpDownLayoutView
    {
        public UpDownLayoutView()
        {
            this.InitializeComponent();
        }

        public static readonly DependencyProperty HeaderProperty =
            DependencyProperty.Register("Header", typeof(object), typeof(UpDownLayoutView), new PropertyMetadata(null));

        public object Header
        {
            get { return this.GetValue(HeaderProperty); }
            set { this.SetValue(HeaderProperty, value); }
        }

        public static readonly DependencyProperty HeaderTemplateProperty =
            DependencyProperty.Register("HeaderTemplate", typeof(DataTemplate), typeof(UpDownLayoutView), new PropertyMetadata(null));

        public DataTemplate HeaderTemplate
        {
            get { return (DataTemplate)this.GetValue(HeaderTemplateProperty); }
            set { this.SetValue(HeaderTemplateProperty, value); }
        }

        public static readonly DependencyProperty ValueStringProperty =
            DependencyProperty.Register("ValueString", typeof(string), typeof(UpDownLayoutView), new PropertyMetadata("---"));

        public string ValueString
        {
            get { return (string)this.GetValue(ValueStringProperty); }
            set { this.SetValue(ValueStringProperty, value); }
        }

        public static readonly DependencyProperty DownCommandProperty =
            DependencyProperty.Register("DownCommand", typeof(ICommand), typeof(UpDownLayoutView), new PropertyMetadata(null));

        public ICommand DownCommand
        {
            get { return (ICommand)this.GetValue(DownCommandProperty); }
            set { this.SetValue(DownCommandProperty, value); }
        }

        public static readonly DependencyProperty UpCommandProperty =
            DependencyProperty.Register("UpCommand", typeof(ICommand), typeof(UpDownLayoutView), new PropertyMetadata(null));

        public ICommand UpCommand
        {
            get { return (ICommand)this.GetValue(UpCommandProperty); }
            set { this.SetValue(UpCommandProperty, value); }
        }
    }
}
