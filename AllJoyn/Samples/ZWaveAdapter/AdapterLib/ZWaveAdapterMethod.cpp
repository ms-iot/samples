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

#include "ZWaveAdapterMethod.h"
#include "Misc.h"

#include "Manager.h"
#include "value_classes\ValueID.h"
#include "command_classes\CommandClass.h"

#include <sstream>
#include <string>

using namespace std;
using namespace OpenZWave;
using namespace Platform;
using namespace DsbCommon;
using namespace BridgeRT;

namespace AdapterLib
{
    ZWaveAdapterMethod::ZWaveAdapterMethod(const ValueID & value)
        : m_valueId(value)
        , m_result(E_FAIL)
    {
    }

    void ZWaveAdapterMethod::Initialize()
    {
        m_name = ref new String(ConvertTo<wstring>(Manager::Get()->GetValueLabel(m_valueId)).c_str());
        m_description = ref new String(ConvertTo<wstring>(Manager::Get()->GetValueHelp(m_valueId)).c_str());
    }

    void ZWaveAdapterMethod::Execute()
    {
        bool bRet = Manager::Get()->PressButton(m_valueId);
        m_result = bRet ? S_OK : E_FAIL;
    }
}