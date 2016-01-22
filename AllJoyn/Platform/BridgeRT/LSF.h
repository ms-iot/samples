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
    class LampService;
    class LampParameters;
    class LampDetails;
    class LampState;
    
    class LSF
    {
    public:
        LSF(_In_ IAdapterDevice^ pLightingDevice);
        virtual ~LSF();

        QStatus Initialize(_In_ alljoyn_busattachment bus);

        alljoyn_busattachment GetBus()
        {
            return m_bus;
        }

        alljoyn_busobject GetBusObject()
        {
            return m_busObject;
        }

        ILSFHandler^ GetLSFHandler()
        {
            return m_pLightingServiceHandler;
        }

        LampService* GetLampServiceInterfacePtr()
        {
            return m_pLampServiceInterface;
        }

        LampParameters* GetLampParametersInterfacePtr()
        {
            return m_pLampParametersInterface;
        }

        LampDetails* GetLampDetailsInterfacePtr()
        {
            return m_pLampDetailsInterface;
        }

        LampState* GetLampStateInterfacePtr()
        {
            return m_pLampStateInterface;
        }

    private:
        static QStatus AJ_CALL getProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val);
        static QStatus AJ_CALL setProperty(_In_ const void* context, _In_z_ const char* ifcName, _In_z_ const char* propName, _In_ alljoyn_msgarg val);

        bool m_bRegistered;
        alljoyn_busobject m_busObject;
        alljoyn_busattachment m_bus;

        ILSFHandler^ m_pLightingServiceHandler;

        LampService* m_pLampServiceInterface;
        LampParameters* m_pLampParametersInterface;
        LampDetails* m_pLampDetailsInterface;
        LampState* m_pLampStateInterface;
        
        IAdapterDevice^ m_pLightingDevice;
    };
}

