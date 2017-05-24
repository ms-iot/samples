#pragma once

#include "pch.h"

namespace BlinkyService
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class BlinkyServiceProvider sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
    private:
        Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> serviceDeferral;
        Windows::ApplicationModel::AppService::AppServiceConnection ^serviceConnection;

        Windows::Devices::Gpio::GpioController ^controller;

        void OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
        void OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^args);
    };
}
