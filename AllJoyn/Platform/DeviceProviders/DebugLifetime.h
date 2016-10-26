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

#if defined(_DEBUG) && defined(DEBUG_VIEWMODEL_CTOR_DTOR)

#include "pch.h"
#include <stdlib.h>

namespace DebuggingTools
{
    class DebugLifetime
    {
    public:
        DebugLifetime(const char* className)
            : m_className(className)
        {
            char buffer[16];
            _itoa_s((int)GetNextId(), buffer, 10);
            m_instanceId = buffer;
            OutputDebugStringA(("C++ : " + m_className + "::ctor [" + m_instanceId + "]\n").c_str());
        }

        ~DebugLifetime()
        {
            OutputDebugStringA(("C++ : " + m_className + "::dtor [" + m_instanceId + "]\n").c_str());
        }

    private:
        static int GetNextId()
        {
            volatile static unsigned long nextId = 0;
            return ::InterlockedIncrement(&nextId);
        }

    private:
        std::string m_className;
        std::string m_instanceId;
    };
}

#define DEBUG_LIFETIME_DECL(className) std::unique_ptr<DebuggingTools::DebugLifetime> _debugLifeTime_##className;

#define DEBUG_LIFETIME_IMPL(className) _debugLifeTime_##className = std::unique_ptr<DebuggingTools::DebugLifetime>(new DebuggingTools::DebugLifetime(#className));

#else //  && defined(DEBUG_VIEWMODEL_CTOR_DTOR)

#define DEBUG_LIFETIME_DECL(className)

#define DEBUG_LIFETIME_IMPL(className)

#endif //  && defined(DEBUG_VIEWMODEL_CTOR_DTOR)