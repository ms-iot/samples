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

#include <string>
#include <vector>

namespace DsbCommon
{
    // Assert
    #if _DEBUG || DBG

        #define DSB_ASSERT(_condition_) if (!(_condition_)) \
        { \
            __debugbreak(); \
        }

    #else   // !_DEBUG && !DBG

        #define DSB_ASSERT(_condition_)

    #endif  // _DEBUG

    // Win32 error codes from HRESULT codes.
    #ifndef WIN32_FROM_HRESULT
        __forceinline DWORD WIN32_FROM_HRESULT(HRESULT HResult) { return HRESULT_FACILITY(HResult) == FACILITY_WIN32 ? HRESULT_CODE(HResult) : HResult; }
    #endif

    __forceinline HRESULT HRESULT_FROM_QSTATUS(int x) { return MAKE_HRESULT(1,FACILITY_ITF,x); }


    // Check status-leave helper macro
    #define CHK_HR(stmt) do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

    //
    //  Class LockObject
    //  Description:
    //      A generic pure virtual lock object
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
            InitializeCriticalSectionEx(&this->critSec, 0, 0);
        }
        virtual ~CSLock()
        {
            DeleteCriticalSection(&this->critSec);
        }

        _Acquires_lock_(this->critSec)
            void Lock(void)
        {
            EnterCriticalSection(&this->critSec);
        }

        _Function_ignore_lock_checking_(this->critSec)
            bool TryLock(void)
        {
            return TryEnterCriticalSection(&this->critSec) != FALSE;
        }

        _Releases_lock_(this->critSec)
            void Unlock(void)
        {
            LeaveCriticalSection(&this->critSec);
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

    //
    // class Singleton
    //  Description:
    //      Create a singleton of a given type
    //
    template <class T>
    class Singleton
    {
    public:
        static T* Instance()
        {
            AutoLock sync(&Singleton<T>::lock, true);

            if (Singleton<T>::instancePtr == nullptr)
            {
                Singleton<T>::instancePtr = new (std::nothrow) T;
            }

            return Singleton<T>::instancePtr;
        }

        static void Kill()
        {
            AutoLock sync(&Singleton<T>::lock, true);

            if (Singleton<T>::instancePtr != nullptr)
            {
                delete Singleton<T>::instancePtr;

                Singleton<T>::instancePtr = nullptr;
            }
        }

    protected:
        Singleton() {};
        ~Singleton() {};

    private:
        static T* instancePtr;
        static CSLock lock;
    };

    template <class T> T* Singleton<T>::instancePtr = nullptr;
    template <class T> CSLock Singleton<T>::lock;

    //
    // class QueueEx
    // Description:
    //  QueueEx is a generic queue, a user can wait on.
    //
    template <class T>
    class QueueEx
    {
    public:
        QueueEx()
        {
            this->notEmptyEvent = ::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
        }
        ~QueueEx()
        {
            if (this->notEmptyEvent != nullptr)
            {
                ::CloseHandle(this->notEmptyEvent);
            }
        }

        bool IsValid() const
        {
            return this->notEmptyEvent != NULL;
        }

        HANDLE GetNotEmptyEvent()
        {
            return this->notEmptyEvent;
        }

        DWORD Add(_In_ const T& NewItem)
        {
            AutoLock sync(&this->lock, true);
            DWORD status = ERROR_SUCCESS;

            try
            {
                this->items.push_back(NewItem);

                ::SetEvent(this->notEmptyEvent);
            }
            catch (std::bad_alloc& baException)
            {
                UNREFERENCED_PARAMETER(baException);

                status = ERROR_NOT_ENOUGH_MEMORY;
            }

            return status;
        }

        bool Remove(_In_ const T& ItemToRemove, _Out_opt_ T* RemovedItemdPtr)
        {
            AutoLock sync(&this->lock, true);

            if (RemovedItemdPtr != nullptr)
            {
                *RemovedItemdPtr = T();
            }

            for (std::vector<T>::iterator it = this->items.begin();
            it != this->items.end();
                it++)
            {
                const T& curItem = *it;

                if (curItem == ItemToRemove)
                {
                    if (RemovedItemdPtr != nullptr)
                    {
                        *RemovedItemdPtr = curItem;
                    }

                    this->items.erase(it);

                    if (this->items.size() == 0)
                    {
                        ::ResetEvent(this->notEmptyEvent);
                    }

                    return true;
                }
            }

            return false;
        }

        size_t Count()
        {
            AutoLock sync(&this->lock, true);

            return this->items.size();
        }

        DWORD Wait(_In_opt_ HANDLE AbortEvent, _In_ DWORD TimeoutMsec)
        {
            DWORD status = ERROR_SUCCESS;
            HANDLE events[2] = { this->notEmptyEvent, AbortEvent };
            DWORD eventCount = AbortEvent == NULL ? 1 : 2;

            DWORD res = ::WaitForMultipleObjectsEx(eventCount, events, FALSE, TimeoutMsec, FALSE);

            switch (res)
            {
            case WAIT_OBJECT_0:
                status = ERROR_SUCCESS;
                break;

            case WAIT_OBJECT_0 + 1:
                status = ERROR_CANCELLED;
                break;

            case WAIT_TIMEOUT:
                status = ERROR_TIMEOUT;
                break;

            default:
                break;
            }

            return status;
        }

        T GetNext()
        {
            AutoLock sync(&this->lock, true);

            if (this->items.size() == 0)
            {
                static T dummyTxItem;

                return dummyTxItem;
            }

            T nextItem = this->items.front();

            this->items.erase(this->items.begin());

            if (this->items.size() == 0)
            {
                ::ResetEvent(this->notEmptyEvent);
            }

            return nextItem;
        }

    private:
        DsbCommon::CSLock lock;

        HANDLE  notEmptyEvent;

        std::vector<T> items;
    };

    //
    // To_Ascii_String()
    // Description:
    //  Converts a wide character string to ASCII.
    //
    std::string To_Ascii_String(const std::wstring& wstr);

    //
    // To_Ascii_String()
    // Description:
    //  Converts a wide character string to ASCII.
    //
    std::string To_Ascii_String(Platform::String^ platformString);

    template<typename _TDest, typename _Tsrc>
    inline _TDest ConvertTo(const _Tsrc& src)
    {
        return _TDest{ src.begin(), src.end() };
    }

    template<typename _TDest>
    inline _TDest ConvertTo(Platform::String^ src)
    {
        return _TDest{ Platform::begin(src), Platform::end(src) };
    }

    //
    // String_To__Wstring()
    // Description:
    //  Converts an ascii string to wide character.
    //
    std::wstring String_To_Wstring(const std::string& str);

    //
    //  Routine Description:
    //      Sets a Platform::String to a formatted (printf like) string.
    //
    //  Arguments:
    //
    //      FormatWsz - String format (printf like)
    //
    //      ... - arguments
    //
    //  Return Value:
    //
    //      The length (in characters) of result string, or -1 if
    //      failed to convert due to resource limitation, or max string size
    //      has been reached.
    //
    #define MAX_STRING_SIZE size_t(1024*1024)
    //
    int FormatString(_Out_ Platform::String^& OutputString, _In_z_ const wchar_t* FormatWsz, ...);

    //
    //  Routine Description:
    //      Conversion from Platform::String to Platform::Array<byte>
    //
    //  Arguments:
    //
    //      SourceString - The source string to be converted
    //
    //      TargetDataArrayPtr - Address of a caller platform::Array<BYTE>^ variable
    //          to receive the array version of the string.
    //
    //  Return Value:
    //
    //      - ERROR_SUCCESS.
    //      - ERROR_INVALID_PARAMETER: SourceString or TargetDataArrayPtr is null.
    //      - ERROR_NOT_ENOUGH_MEMORY: Failed to allocate destination array.
    //
    uint32 StringToArray(
        _In_ Platform::String^ SourceString,
        _Out_ Platform::Array<BYTE>^* TargetDataArrayPtr
        );

    //
    //  Routine Description:
    //      ToLower returns a lower case version of a given string.
    //
    //  Arguments:
    //
    //      SourceStringWsz - The null terminated wide character source string.
    //
    //  Return Value:
    //
    //      A String^ string that contains a lower case version of StringWsz.
    //      An OutOfMemoryException/std::bad_alloc exception is raised when out of resources.
    //      An std::length_error exception is raised when SourceStringWsz is invalid (not null terminated).
    //
    Platform::String^ ToLower(_In_ const wchar_t* SourceStringWsz);

    bool FileExist(_In_ Platform::String^ filePath);
    bool FolderExist(_In_ Platform::String^ folderName);
    HRESULT CreateFolder(_In_ Platform::String^ folderName);
    HRESULT CopyFolder(_In_ Platform::String^ source, _In_ Platform::String^ destination);
    HRESULT DeleteFolder(_In_ Platform::String^ dir);

} // DsbCommon
