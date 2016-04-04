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

#include <collection.h>
#include <algorithm>

namespace BridgeRT
{
    //
    // Adapter objects vector/vector interfaces
    //

    interface class IAdapterValue;
    typedef Platform::Collections::Vector<IAdapterValue^> AdapterValueVector;
    typedef Windows::Foundation::Collections::IVector<IAdapterValue^> IAdapterValueVector;
    typedef Windows::Foundation::PropertyType AdapterValueType;

    interface class IAdapterAttribute;
    typedef Platform::Collections::Vector<IAdapterAttribute^> AdapterAttributeVector;
    typedef Windows::Foundation::Collections::IVector<IAdapterAttribute^> IAdapterAttributeVector;

    interface class IAdapterProperty;
    typedef Platform::Collections::Vector<IAdapterProperty^> AdapterPropertyVector;
    typedef Windows::Foundation::Collections::IVector<IAdapterProperty^> IAdapterPropertyVector;

    interface class IAdapterMethod;
    typedef Platform::Collections::Vector<IAdapterMethod^> AdapterMethodVector;
    typedef Windows::Foundation::Collections::IVector<IAdapterMethod^> IAdapterMethodVector;

    interface class IAdapterSignal;
    typedef Platform::Collections::Vector<IAdapterSignal^> AdapterSignalVector;
    typedef Windows::Foundation::Collections::IVector<IAdapterSignal^> IAdapterSignalVector;

    interface class IAdapterDevice;
    typedef Platform::Collections::Vector<IAdapterDevice^> AdapterDeviceVector;
    typedef Windows::Foundation::Collections::IVector<IAdapterDevice^> IAdapterDeviceVector;

    typedef Platform::Collections::Map<Platform::String^, Platform::String^> AnnotationMap;
    typedef Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^> IAnnotationMap;
}