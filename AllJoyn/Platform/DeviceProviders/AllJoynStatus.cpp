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
#include "AllJoynStatus.h"
#include "AllJoynHelpers.h"

using namespace Platform;
using namespace std;

namespace DeviceProviders
{
    AllJoynStatus::AllJoynStatus(QStatus statusCode)
        : m_statusCode(statusCode)
    {
    }

	AllJoynStatus::AllJoynStatus(QStatus statusCode, const string& statusText)
		: m_statusCode(statusCode)
		, m_statusText(statusText)
	{
	}

    uint32 AllJoynStatus::StatusCode::get()
    {
        return m_statusCode;
    }

    String ^ AllJoynStatus::StatusText::get()
    {
        return AllJoynHelpers::MultibyteToPlatformString(m_statusText.length() ? m_statusText.data() : QCC_StatusText(m_statusCode));
    }

    bool AllJoynStatus::IsSuccess::get()
    {
        return m_statusCode == QStatus::ER_OK;
    }

    bool AllJoynStatus::IsFailure::get()
    {
        return m_statusCode != QStatus::ER_OK;
    }
}