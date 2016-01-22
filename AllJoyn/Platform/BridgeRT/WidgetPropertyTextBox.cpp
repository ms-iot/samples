
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

#include "pch.h"
#include "WidgetConsts.h"
#include "WidgetPropertyTextBox.h"

using namespace BridgeRT;

const uint16_t WIDGET_TEXT_BOX_TYPE = 13;

//**************************************************************************************************************************************
//
//  Constructor
//
//  pControlPanel    The control panel hosting this widget
//
//**************************************************************************************************************************************
WidgetPropertyTextBox::WidgetPropertyTextBox(_In_ ControlPanel* pControlPanel)
    : WidgetProperty(pControlPanel, WIDGET_TEXT_BOX_TYPE)
    , m_myValue("Default")
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
WidgetPropertyTextBox::~WidgetPropertyTextBox()
{
}

void WidgetPropertyTextBox::Set(const char* fixedLabel)
{
    try
    {
        m_myValue = fixedLabel;
    }
    catch (...)
    {
    }
}

//**************************************************************************************************************************************
//
//  Gets the current property value from the text box widget
//
//  val     Variant message arg value to return to the caller.
//
//**************************************************************************************************************************************
QStatus WidgetPropertyTextBox::GetValue(_Out_ alljoyn_msgarg val) const
{
    QStatus status = ER_OK;

    // Create alljoyn message with current text box's string value
    alljoyn_msgarg variantarg = alljoyn_msgarg_create();
    CHK_POINTER(variantarg);
    CHK_AJSTATUS(alljoyn_msgarg_set(variantarg, ARG_STRING_STR, m_myValue.c_str()));
    CHK_AJSTATUS(alljoyn_msgarg_set(val, ARG_VARIANT_STR, variantarg));
    alljoyn_msgarg_stabilize(val);

leave:
    if (variantarg != nullptr)
    {
        alljoyn_msgarg_destroy(variantarg);
        variantarg = nullptr;
    }
    return status;
}


//**************************************************************************************************************************************
//
//  Translate the text box value from an alljoyn message arg to a local string
//
//  val     Variant Message Argument fromn alljoyn Control Panel to translate.
//
//**************************************************************************************************************************************
QStatus WidgetPropertyTextBox::SetValue(_In_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;

    char* newValue = nullptr;
    CHK_AJSTATUS(alljoyn_msgarg_get(val, ARG_STRING_STR, &newValue));
    if (newValue != nullptr)
    {
        m_myValue = newValue;
    }

leave:
    return status;
}

