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
#include <mutex>
#include <vector>

namespace AdapterLib
{
  
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
        _Out_ Platform::Array<BYTE>^* TargetDataArrayPtr);

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
            AutoLock sync(this->lock);
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
            AutoLock sync(this->lock);

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
            AutoLock sync(this->lock);

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
            AutoLock sync(this->lock);

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
        std::recursive_mutex lock;

        HANDLE  notEmptyEvent;

        std::vector<T> items;
    };
} //AdapterLib
