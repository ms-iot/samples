//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
#include "VirtualAudioMicArray.h"
#include <limits.h>
#include <ks.h>
#include "simple.h"
#include "minwavert.h"
#include "minwavertstream.h"

#define MINWAVERTSTREAM_POOLTAG 'SRWM'

#pragma warning (disable : 4127)

//=============================================================================
// CMiniportWaveRTStream
//=============================================================================

//=============================================================================
#pragma code_seg("PAGE")
CMiniportWaveRTStream::~CMiniportWaveRTStream
( 
    void 
)
/*++

Routine Description:

  Destructor for wavertstream 

Arguments:

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();
    if (NULL != m_pMiniport)
    {
        if (m_bUnregisterStream)
        {
            m_pMiniport->StreamClosed(m_ulPin, this);
            m_bUnregisterStream = FALSE;
        }
        
        m_pMiniport->Release();
        m_pMiniport = NULL;
    }

    if (m_pDpc)
    {
        ExFreePoolWithTag( m_pDpc, MINWAVERTSTREAM_POOLTAG );
        m_pDpc = NULL;
    }

    if (m_pTimer)
    {
        ExFreePoolWithTag( m_pTimer, MINWAVERTSTREAM_POOLTAG );
        m_pTimer = NULL;
    }

    if (m_plVolumeLevel)
    {
        ExFreePoolWithTag( m_plVolumeLevel, MINWAVERTSTREAM_POOLTAG );
        m_plVolumeLevel = NULL;
    }
    
    if (m_pWfExt)
    {
        ExFreePoolWithTag( m_pWfExt, MINWAVERTSTREAM_POOLTAG );
        m_pWfExt = NULL;
    }
    if (m_pNotificationTimer)
    {
        KeCancelTimer(m_pNotificationTimer);
        ExFreePoolWithTag(m_pNotificationTimer, MINWAVERTSTREAM_POOLTAG);
    }

    // Since we just cancelled the notification timer, wait for all queued 
    // DPCs to complete before we free the notification DPC.
    //
    KeFlushQueuedDpcs();

    if (m_pNotificationDpc)
    {
        ExFreePoolWithTag( m_pNotificationDpc, MINWAVERTSTREAM_POOLTAG );
    }
    
    DPF_ENTER(("[CMiniportWaveRTStream::~CMiniportWaveRTStream]"));
} // ~CMiniportWaveRTStream

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRTStream::Init
( 
    _In_ PCMiniportWaveRT           Miniport_,
    _In_ PPORTWAVERTSTREAM          PortStream_,
    _In_ ULONG                      Pin_,
    _In_ BOOLEAN                    Capture_,
    _In_ PKSDATAFORMAT              DataFormat_,
    _In_ GUID                       SignalProcessingMode
)
/*++

Routine Description:

  Initializes the stream object.

Arguments:

  Miniport_ -

  Pin_ -

  Capture_ -

  DataFormat -

  SignalProcessingMode - The driver uses the signalProcessingMode to configure
    driver and/or hardware specific signal processing to be applied to this new
    stream.

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    PWAVEFORMATEX pWfEx = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(SignalProcessingMode);

    m_pMiniport = NULL;
    m_ulPin = 0;
    m_bUnregisterStream = FALSE;
    m_bCapture = FALSE;
    m_ulDmaBufferSize = 0;
    m_pDmaBuffer = NULL;
    m_ulNotificationsPerBuffer = 0;
    m_KsState = KSSTATE_STOP;
    m_pTimer = NULL;
    m_pDpc = NULL;
    m_ullPlayPosition = 0;
    m_ullWritePosition = 0;
    m_plVolumeLevel = NULL;
    m_pWfExt = NULL;
  

    m_pAdapterCommon = Miniport_->GetAdapterCommObj();

    m_pPortStream = PortStream_;
    InitializeListHead(&m_NotificationList);
    m_ulNotificationIntervalMs = 0;

    m_pNotificationDpc = (PRKDPC)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        sizeof(KDPC),
        MINWAVERTSTREAM_POOLTAG);
    if (!m_pNotificationDpc)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    m_pNotificationTimer = (PKTIMER)ExAllocatePoolWithTag(
        NonPagedPoolNx,
        sizeof(KTIMER),
        MINWAVERTSTREAM_POOLTAG);
    if (!m_pNotificationTimer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeDpc(m_pNotificationDpc, TimerNotifyRT, this);
    KeInitializeTimerEx(m_pNotificationTimer, NotificationTimer);

    pWfEx = GetWaveFormatEx(DataFormat_);
    if (NULL == pWfEx) 
    { 
        return STATUS_UNSUCCESSFUL; 
    }

    m_pMiniport = reinterpret_cast<CMiniportWaveRT*>(Miniport_);
    if (m_pMiniport == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }
    m_pMiniport->AddRef();
    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }
    m_ulPin = Pin_;
    m_bCapture = Capture_;
    
    m_pDpc = (PRKDPC)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(KDPC), MINWAVERTSTREAM_POOLTAG);
    if (!m_pDpc)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    m_pWfExt = (PWAVEFORMATEXTENSIBLE)ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(WAVEFORMATEX) + pWfEx->cbSize, MINWAVERTSTREAM_POOLTAG);
    if (m_pWfExt == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(m_pWfExt, pWfEx, sizeof(WAVEFORMATEX) + pWfEx->cbSize);

    m_plVolumeLevel = (PLONG)ExAllocatePoolWithTag(NonPagedPoolNx, m_pWfExt->Format.nChannels * sizeof(LONG), MINWAVERTSTREAM_POOLTAG);
    if (m_plVolumeLevel == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(m_plVolumeLevel, m_pWfExt->Format.nChannels * sizeof(LONG));

    //
    // Register this stream.
    //
    ntStatus = m_pMiniport->StreamCreated(m_ulPin, this);
    if (NT_SUCCESS(ntStatus))
    {
        m_bUnregisterStream = TRUE;
    }

    return ntStatus;
} // Init

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CMiniportWaveRTStream::NonDelegatingQueryInterface
( 
    _In_ REFIID  Interface,
    _COM_Outptr_ PVOID * Object 
)
/*++

Routine Description:

  QueryInterface

Arguments:

  Interface - GUID

  Object - interface pointer to be returned

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTWAVERTSTREAM(this)));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportWaveRTStream))
    {
        *Object = PVOID(PMINIPORTWAVERTSTREAM(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportWaveRTStreamNotification))
    {
        *Object = PVOID(PMINIPORTWAVERTSTREAMNOTIFICATION(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
} // NonDelegatingQueryInterface

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::AllocateBufferWithNotification
(
    _In_    ULONG               NotificationCount_,
    _In_    ULONG               RequestedSize_,
    _Out_   PMDL                *AudioBufferMdl_,
    _Out_   ULONG               *ActualSize_,
    _Out_   ULONG               *OffsetFromFirstPage_,
    _Out_   MEMORY_CACHING_TYPE *CacheType_
)
{
    PAGED_CODE();

    ULONG ulBufferDurationMs = 0;
    NTSTATUS ntStatus;

    if ( (0 == RequestedSize_) || (RequestedSize_ < m_pWfExt->Format.nBlockAlign) )
    { 
        return STATUS_UNSUCCESSFUL; 
    }
    
    if ((NotificationCount_ == 0) || (RequestedSize_ % NotificationCount_ != 0))
    {
        return STATUS_INVALID_PARAMETER;
    }

    RequestedSize_ -= RequestedSize_ % (m_pWfExt->Format.nBlockAlign);

    ROUND_TO_PAGES(RequestedSize_);
    
    PHYSICAL_ADDRESS highAddress;
    highAddress.HighPart = 0;
    highAddress.LowPart = MAXULONG;

    PMDL pBufferMdl = m_pPortStream->AllocatePagesForMdl (highAddress, RequestedSize_);

    if (NULL == pBufferMdl)
    {
        return STATUS_UNSUCCESSFUL;
    }

    m_pDmaBuffer = (BYTE*)m_pPortStream->MapAllocatedPages(pBufferMdl, MmCached);
    m_ulNotificationsPerBuffer = NotificationCount_;
    m_ulDmaBufferSize = RequestedSize_;
    ulBufferDurationMs = (RequestedSize_ * 1000) / m_pWfExt->Format.nAvgBytesPerSec;
    m_ulNotificationIntervalMs = ulBufferDurationMs / NotificationCount_;

    ntStatus = m_pAdapterCommon->RegisterStream(this);

    if (NT_SUCCESS(ntStatus))
    {
        *AudioBufferMdl_ = pBufferMdl;
        *ActualSize_ = RequestedSize_;
        *OffsetFromFirstPage_ = 0;
        *CacheType_ = MmCached;
    }
    else
    {
        m_pPortStream->FreePagesFromMdl(pBufferMdl);
        m_ulDmaBufferSize = 0;
        m_pDmaBuffer = NULL;
        DPF(D_ERROR, ("[CMiniportWaveRTStream::AllocateBufferWithNotification] failed to register stream with CSoc class."));
    }

    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
VOID CMiniportWaveRTStream::FreeBufferWithNotification
(
    _In_        PMDL    Mdl_,
    _In_        ULONG   Size_
)
{
    UNREFERENCED_PARAMETER(Size_);

    PAGED_CODE();

    m_pAdapterCommon->UnregisterStream(this);
    if (Mdl_ != NULL)
    {
        if (m_pDmaBuffer != NULL)
        {
            m_pPortStream->UnmapAllocatedPages(m_pDmaBuffer, Mdl_);
            m_pDmaBuffer = NULL;
        }
        
        m_pPortStream->FreePagesFromMdl(Mdl_);
    }
    
    m_ulDmaBufferSize = 0;
    m_ulNotificationsPerBuffer = 0;

    return;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::RegisterNotificationEvent
(
    _In_ PKEVENT NotificationEvent_
)
{
    UNREFERENCED_PARAMETER(NotificationEvent_);

    PAGED_CODE();

    NotificationListEntry *nleNew = (NotificationListEntry*)ExAllocatePoolWithTag( 
        NonPagedPoolNx,
        sizeof(NotificationListEntry),
        MINWAVERTSTREAM_POOLTAG);
    if (NULL == nleNew)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nleNew->NotificationEvent = NotificationEvent_;

    if (!IsListEmpty(&m_NotificationList))
    {
        PLIST_ENTRY leCurrent = m_NotificationList.Flink;
        while (leCurrent != &m_NotificationList)
        {
            NotificationListEntry* nleCurrent = CONTAINING_RECORD( leCurrent, NotificationListEntry, ListEntry);
            if (nleCurrent->NotificationEvent == NotificationEvent_)
            {
                RemoveEntryList( leCurrent );
                ExFreePoolWithTag( nleNew, MINWAVERTSTREAM_POOLTAG );
                return STATUS_UNSUCCESSFUL;
            }

            leCurrent = leCurrent->Flink;
        }
    }

    InsertTailList(&m_NotificationList, &(nleNew->ListEntry));
    
    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::UnregisterNotificationEvent
(
    _In_ PKEVENT NotificationEvent_
)
{
    UNREFERENCED_PARAMETER(NotificationEvent_);

    PAGED_CODE();

    if (!IsListEmpty(&m_NotificationList))
    {
        PLIST_ENTRY leCurrent = m_NotificationList.Flink;
        while (leCurrent != &m_NotificationList)
        {
            NotificationListEntry* nleCurrent = CONTAINING_RECORD( leCurrent, NotificationListEntry, ListEntry);
            if (nleCurrent->NotificationEvent == NotificationEvent_)
            {
                RemoveEntryList( leCurrent );
                ExFreePoolWithTag( nleCurrent, MINWAVERTSTREAM_POOLTAG );
                return STATUS_SUCCESS;
            }

            leCurrent = leCurrent->Flink;
        }
    }

    return STATUS_NOT_FOUND;
}


//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::GetClockRegister
(
    _Out_ PKSRTAUDIO_HWREGISTER Register_
)
{
    UNREFERENCED_PARAMETER(Register_);

    PAGED_CODE();

    return STATUS_NOT_IMPLEMENTED;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::GetPositionRegister
(
    _Out_ PKSRTAUDIO_HWREGISTER Register_
)
{
    PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveRTStream::GetPositionRegister]"));

    Register_->Register = &m_ullWritePosition;
    Register_->Width = 32; // bits.
    Register_->Accuracy = 15 * m_pWfExt->Format.nBlockAlign;

    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
VOID CMiniportWaveRTStream::GetHWLatency
(
    _Out_ PKSRTAUDIO_HWLATENCY  Latency_
)
{
    PAGED_CODE();

    ASSERT(Latency_);

    Latency_->ChipsetDelay = 0;
    Latency_->CodecDelay = 0;
    Latency_->FifoSize = 0;
}

//=============================================================================
#pragma code_seg("PAGE")
VOID CMiniportWaveRTStream::FreeAudioBuffer
(
_In_opt_    PMDL        Mdl_,
_In_        ULONG       Size_
)
{
    UNREFERENCED_PARAMETER(Size_);

    PAGED_CODE();

    m_pAdapterCommon->UnregisterStream(this);
    if (Mdl_ != NULL)
    {
        if (m_pDmaBuffer != NULL)
        {
            m_pPortStream->UnmapAllocatedPages(m_pDmaBuffer, Mdl_);
            m_pDmaBuffer = NULL;
        }

        m_pPortStream->FreePagesFromMdl(Mdl_);
    }

    m_ulDmaBufferSize = 0;
    m_ulNotificationsPerBuffer = 0;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::AllocateAudioBuffer
(
_In_    ULONG                   RequestedSize_,
_Out_   PMDL                   *AudioBufferMdl_,
_Out_   ULONG                  *ActualSize_,
_Out_   ULONG                  *OffsetFromFirstPage_,
_Out_   MEMORY_CACHING_TYPE    *CacheType_
)
{
    NTSTATUS ntStatus;

    PAGED_CODE();

    if ((0 == RequestedSize_) || (RequestedSize_ < m_pWfExt->Format.nBlockAlign))
    {
        return STATUS_UNSUCCESSFUL;
    }

    RequestedSize_ -= RequestedSize_ % (m_pWfExt->Format.nBlockAlign);

    PHYSICAL_ADDRESS highAddress;
    highAddress.HighPart = 0;
    highAddress.LowPart = MAXULONG;

    PMDL pBufferMdl = m_pPortStream->AllocatePagesForMdl(highAddress, RequestedSize_);

    if (NULL == pBufferMdl)
    {
        return STATUS_UNSUCCESSFUL;
    }

    // From MSDN: 
    // "Since the Windows audio stack does not support a mechanism to express memory access 
    //  alignment requirements for buffers, audio drivers must select a caching type for mapped
    //  memory buffers that does not impose platform-specific alignment requirements. In other 
    //  words, the caching type used by the audio driver for mapped memory buffers, must not make 
    //  assumptions about the memory alignment requirements for any specific platform.
    //
    //  This method maps the physical memory pages in the MDL into kernel-mode virtual memory. 
    //  Typically, the miniport driver calls this method if it requires software access to the 
    //  scatter-gather list for an audio buffer. In this case, the storage for the scatter-gather 
    //  list must have been allocated by the IPortWaveRTStream::AllocatePagesForMdl or 
    //  IPortWaveRTStream::AllocateContiguousPagesForMdl method. 
    //
    //  A WaveRT miniport driver should not require software access to the audio buffer itself."
    //   
    m_pDmaBuffer = (BYTE*)m_pPortStream->MapAllocatedPages(pBufferMdl, MmCached);

    m_ulDmaBufferSize = RequestedSize_;
    m_ulNotificationsPerBuffer = 0;

    ntStatus = m_pAdapterCommon->RegisterStream(this);

    if (NT_SUCCESS(ntStatus))
    { 
        *AudioBufferMdl_ = pBufferMdl;
        *ActualSize_ = RequestedSize_;
        *OffsetFromFirstPage_ = 0;
        *CacheType_ = MmCached;
    }
    else
    {
        m_pPortStream->FreePagesFromMdl(pBufferMdl);
        m_ulDmaBufferSize = 0;
        m_pDmaBuffer = NULL;
    }

    return ntStatus;
}

//=============================================================================
#pragma code_seg()
NTSTATUS CMiniportWaveRTStream::GetPosition
(
    _Out_   KSAUDIO_POSITION    *Position_
)
{
    NTSTATUS ntStatus;

    Position_->PlayOffset = m_ullPlayPosition;
    Position_->WriteOffset = m_ullWritePosition;

    ntStatus = STATUS_SUCCESS;
    
    return ntStatus;
}


//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::SetState
(
    _In_    KSSTATE State_
)
{
    PAGED_CODE();

    NTSTATUS        ntStatus        = STATUS_SUCCESS;

    // Spew an event for a pin state change request from portcls

    switch (State_)
    {
        case KSSTATE_STOP:
            // Reset DMA
            m_ullPlayPosition = 0;
            m_ullWritePosition = 0;

            // Reset DMA
            ntStatus = m_pAdapterCommon->StopDma();

            break;

        case KSSTATE_ACQUIRE:
            break;

        case KSSTATE_PAUSE:


            if (m_KsState > KSSTATE_PAUSE)
            {
                // Pause DMA
                if (m_ulNotificationIntervalMs > 0)
                {
                ntStatus = m_pAdapterCommon->PauseDma();
                    KeCancelTimer(m_pNotificationTimer);
                    ExSetTimerResolution(0, FALSE);
                    KeFlushQueuedDpcs(); 
                }
            }

            break;

        case KSSTATE_RUN:
            // Start DMA

            if (m_ulNotificationIntervalMs > 0)
            {
                LARGE_INTEGER   delay;

                delay.QuadPart = (-2) * (((LONGLONG)m_ulNotificationIntervalMs) * HNSTIME_PER_MILLISECOND);

                // ISSUE-2014/10/20 Set resolution based on packet size
                ExSetTimerResolution(HNSTIME_PER_MILLISECOND, TRUE);

                KeSetTimerEx
                (
                    m_pNotificationTimer,
                    delay,
                    m_ulNotificationIntervalMs,
                    m_pNotificationDpc
                );
                
            }
            ntStatus = m_pAdapterCommon->StartDma();

            break;
    }

    if (NT_SUCCESS(ntStatus))
    {
    m_KsState = State_;
    }

    return ntStatus;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS CMiniportWaveRTStream::SetFormat
(
    _In_    KSDATAFORMAT    *DataFormat_
)
{
    UNREFERENCED_PARAMETER(DataFormat_);

    PAGED_CODE();



    return STATUS_NOT_SUPPORTED;
}

//=============================================================================
#pragma code_seg()
VOID 
CMiniportWaveRTStream::UpdateVirtualPositionRegisters
(
    ULONG cSamplesTransferred
)
/*++

Routine Description:

    Updates the playback position.  The virtual position points to the last sample sent to the codec.

Arguments:

    None

Return Value:

    None

--*/
{
    DPF_ENTER(("[CMiniportWaveRTStream::UpdateVirtualPositionRegisters]"));


    ULONGLONG oldPosition, newPosition;
    BOOLEAN updateClients = FALSE;

    //
    // Compute if we need to notify clients.
    //
    // m_ulNotificationsPerBuffer can be set to either 1 or 2.  If set to 1, we notify when the DMA pointer wraps around 
    // the end of the buffer.  If set to two, we notify when the new position wraps around the end of the buffer or passes
    // the 50% marker.
    //

    oldPosition = m_ullPlayPosition;
    newPosition = (cSamplesTransferred * m_pWfExt->Format.nBlockAlign) + m_ullPlayPosition;
    ASSERT(newPosition <= m_ulDmaBufferSize);
    if (newPosition == m_ulDmaBufferSize)
    {
        newPosition = 0;
    }

    if ((m_ulNotificationsPerBuffer == 2) && 
        (newPosition >= m_ulDmaBufferSize / 2) && 
        (oldPosition < (m_ulDmaBufferSize / 2)))
    {
        updateClients = TRUE;
    }

    if (newPosition < oldPosition)
    {
        // We have wrapped - always notify.
        updateClients = TRUE;
    }

    m_ullPlayPosition = newPosition;
    m_ullWritePosition = newPosition;

    if (updateClients == TRUE && m_KsState == KSSTATE_RUN && m_ulNotificationsPerBuffer != 0)
    {
        KeInsertQueueDpc(m_pDpc, NULL, NULL);
    }
}

//=============================================================================
#pragma code_seg()
void
TimerNotifyRT
(
    _In_      PKDPC         Dpc,
    _In_opt_  PVOID         DeferredContext,
    _In_opt_  PVOID         SA1,
    _In_opt_  PVOID         SA2
)
{
    LARGE_INTEGER qpc;
    LARGE_INTEGER qpcFrequency;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SA1);
    UNREFERENCED_PARAMETER(SA2);

    _IRQL_limited_to_(DISPATCH_LEVEL);

    qpc = KeQueryPerformanceCounter(&qpcFrequency);

    CMiniportWaveRTStream* _this = (CMiniportWaveRTStream*)DeferredContext;
    
    if (NULL == _this)
    {
        return;
    }

    if (_this->m_KsState != KSSTATE_RUN)
    {
        return;
    }
    
    if (!IsListEmpty(&_this->m_NotificationList))
    {
        PLIST_ENTRY leCurrent = _this->m_NotificationList.Flink;
        while (leCurrent != &_this->m_NotificationList)
        {
            NotificationListEntry* nleCurrent = CONTAINING_RECORD( leCurrent, NotificationListEntry, ListEntry);

            KeSetEvent(nleCurrent->NotificationEvent, 0, 0);

            leCurrent = leCurrent->Flink;
        }
    }
}
//=============================================================================

