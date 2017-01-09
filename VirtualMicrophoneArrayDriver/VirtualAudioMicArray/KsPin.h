//
// Copyright (C) Microsoft Corporation. All rights reserved.
//

#pragma once

class VirtualPin;

class KsPin
{
protected:
    BOOLEAN m_bPinRunning;
    BOOLEAN m_bPinOpened;
    KSDATAFORMAT_WAVEFORMATEXTENSIBLE m_Format;
    KSRTAUDIO_BUFFER m_RtBuffer;
    PULONG m_pPositionRegister;
    ULONG m_FifoSizeInFrames;
    UNICODE_STRING m_Name;
    BOOLEAN m_bMuted;
    PDEVICE_OBJECT m_pDeviceObject;
    VirtualPin* m_pVirtualPin;
    PFILE_OBJECT m_pPinFileObject;
    PFILE_OBJECT m_pFilterFileObject;

private:

    HANDLE m_FileHandle;
    HANDLE m_PinHandle;

    FAST_MUTEX m_Mutex;
    BOOLEAN m_bIsUsbDevice;

    NTSTATUS OpenWaveFilter(_In_ PUNICODE_STRING InterfaceName);

    void InitFormat(ULONG rate, ULONG numChannels, ULONG ValidBitsPerSample);

    NTSTATUS OpenPin(_In_ PUNICODE_STRING InterfaceName);

    NTSTATUS FindDataSinkPin(_Out_ ULONG* PinId);

    NTSTATUS Init(_In_ PUNICODE_STRING Name);

public:
    KsPin(_In_ VirtualPin* virtualPin, BOOLEAN IsUsbDevice);
    ~KsPin();

    PUNICODE_STRING GetName() { return &m_Name; }

    NTSTATUS OpenPin(_In_ PUNICODE_STRING InterfaceName, ULONG rate, ULONG numChannels, ULONG ValidBitsPerSample);


    virtual NTSTATUS SetState(BOOLEAN running);

    NTSTATUS SendIoctl(_In_       ULONG IoControlCode,
        _In_opt_   PVOID InputBuffer,
        _In_       ULONG InputBufferLength,
        _Out_opt_  PVOID OutputBuffer,
        _In_       ULONG OutputBufferLength,
        _Out_      PIO_STATUS_BLOCK StatusBlock,
        BOOL  bPinRequest);

    NTSTATUS GetPinPropertySimple
    (
        ULONG   nPinID,
        REFGUID guidPropertySet,
        ULONG   nProperty,
        _Out_writes_bytes_(cbValue) PVOID   pvValue,
        ULONG   cbValue);

    BOOLEAN  GetState() { return m_bPinRunning; }
    PWAVEFORMATEX GetFormat() { return (PWAVEFORMATEX)&m_Format.WaveFormatExt; }
    PWAVEFORMATEXTENSIBLE GetFormatEx() { return &m_Format.WaveFormatExt; }
    

    PDEVICE_OBJECT GetDeviceObject() { return m_pDeviceObject; }

    virtual NTSTATUS ClosePin();
   
    virtual NTSTATUS InitRtBuffer(ULONG BufferSize);

    PBYTE GetWritePosition();
    ULONG GetNumChannels();
    ULONG GetBlockAlign();
    ULONG GetBufferSize();
    ULONG GetPadding(ULONG LastWritePosition);
    ULONG GetCurrentPadding();
    
    void SetMuted(BOOLEAN bMuted);

    PBYTE GetDmaBuffer() { return (PBYTE) m_RtBuffer.BufferAddress; }
    
};
