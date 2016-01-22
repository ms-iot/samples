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

#include "pch.h"
#include "collection.h"
#include "IMethod.h"

namespace DeviceProviders
{
    ref class AllJoynInterface;

    ref class AllJoynMethod sealed : public IMethod
    {
        DEBUG_LIFETIME_DECL(AllJoynMethod);

    internal:
        AllJoynMethod(_In_ AllJoynInterface ^ parent, const alljoyn_interfacedescription_member& methodDescription);
        AllJoynInterface ^ GetParent() const { return m_interface; }
        void Shutdown();

    public:
        virtual ~AllJoynMethod();

        virtual Windows::Foundation::IAsyncOperation<InvokeMethodResult^>^ InvokeAsync(
            Windows::Foundation::Collections::IVector<Platform::Object ^>^ params);

        virtual property Windows::Foundation::Collections::IVector<ParameterInfo ^>^ InSignature
        {
            Windows::Foundation::Collections::IVector<ParameterInfo ^>^ get();
        }
        virtual property Windows::Foundation::Collections::IVector<ParameterInfo ^>^ OutSignature
        {
            Windows::Foundation::Collections::IVector<ParameterInfo ^>^ get();
        }
        virtual property Platform::String ^ Name
        {
            Platform::String ^ get();
        }
        virtual property IInterface^ Interface
        {
            inline IInterface^ get() { return m_interface; }
        }
        virtual property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ Annotations
        {
            Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ get() { return m_annotations; }
        }

    private:
        std::atomic<bool> m_active;
        AllJoynInterface ^ m_interface;

        std::string m_name;
        Platform::Collections::Vector<ParameterInfo ^>^ m_inSignature;
        Platform::Collections::Vector<ParameterInfo ^>^ m_outSignature;
        std::string m_inSignatureString;
        std::string m_outSignatureString;
        Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ m_annotations;
    };
}