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

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace OnScreenKeyboardSample
{
    public class ContentBuffer
    {
        public ContentBuffer(){}

        private string _str = null;
        private int _selectionStart = 0;
        private int _selectionLength = 0;

        public int SelectionStart
        {
            get
            {
                return _selectionStart;
            }
            set
            {
                _selectionStart = value;
            }    
        }
        public int SelectionLength
        {
            get
            {
                return _selectionLength;
            }
            set
            {
                _selectionLength = value;
            }
        }
        public string Content
        {
            get
            {
                return _str;
            }
            set
            {
                _str = value;
            }
        }
    }
    public partial class OnScreenKeyBoard : UserControl
    {
        private static ContentBuffer _buffer;
        private object _host;

        public static ContentBuffer Buffer
        {
            get
            {
                if (_buffer == null)
                {
                    _buffer = new ContentBuffer();
                }
                return _buffer;
            }
        }

        public static readonly DependencyProperty OutputStringProperty = DependencyProperty.Register("OutputString", typeof(string), typeof(OnScreenKeyBoard), new PropertyMetadata(string.Empty, OnTextBoxTextPropertyChanged));
        public string OutputString
        {
            get
            {
                return (string)GetValue(OutputStringProperty);
            }
            set
            {
                SetValue(OutputStringProperty, value);
            }
        }

        private static void OnTextBoxTextPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            //Sync up the string buffer context.
            Buffer.Content = (string)e.NewValue;
        }

        public void RegisterEditControl(Control control)
        {
            var t = control as TextBox;
            t.SelectionChanged += Target_SelectionChanged;
            t.GotFocus += Target_GotFocus;

        }

        public void RegisterHost(object host)
        {
            if (host != null)
            {
                _host = host;
            }
        }

        public object GetHost()
        {
            return _host;
        }

        private void Target_SelectionChanged(object sender, RoutedEventArgs e)
        {
            var t = sender as TextBox;
            if (t.FocusState == FocusState.Pointer)
            {
                Buffer.SelectionStart = t.SelectionStart;
                Buffer.SelectionLength = t.SelectionLength;
            }
        }

        private void Target_GotFocus(object sender, RoutedEventArgs e)
        {
            var t = sender as TextBox;
            if (t.FocusState == FocusState.Pointer)
            {
                Buffer.Content = t.Text;
                Buffer.SelectionStart = t.SelectionStart;
                Buffer.SelectionLength = t.SelectionLength;
            }
        }

        public OnScreenKeyBoard()
        {
            DataContext = new KeyboardViewModel(this);
            InitializeComponent();
        }

    }
}

