using System;

namespace OnScreenKeyboardSample
{   
    /// <summary>
    /// A KeyModel defines what is assigned to one keyboard key (which we're calling key-buttons).
    /// </summary>
    public class KeyModel : ViewModel
    {
        public KeyModel()
        {
            this.UnshiftedCodePoint = '?';
        }

        /// <param name="iCodePoint">The Unicode codepoint for this particular key-button</param>
        /// <param name="isThisALetter">Indicates whether this is an alphabetic letter</param>
        public KeyModel(int iCodePoint, bool isLetter)
        {
            UnshiftedCodePoint = (char)iCodePoint;
            ShiftedCodePoint = (char)0;
            IsLetter = isLetter;
        }

        /// <param name="iCodePoint">The Unicode codepoint for this particular key-button</param>
        /// <param name="iShiftedCodePoint">The Unicode codepoint to use when in the SHIFT state</param>
        public KeyModel(int iCodePoint, int iShiftedCodePoint)
        {
            UnshiftedCodePoint = (char)iCodePoint;
            ShiftedCodePoint = (char)iShiftedCodePoint;
            IsLetter = true;
        }

        /// <summary>
        /// Get the Unicode code-point (as a string) that's assigned to this key-button, as a function of it's shifted or alt state.
        /// </summary>
        public virtual string CodePointString
        {
            get
            {
                if (theKeyboardViewModel != null)
                {
                    if ((theKeyboardViewModel.IsShiftLock || (isThisALetter && theKeyboardViewModel.IsCapsLock)) && shiftedCodePoint != 0)
                    {
                        return ((char)ShiftedCodePoint).ToString();
                    }
                    else
                    {
                        return ((char)UnshiftedCodePoint).ToString();
                    }
                }
                else
                {
                    return "-";
                }
            }
        }

        public char UnshiftedCodePoint
        {
            get { return unshiftedCodePoint; }
            set
            {
                if (value != unshiftedCodePoint)
                {
                    unshiftedCodePoint = value;
                }
            }
        }

        public char ShiftedCodePoint
        {
            get { return shiftedCodePoint; }
            set
            {
                if (value != shiftedCodePoint)
                {
                    shiftedCodePoint = value;
                    Notify("Text");
                }
            }
        }

        public bool IsLetter
        {
            get { return isThisALetter; }
            set { isThisALetter = value; }
        }

        /// <summary>
        /// Get the button name of the WPF Button that this is a view-model of.
        /// </summary>
        public string KeyName
        {
            get
            {
                if (String.IsNullOrEmpty(keyName))
                {
                    return "";
                }
                else
                {
                    return keyName;
                }
            }
            set { keyName = value; }
        }

        /// <summary>
        /// Get a name to identify this key by, whether the display-name or the KeyName.
        /// </summary>
        public string Name
        {
            get
            {
                if (this.KeyName != string.Empty)
                {
                    return this.KeyName;
                }
                else
                {
                    return "NA";
                }
            }
        }

        /// <summary>
        /// Get the Text to be shown on the corresponding Button
        /// </summary>
        public virtual string Text
        {
            get
            {
                {
                    string sText;
                    if (!String.IsNullOrEmpty(text))
                    {
                        sText = text;
                    }
                    else
                    {
                        sText = this.CodePointString;
                    }
                    return sText;
                }
            }
        }

        /// <summary>
        /// This flag indicates whether this key-button contains a 'letter'.
        /// </summary>
        protected bool isThisALetter;
        protected string keyName;
        protected string text;
        public static KeyboardViewModel theKeyboardViewModel;
        protected char unshiftedCodePoint; // the character you normally get from this key-button.
        protected char shiftedCodePoint; // the character you get when the Shift key is active.
    }

    public class KeyModelWithTwoGlyphs : KeyModel
    {
        public KeyModelWithTwoGlyphs(): base() {}

        /// <param name="iCodePoint">The Unicode codepoint for this particular key-button</param>
        /// <param name="iShiftedCodePoint">The Unicode codepoint to use when in the SHIFT state</param>
        /// <param name="isThisALetter">Indicates whether this is an alphabetic letter</param>
        public KeyModelWithTwoGlyphs(int iCodePoint, int iShiftedCodePoint, bool isLetter)
        {
            UnshiftedCodePoint = (char)iCodePoint;
            ShiftedCodePoint = (char)iShiftedCodePoint;
            IsLetter = isLetter;
        }

        /// <returns>A description of the state of this object, or else if in design-time just the key-name</returns>
        public override string ToString()
        {
            return CodePointString;
        }
    }

    public class ShiftKeyViewModel : KeyModel
    {
        public ShiftKeyViewModel()
        {
        }
        /// <summary>
        /// Get/set whether we are in the Shift state.
        /// </summary>
        public bool IsInShiftState
        {
            get { return _isInShiftState; }
            set
            {
                if (value != _isInShiftState)
                {
                    _isInShiftState = value;
                    Notify("Text");
                }
            }
        }
        private bool _isInShiftState;
    }
}
