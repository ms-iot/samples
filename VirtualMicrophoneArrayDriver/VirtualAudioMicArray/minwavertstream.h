/*++

Copyright (c) 1997-2010  Microsoft Corporation All Rights Reserved

Module Name:

    minwavert.h

Abstract:

    Definition of wavert miniport class.

--*/

#ifndef _SYSVAD_MINWAVERTSTREAM_H_
#define _SYSVAD_MINWAVERTSTREAM_H_





//
// Structure to store notifications events in a protected list
//
typedef struct _NotificationListEntry
{
    LIST_ENTRY  ListEntry;
    PKEVENT     NotificationEvent;
} NotificationListEntry;

KDEFERRED_ROUTINE TimerNotifyRT;

//=============================================================================
// Referenced Forward
//=============================================================================
class CMiniportWaveRT;
typedef CMiniportWaveRT *PCMiniportWaveRT;

//=============================================================================
// Classes
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
// CMiniportWaveRTStream 
// 
class CMiniportWaveRTStream : 
    public IMiniportWaveRTStreamNotification,
    public CUnknown
{
protected:
    PPORTWAVERTSTREAM           m_pPortStream;
    LIST_ENTRY                  m_NotificationList;
    PKTIMER                     m_pNotificationTimer;
    PRKDPC                      m_pNotificationDpc;
    ULONG                       m_ulNotificationIntervalMs;

    
public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMiniportWaveRTStream);
    ~CMiniportWaveRTStream();

    IMP_IMiniportWaveRTStream;
    IMP_IMiniportWaveRTStreamNotification;


    NTSTATUS                    Init
    ( 
        _In_  PCMiniportWaveRT    Miniport,
        _In_  PPORTWAVERTSTREAM   Stream,
        _In_  ULONG               Channel,
        _In_  BOOLEAN             Capture,
        _In_  PKSDATAFORMAT       DataFormat,
        _In_  GUID                SignalProcessingMode
    );

    // Friends
    friend class                CMiniportWaveRT;
    friend KDEFERRED_ROUTINE    TimerNotifyRT;

    VOID UpdateVirtualPositionRegisters
    (
        ULONG cSamplesTransferred
    );

    ULONG GetDmaBufferSize() { return m_ulDmaBufferSize; }
    BYTE* GetDmaBuffer() { return m_pDmaBuffer; }
    ULONG GetPaddingToEndOfBuffer() { return (ULONG) ((m_ulDmaBufferSize - m_ullWritePosition) / m_pWfExt->Format.nBlockAlign); }
    PWAVEFORMATEXTENSIBLE       GetDataFormat() { return m_pWfExt; }
protected:
    CMiniportWaveRT*            m_pMiniport;
    ULONG                       m_ulPin;
    BOOLEAN                     m_bCapture;
    BOOLEAN                     m_bUnregisterStream;
    ULONG                       m_ulDmaBufferSize;
    BYTE*                       m_pDmaBuffer;
    ULONG                       m_ulNotificationsPerBuffer;
    KSSTATE                     m_KsState;
    PKTIMER                     m_pTimer;
    PRKDPC                      m_pDpc;
    ULONGLONG                   m_ullPlayPosition;
    ULONGLONG                   m_ullWritePosition;
    PLONG                       m_plVolumeLevel;
    PWAVEFORMATEXTENSIBLE       m_pWfExt;

    PADAPTERCOMMON              m_pAdapterCommon;
    



};
typedef CMiniportWaveRTStream *PCMiniportWaveRTStream;
#endif // _SYSVAD_MINWAVERTSTREAM_H_

