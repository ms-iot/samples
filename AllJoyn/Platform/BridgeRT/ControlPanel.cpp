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
#include "WidgetContainer.h"
#include "WidgetAction.h"
#include "WidgetPropertyTextBox.h"
#include "WidgetPropertySwitch.h"
#include "WidgetPropertyLabel.h"
#include "Bridge.h"
#include "BridgeUtils.h"

using namespace BridgeRT;

static const char CP_INTERFACE_NAME[] = "org.alljoyn.ControlPanel.ControlPanel";
static const char CP_BUSOBJECT_ROOT_NAME[] = "/ControlPanel/";

const uint16_t CONTROL_PANEL_VERSION = 1;

//**************************************************************************************************************************************
//
//  Control Panel Constructor
//
//  controlPanelHandler     Custom Control Panel Data Provider and Action Handler for this Control Panel Producer
//  controlledDevice        Adapter Device controlled by this AllJoyn Control Panel Producer
//
//**************************************************************************************************************************************
ControlPanel::ControlPanel(_In_ IControlPanelHandler^ controlPanelHandler, _In_ IAdapterDevice^ controlledDevice)
    : m_bRegistered(false)
    , m_busObject(nullptr)
    , m_bus(nullptr)
    , m_interface(nullptr)
    , m_pRootContainer(nullptr)
    , m_controlPanelHandler(controlPanelHandler)
    , m_controlledDevice(controlledDevice)
{
}

//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
ControlPanel::~ControlPanel()
{
    // The Root Control Panel Container provided by this AllJoyn Control Panel
    if (m_pRootContainer != nullptr)
    {
        delete m_pRootContainer;
        m_pRootContainer = nullptr;
    }

    // The Bus Object for this AllJoyn Control Panel
    if (m_busObject != nullptr)
    {
        if (m_bRegistered)
        {
            alljoyn_busattachment_unregisterbusobject(m_bus, m_busObject);
        }
        alljoyn_busobject_destroy(m_busObject);
        m_busObject = nullptr;
    }
}


//**************************************************************************************************************************************
//
//  Get Property Handler for this Control Panel.
//  Returns the Version Property for this Control Panel
//
//  context     Pointer to a Control Panel Instance
//  ifcName     InterfaceName
//  propName    Property Name to read from the specified Control Panel
//  val         Value of queried property returned to the Alljoyn Caller.
//
//**************************************************************************************************************************************
QStatus AJ_CALL ControlPanel::GetPropertyHandler(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val)
{
    QStatus status = ER_OK;
    ControlPanel *controlPanel = nullptr;

    UNREFERENCED_PARAMETER(ifcName);

    controlPanel = (ControlPanel *)context;
    if (nullptr == controlPanel)	// sanity test
    {
        return ER_BAD_ARG_1;
    }

    if (strcmp(propName, PROPERTY_VERSION_STR) == 0)
    {
        status = alljoyn_msgarg_set(val, ARG_UINT16_STR, CONTROL_PANEL_VERSION);
    }
    else
    {
        status = ER_BUS_NO_SUCH_PROPERTY;
    }

    return status;
}

//**************************************************************************************************************************************
//
//  Initialize Method.
//
//  bus         AllJoyn Bus Attachment to use for announcing this control panel on
//  deviceName  Name of the device that this Control Panel will Control.  (e.g. "Light Switch")
//  panelName   Name of the Control Panel's Panel (e.g. "Simple")
//
//**************************************************************************************************************************************
QStatus ControlPanel::Initialize(_In_ alljoyn_busattachment bus, _In_z_ const wchar_t *deviceName, _In_z_ const wchar_t* panelName)
{
    alljoyn_busobject_callbacks controlPanelCallbacks =
    {
        ControlPanel::GetPropertyHandler,
        nullptr,
        nullptr,
        nullptr
    };

    QStatus status = ER_OK;
    std::string narrowDeviceName;
    std::string narrowPanelName;
    std::string objectPath;

    // sanity check
    if (nullptr == bus)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    if (nullptr == deviceName)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }
    if (nullptr == panelName)
    {
        status = ER_BAD_ARG_3;
        goto leave;
    }

    // Narrow source strings for alljoyn compatibility
    narrowDeviceName = ConvertTo<std::string>(deviceName);
    narrowPanelName = ConvertTo<std::string>(panelName);

    // save bus attachment and reset members
    m_bus = bus;

    // Create the Control Panel object name
    objectPath = CP_BUSOBJECT_ROOT_NAME;
    objectPath += narrowDeviceName;
    objectPath += "/";
    objectPath += narrowPanelName;
    std::replace(objectPath.begin(), objectPath.end(), ' ', '_');

    // Create Control Panel Bus Object
    m_busObject = alljoyn_busobject_create(objectPath.c_str(), QCC_FALSE, &controlPanelCallbacks, this);
    CHK_POINTER(m_busObject);

    // Create Bus Control Panel Interface
    CHK_AJSTATUS(alljoyn_busattachment_createinterface(m_bus, CP_INTERFACE_NAME, &m_interface));

    // Add "Version" Property to Control Panel Interface
    CHK_AJSTATUS(alljoyn_interfacedescription_addproperty(m_interface, PROPERTY_VERSION_STR, ARG_UINT16_STR, ALLJOYN_PROP_ACCESS_READ));

    // Activate the Control Panel Interface
    alljoyn_interfacedescription_activate(m_interface);

    // add Control Panel interface to bus object
    status = alljoyn_busobject_addinterface_announced(m_busObject, m_interface);
    if (ER_BUS_IFACE_ALREADY_EXISTS == status)
    {
        // this is OK
        status = ER_OK;
    }
    CHK_AJSTATUS(status);

    // register Control Panel Bus Object on Bus attachment
    CHK_AJSTATUS(alljoyn_busattachment_registerbusobject(m_bus, m_busObject));
    m_bRegistered = true;

    // Create the Root English Container
    objectPath += "/en";
    m_pRootContainer = new (std::nothrow) WidgetContainer(this);
    CHK_POINTER(m_pRootContainer);

    // Initialize the root container
    CHK_AJSTATUS(m_pRootContainer->Initialize(nullptr, objectPath.c_str(), narrowDeviceName.c_str()));

leave:

    return status;
}

//**************************************************************************************************************************************
//
//  Simple Control Panel Constructor.  The simple control panel creates an alljoyn control panel with the following controls,
//  created in the following order:
//
//  [Labeled Boolean Switch]
//  [Labeled Property]
//  [Text Edit Box]
//  [Run Button]
//
//  Data Values for each control are provided by the Control Panel Handler.
//
//  The Boolean Switch can be used to turn something on or off.  The switch both modifies a devices value and updates to
//  reflect external changes made on the switch.  (The Switch may be disabled, see IControlPanelHandlerSimple for details)
//
//  The Labeled Property shows a data property with a label.  The label value updates to reflect external changes made to the data value,
//  but is not directly settable by an AllJoyn Client. (The Label may be disabled, see IControlPanelHandlerSimple for details)
//
//  The Text Edit Box and Run Button are tied together.  Text changes are cached within the Bridge until the Run Button is pressed.
//  When the Run Button is pressed the event is handled by this control panel.  The Run command is forwarded to the registered Control Panel
//  Handler with the Text Edit Box value for custom handling. (The Text Edit Box/Button combo may be disabled, see IControlPanelHandlerSimple for details)
//
//
//  controlPanelHandler     Custom Control Panel Data Provider and Action Handler for this Control Panel Producer
//  controlledDevice        Adapter Device controlled by this AllJoyn Control Panel Producer
//
//**************************************************************************************************************************************
ControlPanelSimple::ControlPanelSimple(_In_ IControlPanelHandlerSimple^ controlPanelHandler, _In_ IAdapterDevice^ controlledDevice)
    : ControlPanel(controlPanelHandler, controlledDevice)
    , m_pRunEditBox(nullptr)
    , m_pOutputLabel(nullptr)
    , m_pRunButton(nullptr)
    , m_pSwitch(nullptr)
{
    m_pSignalHandler = ref new WidgetSignalHandler(this);
}


//**************************************************************************************************************************************
//
//  Destructor
//
//**************************************************************************************************************************************
ControlPanelSimple::~ControlPanelSimple()
{
    if (m_pSwitch != nullptr)
    {
        delete m_pSwitch;
        m_pSwitch = nullptr;
    }

    if (m_pRunEditBox != nullptr)
    {
        delete m_pRunEditBox;
        m_pRunEditBox = nullptr;
    }

    if (m_pRunButton != nullptr)
    {
        delete m_pRunButton;
        m_pRunButton = nullptr;
    }

    if (m_pOutputLabel != nullptr)
    {
        delete m_pOutputLabel;
        m_pOutputLabel = nullptr;
    }
}

//**************************************************************************************************************************************
//
// Initialize Method Override.  Calls base class Initialize method and then attaches each of the Simple Control Panels' Widgets to
// the root container created by the base class.
//
//  bus         AllJoyn Bus Attachment to use for announcing this control panel on
//  deviceName  Name of the device that this Control Panel will Control.  (e.g. "Light Switch")
//  panelName   Name of the Control Panel Panel (e.g. "Simple")
//
//**************************************************************************************************************************************
QStatus ControlPanelSimple::Initialize(_In_ alljoyn_busattachment bus, _In_z_ const wchar_t *deviceName, _In_z_ const wchar_t* panelName)
{
    QStatus status = ER_OK;
    std::string adapterName;

    IControlPanelHandlerSimple^ cntrlPanelHandler = (IControlPanelHandlerSimple^)GetControlPanelHandler();
    CHK_AJSTATUS(ControlPanel::Initialize(bus, deviceName, panelName));

    adapterName = ConvertTo<std::string>(deviceName);

    WidgetContainer* pRoot = GetRootContainer();

    // Attach a switch to the root container widget if the Switch Value is non-null
    if (cntrlPanelHandler->SwitchValue)
    {
        std::string label = ConvertTo<std::string>(cntrlPanelHandler->SwitchLabel);
        m_pSwitch = new (std::nothrow) WidgetPropertySwitch(this, cntrlPanelHandler->SwitchProperty, cntrlPanelHandler->SwitchValue);
        CHK_POINTER(m_pSwitch);
        CHK_AJSTATUS(m_pSwitch->Initialize(pRoot, "Switch", label.c_str()));
    }

    // Attach label to root container widget if the Label Output Value and OutputValue label is non-null
    if ((cntrlPanelHandler->OutputValue != nullptr) && (cntrlPanelHandler->OutputValueLabel != nullptr))
    {
        std::string label = ConvertTo<std::string>(cntrlPanelHandler->OutputValueLabel);
        m_pOutputLabel = new (std::nothrow) WidgetPropertyLabel(this, cntrlPanelHandler->OutputValue);
        CHK_POINTER(m_pOutputLabel);
        CHK_AJSTATUS(m_pOutputLabel->Initialize(pRoot, "OutputValue", label.c_str()));
    }

    // Attach Run Button, and potentially a Text Edit Box, if the Run-ButtonLabel is non-null
    if (cntrlPanelHandler->RunButtonLabel != nullptr)
    {
        std::string labelName;
        if (cntrlPanelHandler->RunEntryBoxLabel != nullptr)
        {
            labelName = ConvertTo<std::string>(cntrlPanelHandler->RunEntryBoxLabel);
            // Attach a text entry box to the root container widget
            m_pRunEditBox = new (std::nothrow) WidgetPropertyTextBox(this);
            m_pRunEditBox->Set("");
            CHK_POINTER(m_pRunEditBox);
            CHK_AJSTATUS(m_pRunEditBox->Initialize(pRoot, "RunEditBox", labelName.c_str()));
        }

        labelName = ConvertTo<std::string>(cntrlPanelHandler->RunButtonLabel);
        // Attach a text entry box to the root container widget
        m_pRunButton = new (std::nothrow) WidgetAction(this, &ControlPanelSimple::ButtonHandler);
        CHK_POINTER(m_pRunButton);
        CHK_AJSTATUS(m_pRunButton->Initialize(pRoot, "RunButton", labelName.c_str()));
    }

    // Register a Change of Value Handler if the Change of Value Signal is non-null
    if (cntrlPanelHandler->ChangeOfValueSignal != nullptr)
    {
        HRESULT hr = DsbBridge::SingleInstance()->GetAdapter()->RegisterSignalListener(cntrlPanelHandler->ChangeOfValueSignal, m_pSignalHandler, nullptr);
        if (FAILED(hr))
        {
            status = ER_FAIL;
        }
    }

leave:
    return status;
}

//**************************************************************************************************************************************
//
//  The Static Run-Button Handler.  Forwards the ButtonHandler Request to the specific Control Panel Handler.  (See RunButton)
//
//  pContext     Pointer to the ControlPanelSimple instance
//
//**************************************************************************************************************************************
QStatus AJ_CALL ControlPanelSimple::ButtonHandler(_In_ ControlPanel* pContext)
{
    ControlPanelSimple* pThis = reinterpret_cast<ControlPanelSimple*>(pContext);
    return pThis->RunButton();
}

//**************************************************************************************************************************************
//
// RunButton instance handler.  Gets the textbox value (if supported) and calls the control panel handler registered with this
// control panel instance.
//
//**************************************************************************************************************************************
QStatus ControlPanelSimple::RunButton()
{
    QStatus result = ER_OK;

    Platform::String^ runArgument = nullptr;
    try
    {

        // If the Edit Box was created for this control panel, get its current value and convert it to a platform string.
        if (m_pRunEditBox != nullptr)
        {
            std::wstring unicodeArgument;
            std::string asciiArgument(m_pRunEditBox->Get());
            unicodeArgument = ConvertTo<std::wstring>(asciiArgument);
            runArgument = ref new Platform::String(unicodeArgument.c_str());
        }

        // Call the Control Panel Handler's Run method with either a null string or the text edit box's content
        IControlPanelHandlerSimple^ cntrlPanelHandler = (IControlPanelHandlerSimple^)GetControlPanelHandler();
        cntrlPanelHandler->Run(runArgument);
    }
    catch (...)
    {
        result = ER_FAIL;
    }

    return result;
}


//**************************************************************************************************************************************
//
// If a value on this device changed, walk through each widget and signal that the value changed on the AllJoyn Bus.
//
//**************************************************************************************************************************************

void ControlPanelSimple::ValueChanged()
{
    if (m_pSwitch != nullptr)
    {
        m_pSwitch->RaiseValueChangedSignal();
    }

    if (m_pOutputLabel != nullptr)
    {
        m_pOutputLabel->RaiseValueChangedSignal();
    }
}



//***********************************************************************************************************************************************
//
//  Control Panel Constructor  Implementation of a universal style control panel
//
//***********************************************************************************************************************************************
ControlPanelUniversal::ControlPanelUniversal(_In_ IControlPanelHandlerUniversal^ controlPanelHandler, _In_ IAdapterDevice^ controlledDevice)
    : ControlPanel(controlPanelHandler, controlledDevice)
{
    m_pSignalHandler = ref new WidgetSignalHandler(this);
}


//***********************************************************************************************************************************************
//
//  Control Panel Destructor
//
//***********************************************************************************************************************************************
ControlPanelUniversal::~ControlPanelUniversal()
{
    if (m_widgets.size() > 0)
    {
        for (auto widget : m_widgets)
        {
            delete widget;
        }
        m_widgets.clear();
    }
}

//***********************************************************************************************************************************************
//
//  Control Panel Destructor
//
//***********************************************************************************************************************************************
QStatus ControlPanelUniversal::Initialize(_In_ alljoyn_busattachment bus, _In_z_ const wchar_t *deviceName, _In_z_ const wchar_t* panelName)
{
    QStatus status = ER_OK;
    std::string adapterName = ConvertTo<std::string>(deviceName);
    IControlPanelHandlerUniversal^ cntrlPanelHandler = (IControlPanelHandlerUniversal^)GetControlPanelHandler();

    // Initialize Control Panel base class
    CHK_AJSTATUS(ControlPanel::Initialize(bus, deviceName, panelName));

    WidgetContainer* pRoot = GetRootContainer();
    int idx = 0;

    // If there are user controls to render, then create widgets for them.
    if (cntrlPanelHandler->ControlledProperties)
    {
        std::string allJoynWidgetName;

        // fore each property of the device that we want the user to control
        for (auto sourcePropertyForControl : cntrlPanelHandler->ControlledProperties)
        {
            // create a unique, control relative, name for each widget
            allJoynWidgetName = "Widget";
            allJoynWidgetName += std::to_string(idx);

            // Ask the Control Panel Handler for the label and type of the specified property
            std::string label = ConvertTo<std::string>(cntrlPanelHandler->GetLabel(sourcePropertyForControl));
            auto type = cntrlPanelHandler->GetType(sourcePropertyForControl);

            // Based on the Property's Control Panel Type, create the appropriate widget for it and add it to the list of widgets
            // owned by this control panel
            switch (type)
            {
            case BridgeRT::ControlType::Switch:
            {
                auto pSwitch = new (std::nothrow) WidgetPropertySwitch(this, sourcePropertyForControl, cntrlPanelHandler->GetValue(sourcePropertyForControl));
                CHK_POINTER(pSwitch);
                m_widgets.push_back(pSwitch);
                CHK_AJSTATUS(pSwitch->Initialize(pRoot, allJoynWidgetName.c_str(), label.c_str()));
                break;
            }

            case BridgeRT::ControlType::Sensor:
            {
                auto pSensor = new (std::nothrow) WidgetPropertyLabel(this, cntrlPanelHandler->GetValue(sourcePropertyForControl));
                CHK_POINTER(pSensor);
                m_widgets.push_back(pSensor);
                CHK_AJSTATUS(pSensor->Initialize(pRoot, allJoynWidgetName.c_str(), label.c_str()));
                break;
            }

            default:
                status = ER_INIT_FAILED;
                goto leave;
            }

            // Increment counter
            ++idx;
        }
    }

    // There must be at least one widgetable item to create a control panel
    if (idx == 0)
    {
        status = ER_INIT_FAILED;
        goto leave;
    }

    // Register a Change of Value Handler if the Change of Value Signal is non-null
    if (cntrlPanelHandler->ChangeOfValueSignal != nullptr)
    {
        HRESULT hr = DsbBridge::SingleInstance()->GetAdapter()->RegisterSignalListener(cntrlPanelHandler->ChangeOfValueSignal, m_pSignalHandler, nullptr);
        if (FAILED(hr))
        {
            status = ER_FAIL;
        }
    }

leave:
    return status;
}

//***********************************************************************************************************************************************
//
//  Value Changed handler.  Iterate through all widgets and raise their property changed signal
//
//***********************************************************************************************************************************************
void ControlPanelUniversal::ValueChanged()
{
    for (auto widget : m_widgets)
    {
        widget->RaiseValueChangedSignal();
    }

}
