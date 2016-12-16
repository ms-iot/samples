//
// Copyright (C) Microsoft Corporation. All rights reserved.
//

#pragma once


class RtAudioPin : public KsPin
{
protected:

    PMDL m_BufferMdl;
    PMDL m_PositionPointerMdl;

public:
    RtAudioPin(_In_ VirtualPin* virtualPin);
    virtual ~RtAudioPin();
    
    NTSTATUS InitRtBuffer(ULONG BufferSize);

    NTSTATUS ClosePin();

    void* operator new(size_t t, ULONG tag = 'nPtR')
    {
        return ExAllocatePoolWithTag(NonPagedPoolNx, t, tag);
    }
    void operator delete(_In_ void* Goner) { ExFreePool(Goner); }

};


