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

#include "ZWaveAdapterValue.h"
#include "Misc.h"

#include <string>

using namespace Windows::Foundation;
using namespace Platform;
using namespace std;
using namespace DsbCommon;

namespace AdapterLib
{
    ZWaveAdapterValue::ZWaveAdapterValue(String^ name, Object^ data)
        : m_name(name)
        , m_data(data)
    {
    }

    ZWaveAdapterValue::ZWaveAdapterValue(const ZWaveAdapterValue^ other)
        : m_name(other->m_name)
        , m_data(other->m_data)
    {

    }
}