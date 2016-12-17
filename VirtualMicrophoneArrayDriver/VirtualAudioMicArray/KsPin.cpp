//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
#include "VirtualAudioMicArray.h"
#include "KsPin.h"

#include <ksmedia.h>
#include <ks.h>

#pragma code_seg("PAGE")
KsPin::KsPin(_In_ VirtualPin* virtualPin, BOOLEAN IsUsbDevice) :
    m_pVirtualPin(virtualPin),
    m_bPinRunning(FALSE),
    m_bPinOpened(FALSE),
    m_pDeviceObject(NULL),
    m_FileHandle(0),
    m_PinHandle(0),
    m_pPinFileObject(NULL),
    m_pFilterFileObject(NULL),
    m_bIsUsbDevice(IsUsbDevice),
    m_pPositionRegister(NULL),
    m_FifoSizeInFrames(0),
    m_bMuted(FALSE)
{
    PAGED_CODE();

    RtlZeroMemory(&m_Format, sizeof(m_Format));
    RtlZeroMemory((PVOID)&m_RtBuffer, sizeof(m_RtBuffer));
    RtlInitUnicodeString(&m_Name, NULL);

    ExInitializeFastMutex(&m_Mutex);
}

#pragma code_seg("PAGE")
KsPin::~KsPin()
{
    PAGED_CODE();

    ClosePin();

    if (m_Name.Buffer != NULL)
    {
        ExFreePool(m_Name.Buffer);
    }
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::ClosePin()
{
    PAGED_CODE();
    ExAcquireFastMutex(&m_Mutex);

    HANDLE PinHandle = NULL, FileHandle = NULL;

    RtlZeroMemory(&m_Format, sizeof(m_Format));
    RtlZeroMemory((PVOID)&m_RtBuffer, sizeof(m_RtBuffer));

    m_pPositionRegister = NULL;

    m_FifoSizeInFrames = 0;

    m_pDeviceObject = NULL;

    if (m_pPinFileObject != NULL)
    {
        ObDereferenceObject(m_pPinFileObject);
        m_pPinFileObject = NULL;
    }

    if (m_pFilterFileObject != NULL)
    {
        ObDereferenceObject(m_pFilterFileObject);
        m_pFilterFileObject = NULL;
    }

    if (m_PinHandle != NULL)
    {
        PinHandle = m_PinHandle;
        m_PinHandle = NULL;
    }

    if (m_FileHandle != NULL)
    {
        FileHandle = m_FileHandle;
        m_FileHandle = NULL;
    }

    m_bPinRunning = FALSE;
    m_bPinOpened = FALSE;

    ExReleaseFastMutex(&m_Mutex);

    if (PinHandle != NULL)
    {
        ZwClose(PinHandle);
    }

    if (FileHandle != NULL)
    {
        ZwClose(FileHandle);
    }

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::SetState(BOOLEAN running)
{
    KSPROPERTY ksProperty;
    KSSTATE state = running ? KSSTATE_RUN : KSSTATE_STOP;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;

    PAGED_CODE();
    ExAcquireFastMutex(&m_Mutex);

    if (m_bPinOpened == FALSE)
    {
        ExReleaseFastMutex(&m_Mutex);
        return STATUS_INVALID_DEVICE_STATE;
    }

    ExReleaseFastMutex(&m_Mutex);

    RtlZeroMemory(&ksProperty, sizeof(ksProperty));
    ksProperty.Set = KSPROPSETID_Connection;
    ksProperty.Id = KSPROPERTY_CONNECTION_STATE;
    ksProperty.Flags = KSPROPERTY_TYPE_SET;

    status = SendIoctl(IOCTL_KS_PROPERTY,
        &ksProperty,
        sizeof(ksProperty),
        &state,
        sizeof(state),
        &IoStatus,
        TRUE);


    if (NT_SUCCESS(status))
    {
        ExAcquireFastMutex(&m_Mutex);
        if (state == KSSTATE_RUN)
        {
            m_bPinRunning = TRUE;
        }
        else
        {
            m_bPinRunning = FALSE;
        }
        ExReleaseFastMutex(&m_Mutex);
    }

    return status;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::SendIoctl(
    _In_       ULONG IoControlCode,
    _In_opt_   PVOID InputBuffer,
    _In_       ULONG InputBufferLength,
    _Out_opt_  PVOID OutputBuffer,
    _In_       ULONG OutputBufferLength,
    _Out_      PIO_STATUS_BLOCK IoStatus,
    BOOL bPinRequest)
{
    PAGED_CODE();

    NTSTATUS status = STATUS_NO_MEMORY;

    ExAcquireFastMutex(&m_Mutex);

    KEVENT Event;
    PIO_STACK_LOCATION stackLoc;
    PFILE_OBJECT target = NULL;
    ASSERT(m_pDeviceObject != NULL);

    if (m_pDeviceObject == NULL)
    {
        ExReleaseFastMutex(&m_Mutex);
        return STATUS_NO_SUCH_DEVICE;
    }

    PDEVICE_OBJECT DeviceObject = m_pDeviceObject;
    ObReferenceObject(DeviceObject);

    RtlZeroMemory(IoStatus, sizeof(IO_STATUS_BLOCK));

    if (bPinRequest == TRUE)
    {
        target = m_pPinFileObject;
    }
    else
    {
        target = m_pFilterFileObject;
    }

    if (target == NULL)
    {
        ObDereferenceObject(DeviceObject);
        ExReleaseFastMutex(&m_Mutex);
        return STATUS_NO_SUCH_DEVICE;
    }

    ObReferenceObject(target);

    ExReleaseFastMutex(&m_Mutex);

    ASSERT(target != NULL);
    if (target == NULL)
    {
        ObDereferenceObject(DeviceObject);
        ObDereferenceObject(target);
        return STATUS_INVALID_DEVICE_STATE;
    }

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PIRP Irp = IoBuildDeviceIoControlRequest(IoControlCode,
        DeviceObject,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        FALSE,
        &Event,
        IoStatus);

    ASSERT(Irp != NULL);

    if (Irp != NULL)
    {
        Irp->RequestorMode = KernelMode;

        stackLoc = IoGetNextIrpStackLocation(Irp);

        stackLoc->FileObject = target;

        status = IoCallDriver(DeviceObject, Irp);
        if (STATUS_PENDING == status)
        {
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        }
        status = IoStatus->Status;
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }

    ObDereferenceObject(DeviceObject);
    ObDereferenceObject(target);

    return status;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::OpenWaveFilter(_In_ PUNICODE_STRING InterfaceName)
{
    PAGED_CODE();
    NTSTATUS status = STATUS_SUCCESS;

    OBJECT_ATTRIBUTES Attributes;
    IO_STATUS_BLOCK IoStatus;
    HANDLE FileHandle = NULL;
    PVOID FilterFileObject = NULL;

    ExAcquireFastMutex(&m_Mutex);

    if (m_pDeviceObject != NULL && m_pFilterFileObject != NULL && m_FileHandle != NULL)
    {
        ExReleaseFastMutex(&m_Mutex);
        return STATUS_SUCCESS;
    }

    ExReleaseFastMutex(&m_Mutex);

    // Open a handle to the device interface.

    InitializeObjectAttributes(&Attributes, InterfaceName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenFile(&FileHandle, FILE_SHARE_READ | FILE_SHARE_WRITE, &Attributes, &IoStatus, 0, FILE_NON_DIRECTORY_FILE);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    // Now add a reference to the filters's FILE_OBJECT.
    status = ObReferenceObjectByHandle(FileHandle,
        GENERIC_READ,
        *IoFileObjectType,
        KernelMode,
        &FilterFileObject,
        NULL);

    if (!NT_SUCCESS(status))
    {
        ClosePin();
        return status;
    }

    ExAcquireFastMutex(&m_Mutex);

    m_FileHandle = FileHandle;
    m_pFilterFileObject = (PFILE_OBJECT)FilterFileObject;

    if (m_pFilterFileObject->DeviceObject)
    {
        m_pDeviceObject = IoGetRelatedDeviceObject(m_pFilterFileObject);
        ExReleaseFastMutex(&m_Mutex);
    }
    else
    {
        ExReleaseFastMutex(&m_Mutex);
        ClosePin();
    }

    ASSERT(m_pDeviceObject != NULL);

    return status;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::Init(_In_ PUNICODE_STRING Name)
{
    PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;

    if (m_Name.Buffer == NULL)
    {
        m_Name.MaximumLength = Name->MaximumLength;
        m_Name.Length = m_Name.MaximumLength - sizeof(WCHAR);
        m_Name.Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPoolNx, Name->MaximumLength, 'mnPA');
        ASSERT(m_Name.Buffer != NULL);
        if (m_Name.Buffer != NULL)
        {
            RtlCopyMemory(m_Name.Buffer, Name->Buffer, Name->MaximumLength);
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }

    return status;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::OpenPin(_In_ PUNICODE_STRING InterfaceName, ULONG rate, ULONG numChannels, ULONG ValidBitsPerSample)
{
    PAGED_CODE();
    InitFormat(rate, numChannels, ValidBitsPerSample);
    return OpenPin(InterfaceName);
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::OpenPin(_In_ PUNICODE_STRING InterfaceName)
{
    PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;
    HANDLE PinHandle = NULL;
    PVOID PinFileObject = NULL;

    status = OpenWaveFilter(InterfaceName);

    if (!NT_SUCCESS(status))
    {
        ClosePin();
        return status;
    }

    // Now open a handle to the pin.
    // The pin name is a concatination of KSSTRING_Pin + L'\\' + a KSPIN_CONNECT + KSDATAFORMAT + WAVEFORMATEX.

    USHORT cbFormat = (USHORT)m_Format.DataFormat.FormatSize;
    USHORT cbKsPinCreate = sizeof(KSPIN_CONNECT) + cbFormat;
    KSPIN_CONNECT* KsPinCreate = (KSPIN_CONNECT*)ExAllocatePoolWithTag(NonPagedPoolNx, cbKsPinCreate, 'CKcU');

    ASSERT(KsPinCreate != NULL);
    if (KsPinCreate == NULL)
    {
        ClosePin();
        return STATUS_NO_MEMORY;
    }

    UNICODE_STRING PinName;
    PinName.MaximumLength = PinName.Length = cbKsPinCreate + sizeof(WCHAR) + sizeof(KSSTRING_Pin);
    PinName.Buffer = (PWCH)ExAllocatePoolWithTag(NonPagedPoolNx, PinName.MaximumLength, 'niPK');
    if (PinName.Buffer == NULL)
    {
        ExFreePool(KsPinCreate);
        ClosePin();
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(KsPinCreate, cbKsPinCreate);

    // Set the Connection structure.
    KsPinCreate->Interface.Set = KSINTERFACESETID_Standard;
    KsPinCreate->Interface.Id = m_bIsUsbDevice ? KSINTERFACE_STANDARD_STREAMING : KSINTERFACE_STANDARD_LOOPED_STREAMING;
    KsPinCreate->Interface.Flags = 0;

    KsPinCreate->Medium.Set = KSMEDIUMSETID_Standard;
    KsPinCreate->Medium.Id = KSMEDIUM_TYPE_ANYINSTANCE;
    KsPinCreate->Medium.Flags = 0;

    status = FindDataSinkPin(&KsPinCreate->PinId);

    if (!NT_SUCCESS(status))
    {
        ExFreePool(KsPinCreate);
		ExFreePool(PinName.Buffer);
        ClosePin();
        return STATUS_NO_MEMORY;
    }

    KsPinCreate->PinToHandle = 0;

    KsPinCreate->Priority.PriorityClass = KSPRIORITY_NORMAL;
    KsPinCreate->Priority.PrioritySubClass = KSPRIORITY_NORMAL;

    // Set Format structure. (Connection structure is followed by format)
    KSDATAFORMAT_WAVEFORMATEXTENSIBLE* KsFormat;
    KsFormat = (KSDATAFORMAT_WAVEFORMATEXTENSIBLE*)(KsPinCreate + 1);

    RtlCopyMemory(KsFormat, &m_Format, cbFormat);

    RtlCopyMemory(PinName.Buffer, KSSTRING_Pin, sizeof(KSSTRING_Pin));
    RtlCopyMemory((PUCHAR)PinName.Buffer + sizeof(KSSTRING_Pin), KsPinCreate, cbKsPinCreate);
    PinName.Buffer[(sizeof(KSSTRING_Pin) / sizeof(WCHAR)) - 1] = L'\\';

    OBJECT_ATTRIBUTES Attributes;
    IO_STATUS_BLOCK IoStatus;

    InitializeObjectAttributes(&Attributes, &PinName, 
                               OBJ_INHERIT | OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_KERNEL_HANDLE, m_FileHandle, NULL);

    status = ZwCreateFile(&PinHandle,
        0xC0000000,
        &Attributes,
        &IoStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_OPEN,
        FILE_SESSION_AWARE,
        0x40000,
        NULL,
        0);

    ExFreePool(PinName.Buffer);
    ExFreePool(KsPinCreate);

    if (!NT_SUCCESS(status))
    {
        ClosePin();
        return status;
    }

    // Now add a reference to the pin's FILE_OBJECT.
    status = ObReferenceObjectByHandle(PinHandle,
        GENERIC_READ,
        *IoFileObjectType,
        KernelMode,
        (PVOID*)&PinFileObject,
        NULL);

    ASSERT(PinFileObject != NULL);

    if (!NT_SUCCESS(status))
    {
        ClosePin();
    }
    else
    {
        ExAcquireFastMutex(&m_Mutex);
        m_bPinOpened = TRUE;
        m_PinHandle = PinHandle;
        m_pPinFileObject = (PFILE_OBJECT)PinFileObject;
        ExReleaseFastMutex(&m_Mutex);
    }

    return status;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::GetPinPropertySimple
(
    ULONG   nPinID,
    REFGUID guidPropertySet,
    ULONG   nProperty,
    _Out_writes_bytes_(cbValue) PVOID   pvValue,
    ULONG   cbValue
)
{
    KSP_PIN KsPProp;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;

    PAGED_CODE();

    KsPProp.Property.Set = guidPropertySet;
    KsPProp.Property.Id = nProperty;
    KsPProp.Property.Flags = KSPROPERTY_TYPE_GET;
    KsPProp.PinId = nPinID;
    KsPProp.Reserved = 0;

    status =  SendIoctl(IOCTL_KS_PROPERTY, &KsPProp, sizeof(KSP_PIN), pvValue, cbValue, &IoStatus, FALSE);

    if (!NT_SUCCESS(status))
    {
        return status;
    }
    else
    {
        return IoStatus.Status;
    }
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::FindDataSinkPin(_Out_ ULONG * PinId)
{
    // send KSPROPERTY_PIN_CTYPES to get count
    // for each pin, send a KSPROPERTY_PIN_COMMUNICATION until we find a SINK

    ULONG pinCount, i;
    NTSTATUS status;
    KSPIN_COMMUNICATION communication;

    PAGED_CODE();

	*PinId = ULONG_MAX;

    status = GetPinPropertySimple(0, KSPROPSETID_Pin, KSPROPERTY_PIN_CTYPES, &pinCount, sizeof(pinCount));

    if (NT_SUCCESS(status))
    {
        for (i = 0; i < pinCount; i++)
        {
            status = GetPinPropertySimple(i, KSPROPSETID_Pin, KSPROPERTY_PIN_COMMUNICATION, 
                                          &communication, sizeof(communication));

            if (communication == KSPIN_COMMUNICATION_SINK || communication == KSPIN_COMMUNICATION_BOTH)
            {
                *PinId = i;
                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_NO_SUCH_DEVICE;
}



#pragma code_seg("PAGE")
void KsPin::InitFormat(ULONG rate, ULONG numChannels, ULONG ValidBitsPerSample)
{
    PAGED_CODE();

    RtlZeroMemory(&m_Format, sizeof(m_Format));
    ASSERT(ValidBitsPerSample == 8 || ValidBitsPerSample == 16 || ValidBitsPerSample == 24 || ValidBitsPerSample == 32);

    m_Format.WaveFormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    m_Format.WaveFormatExt.Format.nChannels = (WORD)numChannels;
    m_Format.WaveFormatExt.Format.nSamplesPerSec = rate;
    m_Format.WaveFormatExt.Format.wBitsPerSample = (WORD)(ValidBitsPerSample == 24 ? 32 : ValidBitsPerSample);
    m_Format.WaveFormatExt.Format.nBlockAlign = (WORD)(m_Format.WaveFormatExt.Format.nChannels 
                                                       * m_Format.WaveFormatExt.Format.wBitsPerSample / 8);
    m_Format.WaveFormatExt.Format.nAvgBytesPerSec = rate * m_Format.WaveFormatExt.Format.nBlockAlign;
    m_Format.WaveFormatExt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
    m_Format.WaveFormatExt.Samples.wValidBitsPerSample = (WORD)ValidBitsPerSample;

    m_Format.DataFormat.FormatSize = sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE);
    m_Format.DataFormat.Flags = 0;
    m_Format.DataFormat.SampleSize = m_Format.WaveFormatExt.Format.nBlockAlign;
    m_Format.DataFormat.Reserved = 0;
    m_Format.DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
    m_Format.DataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    m_Format.DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

    if (m_Format.WaveFormatExt.Format.nChannels == 8)
    {
        m_Format.WaveFormatExt.dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND;
    }
    else if (m_Format.WaveFormatExt.Format.nChannels == 6)
    {
        m_Format.WaveFormatExt.dwChannelMask = KSAUDIO_SPEAKER_5POINT1_SURROUND;
    }
    else if (m_Format.WaveFormatExt.Format.nChannels == 2)
    {
        m_Format.WaveFormatExt.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    }
    else if (m_Format.WaveFormatExt.Format.nChannels == 1)
    {
        m_Format.WaveFormatExt.dwChannelMask = KSAUDIO_SPEAKER_MONO;
    }

    m_Format.WaveFormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
}

#pragma code_seg()
ULONG KsPin::GetBufferSize()
{
    return m_RtBuffer.ActualBufferSize;
}

#pragma code_seg()
PBYTE KsPin::GetWritePosition()
{
    return ((PBYTE)m_RtBuffer.BufferAddress + *m_pPositionRegister);
}

#pragma code_seg()
ULONG KsPin::GetNumChannels()
{
    return m_Format.WaveFormatExt.Format.nChannels;
}

#pragma code_seg()
ULONG KsPin::GetBlockAlign()
{
    return m_Format.WaveFormatExt.Format.nBlockAlign;
}

#pragma code_seg()
ULONG KsPin::GetCurrentPadding()
{
    return GetPadding(*m_pPositionRegister);
}

#pragma code_seg()
ULONG KsPin::GetPadding(ULONG Position)
{
    if (m_pPositionRegister == NULL || m_bPinRunning == FALSE)
    {
        return 0;
    }

    ULONG CurrentReadPosition = *m_pPositionRegister;
    ULONG NumFrames = 0;

    // Read pointer ahead of write pointer?
    if (CurrentReadPosition < Position)
    {
        // Write to the end of the buffer.
        NumFrames = (UINT32)(m_RtBuffer.ActualBufferSize - Position) / m_Format.WaveFormatExt.Format.nBlockAlign;
    }
    // Write pointer ahead of read pointer?
    else if (CurrentReadPosition > Position)
    {
        // Write up to the CurrentReadPosition
        NumFrames = (CurrentReadPosition - Position) / m_Format.WaveFormatExt.Format.nBlockAlign;

        //
        // Adjust for FifoSize.
        //

        if (NumFrames > m_FifoSizeInFrames)
        {
            NumFrames -= m_FifoSizeInFrames;
        }
        else
        {
            NumFrames = 0;
        }
    }

    return NumFrames;
}

#pragma code_seg("PAGE")
NTSTATUS KsPin::InitRtBuffer(ULONG size)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(size);

    ASSERT(0);
        
    return STATUS_NOT_SUPPORTED;
}


#pragma code_seg()
void KsPin::SetMuted(BOOLEAN bMuted)
{
    m_bMuted = bMuted;

    if (m_bMuted)
    {
        if (m_RtBuffer.BufferAddress)
        {
            // Clear the buffer        
            RtlZeroMemory((PBYTE)m_RtBuffer.BufferAddress, m_RtBuffer.ActualBufferSize);

            if (m_RtBuffer.CallMemoryBarrier == TRUE)
            {
                KeMemoryBarrier();
            }
        }
    }
}
