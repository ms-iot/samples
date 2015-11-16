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
#include "UniversalControlPanelHandler.h"
#include "SwitchControlPanelHandler.h"
#include "command_classes\SwitchBinary.h"
#include "command_classes\SwitchToggleBinary.h"

using namespace BridgeRT;
using namespace AdapterLib;
using namespace OpenZWave;

UniversalControlPanelHandler::UniversalControlPanelHandler(AdapterLib::ZWaveAdapterDevice^ device)
    : m_device(device)
    , m_controlledProperties(nullptr)
{
    for (IAdapterProperty^ aProperty : device->Properties)
    {
        ZWaveAdapterProperty^ aZWProperty = dynamic_cast<ZWaveAdapterProperty^>(aProperty);

        if ((GetValue(aProperty) != nullptr) && (GetLabel(aProperty) != nullptr))
        {
            ZWaveAdapterValue^ genre = aZWProperty->GetAttributeByName(ATTRIBUTE_GENRE);

            if ( genre->Data->ToString() != ATTRIBUTE_GENRE_TYPE_USER)
            {
                continue;
            }

            if (m_controlledProperties == nullptr)
            {
                m_controlledProperties = ref new Platform::Collections::Vector<IAdapterProperty^>();
            }
            m_controlledProperties->Append(aProperty);
        }

    }

    m_covSignal = dynamic_cast<ZWaveAdapterSignal^>(device->GetSignal(BridgeRT::Constants::CHANGE_OF_VALUE_SIGNAL));
}


Platform::String^ UniversalControlPanelHandler::GetLabel(IAdapterProperty^ prop)
{
    ZWaveAdapterProperty^ zwProp = dynamic_cast<ZWaveAdapterProperty^>(prop);
    return zwProp->GetAttributeByName(ATTRIBUTE_LABEL)->Data->ToString();
}

IAdapterValue^ UniversalControlPanelHandler::GetValue(IAdapterProperty^ prop)
{
    ZWaveAdapterProperty^ zwProp = dynamic_cast<ZWaveAdapterProperty^>(prop);
    return zwProp->GetAttributeByName(ATTRIBUTE_VALUE);
}

ControlType UniversalControlPanelHandler::GetType(IAdapterProperty^ prop)
{
    ZWaveAdapterProperty^ zwProp = dynamic_cast<ZWaveAdapterProperty^>(prop);

    volatile int8 commandClassId = zwProp->m_valueId.GetCommandClassId();
    if ((commandClassId == SwitchBinary::StaticGetCommandClassId()) ||
        (commandClassId == SwitchToggleBinary::StaticGetCommandClassId()))
    {
        return BridgeRT::ControlType::Switch;
    }
    else
    {
        return BridgeRT::ControlType::Sensor;
    }

}
