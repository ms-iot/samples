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
#include "WidgetAction.h"

using namespace BridgeRT;

const char CP_ACTION_INTERFACE_NAME[] = "org.alljoyn.ControlPanel.Action";
const char METHOD_EXEC_STR[] = "Exec";
const uint16_t ACTION_BUTTON = 1;

WidgetAction::THandlerMap WidgetAction::s_execHandlerMap;

//**************************************************************************************************************************************
//
//  WidgetAction Constructor.  (This creates a button on the Control Panel Controller)
//
//  pControlPanel       Control Panel that hosts this action (this Control Panel Button)
//  pCallback           Callback to invoke when Control Panel Button is pressed
//
//**************************************************************************************************************************************
WidgetAction::WidgetAction(_In_ ControlPanel* pControlPanel, _In_ WidgetActionCallback pCallback)
    : Widget(pControlPanel)
    , m_pActionCallback(pCallback)
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
WidgetAction::~WidgetAction()
{
}


//**************************************************************************************************************************************
//
//  AllJoyn Interface Name for this Widget Type
//
//**************************************************************************************************************************************
const char* WidgetAction::GetInterfaceName()
{
    return CP_ACTION_INTERFACE_NAME;
}


//**************************************************************************************************************************************
//
//  Action Handler Callback.  This static function is invoked when a Control Panel's button is pressed.
//  Looks up the WidgetAction that the Exec call is intended for and invokes the Action Handler's Exec method
//
//  busObject        The Bus Object that the Control Panel invoked.
//  member           The interface description member that was invoked.
//  message          The message arguments.  This is not used for WidgetActions.
//
//**************************************************************************************************************************************
void WidgetAction::ExecHandler(
    alljoyn_busobject busObject,
    const alljoyn_interfacedescription_member* member,
    alljoyn_message message)
{
    UNREFERENCED_PARAMETER(message);
    QStatus status = ER_BUS_OBJ_NOT_FOUND;

    auto busObjectActionPair = s_execHandlerMap.find(busObject);
    if (busObjectActionPair != s_execHandlerMap.end())
    {
        status = busObjectActionPair->second->Exec(busObject, member);
    }
    alljoyn_busobject_methodreply_status(busObject, message, status);
}


QStatus WidgetAction::Exec(alljoyn_busobject bus, const alljoyn_interfacedescription_member* member)
{
    UNREFERENCED_PARAMETER(bus);
    UNREFERENCED_PARAMETER(member);
    return (*m_pActionCallback)(GetControlPanel());
}

//**************************************************************************************************************************************
//
//  Helper method that adds custom properties/methods or signals for this Widget type to the businterface.
//  The Base class implements the standard properties
//
//  busInterface        The Bus Interface to add this Widget's custom properties to
//
//**************************************************************************************************************************************
QStatus WidgetAction::AddCustomInterfaces(_In_ alljoyn_interfacedescription busInterface)
{
    QStatus status = ER_OK;

    // Add alljoyn method to the interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addmethod(busInterface, METHOD_EXEC_STR, ARG_NONE_STR, ARG_NONE_STR, ARG_NONE_STR, 0, nullptr));

leave:
    return status;
}

//**************************************************************************************************************************************
//
//  Helper method that adds custom properties/methods or signals for this Widget type to the businterface.
//  The Base class implements the standard properties
//
//  busInterface        The Bus Interface to add this Widget's custom properties to
//
//**************************************************************************************************************************************
QStatus WidgetAction::AddCustomInterfaceHandlers(_In_ alljoyn_busobject busObject, _In_ alljoyn_interfacedescription busInterface)
{
    QStatus status = ER_OK;
    QCC_BOOL bFound = QCC_FALSE;
    alljoyn_interfacedescription_member execMember = { 0 };

    // Get member description
    bFound = alljoyn_interfacedescription_getmember(busInterface, METHOD_EXEC_STR, &execMember);
    if (bFound != QCC_TRUE)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    CHK_AJSTATUS(alljoyn_busobject_addmethodhandler(
        busObject,
        execMember,
        reinterpret_cast<alljoyn_messagereceiver_methodhandler_ptr>(&WidgetAction::ExecHandler),
        this));


    s_execHandlerMap.insert(std::make_pair(busObject, this));

leave:
    return status;
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
QStatus WidgetAction::Get(_In_z_ const char* interfaceName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val) const
{
    QStatus status = ER_OK;
    alljoyn_msgarg values = nullptr;
    alljoyn_msgarg dictEntries = nullptr;

    // Handle the Optional Parameters if requested
    if (strcmp(propName, PROPERTY_OPTPARAMS_STR) == 0)
    {
        uint16_t keys[] = { LABEL_KEY, BGCOLOR_KEY, HINT_KEY };
        // Set of optional parameters
        uint16_t actionHints[] = { ACTION_BUTTON };

        // Create an array of Variant Type-Data value pairs
        values = alljoyn_msgarg_array_create(_countof(keys));
        CHK_POINTER(values);

        // Set each Variant "Type-Data Pair"
        CHK_AJSTATUS(alljoyn_msgarg_set(alljoyn_msgarg_array_element(values, LABEL_KEY), ARG_STRING_STR, GetLabel()));
        CHK_AJSTATUS(alljoyn_msgarg_set(alljoyn_msgarg_array_element(values, BGCOLOR_KEY), ARG_UINT32_STR, GetBgColor()));
        CHK_AJSTATUS(alljoyn_msgarg_set(alljoyn_msgarg_array_element(values, HINT_KEY), ARG_UINT16_ARRY_STR, _countof(actionHints), actionHints));

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