//
// Copyright (C) Microsoft Corporation. All rights reserved.
//

#include "VirtualAudioMicArray.h"



#pragma code_seg("PAGE")
UsbAudioPin::UsbAudioPin(_In_ VirtualPin* virtualPin)
    : KsPin(virtualPin, TRUE),
    m_WritePosition(0),
    m_cRequests(-1),
    m_ThreadHandle(NULL),
    m_bThreadShutdown(FALSE)
{
    ULONG i;
    PAGED_CODE();

    for (i = 0; i < cPingPongIrps; i++)
    {
        RtlZeroMemory(&m_KsStreamHeader[i], sizeof(m_KsStreamHeader[i]));
        KeInitializeEvent(&m_IoctlCompleteEvent[i], NotificationEvent, FALSE);
        m_rgIoctlCompleteEvents[i] = &m_IoctlCompleteEvent[i];
        m_pBuffer[i] = NULL;
    }
    
}

#pragma code_seg("PAGE")
UsbAudioPin::~UsbAudioPin()
{
    PAGED_CODE();

    ClosePin();
}



#pragma code_seg("PAGE")
NTSTATUS UsbAudioPin::InitRtBuffer(ULONG size)
{
    NTSTATUS status = STATUS_NO_MEMORY;
    PVOID buffer;
    HANDLE ThreadHandle;
    ULONG i;

    PAGED_CODE();

    //
    // Allocate a 'mock' realtime audio buffer.
    //

    buffer = ExAllocatePoolWithTag(NonPagedPoolNx, size, 'rBbU');

    if (buffer != NULL)
    {
        m_cbPingPongBufferSize = size / cPingPongIrps;

        ASSERT((size % cPingPongIrps) == 0);

        for (i = 0; i < cPingPongIrps; i++)
        {
            m_pBuffer[i] = (PBYTE)buffer + i * m_cbPingPongBufferSize;
        }

        m_RtBuffer.BufferAddress = buffer;
        m_RtBuffer.ActualBufferSize = size;
        m_RtBuffer.CallMemoryBarrier = FALSE;

        //
        // Setup m_pPositionRegister.
        //

        m_pPositionRegister = &m_WritePosition;

        status = STATUS_SUCCESS;


        //
        // USB audio doesn't have a FIFO in the RtAudio sense, so this is zero.
        //

        m_FifoSizeInFrames = 0;
        m_bThreadShutdown = FALSE;

        status = PsCreateSystemThread(&ThreadHandle,
            THREAD_ALL_ACCESS,
            NULL,
            NULL,
            NULL,
            &WorkerThread,
            this);

        if (NT_SUCCESS(status))
        {
            status = ObReferenceObjectByHandle(
                ThreadHandle,
                THREAD_ALL_ACCESS,
                NULL,
                KernelMode,
                (PVOID*)&m_ThreadHandle,
                NULL);
        }

        ZwClose(ThreadHandle);
    }
    return status;
}

#pragma code_seg("PAGE")
void
UsbAudioPin::WorkerThread(_In_ PVOID Context)
{
    PAGED_CODE();

    UsbAudioPin* me = (UsbAudioPin*)Context;
    me->DoThreadWork();
}

#pragma code_seg("PAGE")
void
UsbAudioPin::DoThreadWork()
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    PAGED_CODE();

    status = SetState(TRUE);
    ASSERT(NT_SUCCESS(status));

    if (!NT_SUCCESS(status))
    {
        return;
    }

    //
    // Queue up the first set of ping pong irps.
    //
    for (i = 0; i < cPingPongIrps && NT_SUCCESS(status); i++)
    {
        status = SendReadStreamIoctl();
        ASSERT(NT_SUCCESS(status));
    }

    if (!NT_SUCCESS(status))
    {
        return;
    }

    // 
    // Now wait for the irps to complete, and requeue each as each irp is completed.
    // 
    while (!m_bThreadShutdown)
    {

        status = KeWaitForMultipleObjects(cPingPongIrps, (PVOID*)&m_rgIoctlCompleteEvents, WAIT_TYPE::WaitAny, KWAIT_REASON::Executive, KernelMode, FALSE, NULL, NULL);

        if (m_bThreadShutdown || m_bPinRunning == FALSE)
        {
            break;
        }

        i = (ULONG) status;
        if (i < cPingPongIrps && NT_SUCCESS(m_IoStatusBlock[i].Status))
        {
            ASSERT(m_KsStreamHeader[i].OptionsFlags == 0 || m_KsStreamHeader[i].OptionsFlags == KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY);
            UpdateWritePosition(m_KsStreamHeader[i].DataUsed / m_Format.WaveFormatExt.Format.nBlockAlign);

            status = SendReadStreamIoctl();
        }

    } //end of while not shutting down

    
}

#pragma code_seg("PAGE")
NTSTATUS UsbAudioPin::SendReadStreamIoctl()
{
    ULONG i = InterlockedIncrement(&m_cRequests) % cPingPongIrps;
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION stackLoc;

    PAGED_CODE();

    RtlZeroMemory(&m_KsStreamHeader[i], sizeof(KSSTREAM_HEADER));

    m_KsStreamHeader[i].Data = m_pBuffer[i];
    m_KsStreamHeader[i].FrameExtent = m_cbPingPongBufferSize;
    m_KsStreamHeader[i].DataUsed = 0;
    m_KsStreamHeader[i].Size = sizeof(KSSTREAM_HEADER);
    m_KsStreamHeader[i].OptionsFlags = 0;
    m_KsStreamHeader[i].PresentationTime.Time = 0;
    m_KsStreamHeader[i].PresentationTime.Numerator = 1;
    m_KsStreamHeader[i].PresentationTime.Denominator = 1;

    KeClearEvent(&m_IoctlCompleteEvent[i]);

    irp = IoBuildDeviceIoControlRequest(IOCTL_KS_READ_STREAM, m_pDeviceObject,
        NULL, 0, &m_KsStreamHeader[i], sizeof(KSSTREAM_HEADER), FALSE, &m_IoctlCompleteEvent[i],
        &m_IoStatusBlock[i]);

    ASSERT(irp != NULL);
    if (irp != NULL)
    {
        stackLoc = IoGetNextIrpStackLocation(irp);
        stackLoc->FileObject = m_pPinFileObject;
        
        m_PingPongIrps[i] = irp;
        status = IoCallDriver(m_pDeviceObject, irp);

        ASSERT(NT_SUCCESS(status));
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }

    return status;
}

#pragma code_seg("PAGE")
NTSTATUS UsbAudioPin::ClosePin()
{
    ULONG i;
    PAGED_CODE();

    m_bThreadShutdown = TRUE;
    for (i = 0; i < cPingPongIrps; i++)
    {
        KeSetEvent(&m_IoctlCompleteEvent[i], IO_NO_INCREMENT, FALSE);
    }

    if (m_ThreadHandle != NULL)
    {
        KeWaitForSingleObject(m_ThreadHandle, Executive, KernelMode, FALSE, NULL);
        ObDereferenceObject(m_ThreadHandle);
        m_ThreadHandle = NULL;
    }

    for (i = 0; i < cPingPongIrps; i++)
    {
        m_pBuffer[i] = NULL;
    }

    if (m_RtBuffer.BufferAddress != NULL)
    {
        ExFreePool(m_RtBuffer.BufferAddress);
        m_RtBuffer.BufferAddress = NULL;
        m_RtBuffer.ActualBufferSize = 0;
    }

    return KsPin::ClosePin();
}

#pragma code_seg()
void UsbAudioPin::UpdateWritePosition(ULONG NumFrames)
{
    ULONG newPosition;
    ASSERT(NumFrames * m_Format.WaveFormatExt.Format.nChannels <= m_RtBuffer.ActualBufferSize);

    newPosition = m_WritePosition + NumFrames * m_Format.WaveFormatExt.Format.nBlockAlign;

    ASSERT(newPosition <= m_RtBuffer.ActualBufferSize);

    if (newPosition >= m_RtBuffer.ActualBufferSize)
    {
        newPosition = 0;
    }

    m_WritePosition = newPosition;
}
