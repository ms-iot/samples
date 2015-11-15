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
    ///
    ///  A Widget Text Box.  This widget facilitates the exchange of data from an AllJoyn Control Panel to the Bridge.
    ///  Data for this widget is accumulated, but not passed to a target device immediately, It is expected that a
    ///  Bridge Control Panel will forward this data as part of a seperate button action.  This design minimizes the
    ///  amount of data transferred across the device's bus.  (Otherwise each individual keystroke would get passed)
    ///
    class WidgetPropertyTextBox : public WidgetProperty
    {
    public:
        WidgetPropertyTextBox(_In_ ControlPanel* pControlPanel);
        virtual ~WidgetPropertyTextBox();

        // Public Setter method, used for changing the text box content
        void Set(const char* fixedLabel);

        // Publice Getter method, used for reading the current text box content.
        const char* Get() const { return m_myValue.c_str(); };

    protected:
        // Text Box Value Setter
        virtual QStatus GetValue(_Out_ alljoyn_msgarg val) const;

        // Text Box Value Setter
        virtual QStatus SetValue(_In_ alljoyn_msgarg val);

        // A cached text box value.  This string absorbs all values entered into a text box.
        std::string m_myValue;
    };
}