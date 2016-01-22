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

#include "ZWaveAdapterDevice.h"
#include "ZWaveAdapterProperty.h"
#include "ZWaveAdapterValue.h"
#include "ZWaveADapterSignal.h"

#define PROPERTY_SWITCH  L"SwitchBinary.Switch"
#define PROPERTY_POWER   L"Meter.Power"
#define ATTRIBUTE_VALUE  L"value"
#define ATTRIBUTE_UNITS  L"units"
#define ATTRIBUTE_LABEL  L"label"
#define ATTRIBUTE_GENRE  L"genre"
#define ATTRIBUTE_GENRE_TYPE_USER  L"user"

namespace AdapterLib
{
    //*********************************************************************************************************************
    //
    // This class provides an example of a Basic Switch Control Panel Handler.  For a device with a single binary switch, a
    // single device with a single switch will be displayed on an AllJoyn Control Panel Controller.  For a smart switch device
    // with a single binary switch and a Power property, the Switch Control and Power will be displayed on an AllJoyn Control
    // Panel Controller.
    //
    // Other devices that have a single switch can also use this control panel handler as well.  For example a fan or
    // dimmer switch could use this, but the functionality will be limited to on-off.  To gain additional functionality
    // or controls, derive a new control panel handler from IControlPanelHandlerSimple and add a similar class in the
    // AllJoyn Bridge.  See ControlPanel.h in the Alljoyn Bridge.
    //
    //*********************************************************************************************************************
    ref class SimpleSwitchControlPanelHandler sealed :
        public BridgeRT::IControlPanelHandlerSimple
    {
    public:
        SimpleSwitchControlPanelHandler(AdapterLib::ZWaveAdapterDevice^ device);

        // Return the Switch Property for this device
        virtual property BridgeRT::IAdapterProperty^ SwitchProperty
        {
            BridgeRT::IAdapterProperty^ get() { return m_switchProperty; };
        }

        // Return the Switch Label for this device
        virtual property Platform::String^ SwitchLabel
        {
            Platform::String^  get() { return ref new Platform::String(L"Switch"); }
        }

        // Return the Switch Value for this device
        virtual property BridgeRT::IAdapterValue^ SwitchValue
        {
            BridgeRT::IAdapterValue^ get() { return m_switchValue; };
        }

        // Return the label to use for the Output Value (Power in <units>)
        virtual property Platform::String^ OutputValueLabel
        {
            Platform::String^ get() { return Platform::StringReference(L"Power in ") + m_powerUnits; };
        }

        // Return the Power Value for this device
        virtual property BridgeRT::IAdapterValue^ OutputValue
        {
            BridgeRT::IAdapterValue^ get() { return m_powerValue; };
        }

        // This Control Panel Handler does not support a RunButton
        virtual property Platform::String^ RunButtonLabel
        {
            Platform::String^ get()
            {
                return nullptr;
            };
        }

        // This Control Panel does not support the RunButton
        virtual property Platform::String^ RunEntryBoxLabel
        {
            Platform::String^ get() { return nullptr; };
        }

        // Run Handler
        virtual void Run(Platform::String ^);

        // Return the Change of Value Signal
        virtual property BridgeRT::IAdapterSignal^ ChangeOfValueSignal
        {
            BridgeRT::IAdapterSignal^ get() { return m_covSignal; };
        }

    private:
        // Device to control with this control panel handler
        AdapterLib::ZWaveAdapterDevice^ m_device;

        // Switch Property
        AdapterLib::ZWaveAdapterProperty^ m_switchProperty;

        // Switch Value
        AdapterLib::ZWaveAdapterValue^ m_switchValue;

        // Power Property of device (if present)
        AdapterLib::ZWaveAdapterProperty^ m_powerProperty;

        // Power Value of device (if present)
        AdapterLib::ZWaveAdapterValue^ m_powerValue;

        // Units of Power for device (if present)
        Platform::String^ m_powerUnits;

        // The Dimmable Light's change of value signal
        AdapterLib::ZWaveAdapterSignal^ m_covSignal;
    };
}
