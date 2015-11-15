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

namespace DeviceProviders
{
    interface class IBusObject;
    interface class IProperty;
    interface class ISignal;
    interface class IMethod;

    public interface class IInterface
    {
        property Windows::Foundation::Collections::IVectorView<IProperty ^>^ Properties
        {
            Windows::Foundation::Collections::IVectorView<IProperty ^>^ get();
        }
        property Windows::Foundation::Collections::IVectorView<IMethod ^>^ Methods
        {
            Windows::Foundation::Collections::IVectorView<IMethod ^>^ get();
        }
        property Windows::Foundation::Collections::IVectorView<ISignal ^>^ Signals
        {
            Windows::Foundation::Collections::IVectorView<ISignal ^>^ get();
        }
        property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ Annotations
        {
            Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ get();
        }
        property Platform::String ^ IntrospectXml
        {
            Platform::String ^ get();
        }
        property Platform::String ^ Name
        {
            Platform::String ^ get();
        }
        property IBusObject^ BusObject
        {
            IBusObject^ get();
        }

        IProperty^ GetProperty(Platform::String^ propertyName);
        IMethod^ GetMethod(Platform::String^ methodName);
        ISignal^ GetSignal(Platform::String^ SignalName);
    };
}