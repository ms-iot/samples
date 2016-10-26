
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
#include "IAdapter.h"


namespace BridgeRT
{
    class ControlPanel;

    // To handle a device's change of value signal, a WinRT class must be registered with the Adapter.  The Widget and Control Panel classes are implemented
    // using C++, this class converts a WinRT callback to a C++ callback.  Or said another way, when a signal changes, this WinRT signal handler is called,
    // which forwards the call to a standard C++ Control Panel.
    ref class WidgetSignalHandler sealed :
        public IAdapterSignalListener
    {
    public:

        // WiNRT Signal Handler Called by Signal Dispatcher.
        virtual void AdapterSignalHandler(
            _In_ IAdapterSignal^ Signal,
            _In_opt_ Platform::Object^ Context);

    internal:
        // Constructor
        WidgetSignalHandler(ControlPanel* pControlPanel);

        // Control Panel to forward Signal Change to
        ControlPanel* m_pControlPanel;
    };
}
