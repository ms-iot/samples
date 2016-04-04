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

#include <alljoyn_c/AboutIcon.h>
#include <alljoyn_c/AboutIconObj.h>

namespace BridgeRT
{

	class AllJoynAboutIcon
	{
	public:
		AllJoynAboutIcon();
		~AllJoynAboutIcon();

		QStatus Initialize(_In_ alljoyn_busattachment bus, _In_ IAdapterIcon^ icon);
		void ShutDown();
       
	private:
        alljoyn_abouticon m_icon = nullptr;
        alljoyn_abouticonobj m_iconObj = nullptr;
        std::vector<BYTE> m_imageData;
	};
}
