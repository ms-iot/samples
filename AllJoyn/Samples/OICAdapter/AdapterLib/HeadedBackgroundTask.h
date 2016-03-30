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
#include "Adapter.h"

namespace AdapterLib
{
    namespace bhht = ::BackgroundHost::Headed::Tasks;
    namespace dsb = ::BridgeRT;
    namespace wamb = ::Windows::ApplicationModel::Background;
    namespace wfm = ::Windows::Foundation::Metadata;

    [wfm::WebHostHidden]
    public ref class HeadedBackgroundTask sealed : wamb::IBackgroundTask, dsb::IAdapterFactory
    {
    public:
        virtual void Run(wamb::IBackgroundTaskInstance^ taskInstance)
        {
            auto serviceImplementation = ref new dsb::AdapterBridgeServiceImplementation(this);

            auto task = ref new bhht::HeadedBackgroundTaskImplementation();
            task->Run(taskInstance, serviceImplementation);
        }

    public:
        virtual dsb::IAdapter^ CreateAdapter()
        {
            return ref new Adapter();
        }
    };
}
