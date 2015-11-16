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
#include "WidgetSignalHandler.h"

namespace BridgeRT
{
    class Widget;
    class WidgetContainer;
    class WidgetProperty;
    class WidgetAction;
    class WidgetPropertyLabel;
    class WidgetPropertyTextBox;
    class WidgetPropertySwitch;

    //***********************************************************************************************************************************************
    //
    //  Base Control Panel Class.  Derive custom control panel handlers from here to provide advanced control panels.  Derived classes can
    //  vary the layout and available controls.  Derived classes can also extend the amount of actions and properties that are displayed on a
    //  control panel.
    //
    //  See ControlPanelSimple (below) for an example control panel.
    //
    //***********************************************************************************************************************************************
    class ControlPanel
    {
    public:
        ControlPanel(_In_ IControlPanelHandler^ controlPanelHandler, _In_ IAdapterDevice^ controlledDevice);
        virtual ~ControlPanel();

        virtual QStatus Initialize(_In_ alljoyn_busattachment bus, _In_z_ const wchar_t *unitName, _In_z_ const wchar_t* panelName);

        alljoyn_busattachment GetBus()
        {
            return m_bus;
        }

    protected:
        virtual WidgetContainer* GetRootContainer()
        {
            return m_pRootContainer;
        }

        IControlPanelHandler^ GetControlPanelHandler()
        {
            return m_controlPanelHandler;
        }

        IAdapterDevice^ GetDevice()
        {
            return m_controlledDevice;
        }

        friend WidgetSignalHandler;
        virtual void ValueChanged() = 0;

    private:
        bool m_bRegistered;
        alljoyn_busobject m_busObject;
        alljoyn_busattachment m_bus;
        alljoyn_interfacedescription m_interface;

        WidgetContainer* m_pRootContainer;

        IControlPanelHandler^ m_controlPanelHandler;
        IAdapterDevice^ m_controlledDevice;
        static QStatus AJ_CALL GetPropertyHandler(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val);
    };

    //***********************************************************************************************************************************************
    //
    //  Implementation of a simple control panel that is adequate to handle a wide range of very simple devices like switches, dimmers or
    //  thermostats.
    //
    //***********************************************************************************************************************************************
    class ControlPanelSimple : public ControlPanel
    {
    public:
        ControlPanelSimple(_In_ IControlPanelHandlerSimple^ controlPanelHandler, _In_ IAdapterDevice^ controlledDevice);
        virtual ~ControlPanelSimple();

        virtual QStatus Initialize(_In_ alljoyn_busattachment bus, _In_z_ const wchar_t *unitName, _In_z_ const wchar_t* panelName);

    protected:
        virtual QStatus RunButton();

        virtual void ValueChanged();

    private:
        WidgetPropertyTextBox* m_pRunEditBox;
        WidgetPropertyLabel* m_pOutputLabel = nullptr;
        WidgetAction* m_pRunButton = nullptr;
        WidgetPropertySwitch* m_pSwitch = nullptr;
        static QStatus AJ_CALL ButtonHandler(_In_ ControlPanel* pContext);
        WidgetSignalHandler^ m_pSignalHandler;
    };


    //***********************************************************************************************************************************************
    //
    //  Implementation of a universal style control panel
    //
    //***********************************************************************************************************************************************
    class ControlPanelUniversal : public ControlPanel
    {
    public:
        ControlPanelUniversal(_In_ IControlPanelHandlerUniversal^ controlPanelHandler, _In_ IAdapterDevice^ controlledDevice);
        virtual ~ControlPanelUniversal();

        virtual QStatus Initialize(_In_ alljoyn_busattachment bus, _In_z_ const wchar_t *unitName, _In_z_ const wchar_t* panelName);

    protected:
        virtual void ValueChanged();

    private:
        // ordered list of widgets displayed by the control panel
        std::vector<Widget*> m_widgets;

        // WinRT to Native Signal Handler Helper
        WidgetSignalHandler^ m_pSignalHandler;
    };
}


