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
#include "Widget.h"
#include "WidgetProperty.h"

namespace BridgeRT
{

    typedef QStatus(AJ_CALL * WidgetActionCallback)(_In_ ControlPanel* pContext);

    class WidgetAction : public Widget
    {
    public:
        WidgetAction(_In_ ControlPanel* pControlPanel, _In_ WidgetActionCallback pCallback);
        virtual ~WidgetAction();

        virtual QStatus Get(_In_z_ const char* interfaceName, _In_z_ const char* propName, _Out_ alljoyn_msgarg val) const;

        virtual const char* GetInterfaceName();

    protected:
        virtual QStatus AddCustomInterfaces(_In_ alljoyn_interfacedescription busInterface);
        virtual QStatus AddCustomInterfaceHandlers(_In_ alljoyn_busobject busObject, _In_ alljoyn_interfacedescription busInterface);

        QStatus Exec(alljoyn_busobject bus, const alljoyn_interfacedescription_member* member);

        static void ExecHandler(alljoyn_busobject bus, const alljoyn_interfacedescription_member* member, alljoyn_message message);

        typedef  std::map<alljoyn_busobject, WidgetAction*> THandlerMap;
        static THandlerMap s_execHandlerMap;

        WidgetActionCallback m_pActionCallback;

    };
}
