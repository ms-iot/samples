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
#include "IServiceOperationModel.h"
#include "Tasks\HeartbeatDuration.h"
#include "Tasks\TaskProgress.h"

namespace BackgroundHost { namespace Headed { namespace Models
{
    using namespace ::Concurrency;
    using namespace ::Platform;
    using namespace ::Platform::Details;
    using namespace ::BackgroundHost;
    using namespace ::Windows::ApplicationModel::Core;
    using namespace ::Windows::ApplicationModel::Background;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::Foundation::Collections;
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::UI::Xaml;

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class BackgroundServiceOperationModel sealed : IServiceOperationModel
    {
    public:
        BackgroundServiceOperationModel(String^ backgroundTaskEntryPointName)
            : _backgroundTaskEntryPointName(backgroundTaskEntryPointName)
            , _dispatcher(CoreApplication::MainView->Dispatcher)
        {
            Application::Current->Resuming += ref new EventHandler<Object^>([=] (Object^ s, Object^ e) { this->Initialize(); });

            _watchdogTimer = ref new DispatcherTimer();
            _watchdogTimer->Interval = HeartbeatDuration::Watchdog;
            _watchdogTimer->Tick += ref new EventHandler<Object^>(this, &BackgroundServiceOperationModel::WatchdogTimer_Tick);
        }

    public:
        virtual void Initialize()
        {
            create_task(BackgroundExecutionManager::RequestAccessAsync()).then([=] (BackgroundAccessStatus status)
            {
                _taskRegistration = FindTaskRegistration();

                if (_taskRegistration != nullptr)
                {
                    _taskRegistrationProgressToken = _taskRegistration->Progress += ref new BackgroundTaskProgressEventHandler(this, &BackgroundServiceOperationModel::TaskRegistration_Progress);
                    _taskRegistrationCompletedToken = _taskRegistration->Completed += ref new BackgroundTaskCompletedEventHandler(this, &BackgroundServiceOperationModel::TaskRegistration_Completed);

                    _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] { _watchdogTimer->Start(); }));

                    this->RaiseStartedEvent();
                }
                else
                {
                    this->RaiseStoppedEvent();
                }
            });
        }

        virtual void Start()
        {
            // NOTE: NOT a good way to do this, but prevents a crash that happens when registering the task immediately after stopping the existing one.
            SleepEx(300, false);

            auto trigger = ref new ApplicationTrigger();
            auto builder = ref new BackgroundTaskBuilder();

            builder->Name = BackgroundServiceOperationModel::BackgroundServiceTaskName;
            builder->TaskEntryPoint = _backgroundTaskEntryPointName;
            builder->SetTrigger(trigger);

            _taskRegistration = builder->Register();

            _taskRegistrationProgressToken = _taskRegistration->Progress +=
                ref new BackgroundTaskProgressEventHandler(this, &BackgroundServiceOperationModel::TaskRegistration_Progress);
            _taskRegistrationCompletedToken = _taskRegistration->Completed +=
                ref new BackgroundTaskCompletedEventHandler(this, &BackgroundServiceOperationModel::TaskRegistration_Completed);

            trigger->RequestAsync();
        }

        virtual void Stop()
        {
            this->StopCore();
            this->RaiseStoppedEvent();
        }

        virtual event EventHandler<Object^>^ Started;
        virtual event EventHandler<ServiceFailedEventArgs^>^ Failed;
        virtual event EventHandler<Object^>^ Stopped;

    private:
        static property String^ BackgroundServiceTaskName
        {
            static String^ get()
            {
                static String^ backgroundServiceTaskNamePrefix = ref new String(L"BackgroundServiceTask.");
                return backgroundServiceTaskNamePrefix;
            }
        }

        static IBackgroundTaskRegistration^ FindTaskRegistration()
        {
            IBackgroundTaskRegistration^ taskRegistration = nullptr;

            for (auto task : BackgroundTaskRegistration::AllTasks)
            {
                if (0 == String::CompareOrdinal(task->Value->Name, BackgroundServiceOperationModel::BackgroundServiceTaskName))
                {
                    taskRegistration = task->Value;
                    break;
                }
            }

            return taskRegistration;
        }

        void RaiseFailedEvent(ServiceFailedEventArgs^ args) { this->Failed(this, args); }
        void RaiseStartedEvent() { this->Started(this, nullptr); }
        void RaiseStoppedEvent() { this->Stopped(this, nullptr); }

        void StopCore()
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] { _watchdogTimer->Stop(); }));
            if (_taskRegistration)
            {
                _taskRegistration->Progress -= _taskRegistrationProgressToken;
                _taskRegistration->Completed -= _taskRegistrationCompletedToken;
                _taskRegistration->Unregister(true);
                _taskRegistration = nullptr;
            }
        }

        void WatchdogTimer_Tick(Object^ sender, Object^ e)
        {
            this->StopCore();
            this->RaiseFailedEvent(ref new ServiceFailedEventArgs(ref new Exception(-1, "....Task timed out....")));
        }

        void TaskRegistration_Progress(BackgroundTaskRegistration^ sender, BackgroundTaskProgressEventArgs^ args)
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                if (args->Progress == TaskProgress::Started)
                {
                    this->RaiseStartedEvent();
                }

                // Reset the watchdog timer
                _watchdogTimer->Stop();
                _watchdogTimer->Start();
            }));
        }

        void TaskRegistration_Completed(BackgroundTaskRegistration^ sender, BackgroundTaskCompletedEventArgs^ args)
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                this->Stop();
            }));
        }

    private:
        String^ _backgroundTaskEntryPointName;
        CoreDispatcher^ _dispatcher;
        DispatcherTimer^ _watchdogTimer;
        IBackgroundTaskRegistration^ _taskRegistration;
        EventRegistrationToken _taskRegistrationProgressToken;
        EventRegistrationToken _taskRegistrationCompletedToken;
    };
}}}
