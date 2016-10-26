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

namespace DeviceProviders
{
    ref class AllJoynService;
    ref class AllJoynSessionImplementation;
    interface class IBusObject;

    public ref class AllJoynSession sealed
    {
    internal:
        AllJoynSession(AllJoynSessionImplementation^ sessionImpl);
        inline AllJoynSessionImplementation^ GetImplementation() { return m_sessionImpl; }
    public:
        virtual ~AllJoynSession();

    public:
        property alljoyn_sessionid SessionId { alljoyn_sessionid get(); }
        property int32 UsageCount { int32 get(); }
        property Windows::Foundation::Collections::IVector<IBusObject^>^ Objects
        {
            Windows::Foundation::Collections::IVector<IBusObject^>^ get();
        }
        IBusObject ^ GetBusObject(Platform::String ^ path);
        Windows::Foundation::Collections::IVector<IBusObject^>^ GetBusObjectsWhichImplementInterface(Platform::String ^ interfaceName);

    private:
        CSLock m_lock;
        AllJoynSessionImplementation^ m_sessionImpl;
    };
}
