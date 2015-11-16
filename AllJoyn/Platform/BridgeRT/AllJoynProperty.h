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
    class PropertyInterface;

    class AllJoynProperty
    {
    public:
        AllJoynProperty();
        virtual ~AllJoynProperty();

        QStatus Create(_In_ IAdapterAttribute ^adapterAttribute, _In_ PropertyInterface *parent);
        bool IsSameType(_In_ IAdapterAttribute ^adapterAttribute);

        inline std::string *GetName()
        {
            return &m_exposedName;
        }
        inline std::string *GetSignature()
        {
            return &m_signature;
        }

    private:
        bool AreAnnotationsSame(_In_ IAnnotationMap ^annotations);

    private:
        QStatus SetName(_In_ Platform::String^ name);

        PropertyInterface *m_parent;
        Platform::String ^m_originalName;
        Platform::TypeCode m_dsbType;
        Windows::Foundation::PropertyType m_dsbSubType;

        std::string m_signature;
        std::string m_exposedName;

        //annotations
        IAnnotationMap^ m_annotations;

        //access
        E_ACCESS_TYPE m_access;

        //COV Behavior
        SignalBehavior m_covBehavior;
    };
}

