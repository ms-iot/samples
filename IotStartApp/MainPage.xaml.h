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

#pragma once

#include "MainPage.g.h"


namespace IotStartApp
{
    [Windows::UI::Xaml::Data::Bindable]
    public ref class AppListItem sealed
    {
    public:
        property Windows::UI::Xaml::Media::Imaging::BitmapImage^ ImgSrc
        {
            Windows::UI::Xaml::Media::Imaging::BitmapImage^ get() { return m_ImgSrc; }
            void set(Windows::UI::Xaml::Media::Imaging::BitmapImage^ img) { m_ImgSrc = img; }
        }

        property Platform::String^ Name
        {
            Platform::String^ get() { return m_Name; }
            void set(Platform::String^ name) { m_Name = name; }
        }

        property Platform::String^ PackageFullName
        {
            Platform::String^ get() { return m_PackageFullName; }
            void set(Platform::String^ name) { m_PackageFullName = name; }
        }

        property Windows::ApplicationModel::Core::AppListEntry^ AppEntry
        {
            Windows::ApplicationModel::Core::AppListEntry^ get() { return m_AppEntry; }
            void set(Windows::ApplicationModel::Core::AppListEntry^ entry) { m_AppEntry = entry; }
        }

    private:
        Windows::UI::Xaml::Media::Imaging::BitmapImage^ m_ImgSrc;
        Platform::String^ m_Name;
        Platform::String^ m_PackageFullName;
        Windows::ApplicationModel::Core::AppListEntry^ m_AppEntry;

    };

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();

        property Windows::Foundation::Collections::IVector<AppListItem^>^ AppItemList
        {
            Windows::Foundation::Collections::IVector<AppListItem^>^ get() { return m_AppItemList; }
        }

    private:
        void EnumApplications();
        void StackPanel_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e);

    private:
        Platform::Collections::Vector<AppListItem^>^ m_AppItemList;
    };
}
