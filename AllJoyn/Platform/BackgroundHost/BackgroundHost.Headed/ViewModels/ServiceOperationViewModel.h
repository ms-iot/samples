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
#include "ComponentModel\BindableBase.h"
#include "ComponentModel\BindableBaseHelpers.h"
#include "ComponentModel\DelegateCommand.h"
#include "Models\IServiceOperationModel.h"
#include "Resources\Resources.h"

namespace BackgroundHost { namespace Headed { namespace ViewModels
{
    using namespace ::Concurrency;
    using namespace ::BackgroundHost::Headed;
    using namespace ::BackgroundHost::Headed::ComponentModel;
    using namespace ::BackgroundHost::Headed::Models;
    using namespace ::Windows::ApplicationModel::Core;
    using namespace ::Windows::ApplicationModel::Resources;
    using namespace ::Windows::UI::Core;
    namespace wfd = ::Windows::Foundation::Metadata;

    [wfd::WebHostHidden]
    public ref class ServiceOperationViewModel sealed : BindableBase
    {
    public:
        ServiceOperationViewModel(IServiceOperationModel^ model)
            : _model(model)
            , _dispatcher(CoreApplication::MainView->Dispatcher)
        {
            if (!model)
                throw ref new InvalidArgumentException(L"model");

            CoreApplication::Resuming += ref new EventHandler<Object^>(this, &ServiceOperationViewModel::Application_Resuming);

            _model->Failed += ref new EventHandler<ServiceFailedEventArgs^>(this, &ServiceOperationViewModel::ServiceOperationModel_Failed);
            _model->Started += ref new EventHandler<Object^>(this, &ServiceOperationViewModel::ServiceOperationModel_Started);
            _model->Stopped += ref new EventHandler<Object^>(this, &ServiceOperationViewModel::ServiceOperationModel_Stopped);

            _startCommand = ref new DelegateCommand(_dispatcher,
                ref new ExecuteHandler(this, &ServiceOperationViewModel::StartCommand_Executed),
                ref new CanExecuteHandler(this, &ServiceOperationViewModel::StartCommand_CanExecute));
            _stopCommand = ref new DelegateCommand(_dispatcher,
                ref new ExecuteHandler(this, &ServiceOperationViewModel::StopCommand_Executed),
                ref new CanExecuteHandler(this, &ServiceOperationViewModel::StopCommand_CanExecute));
            _restartCommand = ref new DelegateCommand(_dispatcher,
                ref new ExecuteHandler(this, &ServiceOperationViewModel::RestartCommand_Executed),
                ref new CanExecuteHandler(this, &ServiceOperationViewModel::RestartCommand_CanExecute));

            this->SetIsCommandExecuting(true);

            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                _model->Initialize();

                if (_autoStart && !this->IsRunning)
                {
                    this->StartCommand_Executed(nullptr);
                }
            }));
        }

    // IsRunning property
    public:
        property bool IsRunning { bool get() { return _isRunning; } }
    private:
        void SetIsRunning(bool value)
        {
            if (BindableBaseHelpers::SetProperty(this, _isRunning, value, L"IsRunning"))
            {
                this->RaisePropertyChanged(ref new String(L"IsInProgress"));
                this->UpdateCommands();
            }
        }
        bool _isRunning;

    // IsCommandExecuting property
    public:
        property bool IsCommandExecuting { bool get() { return _isCommandExecuting; } }
    private:
        void SetIsCommandExecuting(bool value)
        {
            if (BindableBaseHelpers::SetProperty(this, _isCommandExecuting, value, L"IsCommandExecuting"))
            {
                this->RaisePropertyChanged(ref new String(L"IsInProgress"));
                this->UpdateCommands();
            }
        }
        bool _isCommandExecuting;

    // IsInProgress property
    public:
        property bool IsInProgress { bool get() { return this->IsRunning || this->IsCommandExecuting; } }

    // StatusText property
    public:
        property String^ StatusText { String^ get() { return _statusText; } }
    private:
        void SetStatusText(String^ value)
        {
            BindableBaseHelpers::SetProperty(this, _statusText, value, L"StatusText");
        }
        String^ _statusText;

    // StartCommand
    public:
        property ICommand^ StartCommand { ICommand^ get() { return _startCommand; } }
    private:
        DelegateCommand^ _startCommand;

        bool StartCommand_CanExecute(Object^ parameter)
        {
            return !this->IsCommandExecuting && !this->IsRunning;
        }

        void StartCommand_Executed(Object^ parameter)
        {
            this->SetStatusText(Resources::ServiceStarting);
            this->SetIsCommandExecuting(true);
            this->SetIsRunning(_isRestarting);
            _isRestarting = false;

            create_task([=] { _model->Start(); });
        }

    // StopCommand
    public:
        property ICommand^ StopCommand { ICommand^ get() { return _stopCommand; } }
    private:
        DelegateCommand^ _stopCommand;

        bool StopCommand_CanExecute(Object^ parameter)
        {
            return !this->IsCommandExecuting && this->IsRunning;
        }

        void StopCommand_Executed(Object^ parameter)
        {
            this->SetStatusText(Resources::ServiceStopping);
            this->SetIsCommandExecuting(true);

            create_task([=] { _model->Stop(); });
        }

    // RestartCommand
    public:
        property ICommand^ RestartCommand { ICommand^ get() { return _restartCommand; } }
    private:
        DelegateCommand^ _restartCommand;

        bool RestartCommand_CanExecute(Object^ parameter)
        {
            return !this->IsCommandExecuting && this->IsRunning;
        }

        void RestartCommand_Executed(Object^ parameter)
        {
            _isRestarting = true;
            this->StopCommand_Executed(parameter);
        }

    private:
        void UpdateCommands()
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                _startCommand->RaiseCanExecuteChanged(nullptr);
                _stopCommand->RaiseCanExecuteChanged(nullptr);
                _restartCommand->RaiseCanExecuteChanged(nullptr);
            }));
        }

    private:
        void Application_Resuming(Object^ sender, Object^ e)
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                this->SetIsCommandExecuting(false);
                this->SetIsCommandExecuting(true);
                _model->Initialize();
            }));
        }

        void ServiceOperationModel_Failed(Object^ sender, ServiceFailedEventArgs^ e)
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                this->SetIsRunning(false);
                this->SetIsCommandExecuting(false);

                if (e && e->FailureException && e->FailureException->Message && !e->FailureException->Message->IsEmpty())
                {
                    this->SetStatusText(e->FailureException->Message);
                }
                else
                {
                    this->SetStatusText(Resources::ServiceTimedOut);
                }
            }));
        }

        void ServiceOperationModel_Started(Object^ sender, Object^ e)
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                this->SetIsRunning(true);
                this->SetIsCommandExecuting(false);
                this->SetStatusText(Resources::ServiceRunning);
            }));
        }

        void ServiceOperationModel_Stopped(Object^ sender, Object^ e)
        {
            _dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=]
            {
                if (_isRestarting)
                {
                    this->StartCommand_Executed(nullptr);
                }
                else
                {
                    this->SetIsRunning(false);
                    this->SetIsCommandExecuting(false);
                    this->SetStatusText(Resources::ServiceNotRunning);
                }
            }));
        }

    private:
        const bool _autoStart = false;
        IServiceOperationModel^ _model;
        ResourceLoader^ _resourceLoader;
        CoreDispatcher^ _dispatcher;
        bool _isRestarting;
    };
}}}