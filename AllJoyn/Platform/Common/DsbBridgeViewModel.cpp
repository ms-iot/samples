#include "pch.h"
#include "DsbBridgeViewModel.h"

using namespace DsbCommon;
using namespace Platform;
using namespace Windows::UI::Xaml::Data;

DsbBridgeViewModel::DsbBridgeViewModel()
{
}

String^ DsbBridgeViewModel::InitializationErrorMessage::get()
{
    return m_initializationErrorMessage;
}

void DsbBridgeViewModel::InitializationErrorMessage::set(String^ value)
{
    if (m_initializationErrorMessage != value)
    {
        m_initializationErrorMessage = value;
        PropertyChanged(this, ref new PropertyChangedEventArgs("InitializationErrorMessage"));
    }
}