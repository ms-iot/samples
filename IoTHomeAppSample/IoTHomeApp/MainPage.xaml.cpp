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

#include "pch.h"
#include "MainPage.xaml.h"

using namespace ShellHomeApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System;

MainPage::MainPage()
{
    InitializeComponent();
}

void MainPage::OemApp1_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    //Launches the app that has registered as a protocol handler for 'oemapp1' protocol
    //Use Launcher option to specify the Target Application Package Family Name to resolve
    //the ambiguity, in case, more than one app has registered for the same protocol
    //More information about the api can be found at:
    //https://msdn.microsoft.com/en-us/library/windows/apps/hh701484.aspx

    auto options = ref new LauncherOptions();
    options->TargetApplicationPackageFamilyName = ref new String(L"OemApp1_1w720vyc4ccym");

    Launcher::LaunchUriAsync(ref new Uri("oemapp1:"), options);
}


void MainPage::OemApp2_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    //Launches the app that has registered as a protocol handler for 'oemapp2' protocol
    //More information about the api can be found at:
    //https://msdn.microsoft.com/en-us/library/windows/apps/hh701480.aspx

    Launcher::LaunchUriAsync(ref new Uri("oemapp2:"));
}
