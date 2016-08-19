// Copyright (c) Microsoft. All rights reserved.

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace DeviceIoControlUwp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

void AccessComPort ()
{
    using namespace Microsoft::WRL::Wrappers;

    // Open the COM port synchronously for simplicity.
    // You'll almost always want to use FILE_FLAG_OVERLAPPED
    // for COM ports.
    FileHandle fileHandle(CreateFile(
        L"\\\\.\\COM1",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr));

    if (!fileHandle.IsValid()) {
        DWORD lastError = GetLastError();
        throw ref new Exception(                // set a breakpoint here
            HRESULT_FROM_WIN32(lastError),
            L"Failed to open COM port.");
    }

    // Set baud rate
    SERIAL_BAUD_RATE inputBuffer = { 115200 };
    DWORD information;
    if (!DeviceIoControl(
            fileHandle.Get(),
            IOCTL_SERIAL_SET_BAUD_RATE,
            &inputBuffer,
            sizeof(inputBuffer),
            nullptr,
            0,
            &information,
            nullptr)) {

        throw ref new Exception(
            HRESULT_FROM_WIN32(GetLastError()),
            L"Failed to set baud rate.");
    }

    // Write out a string over the serial port
    const char message[] = "Hello serial!\n";
    if (!WriteFile(
            fileHandle.Get(),
            message,
            sizeof(message),
            &information,
            nullptr)) {

        throw ref new Exception(
            HRESULT_FROM_WIN32(GetLastError()),
            L"Failed to write data to COM port.");
    }
}

MainPage::MainPage()
{
    InitializeComponent();
    AccessComPort();
}
