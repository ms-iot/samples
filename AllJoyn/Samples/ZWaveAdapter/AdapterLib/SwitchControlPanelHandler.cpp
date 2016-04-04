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
#include "SwitchControlPanelHandler.h"

using namespace BridgeRT;
using namespace AdapterLib;

SimpleSwitchControlPanelHandler::SimpleSwitchControlPanelHandler(AdapterLib::ZWaveAdapterDevice^ device)
    : m_device(device)
    , m_switchProperty(nullptr)
    , m_switchValue(nullptr)
    , m_powerProperty(nullptr)
    , m_powerValue(nullptr)
    , m_powerUnits(L"")
{
    //find the "Switch" Property
    m_switchProperty = device->GetPropertyByName(PROPERTY_SWITCH);

    //find the "Switch" Value
    m_switchValue = m_switchProperty->GetAttributeByName(ATTRIBUTE_VALUE);

    //find the "Power" Property
    m_powerProperty = device->GetPropertyByName(PROPERTY_POWER);
    if (m_powerProperty != nullptr)
    {
        ZWaveAdapterValue^ units = nullptr;
        //find the "Power" Value
        m_powerValue = m_powerProperty->GetAttributeByName(ATTRIBUTE_VALUE);
        if (m_powerValue != nullptr)
        {
            //find the "Power" Units
            units = m_powerProperty->GetAttributeByName(ATTRIBUTE_UNITS);
            if (units != nullptr)
            {
                m_powerUnits = units->Data->ToString();
            }
        }

        // If we were unable to find the power units or the power value, then don't support them in this instance
        if ((units == nullptr) || (m_powerValue == nullptr))
        {
            m_powerProperty = nullptr;
            m_powerValue = nullptr;
        }

    }

    m_covSignal = dynamic_cast<ZWaveAdapterSignal^>(device->GetSignal(BridgeRT::Constants::CHANGE_OF_VALUE_SIGNAL));
}

void SimpleSwitchControlPanelHandler::Run(Platform::String^ runArg)
{
    UNREFERENCED_PARAMETER(runArg);
}
