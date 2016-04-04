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

#include "pch.h"

namespace BackgroundHost { namespace Headed
{
    using namespace ::Platform;
    namespace wamr = ::Windows::ApplicationModel::Resources;

    private ref class Resources sealed
    {
    public:
        static property String^ ServiceNotRunning { static String^ get() { return GetString(L"ServiceNotRunning"); } }
        static property String^ ServiceRunning { static String^ get() { return GetString(L"ServiceRunning"); } }
        static property String^ ServiceStarting { static String^ get() { return GetString(L"ServiceStarting"); } }
        static property String^ ServiceStopping { static String^ get() { return GetString(L"ServiceStopping"); } }
        static property String^ ServiceTimedOut { static String^ get() { return GetString(L"ServiceTimedOut"); } }

    private:
        static property wamr::ResourceLoader^ ResourceLoader
        {
            static wamr::ResourceLoader^ get()
            {
                static auto resourceLoader = wamr::ResourceLoader::GetForCurrentView(ref new String(L"ms-appx:/BackgroundHost.Headed/Resources"));
                return resourceLoader;
            }
        }

        static String^ GetString(PCWSTR key)
        {
            return ResourceLoader->GetString(ref new String(key));
        }
    };
}}
