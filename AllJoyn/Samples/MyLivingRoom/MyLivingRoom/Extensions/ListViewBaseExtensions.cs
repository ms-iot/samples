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
using System.Windows.Input;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace MyLivingRoom.Views
{
    public static class ListViewBaseExtensions
    {
        #region ItemClickCommand

        public static readonly DependencyProperty ItemClickCommandProperty =
            DependencyProperty.RegisterAttached("ItemClickCommand", typeof(ICommand), typeof(ListViewBaseExtensions),
                new PropertyMetadata(null, ItemClickCommandPropertyChanged));

        public static ICommand GetItemClickCommand(FrameworkElement d)
        {
            if (d == null)
                throw new ArgumentNullException(nameof(d));
            if (!(d is ListViewBase))
                throw new ArgumentException(nameof(d));

            return (ICommand)d.GetValue(ItemClickCommandProperty);
        }

        public static void SetItemClickCommand(FrameworkElement d, ICommand value)
        {
            if (d == null)
                throw new ArgumentNullException(nameof(d));
            if (!(d is ListViewBase))
                throw new ArgumentException(nameof(d));

            d.SetValue(ItemClickCommandProperty, value);
        }

        private static void ItemClickCommandPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var listViewBase = (ListViewBase)d;
            var newValue = (ICommand)e.NewValue;

            if (newValue == null)
            {
                listViewBase.ItemClick -= ListViewBase_ItemClick;
            }
            else
            {
                listViewBase.ItemClick += ListViewBase_ItemClick;
            }
        }

        private static void ListViewBase_ItemClick(object sender, ItemClickEventArgs e)
        {
            var listViewBase = (ListViewBase)sender;
            var command = GetItemClickCommand(listViewBase);
            if (command != null)
            {
                var clickedItem = e.ClickedItem as FrameworkElement;
                var commandParameter = clickedItem != null ? clickedItem.DataContext : e.ClickedItem;

                if (command.CanExecute(commandParameter))
                {
                    command.Execute(commandParameter);
                }
            }
        }

        #endregion ItemClickCommand
    }
}
