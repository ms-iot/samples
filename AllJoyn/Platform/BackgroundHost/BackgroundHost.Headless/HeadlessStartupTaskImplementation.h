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

namespace BackgroundHost { namespace Headless
{
    using namespace ::BackgroundHost;
    using namespace ::Platform;
    using namespace ::Windows::ApplicationModel;
    using namespace ::Windows::ApplicationModel::Background;

    public ref class HeadlessStartupTaskImplementation sealed
    {
    public:
        void Run(IBackgroundTaskInstance^ taskInstance, IServiceImplementation^ serviceImplementation)
        {
            if (!taskInstance)
                throw ref new InvalidArgumentException(L"taskInstance");
            if (!serviceImplementation)
                throw ref new InvalidArgumentException(L"serviceImplementation");

            try
            {
                // Start the task implementation, which will block until finished
                serviceImplementation->Start();

                // If we hold the deferal, the headless task never stops, so it will continue to run once this method exits
                _deferral = taskInstance->GetDeferral();

            }
            
            catch (Exception^ exception)
            {
                // Notify any listeners
                serviceImplementation->RaiseFailedEvent(exception->Message);
            }
            catch (...)
            {
                // Notify any listeners
                serviceImplementation->RaiseFailedEvent("Failed");
            }

        }

    private:
        Agile<BackgroundTaskDeferral> _deferral;
    };
}}
