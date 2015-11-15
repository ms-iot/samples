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

#include <map>

#include "AdapterDefinitions.h"
#include "Misc.h"


//
// adapter IO request.
// Description:
//  adapter IO request information.
//
class BACnetAdapterIoRequestPool;
ref class BACnetAdapterIoRequest : public BridgeRT::IAdapterIoRequest
{
    friend class BACnetAdapterIoRequestPool;

public:

    //
    // BridgeRT::IAdapterIoRequest overrides
    //
    virtual uint32 Status()
    {
        return this->GetStatus(nullptr);
    }

    virtual uint32 Wait(uint32 TimeoutMsec)
    {
        return this->Wait(TimeoutMsec, NULL);
    }

    virtual uint32 Cancel();

    virtual uint32 Release()
    {
        this->Dereference();

        return ERROR_SUCCESS;
    }

    virtual ~BACnetAdapterIoRequest();

internal:
    //
    // request IO parameters
    //
    struct IO_PARAMETERS
    {
        // Type
        ULONG   Type;

        // Input buffer information
        _Field_size_(InputBufferSize) const void*  InputBufferPtr;
        DWORD   InputBufferSize;

        // Output buffer information
        _Field_size_(OutputBufferSize) void*  OutputBufferPtr;
        DWORD   OutputBufferSize;

        IO_PARAMETERS()
            : Type(ULONG(-1))
            , InputBufferPtr(nullptr)
            , InputBufferSize(0)
            , OutputBufferPtr(nullptr)
            , OutputBufferSize(0)
        {
        }
    };

    //
    // IO Request cancel routine
    //
    typedef
    _Function_class_(CANCEL_REQUEST)
    bool
    (CANCEL_REQUEST)(
        _In_ BridgeRT::IAdapterIoRequest^ Request
        );
    typedef CANCEL_REQUEST (*CANCEL_REQUEST_HANDLER);

    //
    // IO Request completion routine
    //
    typedef
    _Function_class_(COMPLETE_REQUEST)
    void
    (COMPLETE_REQUEST)(
        _In_ BridgeRT::IAdapterIoRequest^ Request,
        _In_ PVOID ContextPtr,
        _In_ DWORD Status
        );
    typedef COMPLETE_REQUEST (*COMPLETE_REQUEST_HANDLER);

    void    Initialialize(
                _In_ const IO_PARAMETERS* RequestPrametersPtr,
                _In_opt_ PDWORD BytesReturnedPtr
                );

    Platform::Object^ GetParent() { return this->parent; }

    bool    Dereference();
    void    Reference();

    void    Complete(DWORD Status, DWORD ActualBytes);
    uint32  Wait(_In_ DWORD TimeoutMsec, _In_opt_ HANDLE AbortEvent);
    void    MarkPending();

    DWORD   GetStatus(_Out_opt_ PDWORD ActualBytesPtr);
    HANDLE  GetCompletionEvent();
    void    GetIoParameters(_Inout_ IO_PARAMETERS* RequestParamatersPtr);
    ULONG   GetType();

    void    SetCancelRoutine(CANCEL_REQUEST_HANDLER CancelRoutinePtr);
    void    SetCompletionRoutine(COMPLETE_REQUEST_HANDLER CompletionRoutinePtr, PVOID ContextPtr);

protected private:

    //
    // The DSB IO request state
    //
    enum STATE
    {
        StateFree,
        StateActive,
        StatePending,
        StateCompleted,
        StateCancelled,
    };

    BACnetAdapterIoRequest(BACnetAdapterIoRequestPool* PoolPtr, Platform::Object^ Parent);

    void reInitialize(Platform::Object^ Parent);

    void setState(STATE NewState);
    STATE getState();

    // Request parameter
    IO_PARAMETERS parameters;

    // Address of the bytes returned variable
    PDWORD bytesReturnedPtr;

    // The request state
    STATE state;

private:

    // Sync object
    DsbCommon::CSLock lock;

    // Parent object
    Platform::Object^ parent;

    // Reference count
    volatile LONG refCount;

    // Reference to the container pool
    BACnetAdapterIoRequestPool* allocatorPtr;

    // Status information
    uint32 reqActualBytes;
    uint32 reqStatus;

    // The completion event
    HANDLE completionEvent;

    // Cancel routine
    CANCEL_REQUEST_HANDLER cancelRoutinePtr;

    // Completion routine
    COMPLETE_REQUEST_HANDLER completionRoutinePtr;
    PVOID completionRoutineContextPtr;
};


//
// BACnetAdapterIoRequestPool class
// Description:
//  DSB request pool.
//
class BACnetAdapterIoRequestPool
{
public:
    BACnetAdapterIoRequestPool();
    ~BACnetAdapterIoRequestPool();

    template <typename T>
    T^ Alloc(Platform::Object^ Parent);

    void Free(BACnetAdapterIoRequest^ Request);

private:
    void push(BACnetAdapterIoRequest^ Request);
    BACnetAdapterIoRequest^ pop();

private:

    // Sync object
    DsbCommon::CSLock   lock;

    // The free requests list
    std::vector<BACnetAdapterIoRequest^> freeRequestList;
};


template <typename T>
T^
BACnetAdapterIoRequestPool::Alloc(Platform::Object^ Parent)
{
    DsbCommon::AutoLock sync(&this->lock, true);

    T^ request = dynamic_cast<T^>(this->pop());

    try
    {
        if (request == nullptr)
        {
            request = ref new T(this, Parent);
        }
        else
        {
            request->reInitialize(Parent);
        }
        request->setState(BACnetAdapterIoRequest::StateActive);
    }
    catch (OutOfMemoryException^)
    {
        DSB_ASSERT(FALSE);
    }

    return request;
}
