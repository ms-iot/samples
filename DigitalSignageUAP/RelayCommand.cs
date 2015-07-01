// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Windows.Input;

namespace OnScreenKeyboardSample
{
    public class RelayCommand : ICommand
    {
        private readonly Action<object> _executionAction;

        public RelayCommand(Action<object> executionAction)
        {
            _executionAction = executionAction;
        }

        public bool CanExecute(object parameter)
        {
            return true;
        }

        public event EventHandler CanExecuteChanged
        {
            add { }
            remove { }
        }

        public void Execute(object parameter)
        {
            _executionAction(parameter);
        }
    }
}
