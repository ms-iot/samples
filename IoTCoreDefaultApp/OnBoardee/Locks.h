/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

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