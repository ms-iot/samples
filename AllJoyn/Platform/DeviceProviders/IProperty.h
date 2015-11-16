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
    interface class ITypeDefinition;
    interface class IInterface;
    ref class AllJoynStatus;

    public ref struct ReadValueResult sealed
    {
        property AllJoynStatus^ Status;
        property Platform::Object ^ Value;
    };

    public interface class IProperty
    {
        Windows::Foundation::IAsyncOperation<ReadValueResult ^> ^ ReadValueAsync();
        Windows::Foundation::IAsyncOperation<AllJoynStatus^> ^ SetValueAsync(Platform::Object ^ newValue);

        property Platform::String ^ Name
        {
            Platform::String ^ get();
        }

        property ITypeDefinition ^ TypeInfo
        {
            ITypeDefinition ^ get();
        }

        property bool CanRead
        {
            bool get();
        }

        property bool CanWrite
        {
            bool get();
        }

        property IInterface^ Interface
        {
            IInterface^ get();
        }

        property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ Annotations
        {
            Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^> ^ get();
        }

        event Windows::Foundation::TypedEventHandler<IProperty^, Object^>^ ValueChanged;
    };
}