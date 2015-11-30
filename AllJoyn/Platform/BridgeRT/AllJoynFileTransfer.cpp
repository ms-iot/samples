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

#include <alljoyn_c/BusAttachment.h>
#include <alljoyn_c/InterfaceDescription.h>
#include <alljoyn_c/BusObject.h>
#include <alljoyn_c/Message.h>
#include <alljoyn_c/MsgArg.h>

#include "AllJoynFileTransfer.h"
#include "Bridge.h"

using namespace BridgeRT;
using namespace DsbCommon;

static const char CSP_INTERFACE_NAME[] = "com.microsoft.alljoynmanagement.Config";
static const ULONG CHUNK_SIZE = (4 * 1024);     // size in bytes
static const WCHAR TEMP_FILE_EXTENSION[] = L".xml";

typedef struct
{
    char *name;
    char *signatureIn;
    char *signatureOut;
    char *paramList;
    alljoyn_messagereceiver_methodhandler_ptr method;
} MethodDescriptor;

static const int STARTWRITECHUNK_INDEX = 0;
static const int WRITENEXTCHUNK_INDEX = 1;
static const int STARTREADCHUNK_INDEX = 2;
static const int READNEXTCHUNK_INDEX = 3;

static const MethodDescriptor g_methodDescriptions[] = {
    {
        "StartChunkWrite",
        "u",
        "(uu)",
        "length,outStruct",                     //outStruct: token, chunkSize
        AllJoynFileTransfer::StartChunkWrite
    },
    {
        "WriteNextChunk",
        "(uay)",
        nullptr,                                //no output parameters
        "inStruct",
        AllJoynFileTransfer::WriteNextChunk
    },
    {
        "StartChunkRead",
        "",
        "(uuu)",
        "outStruct",                            //outStruct: fileSize, token, chunkSize
        AllJoynFileTransfer::StartChunkRead
    },
    {
        "ReadNextChunk",
        "u",
        "ay",
        "token,buffer",
        AllJoynFileTransfer::ReadNextChunk
    }
};

static const ULONG BAD_TOKEN = 0;
ULONG AllJoynFileTransfer::s_transferId = BAD_TOKEN + 1;

AllJoynFileTransfer::AllJoynFileTransfer()
    : m_allJoynBusObject(NULL),
    m_interfaceDescription(NULL),
    m_bus(nullptr),
    m_registeredOnAllJoyn(false),
    m_endOfTransfer(false),
    m_fileSize(0),
    m_nbOfBytesTransfered(0)
{
    ::InterlockedExchange(&m_lockCount, 0);
}

AllJoynFileTransfer::~AllJoynFileTransfer()
{
    Destroy();
}

void AllJoynFileTransfer::Destroy()
{
    // forbid future file transfer
    ::InterlockedExchange(&m_lockCount, 1);

    // release alljoyn bus object
    if (NULL != m_allJoynBusObject)
    {
        if (m_registeredOnAllJoyn)
        {
            // unregister bus object
            alljoyn_busattachment_unregisterbusobject(*m_bus, m_allJoynBusObject);
            m_registeredOnAllJoyn = false;
        }
        alljoyn_busobject_destroy(m_allJoynBusObject);
        m_allJoynBusObject = NULL;
    }

    // clear members
    m_interfaceDescription = NULL;
    m_endOfTransfer = false;
    m_tempFileName.clear();
    m_fileSize = 0;
    m_nbOfBytesTransfered = 0;

    // delete temp file
    DeleteTempFile();
}

QStatus AllJoynFileTransfer::Initialize(_In_ alljoyn_busattachment* bus, _In_z_ const char *objectPath, _In_ ConfigManager *configManager)
{
    QStatus status = ER_OK;
    alljoyn_busobject_callbacks busObjectCallbacks = { nullptr, nullptr, nullptr, nullptr };

    // sanity check
    if (nullptr == bus)
    {
        status = ER_BAD_ARG_1;
        goto leave;
    }
    if (nullptr == objectPath)
    {
        status = ER_BAD_ARG_2;
        goto leave;
    }

    // save bus attachment and reset members
    m_bus = bus;
    m_endOfTransfer = false;

    // create bus object
    m_allJoynBusObject = alljoyn_busobject_create(objectPath, QCC_FALSE, &busObjectCallbacks, nullptr);
    if (NULL == m_allJoynBusObject)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // add interface to bus object (create it if necessary)
    m_interfaceDescription = alljoyn_busattachment_getinterface(*m_bus, CSP_INTERFACE_NAME);
    if (NULL == m_interfaceDescription)
    {
        // create the interface
        if (configManager->IsConfigurationAccessSecured())
        {
            // create the interface
            status = alljoyn_busattachment_createinterface_secure(*m_bus, CSP_INTERFACE_NAME, &m_interfaceDescription, AJ_IFC_SECURITY_REQUIRED);
        }
        else
        {
            status = alljoyn_busattachment_createinterface(*m_bus, CSP_INTERFACE_NAME, &m_interfaceDescription);
        }
        if (ER_OK != status)
        {
            goto leave;
        }

        // add methods to interface
        for (int index = 0; index < ARRAYSIZE(g_methodDescriptions); index++)
        {
            status = alljoyn_interfacedescription_addmethod(m_interfaceDescription,
                g_methodDescriptions[index].name,
                g_methodDescriptions[index].signatureIn,
                g_methodDescriptions[index].signatureOut,
                g_methodDescriptions[index].paramList,
                0,
                nullptr);
            if (ER_OK != status)
            {
                goto leave;
            }
        }

        // activate interface
        alljoyn_interfacedescription_activate(m_interfaceDescription);
    }

    // add interface to bus object
    status = alljoyn_busobject_addinterface(m_allJoynBusObject, m_interfaceDescription);
    if (ER_BUS_IFACE_ALREADY_EXISTS == status)
    {
        // this is OK
        status = ER_OK;
    }
    else if (ER_OK != status)
    {
        goto leave;
    }

    // add methods handlers
    for (int index = 0; index < ARRAYSIZE(g_methodDescriptions); index++)
    {
        QCC_BOOL found = QCC_FALSE;
        alljoyn_interfacedescription_member member = { 0 };

        found = alljoyn_interfacedescription_getmember(m_interfaceDescription, g_methodDescriptions[index].name, &member);
        if (!found)
        {
            status = ER_INVALID_DATA;
            goto leave;
        }

        status = alljoyn_busobject_addmethodhandler(m_allJoynBusObject, member, g_methodDescriptions[index].method, NULL);
        if (ER_OK != status)
        {
            goto leave;
        }
    }


    // register bus object on alljoyn
    status = alljoyn_busattachment_registerbusobject(*m_bus, m_allJoynBusObject);
    if (ER_OK != status)
    {
        goto leave;
    }

    m_registeredOnAllJoyn = true;
    ::InterlockedExchange(&m_lockCount, 0);

leave:
    return status;
}

AllJoynFileTransfer *AllJoynFileTransfer::GetInstance(_In_ alljoyn_busobject busObject)
{
    AllJoynFileTransfer *objectPointer = nullptr;
    ConfigManager *configManager = nullptr;

    // sanity check
    if (NULL == busObject)
    {
        goto leave;
    }
    configManager = DsbBridge::SingleInstance()->GetConfigManager();
    objectPointer = configManager->GetAllJoynFileTransferInstance(busObject);

leave:
    return objectPointer;
}

void AllJoynFileTransfer::EndTransfer()
{
    // increment transfer ID (wrap around is OK)
    s_transferId++;
    if (BAD_TOKEN == s_transferId)
    {
        s_transferId++;
    }

    // reset file transfer lock count
    ::InterlockedExchange(&m_lockCount, 0);
}

void AllJoynFileTransfer::StartChunkWrite(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    HRESULT hr = S_OK;
    bool cancelTransferOnError = false;

    AllJoynFileTransfer *cspInterface = nullptr;
    alljoyn_msgarg outArg = NULL;
    alljoyn_msgarg inArg = NULL;
    uint32_t fileSize = 0;
    ULONG token = BAD_TOKEN;

    UNREFERENCED_PARAMETER(member);

    // find CspInterface instance associated with bus object
    cspInterface = AllJoynFileTransfer::GetInstance(busObject);
    if (nullptr == cspInterface)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    // get size of file to write
    inArg = alljoyn_message_getarg(msg, 0);
    if (NULL == inArg)
    {
        status = ER_BAD_ARG_3;
        goto leave;
    }
    status = alljoyn_msgarg_get(inArg, g_methodDescriptions[STARTWRITECHUNK_INDEX].signatureIn, &fileSize);
    if (ER_OK != status)
    {
        goto leave;
    }

    // verify no file transfer is already in progress
    if (1 == ::InterlockedExchange(&cspInterface->m_lockCount, 1))
    {
        // not authorize if some else already transfers the file
        status = ER_PERMISSION_DENIED;
        goto leave;
    }

    // init transfer related members
    cancelTransferOnError = true;
    cspInterface->m_endOfTransfer = false;
    cspInterface->m_fileSize = fileSize;
    cspInterface->m_nbOfBytesTransfered = 0;

    // set temp file name
    hr = cspInterface->CreateTempFile();
    if (FAILED(hr))
    {
        status = ER_OPEN_FAILED;
        goto leave;
    }

    // set transfer token
    token = s_transferId;

leave:
    // prepare and send response
    //
    if (ER_OK != status)
    {
        alljoyn_busobject_methodreply_status(busObject, msg, status);
    }
    else
    {
        // prepare out arguments
        outArg = alljoyn_msgarg_create();
        if (NULL != outArg)
        {
            status = alljoyn_msgarg_set(outArg, g_methodDescriptions[STARTWRITECHUNK_INDEX].signatureOut, token, CHUNK_SIZE);
        }
        else
        {
            status = ER_OUT_OF_MEMORY;
        }

        // send response (if possible)
        if (ER_OK != status)
        {
            alljoyn_busobject_methodreply_status(cspInterface->m_allJoynBusObject, msg, status);
        }
        else
        {
            alljoyn_busobject_methodreply_args(cspInterface->m_allJoynBusObject, msg, outArg, 1);
        }
    }

    // cancel file transfer if required
    if ((ER_OK != status || FAILED(hr)) &&
        cancelTransferOnError)
    {
        cspInterface->EndTransfer();
    }

    // clean up
    if (NULL != outArg)
    {
        alljoyn_msgarg_destroy(outArg);
    }

    return;
}

void AllJoynFileTransfer::WriteNextChunk(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    HRESULT hr = S_OK;
    bool cancelFileTransferOnError = true;

    AllJoynFileTransfer *cspInterface = nullptr;
    alljoyn_msgarg outArg = NULL;
    alljoyn_msgarg inArg = NULL;
    uint32_t token = 0;
    size_t nbOfBytes = 0;
    byte *bytes = nullptr;

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER position = { 0 };
    HANDLE finalEvent = NULL;

    UNREFERENCED_PARAMETER(member);

    // there is no need to check if file transfer has been canceled since
    // next file transfer will use another temp file and all data needed to write this chunk of file are in params

    // find CspInterface instance associated with bus object
    cspInterface = AllJoynFileTransfer::GetInstance(busObject);
    if (nullptr == cspInterface)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    if (cspInterface->m_endOfTransfer)
    {
        // transfer is already finished
        // => nothing to do
        status = ER_END_OF_DATA;
        goto leave;
    }

    // write this chunk in temp file

    // get bytes to write
    inArg = alljoyn_message_getarg(msg, 0);
    if (NULL == inArg)
    {
        status = ER_BAD_ARG_3;
        goto leave;
    }
    status = alljoyn_msgarg_get(inArg, g_methodDescriptions[WRITENEXTCHUNK_INDEX].signatureIn, &token, &nbOfBytes, &bytes);
    if (ER_OK != status)
    {
        goto leave;
    }

    // verify token
    if (token != s_transferId)
    {
        // don't cancel file transfer in this error case
        cancelFileTransferOnError = false;
        status = ER_PERMISSION_DENIED;
        goto leave;
    }

    // open temp file and set file pointer to end of file
    fileHandle = CreateFile2(cspInterface->m_tempFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING, nullptr);
    if (INVALID_HANDLE_VALUE == fileHandle)
    {
        status = ER_OPEN_FAILED;
        goto leave;
    }
    SetFilePointerEx(fileHandle, position, nullptr, FILE_END);

    // write buffer in file
    // Note:
    //   - nbOfBytes is size_t type which is larger than DWORD for x64 machines however
    //     by design AllJoyn guarantee that size of payload of 1 message cannot be greater than 2^17 which fits in a DWORD.
    //     In addition to this AllJoynFileTransfer payload (chunk size) is by design limited to 4K bytes
    if (!WriteFile(fileHandle, bytes, static_cast<DWORD>(nbOfBytes), nullptr, nullptr))
    {
        status = ER_WRITE_ERROR;
        goto leave;
    }

    // must close file now so commit will work
    CloseHandle(fileHandle);
    fileHandle = INVALID_HANDLE_VALUE;

    // verify end of transfer (see note above for DWORD conversion)
    cspInterface->m_nbOfBytesTransfered += static_cast<DWORD>(nbOfBytes);
    if (cspInterface->m_nbOfBytesTransfered >= cspInterface->m_fileSize)
    {
        cspInterface->m_endOfTransfer = true;
    }

    if (cspInterface->m_endOfTransfer)
    {
        // execute post transfer action
        //
        // Note:
        // this final event is necessary to make sure alljoyn method call
        // is terminated (alljoyn_busobject_methodreply_... has been called)
        // before doing anything else
        hr = cspInterface->PostFileWriteAction(cspInterface->m_tempAppRelativeFileName, &finalEvent);
        if (FAILED(hr))
        {
            // just indicates the error
            // don't leave since transfer needs to be ended
            status = ER_OS_ERROR;
        }
        // end file transfer
        cspInterface->DeleteTempFile();
        cspInterface->EndTransfer();
    }

leave:
    // prepare and send response
    //
    if (ER_OK != status)
    {
        alljoyn_busobject_methodreply_status(busObject, msg, status);
    }
    else
    {
        // send response (if possible)
        if (ER_OK != status)
        {
            alljoyn_busobject_methodreply_status(cspInterface->m_allJoynBusObject, msg, status);
        }
        else
        {
            alljoyn_busobject_methodreply_args(cspInterface->m_allJoynBusObject, msg, nullptr, 0);
        }
    }

    // cancel file transfer if necessary
    if ((ER_OK != status || FAILED(hr)) &&
        cancelFileTransferOnError)
    {
        cspInterface->EndTransfer();
    }

    // clean up
    if (NULL != outArg)
    {
        alljoyn_msgarg_destroy(outArg);
    }

    if (INVALID_HANDLE_VALUE != fileHandle)
    {
        CloseHandle(fileHandle);
    }

    // signal end event that post write file action indicated
    if (NULL != finalEvent)
    {
        SetEvent(finalEvent);
    }

    return;
}

void AllJoynFileTransfer::StartChunkRead(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    HRESULT hr = S_OK;
    bool cancelTransferOnError = false;
    StorageFolder^ tempFolder = ApplicationData::Current->LocalFolder;
    String^ tempFilePath = tempFolder->Path + L"\\";

    AllJoynFileTransfer *cspInterface = nullptr;

    WIN32_FILE_ATTRIBUTE_DATA fileAttribute = { 0 };
    alljoyn_msgarg outArg = NULL;
    ULONG token = BAD_TOKEN;

    UNREFERENCED_PARAMETER(member);

    // find CspInterface instance associated with bus object
    cspInterface = AllJoynFileTransfer::GetInstance(busObject);
    if (nullptr == cspInterface)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    // verify no file transfer is already in progress
    if (1 == ::InterlockedCompareExchange(&cspInterface->m_lockCount, 1, 0))
    {
        // not authorize if some else already transfers the file
        status = ER_PERMISSION_DENIED;
        goto leave;
    }
    cancelTransferOnError = true;

    // init transfer related members
    cspInterface->m_endOfTransfer = false;
    cspInterface->m_fileSize = 0;
    cspInterface->m_nbOfBytesTransfered = 0;

    hr = cspInterface->PreFileReadAction(cspInterface->m_tempAppRelativeFileName);
    if (FAILED(hr))
    {
        status = ER_OS_ERROR;
        goto leave;
    }

    cspInterface->m_tempFileName.clear();
    cspInterface->m_tempFileName = tempFilePath->Data();
    cspInterface->m_tempFileName += cspInterface->m_tempAppRelativeFileName;

    // get file size
    if (!GetFileAttributesEx(cspInterface->m_tempFileName.c_str(), GetFileExInfoStandard, &fileAttribute))
    {
        status = ER_OS_ERROR;
        goto leave;
    }
    // for now assume a DWORD is enough to store config files size
    cspInterface->m_fileSize = fileAttribute.nFileSizeLow;

    // set transfer token
    token = s_transferId;

leave:
    // prepare and send response
    //
    if (ER_OK != status)
    {
        alljoyn_busobject_methodreply_status(busObject, msg, status);
    }
    else
    {
        // prepare out arguments
        outArg = alljoyn_msgarg_create();
        if (NULL != outArg)
        {
            status = alljoyn_msgarg_set(outArg, g_methodDescriptions[STARTREADCHUNK_INDEX].signatureOut, cspInterface->m_fileSize, token, CHUNK_SIZE);
        }
        else
        {
            status = ER_OUT_OF_MEMORY;
        }

        // send response (if possible)
        if (ER_OK != status)
        {
            alljoyn_busobject_methodreply_status(cspInterface->m_allJoynBusObject, msg, status);
        }
        else
        {
            alljoyn_busobject_methodreply_args(cspInterface->m_allJoynBusObject, msg, outArg, 1);
        }
    }

    // cancel file transfer if required
    if ((ER_OK != status || FAILED(hr)) &&
        cancelTransferOnError)
    {
        (void)cspInterface->PostFileReadAction();
        cspInterface->EndTransfer();
    }

    // clean up
    if (NULL != outArg)
    {
        alljoyn_msgarg_destroy(outArg);
    }

    return;
}

void AllJoynFileTransfer::ReadNextChunk(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg)
{
    QStatus status = ER_OK;
    HRESULT hr = S_OK;
    bool cancelFileTransferOnError = true;

    AllJoynFileTransfer *cspInterface = nullptr;
    alljoyn_msgarg outArg = NULL;
    alljoyn_msgarg inArg = NULL;
    uint32_t token = 0;
    DWORD nbOfBytes = 0;
    byte *buffer = nullptr;

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER position = { 0 };

    UNREFERENCED_PARAMETER(member);

    // there is no need to check if file transfer has been canceled

    // find CspInterface instance associated with bus object
    cspInterface = AllJoynFileTransfer::GetInstance(busObject);
    if (nullptr == cspInterface)
    {
        status = ER_INVALID_DATA;
        goto leave;
    }

    if (cspInterface->m_endOfTransfer)
    {
        // transfer is already finished
        // => nothing to do
        status = ER_END_OF_DATA;
        goto leave;
    }

    buffer = new byte[CHUNK_SIZE];
    if (nullptr == buffer)
    {
        status = ER_OUT_OF_MEMORY;
        goto leave;
    }

    // get bytes to input arg
    inArg = alljoyn_message_getarg(msg, 0);
    if (NULL == inArg)
    {
        status = ER_BAD_ARG_3;
        goto leave;
    }
    status = alljoyn_msgarg_get(inArg, g_methodDescriptions[READNEXTCHUNK_INDEX].signatureIn, &token);
    if (ER_OK != status)
    {
        goto leave;
    }

    // verify token
    if (token != s_transferId)
    {
        // don't cancel file transfer in this error case
        cancelFileTransferOnError = false;
        status = ER_PERMISSION_DENIED;
        goto leave;
    }

    // open temp file and set file pointer to end of file
    fileHandle = CreateFile2(cspInterface->m_tempFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
    if (INVALID_HANDLE_VALUE == fileHandle)
    {
        status = ER_OPEN_FAILED;
        goto leave;
    }
    // assume DWORD is enough to store config file size
    position.LowPart = cspInterface->m_nbOfBytesTransfered;
    if (!SetFilePointerEx(fileHandle, position, nullptr, FILE_BEGIN))
    {
        status = ER_OS_ERROR;
        goto leave;
    }

    // read buffer from file
    nbOfBytes = 0;
    if (!ReadFile(fileHandle, buffer, CHUNK_SIZE, &nbOfBytes, nullptr))
    {
        status = ER_READ_ERROR;
        goto leave;
    }

    // verify end of transfer
    cspInterface->m_nbOfBytesTransfered += nbOfBytes;
    if (cspInterface->m_nbOfBytesTransfered >= cspInterface->m_fileSize)
    {
        cspInterface->m_endOfTransfer = true;
    }

leave:
    // prepare and send response
    //
    if (ER_OK != status)
    {
        alljoyn_busobject_methodreply_status(busObject, msg, status);
    }
    else
    {
        // prepare out arguments
        outArg = alljoyn_msgarg_create();
        if (NULL != outArg)
        {
            status = alljoyn_msgarg_set(outArg, g_methodDescriptions[READNEXTCHUNK_INDEX].signatureOut, nbOfBytes, buffer);
        }
        else
        {
            status = ER_OUT_OF_MEMORY;
        }

        // send response (if possible)
        if (ER_OK != status)
        {
            alljoyn_busobject_methodreply_status(cspInterface->m_allJoynBusObject, msg, status);
        }
        else
        {
            alljoyn_busobject_methodreply_args(cspInterface->m_allJoynBusObject, msg, outArg, 1);
        }
    }

    if (INVALID_HANDLE_VALUE != fileHandle)
    {
        CloseHandle(fileHandle);
    }

    // cancel file transfer if necessary
    if (((ER_OK != status || FAILED(hr)) && cancelFileTransferOnError) ||
        cspInterface->m_endOfTransfer)
    {
        (void)cspInterface->PostFileReadAction();
        cspInterface->EndTransfer();
    }


    // clean up
    if (NULL != outArg)
    {
        alljoyn_msgarg_destroy(outArg);
    }

    if (nullptr != buffer)
    {
        delete buffer;
    }

    return;
}

HRESULT AllJoynFileTransfer::CreateTempFile()
{
    HRESULT hr = S_OK;
    GUID guid = { 0 };
    WCHAR tempString[MAX_GUID_STRING_LEN];
    WCHAR *tempFileName = nullptr;
    int length = 0;
    StorageFolder^ tempFolder = ApplicationData::Current->LocalFolder;
    String^ tempFilePath = tempFolder->Path + L"\\";
    HANDLE fileHandle = INVALID_HANDLE_VALUE;

    // delete previous temp file
    DeleteTempFile();

    // create 'new' temp file name
    hr = CoCreateGuid(&guid);
    if (FAILED(hr))
    {
        goto leave;
    }

    //Convert GUID into String
    length = StringFromGUID2(guid, tempString, MAX_GUID_STRING_LEN);
    if (0 == length)
    {
        hr = E_FAIL;
        goto leave;
    }

    // remove '{' and '}' from GUID string
    if (tempString[length - 2] == L'}')
    {
        tempString[length - 2] = L'\0';
    }
    if (tempString[0] == L'{')
    {
        tempFileName = &tempString[1];
    }
    m_tempAppRelativeFileName = tempFileName;
    m_tempAppRelativeFileName += TEMP_FILE_EXTENSION;

    m_tempFileName = tempFilePath->Data();
    m_tempFileName += m_tempAppRelativeFileName;

    // create 'new' temp file
    fileHandle = CreateFile2(m_tempFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr);
    if (INVALID_HANDLE_VALUE == fileHandle)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leave;
    }

    CloseHandle(fileHandle);

leave:
    return hr;
}

void AllJoynFileTransfer::DeleteTempFile()
{
    // delete previous temp file
    if (0 != m_tempFileName.length())
    {
        AutoLock bridgeLocker(&DsbBridge::SingleInstance()->GetLock(), true);
        DeleteFileW(m_tempFileName.c_str());
        m_tempFileName.clear();
    }
}
