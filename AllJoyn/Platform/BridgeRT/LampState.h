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
#include "LSFSignalHandler.h"

namespace BridgeRT
{
    class LSF;

    class LampState
    {
    public:
        LampState(LSF* pLightingService);
        virtual ~LampState();

        QStatus Initialize();
        
        QStatus Get(_In_z_ const char* propName, _Out_ alljoyn_msgarg val);
        QStatus Set(_In_z_ const char* propName, _In_ alljoyn_msgarg val);

        void RaiseStateChangedSignal();

    private:

        static void AJ_CALL AJMethod(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg);
        QStatus invokeTransitionLampState(_In_ alljoyn_message msg, _Out_ alljoyn_msgarg *outArgs);
        QStatus invokeApplyPulseEffect(_In_ alljoyn_message msg, _Out_ alljoyn_msgarg *outArgs);
        static LampState* GetInstance(_In_ alljoyn_busobject busObject);

        QStatus setStateParameter(_In_ alljoyn_msgarg msgArg, _Out_ State^ newState);

        LSF* m_pLightingService;
        alljoyn_interfacedescription m_lampStateInterface;
        alljoyn_interfacedescription_member m_methodTransitionLampState_Member;
        alljoyn_interfacedescription_member m_methodApplyPulseEffect_Member;

        IAdapterSignal^ m_pLampState_LampStateChanged;
        LSFSignalHandler^ m_pLSFSignalHandler;
    };
}
