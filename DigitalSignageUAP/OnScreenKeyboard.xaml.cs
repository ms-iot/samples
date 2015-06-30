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

