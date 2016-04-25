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

#include "ServiceFailedEventArgs.h"

namespace BackgroundHost { namespace Headed { namespace Models
{
    using namespace ::Platform;

    public interface class IServiceOperationModel
    {
        void Initialize();

        void Start();

        void Stop();

        event Windows::Foundation::EventHandler<ServiceFailedEventArgs^>^ Failed;

        event Windows::Foundation::EventHandler<Object^>^ Started;

        event Windows::Foundation::EventHandler<Object^>^ Stopped;
    };
}}}
