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

#include "pch.h"
#include "BACnetAdapterIoRequest.h"

using namespace Platform;

using namespace BridgeRT;
using namespace DsbCommon;

//
// IO request.
// Description:
//  IO request information.
//
BACnetAdapterIoRequest::BACnetAdapterIoRequest(BACnetAdapterIoRequestPool* PoolPtr, Object^ Parent)
            : parent(Parent)
            , refCount(1)
            , reqStatus(ERROR_SUCCESS)
            , reqActualBytes(0)
            , state(StateFree)
            , allocatorPtr(PoolPtr)
            , cancelRoutinePtr(nullptr)
{
    this->completionEvent = ::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);

    DSB_ASSERT(this->completionEvent != NULL);

    this->reInitialize(Parent);
}


BACnetAdapterIoRequest::~BACnetAdapterIoRequest()
{
    if (this->completionEvent != NULL)
    {
        CloseHandle(this->completionEvent);
    }
}


void
BACnetAdapterIoRequest::reInitialize(Platform::Object^ Parent)
{
    this->parent = Parent;
    this->refCount = 1;
    this->reqStatus = ERROR_SUCCESS;
    this->reqActualBytes = 0;
    this->bytesReturnedPtr = nullptr;
    this->state = StateFree;
    this->cancelRoutinePtr = nullptr;
    this->completionRoutinePtr = nullptr;
    this->completionRoutineContextPtr = nullptr;
    ::ResetEvent(this->completionEvent);

    this->parameters.Type = ULONG(-1);
    this->parameters.InputBufferPtr = nullptr;
    this->parameters.InputBufferSize = 0;
    this->parameters.OutputBufferPtr = nullptr;
    this->parameters.OutputBufferSize = 0;
}


void
BACnetAdapterIoRequest::setState(BACnetAdapterIoRequest::STATE NewState)
{
    AutoLock sync(&this->lock, true);

    if (NewState == StatePending)
    {
        this->reqStatus = ERROR_IO_PENDING;
    }

    this->state = NewState;
}


BACnetAdapterIoRequest::STATE
BACnetAdapterIoRequest::getState()
{
    AutoLock sync(&this->lock, true);

    return this->state;
}


void
BACnetAdapterIoRequest::Reference()
{
    ::InterlockedIncrement(&this->refCount);
}


bool
BACnetAdapterIoRequest::Dereference()
{
    DSB_ASSERT(this->refCount > 0);

    bool isToBeFree = ::InterlockedDecrement(&this->refCount) == 0;

    if (isToBeFree)
    {
        this->allocatorPtr->Free(this);
    }

    return isToBeFree;
}


_Use_decl_annotations_
void
BACnetAdapterIoRequest::Initialialize(
    const BACnetAdapterIoRequest::IO_PARAMETERS* RequestPrametersPtr,
    PDWORD BytesReturnedPtr
    )
{
    this->bytesReturnedPtr = BytesReturnedPtr;

    this->parameters = *RequestPrametersPtr;

    this->setState(StateActive);
}


_Use_decl_annotations_
void
BACnetAdapterIoRequest::GetIoParameters(BACnetAdapterIoRequest::IO_PARAMETERS* RequestParamatersPtr)
{
    AutoLock sync(&this->lock, true);

    *RequestParamatersPtr = this->parameters;
}


ULONG
BACnetAdapterIoRequest::GetType()
{
    AutoLock sync(&this->lock, true);

    return this->parameters.Type;
}


void
BACnetAdapterIoRequest::SetCancelRoutine(CANCEL_REQUEST_HANDLER CancelRoutinePtr)
{
    AutoLock sync(&this->lock, true);

    this->cancelRoutinePtr = CancelRoutinePtr;
}


void
BACnetAdapterIoRequest::SetCompletionRoutine(COMPLETE_REQUEST_HANDLER CompletionRoutinePtr, PVOID ContextPtr)
{
    AutoLock sync(&this->lock, true);

    this->completionRoutinePtr = CompletionRoutinePtr;
    this->completionRoutineContextPtr = ContextPtr;
}


void
BACnetAdapterIoRequest::Complete(DWORD Status, DWORD ActualBytes)
{
    AutoLock sync(&this->lock, true);

    this->SetCancelRoutine(nullptr);

    this->reqStatus = Status;
    this->reqActualBytes = ActualBytes;

    if (this->bytesReturnedPtr != nullptr)
    {
        *this->bytesReturnedPtr = ActualBytes;
    }

    this->setState(StateCompleted);

    if (this->completionRoutinePtr != nullptr)
    {
        this->completionRoutinePtr(
            this,
            this->completionRoutineContextPtr,
            this->reqStatus
            );

        this->completionRoutinePtr = nullptr;
    }

    if (this->completionEvent != NULL)
    {
        ::SetEvent(this->completionEvent);
    }

    this->Dereference();
}


void
BACnetAdapterIoRequest::MarkPending()
{
    this->setState(StatePending);
}


_Use_decl_annotations_
uint32
BACnetAdapterIoRequest::Wait(DWORD TimeoutMsec, HANDLE AbortEvent)
{
    AutoLock sync(&this->lock, true);

    if (this->getState() != StatePending)
    {
        return this->reqStatus;
    }

    sync.Unlock();

    if (this->completionEvent == NULL)
    {
        DSB_ASSERT(FALSE);

        return ERROR_INVALID_HANDLE;
    }

    HANDLE events[2] = { this->completionEvent, AbortEvent };
    DWORD eventCount = AbortEvent == NULL ? 1 : 2;

    DWORD waitRes = ::WaitForMultipleObjectsEx(eventCount, events, FALSE, TimeoutMsec, FALSE);

    // Translate to ERROR_xxx status codes
    DWORD status = ERROR_SUCCESS;
    switch (waitRes)
    {
    case WAIT_OBJECT_0:
        status = ERROR_SUCCESS;
        break;

    case WAIT_OBJECT_0 + 1:
        status = ERROR_REQUEST_ABORTED;
        break;

    case WAIT_TIMEOUT:
        status = ERROR_TIMEOUT;

    case WAIT_FAILED:
    default:
        status = ERROR_GEN_FAILURE;
        break;
    }

    return status;
}


uint32
BACnetAdapterIoRequest::Cancel()
{
    AutoLock sync(&this->lock, true);

    if ((this->state != StatePending) &&
        (this->state != StateActive))
    {
        DSB_ASSERT(FALSE);

        return ERROR_SUCCESS;
    }

    this->setState(StateCancelled);

    bool isCanceled = false;

    if (this->cancelRoutinePtr != nullptr)
    {
        isCanceled = this->cancelRoutinePtr(this);
    }

    return isCanceled ? ERROR_SUCCESS : ERROR_NOT_CAPABLE;
}


_Use_decl_annotations_
DWORD
BACnetAdapterIoRequest::GetStatus(PDWORD ActualBytesPtr)
{
    AutoLock sync(&this->lock, true);

    if (ActualBytesPtr != nullptr)
    {
        *ActualBytesPtr = this->reqActualBytes;
    }

    return this->reqStatus;
}


HANDLE
BACnetAdapterIoRequest::GetCompletionEvent()
{
    AutoLock sync(&this->lock, true);

    return this->completionEvent;
}


//
// BACnetAdapterIoRequestPool class
// Description:
//  DSB request pool.
//
BACnetAdapterIoRequestPool::BACnetAdapterIoRequestPool()
{
}


BACnetAdapterIoRequestPool::~BACnetAdapterIoRequestPool()
{
}


void
BACnetAdapterIoRequestPool::Free(BACnetAdapterIoRequest^ IoRequest)
{
    AutoLock sync(&this->lock, true);

    if (IoRequest != nullptr)
    {
        IoRequest->setState(BACnetAdapterIoRequest::StateFree);

        this->push(IoRequest);
    }
    else
    {
        DSB_ASSERT(FALSE);
    }
}


BACnetAdapterIoRequest^
BACnetAdapterIoRequestPool::pop()
{
    AutoLock sync(&this->lock, true);

    if (this->freeRequestList.size() == 0)
    {
        return nullptr;
    }

    BACnetAdapterIoRequest^ request = this->freeRequestList.front();

    this->freeRequestList.erase(this->freeRequestList.begin());

    return request;
}


void
BACnetAdapterIoRequestPool::push(BACnetAdapterIoRequest^ IoRequest)
{
    AutoLock sync(&this->lock, true);

    if (IoRequest != nullptr)
    {
        this->freeRequestList.push_back(std::move(IoRequest));
    }
    else
    {
        DSB_ASSERT(FALSE);
    }
}