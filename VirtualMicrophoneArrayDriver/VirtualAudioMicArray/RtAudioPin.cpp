//
// Copyright (C) Microsoft Corporation. All rights reserved.
//

#include "VirtualAudioMicArray.h"

#include "RtAudioPin.h"
#include <intrin.h>


#pragma code_seg("PAGE")
RtAudioPin::RtAudioPin(_In_ VirtualPin* virtualPin)
    : KsPin(virtualPin, FALSE),
    m_BufferMdl(NULL),
    m_PositionPointerMdl(NULL)
{
    PAGED_CODE();
}

#pragma code_seg("PAGE")
RtAudioPin::~RtAudioPin()
{
    PAGED_CODE();

    ClosePin();
}

#pragma code_seg("PAGE")
NTSTATUS RtAudioPin::ClosePin()
{
    PAGED_CODE();

    if (m_BufferMdl != NULL)
    {
        MmUnlockPages(m_BufferMdl);
        IoFreeMdl(m_BufferMdl);
        m_BufferMdl = NULL;
    }
    if (m_PositionPointerMdl != NULL)
    {
        MmUnlockPages(m_PositionPointerMdl);
        IoFreeMdl(m_PositionPointerMdl);
        m_PositionPointerMdl = NULL;
    }
    return KsPin::ClosePin();
}


#pragma code_seg("PAGE")
NTSTATUS RtAudioPin::InitRtBuffer(ULONG size)
{
    KSRTAUDIO_BUFFER_PROPERTY_WITH_NOTIFICATION RtAudioProperty = { 0 };
    KSRTAUDIO_NOTIFICATION_EVENT_PROPERTY RtNotificationProperty = { 0 };
    KSRTAUDIO_HWREGISTER_PROPERTY HwRegProperty = { 0 };
    KSRTAUDIO_HWREGISTER HwRegister = { 0 };
    IO_STATUS_BLOCK StatusBlock = { 0 };
    KSPROPERTY KsProperty;
    KSRTAUDIO_HWLATENCY Latency = { 0 };
    NTSTATUS status;

    PAGED_CODE();
    ASSERT(m_BufferMdl == NULL);
    ASSERT(m_PositionPointerMdl == NULL);

    //
    // Allocate a realtime audio buffer
    //
    RtAudioProperty.Property.Set = KSPROPSETID_RtAudio;
    RtAudioProperty.Property.Id = KSPROPERTY_RTAUDIO_BUFFER_WITH_NOTIFICATION;
    RtAudioProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    RtAudioProperty.BaseAddress = NULL;
    RtAudioProperty.RequestedBufferSize = size;
    RtAudioProperty.NotificationCount = 2;

    status = SendIoctl(IOCTL_KS_PROPERTY,
        &RtAudioProperty,
        sizeof(RtAudioProperty),
        &m_RtBuffer,
        sizeof(m_RtBuffer),
        &StatusBlock,
        TRUE);

    ASSERT(NT_SUCCESS(status));
    ASSERT(NT_SUCCESS(StatusBlock.Status));
    ASSERT(StatusBlock.Information == sizeof(m_RtBuffer));

    if (!NT_SUCCESS(status))
    {
        RtlZeroMemory(&m_RtBuffer, sizeof(m_RtBuffer));
        ClosePin();
        return status;
    }

	//
	// Portcls.sys assumes that the RtAudio buffer will be accessed via user
	// mode and we need a kernel mode mapping (a.k.a. 'system address') in order to access the buffer
	// in an arbitrary thread context via the timer DPC.
	//
    if (m_RtBuffer.BufferAddress)
    {
        m_BufferMdl = IoAllocateMdl(m_RtBuffer.BufferAddress, m_RtBuffer.ActualBufferSize, FALSE, FALSE, NULL);
        ASSERT(m_BufferMdl != NULL);

        if (m_BufferMdl != NULL)
        {
            MmProbeAndLockPages(m_BufferMdl, KernelMode, IoModifyAccess);
            m_RtBuffer.BufferAddress = MmGetSystemAddressForMdlSafe(m_BufferMdl, NormalPagePriority | MdlMappingNoExecute);
        }
        else
        {
            ClosePin();
            return STATUS_NO_MEMORY;
        }
    }

    //
    // Setup m_pPositionRegister.
    //
    HwRegProperty.Property.Set = KSPROPSETID_RtAudio;
    HwRegProperty.Property.Id = KSPROPERTY_RTAUDIO_POSITIONREGISTER;
    HwRegProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    HwRegProperty.BaseAddress = NULL;
    
    status = SendIoctl(IOCTL_KS_PROPERTY,
        &HwRegProperty,
        sizeof(HwRegProperty),
        &HwRegister,
        sizeof(HwRegister),
        &StatusBlock,
        TRUE);

    ASSERT(NT_SUCCESS(status));
    ASSERT(NT_SUCCESS(StatusBlock.Status));
    ASSERT(StatusBlock.Information == sizeof(HwRegister));

    if (!NT_SUCCESS(status))
    {
        RtlZeroMemory(&m_RtBuffer, sizeof(m_RtBuffer));
        ClosePin();
        return status;
    }

    ASSERT(HwRegister.Width == 32);
    m_PositionPointerMdl = IoAllocateMdl(HwRegister.Register, HwRegister.Width / 8, FALSE, FALSE, NULL);
    ASSERT(m_PositionPointerMdl != NULL);

    if (m_PositionPointerMdl != NULL)
    {
        MmProbeAndLockPages(m_PositionPointerMdl, KernelMode, IoModifyAccess);
        m_pPositionRegister = (PULONG) MmGetSystemAddressForMdlSafe(m_PositionPointerMdl, 
                                                                    NormalPagePriority | MdlMappingNoExecute);
    }
    else
    {
        ClosePin();
        return STATUS_NO_MEMORY;
    }


    //
    // Calculate m_FifoSizeInFrames.
    //
    if (NT_SUCCESS(status))
    {
        KsProperty.Set = KSPROPSETID_RtAudio;
        KsProperty.Id = KSPROPERTY_RTAUDIO_HWLATENCY;
        KsProperty.Flags = KSPROPERTY_TYPE_GET;

        status = SendIoctl(IOCTL_KS_PROPERTY,
            &KsProperty,
            sizeof(KsProperty),
            &Latency,
            sizeof(Latency),
            &StatusBlock,
            TRUE);

        ASSERT(NT_SUCCESS(status));
        ASSERT(NT_SUCCESS(StatusBlock.Status));
        ASSERT(StatusBlock.Information == sizeof(Latency));
    }

    if (NT_SUCCESS(status))
    {
        WORD BlockAlign = m_Format.WaveFormatExt.Format.nBlockAlign;
        // Make sure we round up instead of down
        m_FifoSizeInFrames = (Latency.FifoSize + BlockAlign - 1) / BlockAlign;
    }
    else
    {
        RtlZeroMemory(&m_RtBuffer, sizeof(m_RtBuffer));
        ClosePin();
        return status;
    }

    return status;
}




