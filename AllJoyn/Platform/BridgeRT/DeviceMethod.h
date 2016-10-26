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
    class DeviceMain;

    class DeviceMethod
    {
    public:
        DeviceMethod();
        virtual ~DeviceMethod();

        QStatus Initialize(_In_ DeviceMain *parent, IAdapterMethod ^adapterMethod);
        uint32 InvokeMethod(_In_ alljoyn_message msg, _Out_ alljoyn_msgarg *outArgs, _Out_ size_t *nbOfArgs);
        inline std::string &GetName()
        {
            return m_exposedName;
        }

    private:
        QStatus SetName(_In_ Platform::String ^name);
        QStatus BuildSignature(_In_ IAdapterValueVector ^valueList, _Inout_ std::string &signature, _Inout_ std::string &parameterNames);

        // device method
        IAdapterMethod ^m_adapterMethod;

        // alljoyn related
        std::string m_exposedName;
        std::string m_inSignature;
        std::string m_outSignature;
        std::string m_parameterNames;

        // parent
        DeviceMain *m_parent;
    };
}

