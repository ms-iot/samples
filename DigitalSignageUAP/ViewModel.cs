// Copyright (c) Microsoft. All rights reserved.

using System.ComponentModel;

namespace OnScreenKeyboardSample
{
    public abstract class ViewModel : INotifyPropertyChanged
    {
        public void Notify(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
        public event PropertyChangedEventHandler PropertyChanged;
    }
}
