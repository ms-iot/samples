//
// Copyright (c) 2016, Microsoft Corporation
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
#include "Models\BackgroundServiceOperationModel.h"
#include "Models\IServiceOperationModel.h"
#include "ViewModels\ServiceOperationViewModel.h"
#include "Views\ServiceOperationView.xaml.h"

namespace BackgroundHost { namespace Headed
{
    using namespace ::BackgroundHost::Headed::Models;
    using namespace ::BackgroundHost::Headed::ViewModels;
    using namespace ::BackgroundHost::Headed::Views;
    using namespace ::BackgroundHost;
    using namespace ::Windows::ApplicationModel::Activation;
    using namespace ::Windows::ApplicationModel::Resources;
    using namespace ::Windows::UI::Xaml;
    using namespace ::Windows::System::Profile;
    namespace wfm = ::Windows::Foundation::Metadata;

    [wfm::WebHostHidden]
    public ref class ApplicationImplementation sealed
    {
    public:
        ApplicationImplementation(String^ backgroundTaskEntryPointName)
            : m_backgroundTaskEntryPointName(backgroundTaskEntryPointName)
        {
            if (m_backgroundTaskEntryPointName->IsEmpty())
                throw ref new InvalidArgumentException(L"backgroundTaskType");
        }

    public:
        void OnLaunched(LaunchActivatedEventArgs^ args)
        {
            // Create the main view to be hosted in the window
            if (!Window::Current->Content)
            {
                // Create the Model
                auto serviceOperationModel = ref new BackgroundServiceOperationModel(m_backgroundTaskEntryPointName);

                // Create the ViewModel
                auto contentViewModel = ref new ServiceOperationViewModel(serviceOperationModel);

                // Create the View
                auto contentView = ref new ServiceOperationView();
                contentView->DataContext = contentViewModel;
                Window::Current->Content = contentView;

                // Set the View's application display title if needed
                if (contentView->ApplicationDisplayTitle->IsEmpty())
                {
                    auto resourceLoader = ResourceLoader::GetForCurrentView();
                    contentView->ApplicationDisplayTitle = resourceLoader->GetString("ApplicationDisplayName");
                }
            }

            // Ensure the current window is active
            Window::Current->Activate();
        }

    private:
        String^ m_backgroundTaskEntryPointName;
    };
}}
