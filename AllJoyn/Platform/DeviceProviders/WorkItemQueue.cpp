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

#include "pch.h"
#include "WorkItemQueue.h"

WorkItemQueue::WorkItemQueue()
    : m_threadId(0)
{
    // Both must be created as manual reset events
    m_availableWorkEvent = ::CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, SYNCHRONIZE | EVENT_MODIFY_STATE);
    m_quitEvent = ::CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, SYNCHRONIZE | EVENT_MODIFY_STATE);
    m_exitEvent = ::CreateEventEx(NULL, NULL, 0, SYNCHRONIZE | EVENT_MODIFY_STATE);
}

WorkItemQueue::~WorkItemQueue()
{
    if (NULL != m_availableWorkEvent)
    {
        ::CloseHandle(m_availableWorkEvent);
        m_availableWorkEvent = NULL;
    }

    if (NULL != m_quitEvent)
    {
        ::CloseHandle(m_quitEvent);
        m_quitEvent = NULL;
    }

    if (NULL != m_exitEvent)
    {
        ::CloseHandle(m_exitEvent);
        m_exitEvent = NULL;
    }

    m_threadId = 0;
}

void WorkItemQueue::Start()
{
    m_threadId = ::GetCurrentThreadId();
    ::ResetEvent(m_availableWorkEvent);
    ::ResetEvent(m_quitEvent);
    ::ResetEvent(m_exitEvent);

    HANDLE eventHandles[] = { m_availableWorkEvent, m_quitEvent };

    for (;;)
    {
        // Wait for either new queued workitems, or a queued quit signal
        DWORD waitResult = ::WaitForMultipleObjectsEx(_countof(eventHandles), eventHandles, false, INFINITE, true);

        if (waitResult != WAIT_OBJECT_0)
        {
            break;
        }

        work_item_function workItem;
        {
            AutoLock lockScope(&m_queueLock, true);
            workItem = m_queue.front();
            m_queue.pop();

            if (m_queue.empty())
            {
                ::ResetEvent(m_availableWorkEvent);
            }
        }
        workItem();
    }

    // Empty any workitems remaining in the queue
    {
        AutoLock lockScope(&m_queueLock, true);
        while (!m_queue.empty())
        {
            m_queue.pop();
        }
    }

    ::SetEvent(m_exitEvent);
}

void WorkItemQueue::PostWorkItem(work_item_function workItem)
{
    // Ignore further calls after quit request
    if (WAIT_OBJECT_0 == WaitForSingleObjectEx(m_quitEvent, 0, false))
        return;

    AutoLock lockScope(&m_queueLock, true);
    m_queue.push(workItem);
    ::SetEvent(m_availableWorkEvent);
}

void WorkItemQueue::PostQuit()
{
    // Ignore further calls after quit request
    if (WAIT_OBJECT_0 == WaitForSingleObjectEx(m_quitEvent, 0, false))
        return;

    // Signal the queue processing thread to quit
    ::SetEvent(m_quitEvent);
}

void WorkItemQueue::WaitForQuit()
{
    if ((0 != m_threadId) && (::GetCurrentThreadId() != m_threadId))
    {
        ::WaitForSingleObjectEx(m_exitEvent, INFINITE, false);
        m_threadId = 0;
    }
}
