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
    ref class AllJoynStatus;
    interface class IService;

    public ref struct ServiceDroppedEventArgs sealed
    {
        ServiceDroppedEventArgs(IService^ service, Windows::Devices::AllJoyn::AllJoynSessionLostReason reason)
        {
            Service = service;
            Reason = reason;
        }
        property IService^ Service;
        property Windows::Devices::AllJoyn::AllJoynSessionLostReason Reason;
    };

    public ref struct ServiceJoinedEventArgs sealed
    {
        ServiceJoinedEventArgs(IService^ service)
        {
            Service = service;
        }
        property IService^ Service;
    };

    public interface class IProvider
    {
        AllJoynStatus^ Start();
        void Shutdown();
        Windows::Foundation::Collections::IVector<IService^>^ GetServicesWhichImplementInterface(Platform::String^ interfaceName);

        property Windows::Foundation::Collections::IVector<IService ^>^ Services
        {
            Windows::Foundation::Collections::IVector<IService ^>^ get();
        }

        event Windows::Foundation::TypedEventHandler<IProvider^, ServiceDroppedEventArgs^>^ ServiceDropped;
        event Windows::Foundation::TypedEventHandler<IProvider^, ServiceJoinedEventArgs^>^ ServiceJoined;
    };
}