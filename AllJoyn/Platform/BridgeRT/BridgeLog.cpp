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
#include "BridgeLog.h"

using namespace Platform;
using namespace Windows::Foundation::Diagnostics;
using namespace Windows::Storage;

//*********************************************************************************************************************
//
// Bridge Log singleton instance getter
//
// Returns instance of Bridge Logger Singleton.
//
//*********************************************************************************************************************
BridgeLog* BridgeLog::Instance(void)
{
    static BridgeLog* s_bridgeLogInstance = nullptr;

    if (s_bridgeLogInstance == nullptr)
    {
        s_bridgeLogInstance = new (std::nothrow) BridgeLog();
    }

    return s_bridgeLogInstance;
}

//*********************************************************************************************************************
//
// Bridge Log Constructor
//
// Creates an in Memory Bridge Log Session
//
//*********************************************************************************************************************
BridgeLog::BridgeLog(void) :
    m_shutdown(FALSE)
{
    // Create the Bridge Logging Channel
    StringReference DEFAULT_CHANNEL_NAME(L"BridgeChannel");
    m_channel = ref new LoggingChannel(DEFAULT_CHANNEL_NAME, nullptr);

    // Create the Bridge Logging Session
    StringReference DEFAULT_SESSION_NAME(L"BridgeSession");
    m_session = ref new LoggingSession(DEFAULT_SESSION_NAME);

    // Associate the Bridge Logging Channel with the Session
    m_session->AddLoggingChannel(m_channel);
}


//*********************************************************************************************************************
//
// Flushes the trace logging session from memory to disk and closes the session.  Further logging is prevented.
//
//*********************************************************************************************************************
void BridgeLog::Shutdown()
{
    if (InterlockedCompareExchange(&m_shutdown, TRUE, FALSE)==FALSE)
    {
        try
        {
            StorageFolder^ localStateFolder = ApplicationData::Current->LocalFolder;
            m_session->SaveToFileAsync(localStateFolder, L"BridgeLog.etl");
        }
        catch (...)
        {
        }
    }
}


//*********************************************************************************************************************
//
// Logs an Exception with contextual message to the Bridge Log and flushes in-memory log to disk.
//
// LogMsg   A descriptive contextual message associated with the exception (Note that the exception's internal message
//          will also be logged)
// ex       The Exception to Log
//
//*********************************************************************************************************************
void BridgeLog::LogError(_In_ String^ LogMsg, _In_ Exception^ ex)
{
    if (m_shutdown==FALSE)
    {
        try
        {
            LoggingFields^ fields = ref new LoggingFields();
            fields->AddString(L"Message", LogMsg);
            fields->AddInt32(L"HResult", ex->HResult);
            fields->AddString(L"ExMessage", ex->Message);
            m_channel->LogEvent(L"Exception", fields, LoggingLevel::Error);
            Shutdown();
        }
        catch (...)
        {
        }
    }
}

//*********************************************************************************************************************
//
// Logs an HRESULT with Message to the Bridge Log and flushes in-memory log to disk.
//
// LogMsg   A descriptive contextual message associated with the error
// hr       The HRESULT error code to log
//
//*********************************************************************************************************************
void BridgeLog::LogError(_In_ Platform::String^ LogMsg, int hr)
{
    if (m_shutdown==FALSE)
    {
        try
        {
            LoggingFields^ fields = ref new LoggingFields();
            fields->AddString(L"Message", LogMsg);
            fields->AddInt32(L"HResult", hr);
            m_channel->LogEvent(L"Error", fields, LoggingLevel::Error);
            Shutdown();
        }
        catch (...)
        {
        }
    }
}

//*********************************************************************************************************************
//
// Logs an informational message to the in memory log file.  The log is not flushed to disk.
//
// LogMsg   A descriptive contextual message associated with the exception
//
//*********************************************************************************************************************
void BridgeLog::LogInfo(_In_ Platform::String^ LogMsg)
{
    if (m_shutdown==FALSE)
    {
        try
        {
            LoggingFields^ fields = ref new LoggingFields();
            m_channel->LogMessage(LogMsg);
        }
        catch (...)
        {
        }
    }
}


//*********************************************************************************************************************
//
// Logs an "Enter <functionName>" message
//
// functionName     name of the Bridge Function that was entered
//
//*********************************************************************************************************************
void BridgeLog::LogEnter(_In_z_ wchar_t* funcName)
{
    if (m_shutdown==FALSE)
    {
        try
        {
            String^ enterString;
            DsbCommon::FormatString(enterString, L"Enter %s", funcName);
            LogInfo(enterString);
        }
        catch (...)
        {
        }
    }
}

//*********************************************************************************************************************
//
// Logs a "Leave <functionName>" message
//
// functionName     name of the Bridge Function that was exited
// hr               hresult to report on exit from a function.  If Non-Zero (not S_OK) this will log an Error and
//                  stop the log
//
//
//*********************************************************************************************************************
void BridgeLog::LogLeave(_In_z_ wchar_t* funcName, int hr)
{
    if (m_shutdown==FALSE)
    {
        try
        {
            String^ leaveString;
            DsbCommon::FormatString(leaveString, L"Leave %s.  Result=0x%X", funcName, hr);
            if (hr == S_OK)
            {
                LogInfo(leaveString);
            }
            else
            {
                LogError(leaveString, hr);
            }
        }
        catch (...)
        {
        }
    }
}