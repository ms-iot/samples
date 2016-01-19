//
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

#include "pch.h"
#include "Views\BooleanValueConverter.h"
#include "Views\ServiceOperationView.g.h"
#include "ViewModels\ServiceOperationViewModel.h"

namespace BackgroundHost { namespace Headed { namespace Views
{
    using namespace ::Platform;
    using namespace ::BackgroundHost::Headed::ViewModels;
    using namespace ::Windows::ApplicationModel;
    using namespace ::Windows::UI::Xaml;

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class ServiceOperationView sealed
    {
    public:
        ServiceOperationView()
        {
            // This is a compile-time workaround for the Progress Bar's Opacity Binding.
            // Ideally the resource would be added to the xaml, but when a C# based adapter
            // launches, the launch fails with an exception because the converter resource is 
            // not found. 
            Resources->Insert("BooleanToOpacityConverter", ref new BooleanToDoubleConverter());

            this->InitializeComponent();

            if (DesignMode::DesignModeEnabled)
            {
                this->ApplicationDisplayTitle = ref new String(L"-Application Display Title-");
            }
        }

    public:
        property DependencyProperty^ ApplicationDisplayTitleProperty
        {
            static DependencyProperty^ get()
            {
                static auto dependencyProperty = DependencyProperty::Register(ref new String(L"ApplicationDisplayTitle"),
                    String::typeid, ServiceOperationView::typeid, ref new PropertyMetadata(nullptr));
                return dependencyProperty;
            }
        }

        property String^ ApplicationDisplayTitle
        {
            String^ get() { return (String^)this->GetValue(ApplicationDisplayTitleProperty); }
            void set(String^ value) { this->SetValue(ApplicationDisplayTitleProperty, value); }
        }

        property ServiceOperationViewModel^ ViewModel
        {
            ServiceOperationViewModel^ get()
            {
                return dynamic_cast<ServiceOperationViewModel^>(this->DataContext);
            }
        }
    };

    
}}}
