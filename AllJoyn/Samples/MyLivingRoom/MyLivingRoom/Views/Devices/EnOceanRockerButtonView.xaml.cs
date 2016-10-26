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

using MyLivingRoom.ViewModels;
using Windows.UI.Xaml.Input;

namespace MyLivingRoom.Views
{
    public sealed partial class EnOceanRockerButtonView
    {
        public EnOceanRockerButtonViewModel ViewModel { get { return this.DataContext as EnOceanRockerButtonViewModel; } }

        public EnOceanRockerButtonView()
        {
            this.InitializeComponent();
        }

        protected override void UpdateBindings()
        {
            this.Bindings.Update();
        }

        private void ButtonA0_PointerPressed(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonA0Pressed = true;
        }

        private void ButtonA0_PointerReleased(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonA0Pressed = false;
        }

        private void ButtonAI_PointerPressed(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonAIPressed = true;
        }

        private void ButtonAI_PointerReleased(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonAIPressed = false;
        }

        private void ButtonB0_PointerPressed(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonB0Pressed = true;
        }

        private void ButtonB0_PointerReleased(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonB0Pressed = false;
        }

        private void ButtonBI_PointerPressed(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonBIPressed = true;
        }

        private void ButtonBI_PointerReleased(object sender, PointerRoutedEventArgs e)
        {
            this.ViewModel.IsButtonBIPressed = false;
        }
    }
}
