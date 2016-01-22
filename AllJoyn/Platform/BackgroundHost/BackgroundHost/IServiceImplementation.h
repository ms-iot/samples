//
// Copyright (c) 2016, Microsoft Corporation
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

namespace BackgroundHost
{
    public interface class IServiceImplementation
    {
        //**********************************************************************************************
        //
        // Implement this method to create, initialize and run an adapter
        //
        //**********************************************************************************************
        void Start();

        //**********************************************************************************************
        //
        // Implement this method to stop and cleanly shutdown a previously started adapter
        //
        //**********************************************************************************************
        void Stop();
    
        //**********************************************************************************************
        //
        // Invoke this method from an adapter when an exception occurs on start.  Any awaiting handlers
        // will be notified
        //
        //**********************************************************************************************
        void RaiseFailedEvent(Platform::String^ desc);
        
        //**********************************************************************************************
        //
        // This event may be registered for if the hosting task wants to be notified of errors on
        // Start.
        //
        //**********************************************************************************************
        event Windows::Foundation::TypedEventHandler<IServiceImplementation^, Platform::String^>^ Failed;
    
    };
}
