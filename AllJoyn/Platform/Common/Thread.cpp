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

#include "pch.h"
#include "Thread.h"

namespace DsbCommon
{
    //
    // Class Thread
    // Description:
    //  Thread interface class.
    //  Thread is a pure virtual class that needs to be inherited
    //  for thread code implementation.
    //
    Thread::Thread()
        : isRunning(FALSE)
        , threadId(0)
        , threadHandle(NULL)
        , startStatus(ERROR_NOT_READY)
        , startedEvent(NULL)
        , stopEvent(NULL)

    {
    }

    Thread::~Thread()
    {
        (void)Thread::Stop();

        if (this->stopEvent != NULL)
        {
            ::CloseHandle(this->startedEvent);
        }

        if (this->stopEvent != NULL)
        {
            ::CloseHandle(this->stopEvent);
        }
    }

    DWORD
    __stdcall
    Thread::ThreadLaunch(LPVOID ContextPtr)
    {
        Thread* threadPtr = static_cast<Thread*>(ContextPtr);

        // Call thread object entry
        DWORD extitConde = threadPtr->ThreadStart();

        ::InterlockedExchange(&threadPtr->isRunning, FALSE);

        threadPtr->threadHandle = NULL;
        threadPtr->threadId = 0;

        return extitConde;
    }

    DWORD
    Thread::Start(DWORD WaitStartedMsec)
    {
        if (this->startedEvent == NULL)
        {
            this->startedEvent = ::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);

            if (this->startedEvent == NULL)
            {
                return::GetLastError();
            }
        }

        if (this->stopEvent == NULL)
        {
            this->stopEvent = ::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);

            if (this->stopEvent == NULL)
            {
                return::GetLastError();
            }
        }

        if (::InterlockedExchange(&this->isRunning, TRUE))
        {
            return ERROR_ALREADY_INITIALIZED;
        }

        this->threadHandle = ::CreateThread(
                                nullptr,
                                0,
                                &ThreadLaunch,
                                this,
                                0,
                                &this->threadId
                                );

        if (this->threadHandle == NULL)
        {
            ::InterlockedExchange(&this->isRunning, FALSE);

            return ::GetLastError();
        }

        if (WaitStartedMsec == 0)
        {
            return startStatus;
        }

        // Wait for thread to start, or for a stop request...
        HANDLE events[2] = {this->stopEvent, this->startedEvent};

        DWORD waitRes = ::WaitForMultipleObjectsEx(ARRAYSIZE(events), events, FALSE, WaitStartedMsec, FALSE);

        if (waitRes == (WAIT_OBJECT_0+1))
        {
            return this->startStatus;
        }

        return ::GetLastError();
    }

    DWORD
    Thread::Stop(void)
    {
        if (!::InterlockedExchange(&this->isRunning, FALSE))
        {
            if (this->threadHandle == NULL)
            {
                return ERROR_SUCCESS;
            }

            return ERROR_IO_PENDING;
        }

        ::SetEvent(this->stopEvent);

        //
        // If we are stopping the thread from within,
        // we cannot wait here...
        //
        if (this->IsMe())
        {
            return ERROR_IO_PENDING;
        }

        (void)::WaitForSingleObjectEx(this->threadHandle, INFINITE, FALSE);

        this->threadHandle = NULL;
        this->threadId = 0;

        return ERROR_SUCCESS;
    }

    void
    Thread::SetStartStatus(DWORD StartStatus)
    {
        this->startStatus = StartStatus;

        SetEvent(this->startedEvent);
    }

    DWORD
    Thread::GetStartStatus(void)
    {
        return this->startStatus;
    }

    bool
    Thread::IsRunning(void)
    {
        return ::InterlockedExchangeAdd(&this->isRunning, 0) != FALSE;
    }

    bool
    Thread::IsMe(void)
    {
        return ::GetCurrentThreadId() == this->threadId;
    }

    HANDLE
    Thread::GetStopEvent(void)
    {
        return this->stopEvent;
    }

} // DsbCommon
