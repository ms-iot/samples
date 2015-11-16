
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
#include "WidgetPropertyLabel.h"

using namespace BridgeRT;

const uint16_t WIDGET_LABEL_TYPE = 11;

//**************************************************************************************************************************************
//
//  Constructor
//
//  pControlPanel    The control panel hosting this widget
//  srcValue         The source content of this widget
//
//**************************************************************************************************************************************
WidgetPropertyLabel::WidgetPropertyLabel(_In_ ControlPanel* pControlPanel, _In_ IAdapterValue^ srcValue)
    : WidgetProperty(pControlPanel, WIDGET_LABEL_TYPE, true)
    , m_srcValue(srcValue)
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
WidgetPropertyLabel::~WidgetPropertyLabel()
{
}

//**************************************************************************************************************************************
//
//  Gets the current value of this label's content
//
//  val     Variant message arg value to return to the caller.
//
//**************************************************************************************************************************************
QStatus WidgetPropertyLabel::GetValue(_Out_ alljoyn_msgarg val) const
{
    QStatus status = ER_OK;

    std::string srcContent = DsbCommon::To_Ascii_String(m_srcValue->Data->ToString());
    alljoyn_msgarg variantarg = alljoyn_msgarg_create();
    CHK_POINTER(variantarg);
    CHK_AJSTATUS(alljoyn_msgarg_set(variantarg, ARG_STRING_STR, srcContent.c_str()));
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
