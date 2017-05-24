#pragma once

#include "pch.h"

namespace BlinkyClient
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class StartupTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
    private:
        void OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
        Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> serviceDeferral;
        Windows::ApplicationModel::AppService::AppServiceConnection ^serviceConnection;
        Windows::System::Threading::ThreadPoolTimer ^timer;
        Platform::String ^command;
        void RequestBlinkService();
    };
}
