//
// Copyright (C) Microsoft Corporation. All rights reserved.
//

#pragma once

const ULONG cPingPongIrps = 2;


class UsbAudioPin : public KsPin
{
protected:
    
    ULONG               m_WritePosition;
    PIRP                m_PingPongIrps[cPingPongIrps];
    KSSTREAM_HEADER     m_KsStreamHeader[cPingPongIrps];
    PBYTE               m_pBuffer[cPingPongIrps];
    IO_STATUS_BLOCK     m_IoStatusBlock[cPingPongIrps];
    LONG                m_cRequests;
    PKEVENT             m_rgIoctlCompleteEvents[cPingPongIrps];
    KEVENT              m_IoctlCompleteEvent[cPingPongIrps];
    PKTHREAD            m_ThreadHandle;
    BOOLEAN             m_bThreadShutdown;
    ULONG               m_cbPingPongBufferSize;

    NTSTATUS SendReadStreamIoctl();
    static KSTART_ROUTINE WorkerThread;
    void DoThreadWork();
    void UpdateWritePosition(ULONG NumFrames);

public:
    UsbAudioPin(_In_ VirtualPin* virtualPin);
    virtual ~UsbAudioPin();

    NTSTATUS InitRtBuffer(ULONG BufferSize);

    NTSTATUS ClosePin();

    void* operator new(size_t t, ULONG tag = 'nPbU')
    {
        return ExAllocatePoolWithTag(NonPagedPoolNx, t, tag);
    }
    void operator delete(_In_ void* Goner) { ExFreePool(Goner); }

};
