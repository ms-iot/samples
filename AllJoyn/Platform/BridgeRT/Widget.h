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

namespace BridgeRT
{
    class ControlPanel;

    class Widget
    {
    public:
        Widget(ControlPanel* pControlPanel);
        virtual ~Widget();

        virtual QStatus Initialize(Widget* pParentWidget, const char* widgetName, const char* labelText);
        virtual void Destroy();


        virtual QStatus Get(const char* interfaceName, const char* propName, alljoyn_msgarg val) const;
        virtual QStatus Set(const char* interfaceName, const char* propName, alljoyn_msgarg val);

        const char* GetObjectPath();

        uint32_t GetState() const { return m_state; }
        void SetState(uint32_t state) { m_state = state; }

        uint32_t GetBgColor() const { return m_bgColor; }
        void SetBgColor(uint32_t bgColor) { m_bgColor = bgColor; }

        const char* GetLabel() const { return m_label.c_str(); };

        void RaiseValueChangedSignal();

    protected:
        virtual QStatus AddCustomInterfaces(_In_ alljoyn_interfacedescription busInterface);
        virtual QStatus AddCustomInterfaceHandlers(_In_ alljoyn_busobject busObject, _In_ alljoyn_interfacedescription busInterface);

        ControlPanel* GetControlPanel() { return m_pControlPanel; };

        virtual const char* GetInterfaceName() = 0;

        virtual QStatus GetValue(_Out_ alljoyn_msgarg val) const
        {
            UNREFERENCED_PARAMETER(val);
            return ER_FAIL;
        }

    private:
        // Flag that identifies whether or not the Bus Object of this widget has been registered
        bool m_bRegistered;

        // The Control Panel hosting this widget
        ControlPanel* m_pControlPanel;

        // The Bus Object for this widget
        alljoyn_busobject m_busObject;

        // The Bus Interface for this widget
        alljoyn_interfacedescription m_busInterface;

        // The Current state of this widget (default state is enabled)
        uint32_t m_state;

        // The Background Color of this Widget (default color is black)
        uint32_t m_bgColor;

        // This Widget's Label.  For Property Widgets this is the Property's Label, the Value is managed by the Property Widget.
        std::string m_label;

        // Static AllJoyn Get Property Handler
        static QStatus AJ_CALL GetPropertyHandler(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val);

        // Static AllJoyn Set Property Handler
        static QStatus AJ_CALL SetPropertyHandler(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _In_ alljoyn_msgarg val);
    };

}