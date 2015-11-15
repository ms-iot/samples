#pragma once

#include "pch.h"

namespace DsbCommon
{
    [Windows::UI::Xaml::Data::Bindable]
    public ref class DsbBridgeViewModel sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
    {
    public:
        DsbBridgeViewModel();

        virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

        property Platform::String^ InitializationErrorMessage
        {
            Platform::String^ get();
            void set(Platform::String^ value);
        }

    private:
        Platform::String^ m_initializationErrorMessage;
    };
}