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
#include "MockAdapterDevice.h"

namespace AdapterLib
{

    //**************************************************************************************************************************************
    //
    // Temperature Sensor Control Panel Handler (Example)
    //
    // This class provides a simple handler that only displays and updates the current temperature.
    // All other IControlPanelHandlerSimple features are disabled.
    //
    //**************************************************************************************************************************************
    ref class ControlPanelHandlerTempSensor sealed :
        BridgeRT::IControlPanelHandlerSimple
    {
    public:

        ControlPanelHandlerTempSensor(MockAdapterDevice^ myDevice);
        virtual ~ControlPanelHandlerTempSensor();

        // Change of Value Signal
        virtual property BridgeRT::IAdapterSignal^ ChangeOfValueSignal
        {
            BridgeRT::IAdapterSignal^ get() { return m_covSignal; };
        }

        // Return the Property Name (Temperature) for this control panel handler
        virtual property Platform::String^ OutputValueLabel
        {
            Platform::String^ get()
            {
                return m_temperatureProperty->Name;
            };
        }

        // Return the temperature value
        virtual property BridgeRT::IAdapterValue^ OutputValue
        {
            BridgeRT::IAdapterValue^ get() { return m_temperatureValue; };
        }

        // Disable the Run Button
        virtual property Platform::String^ RunButtonLabel
        {
            Platform::String^ get() { return nullptr; };
        }

        // Disable the Run Entry Box
        virtual property Platform::String^ RunEntryBoxLabel
        {
            Platform::String^ get() { return nullptr; }
        }

        // Disable the Switch Control
        virtual property Platform::String^  SwitchLabel
        {
            Platform::String^  get() { return nullptr; };
        }

        // Disable the Switch Control
        virtual property  BridgeRT::IAdapterProperty^  SwitchProperty
        {
            BridgeRT::IAdapterProperty^ get() { return nullptr; };
        }

        // Disable the Switch Control
        virtual property BridgeRT::IAdapterValue^ SwitchValue
        {
            BridgeRT::IAdapterValue^ get() { return nullptr; };
        }

        // Implement required run handler (internally this does nothing as it is not used here)
        virtual void Run(Platform::String ^);

    private:

        // Device that sources the Temperature Data for this control panel
        AdapterLib::MockAdapterDevice^ m_device;

        // Property Bag of Device that sources the Temperature Data for this control panel
        AdapterLib::MockAdapterProperty^ m_temperatureProperty;

        // Value of Property Bag that sources the Temperature Data for this control panel
        AdapterLib::MockAdapterValue ^ m_temperatureValue;

        // Signal of the source Device that indicates when a value has changed.
        AdapterLib::MockAdapterSignal^ m_covSignal;

    };


    //**************************************************************************************************************************************
    //
    // Dimmer Switch Control Panel Handler (Example)
    //
    // This class provides a simple contol panel handler that allows the user to turn on or off a dimmer switch.  It also demonstrates how
    // to change the dimmer value through a simple text-box entry and action button.
    //
    //**************************************************************************************************************************************
    ref class ControlPanelHandlerDimmerSwitch sealed :
        BridgeRT::IControlPanelHandlerSimple
    {
    public:
        ControlPanelHandlerDimmerSwitch(MockAdapterDevice^ myDevice);
        virtual ~ControlPanelHandlerDimmerSwitch();

        virtual property BridgeRT::IAdapterSignal^ ChangeOfValueSignal
        {
            BridgeRT::IAdapterSignal^ get() { return m_covSignal; };
        }

        // Current Dim Level
        virtual property Platform::String^ OutputValueLabel
        {
            Platform::String^ get()
            {
                return m_dimProperty->Name;
            };
        }

        // Return the current Dimmer Value
        virtual property BridgeRT::IAdapterValue^ OutputValue
        {
            BridgeRT::IAdapterValue^ get() { return m_dimValue; };
        }

        // Return Apply for the Run Button's Label
        virtual property Platform::String^ RunButtonLabel
        {
            Platform::String^ get()
            {
                return ref new Platform::String(L"Apply");
            };
        }

        // Return the Text Entry Box's label
        virtual property Platform::String^ RunEntryBoxLabel
        {
            Platform::String^ get() { return ref new Platform::String(L"Set the dimmer level"); };
        }

        // Return the Property for the light switch
        virtual property BridgeRT::IAdapterProperty^ SwitchProperty
        {
            BridgeRT::IAdapterProperty^ get() { return m_switchProperty; };
        }

        // Return the lable of the light switch
        virtual property Platform::String^ SwitchLabel
        {
            Platform::String^  get() { return ref new Platform::String(L"Light Switch"); };
        }

        // The current switch value
        virtual property BridgeRT::IAdapterValue^ SwitchValue
        {
            BridgeRT::IAdapterValue^ get() { return m_switchValue; };
        }

        // Run Handler
        virtual void Run(Platform::String ^);

    private:

        // Device that sources the Temperature Data for this control panel
        AdapterLib::MockAdapterDevice^ m_device;

        // Property Bag of Switch that sources the on-off state of the dimmable light
        AdapterLib::MockAdapterProperty^ m_switchProperty;

        // Value from Property Bag that sources the on-off state of the dimmable light
        AdapterLib::MockAdapterValue^ m_switchValue;

        // Property Bag of Dimmer State of dimmable light
        AdapterLib::MockAdapterProperty^ m_dimProperty;

        // Current Value of Dimmer for dimmable light
        AdapterLib::MockAdapterValue^ m_dimValue;

        // The Dimmable Light's change of value signal
        AdapterLib::MockAdapterSignal^ m_covSignal;
    };

}
