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

/**
 * @file
 *
 * This API only works with:
 *      Telegesis ETRX357
 *      CICIE R310 B110615
 *
 */

#ifndef TELEGESISSOCKET_H_
#define TELEGESISSOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "twtypes.h"
#include "plugininterface.h"

typedef enum
{
    TW_NETWORK_INFO = 0,
    TW_JPAN,
    TW_RFD,
    TW_FFD,
    TW_SED,
    TW_ZED,
    TW_MATCHDESC,
    TW_SIMPLEDESC,
    TW_INCLUSTER,
    TW_WRITEATTR,
    TW_RESPATTR,
    TW_TEMPERATURE,
    TW_DFTREP,
    TW_DRLOCRSP,
    TW_DRUNLOCKRSP,
    TW_ACK,
    TW_NACK,
    TW_SEQ,
    TW_ZENROLLREQ,
    TW_ENROLLED,
    TW_ZONESTATUS,
    TW_ADDRESS_RESPONSE,
    TW_NONE,
    TW_MAX_ENTRY
} TWEntryType;

/**
 * Represents a single line within TWEntry struct.
 */
typedef struct
{
    const char * line;
    int length;
} TWLine;

/**
 * Represents a queue item. This is structure built after incoming data from the
 * Telegesis adapter has put something in the buffer. A single TWEntry can contain 0+ TWLines.
 */
typedef struct TWEntry
{
    /** A pointer to the list of lines */
    TWLine * lines;
    /** Number of lines in this entry. */
    int count;
    /** The type of entry. This maps with the leading tag of any given AT response/prompt. */
    TWEntryType type;
    /** First two characters are an AT Error Code,
        while third character is NULL terminator so it can be printed */
    char atErrorCode[3];
    /** Any read, write, parsing, or system errors are captured in this generic resultCode. */
    TWResultCode resultCode;
    struct TWEntry * next; // Ignore; Used internally to manage the queue.
} TWEntry;

/**
 * Starts socket communication with the Telegesis Dongle at com location.
 *
 * @param plugin The plugin' scope which the socket ops will operate within.
 *
 * @param fileLoc The file descriptor location on the file system to start.
 */
TWResultCode TWStartSock(PIPlugin * plugin, const char * fileLoc);

/**
 * Issues command to a specific Telegesis Dongle.
 *
 * @param plugin The plugin' scope which the command will be issued.
 *
 * @param command The command to be issued to the Telegesis Dongle.
 */
TWResultCode TWIssueATCommand(PIPlugin * plugin, const char * command);

/**
 * Returns a response/prompt. If NULL, no response or prompts have been issued
 * back by the Telegesis Dongle.
 *
 * @param plugin The plugin' scope which the socket is managed within.
 *
 * @param entry The line(s) which correspond to a single entry in the
 * response/prompt queue. Returned by-reference.
 *
 * @param type The type of entry this queue item is. If not specified as TW_NONE,
 * this API will block (for up to 5 Seconds) until an entry with the specified type
 * has been enqueued.
 *
 * @brief Its best to call this function in a loop. Break from loop when this
 * function returns NULL. Otherwise, handle the data in TWEntry. Release
 * memory allocated by this function by passing the entry into TWDeleteEntry.
 */
TWResultCode TWDequeueEntry(PIPlugin * plugin, TWEntry ** entry, TWEntryType type);

/**
 * Helper function to deallocate memory of a TWEntry.
 *
 * Use this function when you are finished using the entry returned after
 * calling TWDequeueLine. This will ensure your utilization of this API does
 * not lead to memory leaks in your application.
 *
 * @param plugin The plugin' scope which the socket is managed within.
 *
 * @param entry The entry that was dequeued by calling TWDequeueLine.
 */
TWResultCode TWDeleteEntry(PIPlugin * plugin, TWEntry * entry);

/**
 * Helper function to retrieve the current radio's EUI.
 *
 * @param plugin The plugin' scope which the socket is managed within.
 *
 * @param eui The local radio's EUI.
 */
TWResultCode TWGetEUI(PIPlugin * plugin, char ** eui);

/**
 * Stops socket communication with the Telegesis Dongle within scope of plugin.
 *
 * @param plugin The plugin' scope the socket ops cease to operate within.
 */
TWResultCode TWStopSock(PIPlugin * plugin);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TELEGESISSOCKET_H_ */
