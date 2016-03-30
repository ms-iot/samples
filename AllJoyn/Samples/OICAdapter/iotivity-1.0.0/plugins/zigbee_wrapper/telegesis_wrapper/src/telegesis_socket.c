//******************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "telegesis_socket.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>

#include "twsocketlist.h"
#include "oic_string.h"
#include "oic_malloc.h"
#include "logger.h"

#define TAG "Telegesis_Socket"

/**
 * New thread's main() ftn.
 */
void * readForever(/*PIPlugin */void * plugin);
/**
 * Just grabs the next char in the buffer. Called by readBufferLine() multiple times.
 */
char readBufferChar(int fd, ssize_t * readDataBytes);
/**
 * Calls readBufferChar() until line is formed.
 */
const char * readBufferLine(int fd);
/**
 * Calls readBufferLine() until a full TWEntry is formed.
 */
TWEntry * readEntry(int fd);
/**
 * Posts the TWEntry to the queue.
 */
TWResultCode TWEnqueueEntry(PIPlugin * plugin, TWEntry * entry);

/**
 * Defines the mapping relationships between Telegesis AT response/prompt tags and
 * how many lines we should expect with each following the tag.
 */
typedef struct
{
    const char *resultTxt;
    uint8_t numLines;
    TWEntryType entryType;
} TWEntryTypePair;

static TWEntryTypePair TWEntryTypePairArray[] =
{
    {"+N=",             1, TW_NETWORK_INFO},
    {"JPAN:",           1, TW_JPAN},
    {"RFD:",            1, TW_RFD},
    {"FFD:",            1, TW_FFD},
    {"SED:",            1, TW_SED},
    {"ZED:",            1, TW_ZED},
    {"MatchDesc:",      1, TW_MATCHDESC},
    {"SimpleDesc:",     6, TW_SIMPLEDESC},
    {"InCluster:",      1, TW_INCLUSTER},
    {"WRITEATTR:",      1, TW_WRITEATTR},
    {"RESPATTR:",       1, TW_RESPATTR},
    {"TEMPERATURE:",    1, TW_RESPATTR},
    {"DFTREP",          1, TW_DFTREP},
    {"DRLOCRSP:",       1, TW_DRLOCRSP},
    {"DRUNLOCKRSP:",    1, TW_DRUNLOCKRSP},
    {"ACK:",            1, TW_ACK},
    {"NACK:",           1, TW_NACK},
    {"SEQ:",            1, TW_SEQ},
    {"ZENROLLREQ:",     1, TW_ZENROLLREQ},
    {"ENROLLED:",       1, TW_ENROLLED},
    {"ZONESTATUS:",     1, TW_ZONESTATUS},
    {"AddrResp:",       1, TW_ADDRESS_RESPONSE},
    {"Unknown:",        0, TW_NONE},
    {"Unknown:",        1, TW_MAX_ENTRY}
};

TWEntry * TWEntryList = NULL;

TWEntryTypePair getEntryTypePair(const char * bufferLine)
{
    size_t bufferLength = strlen(bufferLine);
    for(uint8_t i = 0; i < TW_MAX_ENTRY; i++)
    {
        size_t resultTxtLength = strlen(TWEntryTypePairArray[i].resultTxt);
        if((bufferLength >= resultTxtLength) &&
           strncmp(bufferLine, TWEntryTypePairArray[i].resultTxt, resultTxtLength) == 0)
        {
            return TWEntryTypePairArray[i];
        }
    }
    return TWEntryTypePairArray[TW_MAX_ENTRY];
}

TWResultCode TWWait(pthread_cond_t * cond, pthread_mutex_t * mutex, uint8_t timeout)
{
    int ret = 0;
    // This is a blocking call which hold the calling thread until an entry
    // has been enqueued or until the specified timeout has surpassed.
    struct timespec abs_time;
    clock_gettime(CLOCK_REALTIME , &abs_time);
    abs_time.tv_sec += timeout;
    ret = pthread_cond_timedwait(cond, mutex, &abs_time);

    switch (ret)
    {
        case 0:
            return TW_RESULT_OK;
        case ETIMEDOUT:
            OC_LOG(INFO, TAG, "Timed out waiting for CV. Non-error. Please try again.");
            return TW_RESULT_OK;
        case EINVAL:
            OC_LOG(ERROR, TAG, "Cond or Mutex is invalid. OR timeout value for CV is invalid.");
            break;
        case EPERM:
            OC_LOG(ERROR, TAG, "Cannot use CV because the current thread does not own the CV.");
            break;
    }

    return TW_RESULT_ERROR;
}

TWResultCode TWGrabMutex(pthread_mutex_t * mutex)
{
    int ret = pthread_mutex_lock(mutex);

    switch (ret)
    {
        case 0:
            return TW_RESULT_OK;
        case EINVAL:
            OC_LOG(ERROR, TAG, "Mutex was not initialized.");
            break;
        case ETIMEDOUT:
            OC_LOG(INFO, TAG, "Timed out waiting for mutex. Non-error. Please try again.");
            return TW_RESULT_OK;
        case EAGAIN:
            OC_LOG(ERROR, TAG, "Maximum number of locks for mutex have been exceeded.");
            break;
        case EDEADLK:
            OC_LOG(ERROR, TAG, "Deadlock OR this thread already owns the mutex.");
            break;
    }
    return TW_RESULT_ERROR;
}

TWResultCode TWReleaseMutex(pthread_mutex_t * mutex)
{
    int ret = pthread_mutex_unlock(mutex);

    switch (ret)
    {
        case 0:
            return TW_RESULT_OK;
        case EINVAL:
            OC_LOG(ERROR, TAG, "Mutex was not initialized.");
            break;
        case EAGAIN:
            OC_LOG(ERROR, TAG, "Maximum number of locks for mutex have been exceeded.");
            break;
        case EPERM:
            OC_LOG(ERROR, TAG, "Cannot unlock because the current thread does not own the mutex.");
            break;
    }
    return TW_RESULT_ERROR;
}

char readBufferChar(int fd, ssize_t * readDataBytes)
{
    // Performs read operation on fd for one character at a time.
    if(!readDataBytes)
    {
        return '\0';
    }
    *readDataBytes = 0;
    char byte = '\0';
    errno = 0;
    *readDataBytes = read(fd, &byte, sizeof(byte));
    if(*readDataBytes < 0)
    {
        OC_LOG_V(ERROR, TAG, "\tCould not read from port. Errno is: %d\n", errno);
        return '\0';
    }
    return byte;
}

bool isLineIgnored(const char * line, size_t length)
{
    if(length >= 4)
    {
        if(line[0] == 'N' && line[1] == 'A' && line[2] == 'C' && line[3] == 'K')
        {
            return true;
        }
    }
    if(length >= 3)
    {
        if(line[0] == 'S' && line[1] == 'E' && line[2] == 'Q')
        {
            return true;
        }
        else if(line[0] == 'A' && line[1] == 'C' && line[2] == 'K')
        {
            return true;
        }
    }
    if(length >= 2)
    {
        if(line[0] == 'A' && line[1] == 'T')
        {
            // If the line starts with "AT", then this is an echo. We ignore echos.
            return true;
        }
        else if(line[0] == 'O' && line[1] == 'K')
        {
            //If this line is "OK", we ignore success codes. But we do end TWEntry's on "OK".
            // (FYI, error codes are handled in readEntry() which invokes this function.)
            return true;
        }
    }
    return false;
}

const char * readBufferLine(int fd)
{
    char bufferChar = '\0';
    size_t bufferLoc = 0;
    ssize_t readDataBytes = 0;
    char * bufferLineHold = NULL;
    char * bufferLine = NULL;
    bool endOfLine1 = false;
    bool endOfLine2 = false;
    bool ignore = false;
    while(true)
    {
        if(!endOfLine1 || !endOfLine2)
        {
            bufferChar = readBufferChar(fd, &readDataBytes);
            if(bufferChar == '\r')
            {
                endOfLine1 = true;
                continue;
            }
            if(bufferChar == '\n')
            {
                endOfLine2 = true;
                continue;
            }
        }
        if(readDataBytes != 0 && (!endOfLine1 && !endOfLine2))
        {
            size_t bufferLineSize = sizeof(bufferChar)*(bufferLoc+2);
            bufferLine = (char *) OICRealloc(bufferLineHold, bufferLineSize);
            if(!bufferLine)
            {
                OC_LOG(ERROR, TAG, "Ran out of memory.");
                return NULL;
            }
            bufferLine[bufferLoc] = '\0';
            OICStrcat(bufferLine, bufferLineSize+2, &bufferChar);
            bufferLoc++;
            bufferLineHold = bufferLine;

            OC_LOG_V(DEBUG, TAG, "Incoming: %s", bufferLine);
        }
        else
        {
            if(!bufferLine)
            {
                return NULL;
            }
            size_t bufferLineSize = sizeof(bufferChar)*(bufferLoc+2);
            bufferLine = (char *) OICRealloc(bufferLineHold, bufferLineSize);
            if(!bufferLine)
            {
                OC_LOG(ERROR, TAG, "Ran out of memory.");
                return NULL;
            }
            bufferLine[bufferLoc] = '\0';
            bufferLoc++;
            bufferLineHold = bufferLine;
            ignore = isLineIgnored(bufferLine, strlen(bufferLine));
            if(ignore)
            {
                OICFree(bufferLine);
                return readBufferLine(fd);
            }
            if(endOfLine1 && endOfLine2)
            {
                return bufferLine;
            }
            else
            {
                return NULL;
            }
        }
    }
}

TWResultCode TWAddLineToEntry(const char * line, TWEntry * entry)
{
    if(!line || !entry)
    {
        OC_LOG(ERROR, TAG, "Invalid/NULL parameter(s) received.");
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWLine * twLine = (TWLine *) OICCalloc(1, sizeof(TWLine));
    if(!twLine)
    {
        OC_LOG(ERROR, TAG, "Ran out of memory.");
        return TW_RESULT_ERROR_NO_MEMORY;
    }
    size_t lineLength = strlen(line);
    twLine->line = line;
    twLine->length = lineLength;
    size_t errorLength = strlen(TW_ENDCONTROL_ERROR_STRING);
    size_t maxLength = (lineLength > errorLength) ? errorLength : lineLength;

    if((errorLength == lineLength) &&
       strncmp(line, TW_ENDCONTROL_ERROR_STRING, maxLength) == 0)
    {
        entry->atErrorCode[0] = line[errorLength];
        entry->atErrorCode[1] = line[errorLength + 1];
    }
    else
    {
        entry->atErrorCode[0] = '0';
        entry->atErrorCode[1] = '0';
    }

    // Null terminate the string.
    entry->atErrorCode[2] = '\0';

    if(!entry->lines)
    {
        entry->lines = twLine;
    }
    else
    {
        entry->lines[entry->count] = *twLine;
    }
    entry->count++;

    return TW_RESULT_OK;
}

TWEntry * readEntry(int fd)
{
    // Calls readBufferLine().
    // Forms TWEntry from 1-n lines based on the response type.

    TWEntry * entry = (TWEntry *) OICCalloc(1, sizeof(TWEntry));
    if(!entry)
    {
        OC_LOG(ERROR, TAG, "Ran out of memory.");
        return NULL;
    }
    entry->count = 0;

    const char * bufferLine = NULL;
    TWEntryTypePair entryTypePair = { .resultTxt = NULL,
                                      .numLines  = 0,
                                      .entryType = TW_NONE };
    size_t numLines = 0;
    while(numLines == 0 || bufferLine)
    {
        if(numLines == 0)
        {
            bufferLine = readBufferLine(fd);
            if(!bufferLine)
            {
                goto exit;
            }
            entryTypePair = getEntryTypePair(bufferLine);
        }
        else
        {
            bufferLine = readBufferLine(fd);
        }
        if(bufferLine != NULL)
        {
            entry->type = entryTypePair.entryType;
            TWAddLineToEntry(bufferLine, entry);
            numLines++;
            entry->count = numLines;
        }

        if(entryTypePair.numLines != numLines)
        {
            entry->resultCode = TW_RESULT_ERROR_LINE_COUNT;
        }
        else
        {
            entry->resultCode = TW_RESULT_OK;
        }
    }
    return entry;
exit:
    OICFree(entry);
    return NULL;
}

TWResultCode TWRetrieveEUI(PIPlugin * plugin, TWSock * twSock)
{
    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }

    TWEntry * entry = NULL;
    TWResultCode deleteResult = TW_RESULT_OK;
    TWResultCode result = TWIssueATCommand(plugin, AT_CMD_GET_LOCAL_EUI);
    if(result != TW_RESULT_OK)
    {
        return result;
    }
    result = TWGrabMutex(&twSock->mutex);
    if(result != TW_RESULT_OK)
    {
        return result;
    }
    entry = readEntry(twSock->fd);
    if(!entry)
    {
        result = TWReleaseMutex(&twSock->mutex);
        if(result != TW_RESULT_OK)
        {
            goto exit;
        }
    }
    twSock->eui = (char *) OICMalloc(strlen(entry->lines[0].line)+1);
    if(!twSock->eui)
    {
        result = TW_RESULT_ERROR_NO_MEMORY;
        goto exit;
    }

    if(SIZE_EUI != (strlen(entry->lines[0].line)+1))
    {
        OICFree(twSock->eui);
        result = TW_RESULT_ERROR;
        goto exit;
    }

    OICStrcpy(twSock->eui, SIZE_EUI, entry->lines[0].line);

    result = TWReleaseMutex(&twSock->mutex);
    if(result != TW_RESULT_OK)
    {
        OICFree(twSock->eui);
        goto exit;
    }
exit:
    deleteResult = TWDeleteEntry(plugin, entry);
    if(deleteResult != TW_RESULT_OK)
    {
        return deleteResult;
    }
    return result;
}

TWResultCode TWStartSock(PIPlugin * plugin, const char * fileLoc)
{
    TWSock * sock = TWGetSock((PIPlugin *)plugin);
    if(sock && sock->isActive == true)
    {
        // Ignore requests to start up the same socket.
        return TW_RESULT_OK;
    }
    if(!sock)
    {
        sock = (TWSock *) OICCalloc(1, sizeof(TWSock));
        if(!sock)
        {
            return TW_RESULT_ERROR_NO_MEMORY;
        }
    }
    TWResultCode ret = TWAddTWSock(sock, plugin, fileLoc);
    if(ret != 0)
    {
        goto exit;
    }

    ret = TWRetrieveEUI((PIPlugin *)plugin, sock);
    if(ret != TW_RESULT_OK)
    {
        OC_LOG(ERROR, TAG, "Unable to retrieve Zigbee Radio's EUI.");
        return ret;
    }

    int result = pthread_create(&(sock->threadHandle),
                                NULL,
                                readForever,
                                (void *) plugin);
    if(result != 0)
    {
        OC_LOG_V(ERROR, TAG, "Thread start failed with error %d", result);
        ret = TW_RESULT_ERROR;
        goto exit;
    }

    return TW_RESULT_OK;

exit:
    TWDeleteTWSock(sock);
    return ret;
}

static void sigHandler(int signal)
{
    pthread_t tid = pthread_self();
    (void)tid;
    (void)signal;
    OC_LOG_V(INFO, TAG, "Received signal on thread: %lu\n", tid);
    OC_LOG_V(INFO, TAG, "Signal is: %d", signal);
}

void * readForever(/*PIPlugin*/ void * plugin)
{
    TWResultCode result = TW_RESULT_OK;
    TWEntry * entry = NULL;
    TWSock * twSock = TWGetSock((PIPlugin *)plugin);
    if(!twSock)
    {
        OC_LOG(ERROR, TAG, "Unable to retrieve associated socket.");
        return NULL;
    }

    pthread_t tid = pthread_self();
    (void)tid;
    OC_LOG_V(INFO, TAG, "ReadForever Telegesis ThreadId: %lu", tid);

    struct sigaction action = { .sa_handler = 0 };

    sigset_t sigmask;

    action.sa_handler = sigHandler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(EINTR, &action, NULL);

    fd_set readFDS;
    FD_ZERO(&readFDS);
    FD_SET(twSock->fd, &readFDS);
    bool haveMutex = false;
    while(true)
    {
        errno = 0;
        // 'sigmask' is needed to catch intterupts.
        // This interrupt happens after call to pthread_exit(..., EINTR).
        // Once a signal handler is registered, pselect will handle interrupts by returning
        // with '-1' and setting errno appropriately.
        int ret = pselect(twSock->fd+1, &readFDS, NULL, NULL, NULL, &sigmask);
        if(ret < 0)
        {
            if(errno == EINTR)
            {
                if(twSock->isActive)
                {
                    continue;
                    // This EINTR signal is not for us. Do not handle it.
                }
                OC_LOG(DEBUG, TAG, "Thread has been joined. Exiting thread.");
                pthread_exit(PTHREAD_CANCELED);
                return NULL;
            }
            else
            {
                OC_LOG_V(ERROR, TAG, "Unaccounted error occurred. Exiting thread."
                                     "Errno is: %d", errno);
                return NULL;
            }
        }
        else
        {
            ret = FD_ISSET(twSock->fd, &readFDS);
            if(ret != 0)
            {
                // Valid data on valid socket.
                // Grab & parse, then pass up to upper layers.
                if(haveMutex != true)
                {
                    result = TWGrabMutex(&twSock->mutex);
                    if(result != TW_RESULT_OK)
                    {
                        OC_LOG(ERROR, TAG, "Unable to grab mutex.");
                        return NULL;
                    }
                    haveMutex = true;
                }
                entry = readEntry(twSock->fd);
                if(!entry)
                {
                    result = TWReleaseMutex(&twSock->mutex);
                    if(result != TW_RESULT_OK)
                    {
                        OC_LOG(ERROR, TAG, "Unable to release mutex.");
                        return NULL;
                    }
                    haveMutex = false;
                    // This is most likely a parsing error of the received
                    // response. Not necessarily fatal.
                    continue;
                }
                result = TWEnqueueEntry((PIPlugin *)plugin, entry);
                if(result != TW_RESULT_OK)
                {
                    OC_LOG_V(ERROR, TAG, "Could not add TWEntry to queue for"
                                          "consumption by the application"
                                          "layer. Error is: %d", result);
                    TWDeleteEntry((PIPlugin *)plugin, entry);
                    // This is most likely a FATAL error, such as out of memory.
                    break;
                }

                // Notify other threads waiting for a response that an entry has been enqueued.
                pthread_cond_signal(&twSock->queueCV);

                result = TWReleaseMutex(&twSock->mutex);
                haveMutex = false;
            }
            else
            {
                // Unrelated data waiting elsewhere. Continue the loop.
                continue;
            }
        }
    }

    return NULL;
}

TWResultCode TWIssueATCommand(PIPlugin * plugin, const char * command)
{
    if(!plugin || !command)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    OC_LOG_V(INFO, TAG, "Attempt to write %s.", command);
    TWSock * twSock = TWGetSock(plugin);
    if(!twSock)
    {
        return TW_RESULT_ERROR;
    }

    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }

    TWResultCode result = TW_RESULT_OK;
    TWResultCode mutexResult = TW_RESULT_OK;
    mutexResult = TWGrabMutex(&twSock->mutex);
    if(mutexResult != TW_RESULT_OK)
    {
        return mutexResult;
    }
    size_t sendCommandSize = (strlen(command) + strlen("\r") + 1) * sizeof(char);
    char * sendCommand = (char *) OICCalloc(1, sendCommandSize);
    if(!sendCommand)
    {
        return TW_RESULT_ERROR_NO_MEMORY;
    }
    char * temp = OICStrcpy(sendCommand, sendCommandSize, command);
    if(temp != sendCommand)
    {
        result = TW_RESULT_ERROR;
        goto exit;
    }
    temp = OICStrcat(sendCommand, sendCommandSize, "\r");
    if(temp != sendCommand)
    {
        result = TW_RESULT_ERROR;
        goto exit;
    }
    size_t expectedWrittenBytes = strlen(sendCommand);
    errno = 0;
    size_t actualWrittenBytes = write(twSock->fd, sendCommand, expectedWrittenBytes);
    if(actualWrittenBytes <= 0)
    {
        OC_LOG_V(ERROR, TAG, "Could not write to port. Errno is: %d\n", errno);
        result =  TW_RESULT_ERROR;
        goto exit;
    }
    if(actualWrittenBytes != expectedWrittenBytes)
    {
        OC_LOG(ERROR, TAG, "Telegesis Adapter did not receive expected command. Unknown state.");
        result = TW_RESULT_ERROR;
    }

exit:
    mutexResult = TWReleaseMutex(&twSock->mutex);
    if(mutexResult != TW_RESULT_OK)
    {
        return mutexResult;
    }
    OICFree(sendCommand);
    return result;
}

TWResultCode TWEnqueueEntry(PIPlugin * plugin, TWEntry * entry)
{
    if(!plugin || !entry)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWSock * twSock = TWGetSock(plugin);
    if(!twSock)
    {
        return TW_RESULT_ERROR;
    }

    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }
    LL_APPEND(twSock->queue, entry);
    return TW_RESULT_OK;
}

TWResultCode TWDequeueEntry(PIPlugin * plugin, TWEntry ** entry, TWEntryType type)
{
    if(!plugin || !entry)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWSock * twSock = TWGetSock(plugin);
    if(!twSock)
    {
        return TW_RESULT_ERROR;
    }

    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }

    TWResultCode ret = TW_RESULT_OK;

    // If no entry is found, then this code path returns immediately.
    ret = TWGrabMutex(&twSock->mutex);
    if(ret != TW_RESULT_OK)
    {
        return ret;
    }

    if(type != TW_NONE)
    {
        // Wait for up to 10 seconds for the entry to put into the queue.
        ret = TWWait(&twSock->queueCV, &twSock->mutex, TIME_OUT_10_SECONDS);
        if(ret != TW_RESULT_OK)
        {
            return ret;
        }
    }

    *entry = twSock->queue;
    if(*entry)
    {
        LL_DELETE(twSock->queue, *entry);
    }
    ret = TWReleaseMutex(&twSock->mutex);
    if(ret != TW_RESULT_OK)
    {
        return ret;
    }
    return ret;
}

TWResultCode TWFreeQueue(PIPlugin * plugin)
{
    if(!plugin)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWResultCode ret = TW_RESULT_OK;
    TWEntry * entry = NULL;
    while(true)
    {
        ret = TWDequeueEntry(plugin, &entry, TW_NONE);
        if(ret != TW_RESULT_OK)
        {
            return ret;
        }
        if(entry == NULL)
        {
            break;
        }
        ret = TWDeleteEntry(plugin, entry);
        if(ret != TW_RESULT_OK)
        {
            return ret;
        }
    }
    return ret;
}

TWResultCode TWDeleteEntry(PIPlugin * plugin, TWEntry * entry)
{
    if(!plugin || !entry)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }

    TWSock * twSock = TWGetSock(plugin);
    if(!twSock)
    {
        return TW_RESULT_ERROR;
    }

    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }

    TWResultCode ret = TWGrabMutex(&twSock->mutex);
    if(ret != TW_RESULT_OK)
    {
        return ret;
    }
    TWEntry * out = NULL;
    TWEntry * tmp = NULL;
    LL_FOREACH_SAFE(twSock->queue, out, tmp)
    {
        if(out == entry)
        {
            OC_LOG(ERROR, TAG, "Tried to delete an entry that is still in the queue. \
                                Please dequeue the entry first.");
            return TW_RESULT_ERROR;
        }
    }
    ret = TWReleaseMutex(&twSock->mutex);

    OICFree(entry);

    return TW_RESULT_OK;
}

TWResultCode TWGetEUI(PIPlugin * plugin, char ** eui)
{
    if(!plugin || !eui)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWSock * twSock = TWGetSock(plugin);
    if(!twSock)
    {
        return TW_RESULT_ERROR;
    }

    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }

    *eui = OICStrdup(twSock->eui);

    return TW_RESULT_OK;

}

TWResultCode TWStopSock(PIPlugin * plugin)
{
    if(!plugin)
    {
        return TW_RESULT_ERROR_INVALID_PARAMS;
    }
    TWSock * twSock = TWGetSock(plugin);
    if(!twSock)
    {
        return TW_RESULT_ERROR;
    }

    if(twSock->isActive == false)
    {
        return TW_RESULT_ERROR;
    }

    TWResultCode ret = TWFreeQueue(plugin);
    if(ret != TW_RESULT_OK)
    {
        return ret;
    }

    twSock->isActive = false;

    void * retVal = NULL;
    int pthreadRet = pthread_kill(twSock->threadHandle, EINTR);
    if(pthreadRet != 0)
    {
        return TW_RESULT_ERROR;
    }

    pthreadRet = pthread_join(twSock->threadHandle, &retVal);
    if(pthreadRet != 0)
    {
        switch(pthreadRet)
        {
            case EDEADLK:
                OC_LOG(ERROR, TAG, "A deadlock has occurred.");
                break;
            case EINVAL:
                OC_LOG(ERROR, TAG, "Thread is not joinable or another thread has already joined.");
                break;
            case ESRCH:
                OC_LOG(ERROR, TAG, "No thread with the ID could be found.");
                break;
            default:
                OC_LOG_V(ERROR, TAG, "Unknown error occurred when joining thread: %d", pthreadRet);
        }
        return TW_RESULT_ERROR;
    }
    if(retVal != PTHREAD_CANCELED)
    {
        return TW_RESULT_ERROR;
    }
    ret = TWDeleteTWSock(twSock);
    if(ret != TW_RESULT_OK)
    {
        return ret;
    }

    return ret;
}
