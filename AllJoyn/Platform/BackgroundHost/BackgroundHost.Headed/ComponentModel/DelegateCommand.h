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

namespace BackgroundHost { namespace Headed { namespace ComponentModel
{
    using namespace ::Platform;
    using namespace ::Windows::Foundation;
    using namespace ::Windows::UI::Core;
    using namespace ::Windows::UI::Xaml::Input;

    public delegate void ExecuteHandler(Object^ parameter);

    public delegate bool CanExecuteHandler(Object^ parameter);

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class DelegateCommand sealed : ICommand
    {
    private:
        CoreDispatcher^ _dispatcher;
        ExecuteHandler^ _executeDelegate;
        CanExecuteHandler^ _canExecuteDelegate;

    public:
        [Windows::Foundation::Metadata::DefaultOverload]
        DelegateCommand(ExecuteHandler^ executeDelegate)
            : _executeDelegate(executeDelegate)
        {
        }

        DelegateCommand(ExecuteHandler^ executeDelegate, CanExecuteHandler^ canExecuteDelegate)
            : _executeDelegate(executeDelegate)
            , _canExecuteDelegate(canExecuteDelegate)
        {
        }

        DelegateCommand(Windows::UI::Core::CoreDispatcher^ dispatcher, ExecuteHandler^ executeDelegate)
            : _dispatcher(dispatcher)
            , _executeDelegate(executeDelegate)
        {
        }

        DelegateCommand(Windows::UI::Core::CoreDispatcher^ dispatcher, ExecuteHandler^ executeDelegate, CanExecuteHandler^ canExecuteDelegate)
            : _dispatcher(dispatcher)
            , _executeDelegate(executeDelegate)
            , _canExecuteDelegate(canExecuteDelegate)
        {
        }

    public:
        void RaiseCanExecuteChanged(Object^ parameter)
        {
            this->CanExecuteChanged(this, parameter);
        }

        virtual event EventHandler<Object^>^ CanExecuteChanged;

        virtual bool CanExecute(Object^ parameter)
        {
            return _executeDelegate != nullptr && (_canExecuteDelegate == nullptr || _canExecuteDelegate(parameter));
        }

        virtual void Execute(Object^ parameter)
        {
            if (this->CanExecute(parameter))
            {
                if (_dispatcher != nullptr)
                {
                    _dispatcher->RunAsync(CoreDispatcherPriority::Normal,
                        ref new DispatchedHandler([this, parameter]() { _executeDelegate(parameter); }));
                }
                else
                {
                    _executeDelegate(parameter);
                }
            }
        }
    };
}}}
