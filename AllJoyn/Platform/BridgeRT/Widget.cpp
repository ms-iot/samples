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


using namespace BridgeRT;


//**************************************************************************************************************************************
//
//  Constructor
//
//  pControlPanel           The control panel hosting this widget
//
//**************************************************************************************************************************************
Widget::Widget(ControlPanel* pControlPanel)
    : m_bRegistered(false)
    , m_busInterface(nullptr)
    , m_pControlPanel(pControlPanel)
    , m_state(EN_WIDGET)
    , m_bgColor(0)
    , m_busObject(nullptr)
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
Widget::~Widget()
{
    Destroy();
}

//**************************************************************************************************************************************
//
//  Static Get Property Handler
//
//  context          Pointer to Widget that holds property to Read
//  ifcName          AllJoyn Widget Interface Name
//  propName         Name of property to read
//  val              MsgArg value to return to the caller.  The Property data.
//
//**************************************************************************************************************************************
QStatus AJ_CALL Widget::GetPropertyHandler(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus result = ER_BAD_ARG_1;

    const Widget* pWidget = reinterpret_cast<const Widget*>(context);
    if (nullptr != pWidget)
    {
        result = pWidget->Get(ifcName, propName, val);
    }

    return result;
}

//**************************************************************************************************************************************
//
//  Static Set Property Handler
//
//  context          Pointer to Widget that holds Property to Set
//  ifcName          AllJoyn Widget Interface Name
//  propName         Name of property to read
//  val              Alljoyn Message Arg containing the variant property value to set
//
//**************************************************************************************************************************************
QStatus AJ_CALL Widget::SetPropertyHandler(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _In_ alljoyn_msgarg val)
{
    QStatus result = ER_BAD_ARG_1;

    Widget* pWidget = const_cast<Widget*>(reinterpret_cast<const Widget*>(context));
    if (nullptr != pWidget)
    {
        result = pWidget->Set(ifcName, propName, val);
    }

    return result;
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
QStatus Widget::Get(_In_z_ const char* interfaceName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val) const
{
    UNREFERENCED_PARAMETER(interfaceName);

    QStatus status = ER_BUS_NO_SUCH_PROPERTY;

    if (strcmp(propName, PROPERTY_VERSION_STR) == 0)
    {
        CHK_AJSTATUS(alljoyn_msgarg_set(val, ARG_UINT16_STR, 1));
    }
    else if (strcmp(propName, PROPERTY_STATES_STR) == 0)
    {
        CHK_AJSTATUS(alljoyn_msgarg_set(val, ARG_UINT32_STR, m_state));
    }

leave:
    return status;
}

//**************************************************************************************************************************************
//
//  Default Set Value handler.  In general derived classes should override this unless the widget is read-only
//
//  interfaceName    Not used.  Required by AllJoyn
//  propName         Name of property to read
//  val              MsgArg value to return to the caller.  The Property data.
//
//**************************************************************************************************************************************
QStatus Widget::Set(const char* interfaceName, const char* propName, alljoyn_msgarg val)
{
    UNREFERENCED_PARAMETER(interfaceName);
    UNREFERENCED_PARAMETER(propName);
    UNREFERENCED_PARAMETER(val);

    return ER_ALLJOYN_ACCESS_PERMISSION_ERROR;
}

//**************************************************************************************************************************************
//
//  Returns the AllJoyn Path Name of this Widget (e.g. "/ControlPanel/Mock_BACnet_Dimmable_Switch/Simple/en/..." );
//
//**************************************************************************************************************************************
const char* Widget::GetObjectPath()
{
    return alljoyn_busobject_getpath(m_busObject);
}


//**************************************************************************************************************************************
//
//  Widget Initialization Method.  This method:
//      -   Creates the default Widget Interface and Properties
//      -   Register Property Callback Handlers
//      -   Adds custom interfaces and Properties of derived classes
//      -   Announces the Interface and
//      -   Adds custom interface handlers for derived classes.
//
//  parentWidget        The widget that contains this widget (or null if the root container only)
//  widgetName          The name of this widget.  This value is used on the alljoyn bus to identify the widget
//  labelText           The static label of this widget.  This value is displayed by a control panel
//
//**************************************************************************************************************************************
QStatus Widget::Initialize(Widget* pParentWidget, const char* widgetName, const char* labelText)
{
    alljoyn_busobject_callbacks callbacks =
    {
        Widget::GetPropertyHandler,
        Widget::SetPropertyHandler,
        nullptr,
        nullptr
    };

    QStatus status = ER_OK;
    std::string objectPath;
    m_label = labelText;

    if (pParentWidget != nullptr)
    {
        objectPath = pParentWidget->GetObjectPath();
        objectPath += "/";
        objectPath += widgetName;
    }
    else
    {
        objectPath = widgetName;
    }

    // Create the Bus Object
    m_busObject = alljoyn_busobject_create(objectPath.c_str(), QCC_FALSE, &callbacks, this);
    CHK_POINTER(m_busObject);

    // Create the Bus Attachment
    status = alljoyn_busattachment_createinterface(m_pControlPanel->GetBus(), GetInterfaceName(), &m_busInterface);
    if (status == ER_BUS_IFACE_ALREADY_EXISTS)
    {
        m_busInterface = alljoyn_busattachment_getinterface(m_pControlPanel->GetBus(), GetInterfaceName());
        CHK_POINTER(m_busInterface);
        status = ER_OK;
    }
    CHK_AJSTATUS(status);

    // Add Verstion property bus interface
    status = alljoyn_interfacedescription_addproperty(m_busInterface, PROPERTY_VERSION_STR, ARG_UINT16_STR, ALLJOYN_PROP_ACCESS_READ);
    if ((status != ER_BUS_PROPERTY_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

    // Add States property to bus interface
    status = alljoyn_interfacedescription_addproperty(m_busInterface, PROPERTY_STATES_STR, ARG_UINT32_STR, ALLJOYN_PROP_ACCESS_READ);
    if ((status != ER_BUS_PROPERTY_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

    // Add OptParams property to bus interface
    status = alljoyn_interfacedescription_addproperty(m_busInterface, PROPERTY_OPTPARAMS_STR, ARG_DICT_STR, ALLJOYN_PROP_ACCESS_READ);
    if ((status != ER_BUS_PROPERTY_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

    // Add additional properties, methods or annotations belonging to derived classes
    status = AddCustomInterfaces(m_busInterface);
    if ((status != ER_BUS_PROPERTY_ALREADY_EXISTS) && (status != ER_BUS_MEMBER_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

    // Add metadata changed signal
    status = alljoyn_interfacedescription_addsignal(m_busInterface, SIGNAL_METACHANGED_STR, ARG_NONE_STR, ARG_NONE_STR, 0, nullptr);
    if ((status != ER_BUS_MEMBER_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

    // add interface to bus object
    status = alljoyn_busobject_addinterface_announced(m_busObject, m_busInterface);
    if ((status != ER_BUS_IFACE_ALREADY_EXISTS) && (status != ER_OK))
    {
        // this is OK
        goto leave;
    }

    // Add additional properties belonging to derived class
    status = AddCustomInterfaceHandlers(m_busObject, m_busInterface);
    if ((status != ER_BUS_PROPERTY_ALREADY_EXISTS) && (status != ER_BUS_MEMBER_ALREADY_EXISTS) && (status != ER_OK))
    {
        goto leave;
    }

    // Register the bus object on the bus attachment
    CHK_AJSTATUS(alljoyn_busattachment_registerbusobject(m_pControlPanel->GetBus(), m_busObject));
    m_bRegistered = true;

leave:
    return status;
}

//**************************************************************************************************************************************
//
//  Adds custom interfaces.  Default implementation adds no custom interfaces.  This is called before AddCustomInterfaces
//
//**************************************************************************************************************************************
QStatus Widget::AddCustomInterfaces(_In_ alljoyn_interfacedescription busInterface)
{
    UNREFERENCED_PARAMETER(busInterface);
    return ER_OK;
}

//**************************************************************************************************************************************
//
//  Adds custom interface handlers.  Default implementation adds no custom interfaces.  This is called after AddCustomInterfaces.
//
//**************************************************************************************************************************************
QStatus Widget::AddCustomInterfaceHandlers(_In_ alljoyn_busobject busObject, _In_ alljoyn_interfacedescription busInterface)
{
    UNREFERENCED_PARAMETER(busObject);
    UNREFERENCED_PARAMETER(busInterface);

    return ER_OK;
}

//**************************************************************************************************************************************
//
//  Cleanup method
//
//**************************************************************************************************************************************
void Widget::Destroy()
{
    if (m_busInterface != nullptr)
    {
        alljoyn_busattachment_deleteinterface(m_pControlPanel->GetBus(), m_busInterface);
        m_busInterface = nullptr;
    }

    if (m_busObject != nullptr)
    {
        if (m_bRegistered)
        {
            alljoyn_busattachment_unregisterbusobject(m_pControlPanel->GetBus(), m_busObject);
        }
        alljoyn_busobject_destroy(m_busObject);
        m_busObject = nullptr;
    }
}


//**************************************************************************************************************************************
//
//  Gets the new value of value managed by this widget and raises the value changed Signal for this widget.
//
//**************************************************************************************************************************************
void Widget::RaiseValueChangedSignal()
{
    QStatus status = ER_OK;
    alljoyn_interfacedescription_member signalDescription = { 0 };
    alljoyn_msgarg variantarg = nullptr;

    // send signal on AllJoyn
    QCC_BOOL signalFound = alljoyn_interfacedescription_getsignal(m_busInterface, SIGNAL_VALUCHANGED_STR, &signalDescription);
    if (QCC_TRUE == signalFound)
    {
        variantarg = alljoyn_msgarg_create();
        CHK_AJSTATUS(GetValue(variantarg));
        CHK_AJSTATUS(alljoyn_busobject_signal(m_busObject, nullptr, ALLJOYN_SESSION_ID_ALL_HOSTED, signalDescription, variantarg, 1, 0, 0, NULL));       
    }

leave:
    if (variantarg != nullptr)
    {
        alljoyn_msgarg_destroy(variantarg);
        variantarg = nullptr;
    }

    return;
}