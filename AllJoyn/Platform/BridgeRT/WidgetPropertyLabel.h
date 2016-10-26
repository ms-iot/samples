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
#include "WidgetProperty.h"

namespace BridgeRT
{
    class WidgetPropertyLabel : public WidgetProperty
    {
    public:
        WidgetPropertyLabel(_In_ ControlPanel* pControlPanel, _In_ IAdapterValue^ srcValue);
        virtual ~WidgetPropertyLabel();

    protected:
        // The current data value of this widget
        virtual QStatus GetValue(_Out_ alljoyn_msgarg val) const;

        // The source data value that this label displays
        IAdapterValue^ m_srcValue;
    };
}