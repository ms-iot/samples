#pragma once

#include "App.g.h"

namespace HeadedAdapterApp
{
    using namespace AdapterLib;
    using namespace BackgroundHost::Headed;
    using namespace BackgroundHost::Headed::Models;
    using namespace Windows::ApplicationModel::Activation;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::UI::Xaml;

    ref class App sealed
    {
    internal:
        App()
        {
            this->InitializeComponent();
            auto bgTaskEntryPointName = (HeadedBackgroundTask::typeid)->FullName;
            _applicationImplementation = ref new ApplicationImplementation(bgTaskEntryPointName);
        }

    protected:
        virtual void OnLaunched(LaunchActivatedEventArgs^ args) override
        {
            _applicationImplementation->OnLaunched(args);
        }

    private:
        ApplicationImplementation^ _applicationImplementation;
    };
}
