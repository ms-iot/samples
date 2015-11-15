
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
#include "WidgetProperty.h"

using namespace BridgeRT;

const char CP_PROPERTY_INTERFACE_NAME[] = "org.alljoyn.ControlPanel.Property";
const char PROPERTY_VALUE_STR[] = "Value";

//**************************************************************************************************************************************
//
//  Constructor
//
//  pControlPanel           The control panel hosting this widget
//  alljoynWidgetTypeId     The alljoyn widget type ID (See the AllJoyn Control Panel Interface Definition
//                          (https://allseenalliance.org/developers/learn/base-services/controlpanel/interface)
//  bReadOnly               Indicates whether or not this widget is read-only
//
//
//**************************************************************************************************************************************
WidgetProperty::WidgetProperty(_In_ ControlPanel* pControlPanel, _In_ uint16_t alljoynWidgetTypeId, _In_ bool bReadOnly)
    : Widget(pControlPanel)
    , m_alljoynWidgetTypeId(alljoynWidgetTypeId)
{
    if (!bReadOnly)
    {
        Widget::SetState(GetState() | EN_WRITABLE);
    }
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
WidgetProperty::~WidgetProperty()
{
}


//**************************************************************************************************************************************
//
//  Helper method that adds custom properties/methods or signals for this Widget type to the businterface.
//  The Base class implements the standard properties
//
//  busInterface        The Bus Interface to add this Widget's custom properties to
//
//**************************************************************************************************************************************
QStatus WidgetProperty::AddCustomInterfaces(_In_ alljoyn_interfacedescription busInterface)
{
    QStatus status = ER_OK;

    // Add the Value Property to the set of standard properties
    status = alljoyn_interfacedescription_addproperty(busInterface, PROPERTY_VALUE_STR, ARG_VARIANT_STR, ALLJOYN_PROP_ACCESS_RW);
    if ((status != ER_BUS_PROPERTY_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }
    
    // Add value changed signal
    status = alljoyn_interfacedescription_addsignal(busInterface, SIGNAL_VALUCHANGED_STR, ARG_VARIANT_STR, nullptr, 0, nullptr);
    if ((status != ER_BUS_MEMBER_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

leave:
    return status;
}

//**************************************************************************************************************************************
//
//  AllJoyn Interface Name for this Widget Type
//
//**************************************************************************************************************************************
const char* WidgetProperty::GetInterfaceName()
{
    return CP_PROPERTY_INTERFACE_NAME;
}

//**************************************************************************************************************************************
//
//  Gets the specified property valuename
//
//  interfaceName    Not used.  Required by AllJoyn
//  propName         Name of property to read
//  val              MsgArg value to return to the caller.  The Property data.
//
//**************************************************************************************************************************************
QStatus WidgetProperty::Get(_In_z_ const char* interfaceName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val) const
{
    QStatus status = ER_OK;
    alljoyn_msgarg values = nullptr;
    alljoyn_msgarg dictEntries = nullptr;

    // Handle the Get Value Property if requested
    if (strcmp(propName, PROPERTY_VALUE_STR) == 0)
    {
        CHK_AJSTATUS(GetValue(val));
    }
    // Handle the Optional Parameters if requested
    else if (strcmp(propName, PROPERTY_OPTPARAMS_STR) == 0)
    {
        uint16_t keys[] = { LABEL_KEY, BGCOLOR_KEY, HINT_KEY };  // Set of optional parameters
        // Set of optional parameters
        uint16_t propertyHints[] = { m_alljoynWidgetTypeId };

        // Create an array of Variant Type-Data value pairs
        values = alljoyn_msgarg_array_create(_countof(keys));
        CHK_POINTER(values);

        // Set each Variant "Type-Data Pair"
        CHK_AJSTATUS(alljoyn_msgarg_set(alljoyn_msgarg_array_element(values, LABEL_KEY), ARG_STRING_STR, GetLabel()));
        CHK_AJSTATUS(alljoyn_msgarg_set(alljoyn_msgarg_array_element(values, BGCOLOR_KEY), ARG_UINT32_STR, GetBgColor()));
        CHK_AJSTATUS(alljoyn_msgarg_set(alljoyn_msgarg_array_element(values, HINT_KEY), ARG_UINT16_ARRY_STR, _countof(propertyHints), propertyHints));

        // Create an array of Dictionary Entries
        dictEntries = alljoyn_msgarg_array_create(_countof(keys));
        CHK_POINTER(dictEntries);

        // Load the Dictionary Entries into the array of dictionary entries where the Keys are Number values that map to Variant Type-Data pairs
        for (int idx = 0; idx < _countof(keys); idx++)
        {
            CHK_AJSTATUS(alljoyn_msgarg_set(
                alljoyn_msgarg_array_element(dictEntries, idx),
                ARG_DICT_ITEM_STR,
                keys[idx],
                alljoyn_msgarg_array_element(values, keys[idx])));
        }

        // Return the array of dictionary entries for the Optional Parameters Output Property
        CHK_AJSTATUS(alljoyn_msgarg_set(val, ARG_DICT_STR, _countof(keys), dictEntries));
        alljoyn_msgarg_stabilize(val);
    }
    // The Properties are unknown, hand them up to the base Widget to process.
    else
    {
        CHK_AJSTATUS(Widget::Get(interfaceName, propName, val));
    }

leave:
    if (values != nullptr)
    {
        alljoyn_msgarg_destroy(values);
        values = nullptr;
    }
    if (dictEntries != nullptr)
    {
        alljoyn_msgarg_destroy(dictEntries);
        dictEntries = nullptr;
    }
    
    return status;
}

//**************************************************************************************************************************************
//
//  Default Set Value handler.  In general derived classes should override this unless the widget is read-only
//
//  val              Variant value arg passed from the caller to process
//
//**************************************************************************************************************************************
QStatus WidgetProperty::SetValue(alljoyn_msgarg val)
{
    UNREFERENCED_PARAMETER(val);
    return ER_FAIL;
}


//**************************************************************************************************************************************
//
//  Sets the specified property value
//
//  interfaceName    Not used.  Required by AllJoyn
//  propName         Name of property to set
//  val              Variant value arg passed from the caller
//
//**************************************************************************************************************************************
QStatus WidgetProperty::Set(const char* interfaceName, const char* propName, alljoyn_msgarg val)
{
    QStatus status = ER_OK;

    // Handle the Value Property setter if requested
    if (strcmp(propName, PROPERTY_VALUE_STR) == 0)
    {
        CHK_AJSTATUS(SetValue(val));
    }
    // The Properties are unknown, hand them up to the base Widget to process.
    else
    {
        CHK_AJSTATUS(WidgetProperty::Set(interfaceName, propName, val));
    }
leave:
    return status;
}
