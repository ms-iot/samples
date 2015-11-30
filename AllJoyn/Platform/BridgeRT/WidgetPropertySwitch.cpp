
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
#include "WidgetPropertySwitch.h"
#include "Bridge.h"

using namespace BridgeRT;

const uint16_t WIDGET_SWITCH_TYPE = 1;

//**************************************************************************************************************************************
//
//  Constructor
//
//  pControlPanel    The control panel hosting this widget
//  switchProperty   The Adapter Property Bag that this switch controls
//  switchValue      The Value inside the Switch Property Bag that this switch controls. (The actual value)
//
//**************************************************************************************************************************************
WidgetPropertySwitch::WidgetPropertySwitch(_In_ ControlPanel* pControlPanel, _In_ IAdapterProperty^ switchProperty, _In_ IAdapterValue^ switchValue)
    : WidgetProperty(pControlPanel, WIDGET_SWITCH_TYPE, false)
    , m_switchProperty(switchProperty)
    , m_switchValue(switchValue)
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
WidgetPropertySwitch::~WidgetPropertySwitch()
{
}

//**************************************************************************************************************************************
//
//  Gets the current property value from the source device and translate into an alljoyn value
//
//  val     Variant message arg value to return to the caller.
//
//**************************************************************************************************************************************
QStatus WidgetPropertySwitch::GetValue(_Out_ alljoyn_msgarg val) const
{
    QStatus status = ER_OK;
    alljoyn_msgarg variantarg = nullptr;

    // Get the current switch value as a boolean (if not boolean data type, this routine always reports true)
    bool bState = true;
    auto ipv = dynamic_cast<Windows::Foundation::IPropertyValue^>(m_switchValue->Data);
    if (ipv != nullptr)
    {
        if (ipv->Type == Windows::Foundation::PropertyType::Boolean)
        {
            bState = ipv->GetBoolean();
        }
    }

    // Create and return an alljoyn message arg with the current switch value
    variantarg = alljoyn_msgarg_create();
    CHK_POINTER(variantarg);
    CHK_AJSTATUS(alljoyn_msgarg_set(variantarg, ARG_BOOLEAN_STR, bState));
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
//  Translate the switch value from an alljoyn message arg to a boolean and set the underlying device's value
//
//  val     Variant Message Argument fromn alljoyn Control Panel to translate.
//
//**************************************************************************************************************************************
QStatus WidgetPropertySwitch::SetValue(_In_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    HRESULT result = S_OK;

    // Get the AllJoyn Argument
    bool bState = true;
    CHK_AJSTATUS(alljoyn_msgarg_get(val, ARG_BOOLEAN_STR, &bState));

    // Update the brown-field device
    m_switchValue->Data = bState;
    result = DsbBridge::SingleInstance()->GetAdapter()->SetPropertyValue(m_switchProperty, m_switchValue, nullptr);
    if (FAILED(result))
    {
        status = ER_FAIL;
    }

leave:
    return status;
}

