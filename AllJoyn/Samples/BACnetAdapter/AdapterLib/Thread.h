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

namespace AdapterLib
{
    // Default thread start time in mSec
    static DWORD DEF_THREAD_START_TIMEOUT_MSEC = DWORD(1000);

    //
    // Class Thread
    // Description:
    //  Thread interface class.
    //  Thread is a pure virtual class that needs to be inherited
    //  for thread code implementation.
    //
    class Thread
    {
    public:
        Thread();
        virtual ~Thread();

        DWORD	Start(DWORD WaitStartedMsec);
        DWORD	Stop(void);
        bool    IsRunning(void);
        bool    IsMe(void);
        HANDLE  GetStopEvent(void);
        DWORD   GetStartStatus(void);
        void    SetStartStatus(DWORD StartStatus);

    protected:
        //
        // ThreadStart() is the thread entry point, which
        // should be implemented within the derived class.
        //
        virtual DWORD ThreadStart() = 0;

    private:
        static DWORD __stdcall ThreadLaunch(LPVOID ContextPtr);

    private:
        HANDLE          startedEvent;
        HANDLE          stopEvent;
        DWORD           startStatus;
        volatile LONG   isRunning;
        DWORD	        threadId;
        HANDLE          threadHandle;
    };

    //
    // Template MemberThread
    // Description:
    //  A template that enables to have a class member running in a different thread
    //
    template <class T>
    class MemberThread : public Thread
    {
    public:
        typedef DWORD (T::* PTHREAD_START)();

        MemberThread()
            : instance(nullptr)
            , threadStartPtr(nullptr)
        {
        }

        MemberThread(_In_ T^ Instance, _In_ PTHREAD_START ThreadStartPtr)
            : instance(Instance)
            , threadStartPtr(ThreadStartPtr)
        {
        }

        virtual ~MemberThread()
        {
        }

        void Init(_In_ T^ Instance, _In_ PTHREAD_START ThreadStartPtr)
        {
            this->instance = Instance;
            this->threadStartPtr = ThreadStartPtr;
        }

        virtual DWORD ThreadStart()
        {
            if (this->instance == nullptr ||
                this->threadStartPtr == nullptr)
            {
                return ERROR_NOT_READY;
            }

            return (this->instance->*threadStartPtr)();
        }

    private:
        T^ instance;
        PTHREAD_START threadStartPtr;
    };

} // AdapterLib
