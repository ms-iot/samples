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

using System;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Data;

namespace OnScreenKeyboardSample
{
    public class BoolToSolidBrushConverter : IValueConverter
    { 
        /// <summary>
        /// Convert from source-type to target-type
        /// </summary>
        public object Convert(object value, Type targetType, object parameter, string str)
        {
            if ((bool)value)
            {
                return new SolidColorBrush(Windows.UI.Color.FromArgb(128,128,128,128));
            }
            else
            {
                return new SolidColorBrush(Windows.UI.Color.FromArgb(255,128,128,128));
            }
        }

        /// <summary>
        /// Convert-back from target to source.
        /// </summary>
        public object ConvertBack(object value, Type targetType, object parameter, string str)
        {
            return null;
        }
    }
    public class KeyboardViewModel : ViewModel
    {
        #region constructor
        public KeyboardViewModel(OnScreenKeyBoard container)
        {
            _container = container;
            KeyModel.theKeyboardViewModel = this;
        }
        #endregion constructor

        #region Commanding

        #region BackspaceCommand

        public RelayCommand BackspaceCommand
        {
            get
            {
                if (_backspaceCommand == null)
                {
                    _backspaceCommand = new RelayCommand((x) => ExecuteBackspaceCommand());
                }
                return _backspaceCommand;
            }
        }

        public void ExecuteBackspaceCommand()
        {
            int currentSelectionStart = OnScreenKeyBoard.Buffer.SelectionStart;
            int currentSelectionLength = OnScreenKeyBoard.Buffer.SelectionLength;

            if (currentSelectionLength != 0)
            {
                _container.OutputString = _container.OutputString.Remove(currentSelectionStart, currentSelectionLength);
                OnScreenKeyBoard.Buffer.SelectionLength = 0;
            }

            else if (OnScreenKeyBoard.Buffer.SelectionStart > 0)
            {
                _container.OutputString = _container.OutputString.Remove(currentSelectionStart - 1, 1);
                if (OnScreenKeyBoard.Buffer.SelectionStart > 0)
                {
                    OnScreenKeyBoard.Buffer.SelectionStart--;
                }
            }
        }

        private RelayCommand _backspaceCommand;

        #endregion BackspaceCommand

        #region CapsLockCommand

        public RelayCommand CapsLockCommand
        {
            get
            {
                if (_capsLockCommand == null)
                {
                    _capsLockCommand = new RelayCommand((x) => ExecuteCapsLockCommand());
                }
                return _capsLockCommand;
            }
        }

        #region ExecuteCapsLockCommand
        /// <summary>
        /// Execute the CapsLockCommand, which ocurrs when the user clicks on the CAPS button.
        /// </summary>
        public void ExecuteCapsLockCommand()
        {
            if (IsCapsLock)
            {
                IsCapsLock = false;
            }
            else
            {
                IsCapsLock = true;
            }
        }
        #endregion

        private RelayCommand _capsLockCommand;

        #endregion CapsLockCommand

        #region KeyPressedCommand

        public RelayCommand KeyPressedCommand
        {
            get
            {
                if (_keyPressedCommand == null)
                {
                    _keyPressedCommand = new RelayCommand((x) => ExecuteKeyPressedCommand(x));
                }
                return _keyPressedCommand;
            }
        }

        #region ExecuteKeyPressedCommand
        /// <summary>
        /// Execute the KeyPressedCommand.
        /// </summary>
        /// <param name="arg">The KeyModel of the key-button that was pressed</param>
        public void ExecuteKeyPressedCommand(object arg)
        {
            if (_container.OutputString != null)
            {
                int currentSelectionStart = OnScreenKeyBoard.Buffer.SelectionStart;
                int currentSelectionLength = OnScreenKeyBoard.Buffer.SelectionLength;

                if (currentSelectionLength != 0)
                {
                    _container.OutputString = _container.OutputString.Remove(currentSelectionStart, currentSelectionLength);
                    OnScreenKeyBoard.Buffer.SelectionLength = 0;
                }
                _container.OutputString = _container.OutputString.Insert(currentSelectionStart, (string)arg);
                OnScreenKeyBoard.Buffer.SelectionStart++;

                //Return to un-shift mode if currently in shift mode
                if (IsShiftLock)
                {
                    IsShiftLock = false;
                }
            }
        }
        #endregion

        private RelayCommand _keyPressedCommand;

        #endregion KeyPressedCommand

        #region ShiftCommand

        public RelayCommand ShiftCommand
        {
            get
            {
                if (_shiftCommand == null)
                {
                    _shiftCommand = new RelayCommand((x) => ExecuteShiftCommand());
                }
                return _shiftCommand;
            }
        }

        #region ExecuteShiftCommand
        /// <summary>
        /// Execute the ShiftCommand, which ocurrs when the user clicks on the SHIFT key.
        /// </summary>
        public void ExecuteShiftCommand()
        {
            ToggleShiftState();
        }
        #endregion


        private RelayCommand _shiftCommand;

        #endregion ShiftCommand

        #endregion Commanding

        #region Methods

        #region NotifyTheIndividualKeys
        /// <summary>
        /// Make the individual key-button view models notify their views that their properties have changed.
        /// </summary>
        public void NotifyTheIndividualKeys(string notificationText)
        {
            if (KB != null && KB.KeyAssignments != null)
            {
                if (notificationText != null)
                {
                    foreach (var keyModel in KB.KeyAssignments)
                    {
                        if (keyModel != null)
                        {
                            keyModel.Notify(notificationText);
                        }
                    }
                }
            }
        }
        #endregion

        #region ToggleShiftState
        /// <summary>
        /// Turn the shift-lock mode off if it's on, and on if it's off.
        /// </summary>
        public void ToggleShiftState()
        {
            if (IsShiftLock)
            {
                IsShiftLock = false;
            }
            else
            {
                IsShiftLock = true;
            }
        }
        #endregion

        #endregion Methods

        #region Properties

        #region IsCapsLock
        /// <summary>
        /// Get/set whether the VirtualKeyboard is currently is Caps-Lock mode.
        /// </summary>
        public bool IsCapsLock
        {
            get
            {
                return _isCapsLock;
            }
            set
            {
                if (value != _isCapsLock)
                {
                    _isCapsLock = value;
                    if (IsShiftLock)
                    {
                        IsShiftLock = false;
                    }
                    else
                    {
                        NotifyTheIndividualKeys("Text");
                    }
                    Notify("IsCapsLock");
                }
            }
        }
        #endregion

        #region IsShiftLock
        /// <summary>
        /// Get/set whether the VirtualKeyboard is currently is Shift-Lock mode.
        /// </summary>
        public bool IsShiftLock
        {
            get
            {
                return _ShiftKey.IsInShiftState;
            }
            set
            {
                if (value != _ShiftKey.IsInShiftState)
                {
                    _ShiftKey.IsInShiftState = value;
                    NotifyTheIndividualKeys("Text");
                    Notify("IsShiftLock");
                }
            }
        }
        #endregion

        #region KB
        /// <summary>
        /// Get/set the specific subclass of KeyAssignmentSet that is currently attached to this keyboard.
        /// </summary>
        public KeyAssignmentSet KB
        {
            get
            {
                if (_currentKeyboardAssignment == null)
                {
                    _currentKeyboardAssignment = KeyAssignmentSet.KeyAssignment;
                }
                return _currentKeyboardAssignment;
            }
        }
        #endregion

        #region The view-model properties for the individual key-buttons of the keyboard

        public KeyModel TabKey
        {
            get { return _Tab; }
            set { _Tab = value; }
        }

        public KeyModel EnterKey
        {
            get { return _Enter; }
            set { _Enter = value; }
        }

        public ShiftKeyViewModel ShiftKey
        {
            get { return _ShiftKey; }
        }

        #endregion The view-model properties for the individual key-buttons of the keyboard

        #region TheKeyModels array
        /// <summary>
        /// Get the array of KeyModels from the KB that comprise the view-models for the keyboard key-buttons
        /// </summary>
        public KeyModel[] TheKeyModels
        {
            get
            {
                if (KB != null)
                {
                    return KB.KeyAssignments;
                }
                return null;
            }
        }
        #endregion

        #endregion Properties

        #region fields

        private bool _isCapsLock;

        /// <summary>
        /// The KeyAssignmentSet that is currently attached to this keyboard.
        /// </summary>
        private KeyAssignmentSet _currentKeyboardAssignment;
        // These are view-models for the individual key-buttons of the keyboard which are not provided by the KeyAssignmentSet:
        private KeyModel _Tab = new KeyModel('\t', '\t');
        private KeyModel _Enter = new KeyModel('\n', '\n');
        private ShiftKeyViewModel _ShiftKey = new ShiftKeyViewModel();
        private OnScreenKeyBoard _container = null;

        #endregion fields
    }
}
