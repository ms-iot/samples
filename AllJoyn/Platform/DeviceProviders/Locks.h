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

#include <Windows.h>

//
//  Class LockObject
//  Description:
//      A generic pure virtual lock object
//
//
class LockObject
{
public:
    LockObject() {}
    virtual ~LockObject() {}

    virtual void Lock(void) = 0;
    virtual bool TryLock(void) = 0;
    virtual void Unlock(void) = 0;
};


//
//  Class CSLock
//  Description:
//      Multi thread safe lock (CRITICAL_SECTION based)
//
class CSLock : public LockObject
{
public:
    CSLock()
    {
        ::InitializeCriticalSectionEx(&this->critSec, 0, 0);
    }
    virtual ~CSLock()
    {
        ::DeleteCriticalSection(&this->critSec);
    }

    _Acquires_lock_(this->critSec)
        void Lock(void)
    {
        ::EnterCriticalSection(&this->critSec);
    }

    _Function_ignore_lock_checking_(this->critSec)
        bool TryLock(void)
    {
        return ::TryEnterCriticalSection(&this->critSec) != FALSE;
    }

    _Releases_lock_(this->critSec)
        void Unlock(void)
    {
        ::LeaveCriticalSection(&this->critSec);
    }

private:
    CRITICAL_SECTION critSec;
};


//
//  Class AutoLock
//  Description:
//      Automatic lock, which unlocks when out of scope.
//
class AutoLock
{
public:
    AutoLock(
        _In_ LockObject* LockObjectPtr,
        _In_ bool IsInitiallyLocked
        )
        : lockObjectPtr(LockObjectPtr)
            , lockCount(0)
    {
        if (IsInitiallyLocked)
        {
            this->Lock();
        }
    }

    ~AutoLock()
    {
        do
        {
            AutoLock::Unlock();
        } while (::InterlockedAdd(&this->lockCount, 0) > 0);
    }

    _Function_ignore_lock_checking_(*this->lockObjectPtr)
        void Lock()
    {
        ::InterlockedAdd(&this->lockCount, 1);
        lockObjectPtr->Lock();
    }

    _Function_ignore_lock_checking_(*this->lockObjectPtr)
        void Unlock()
    {
        if (::InterlockedAdd(&this->lockCount, -1) >= 0)
        {
            lockObjectPtr->Unlock();
        }
    }

private:
    LockObject* lockObjectPtr;
    volatile LONG lockCount;
};


//template <typename T> T* Singleton<T>::instancePtr = nullptr;
//template <typename T> CSLock Singleton<T>::lock;

